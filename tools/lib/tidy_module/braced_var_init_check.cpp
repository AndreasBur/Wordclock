// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "braced_var_init_check.h"

#include <optional>
#include <string>

#include "braced_init.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TokenKinds.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Token.h"
#include "llvm/ADT/SmallVector.h"

namespace house {
namespace {

using clang::ASTContext;
using clang::CharSourceRange;
using clang::Expr;
using clang::FixItHint;
using clang::QualType;
using clang::SourceLocation;

// What the check does with an initializer.
enum class Verdict {
  kFix,        // reported with a fix-it
  kNarrowing,  // reported without one: an argument narrows
  kDependent,  // reported without one: depends on the instantiation
  kExplicit,   // reported without one: braces would consider an explicit
               // constructor
  kNone,       // not reported
};

Verdict FromBraces(Braces braces) {
  switch (braces) {
    case Braces::kSame:
      return Verdict::kFix;
    case Braces::kNarrowing:
      return Verdict::kNarrowing;
    case Braces::kDependent:
      return Verdict::kDependent;
    case Braces::kInitializer:
      return Verdict::kNone;
  }
  return Verdict::kNone;
}

// What a report says and changes.
struct Finding {
  Verdict verdict{Verdict::kNone};
  SourceLocation where;
  char const* written{""};  // how the initializer is written, for the message
  llvm::SmallVector<FixItHint, 2> fixes;
};

// The source and the language, which the lexer needs.
struct Source {
  clang::SourceManager const& manager;
  clang::LangOptions const& lang;

  std::optional<clang::Token> After(SourceLocation token) const {
    return clang::Lexer::findNextToken(token, manager, lang);
  }
  SourceLocation EndOf(SourceLocation token) const {
    return clang::Lexer::getLocForEndOfToken(token, 0, manager, lang);
  }
  std::string Text(SourceLocation begin, SourceLocation end) const {
    return clang::Lexer::getSourceText(
               CharSourceRange::getCharRange(begin, end), manager, lang)
        .str();
  }
};

// How the type of a declaration is written: `auto` takes braces whatever it
// deduces; `decltype(auto)` and a class template's arguments deduced do not.
enum class Deduced { kNone, kAuto, kOther };

Deduced DeducedOf(clang::DeclaratorDecl const& decl) {
  clang::TypeSourceInfo const* written = decl.getTypeSourceInfo();
  if (written == nullptr) {
    return Deduced::kNone;
  }
  clang::DeducedType const* deduced =
      written->getType()->getContainedDeducedType();
  if (deduced == nullptr) {
    return Deduced::kNone;
  }
  auto const* automatic = llvm::dyn_cast<clang::AutoType>(deduced);
  return automatic != nullptr && !automatic->isDecltypeAuto() ? Deduced::kAuto
                                                              : Deduced::kOther;
}

// The initializer of a copy-initialization of a class, without what the AST
// adds around it: C++11's elidable copy from a temporary and the conversion by
// a constructor. The same in C++11 as in C++17.
Expr const* Unwrap(Expr const* expr) {
  while (true) {
    if (auto const* cleanups = llvm::dyn_cast<clang::ExprWithCleanups>(expr)) {
      expr = cleanups->getSubExpr();
    } else if (auto const* bind =
                   llvm::dyn_cast<clang::CXXBindTemporaryExpr>(expr)) {
      expr = bind->getSubExpr();
    } else if (auto const* materialize =
                   llvm::dyn_cast<clang::MaterializeTemporaryExpr>(expr)) {
      expr = materialize->getSubExpr();
    } else if (auto const* construct =
                   llvm::dyn_cast<clang::CXXConstructExpr>(expr);
               construct != nullptr && construct->isElidable() &&
               construct->getNumArgs() > 0) {
      expr = construct->getArg(0);
    } else if (auto const* cast = llvm::dyn_cast<clang::ImplicitCastExpr>(expr);
               cast != nullptr &&
               (cast->getCastKind() == clang::CK_ConstructorConversion ||
                cast->getCastKind() == clang::CK_NoOp)) {
      expr = cast->getSubExpr();
    } else {
      return expr;
    }
  }
}

// Whether the class has an explicit constructor other than a copy or a move:
// one that `T x{value}` considers and `T x = value` does not.
bool HasExplicitConstructor(clang::CXXRecordDecl const& record) {
  for (clang::Decl const* member : record.decls()) {
    auto const* constructor = llvm::dyn_cast<clang::CXXConstructorDecl>(member);
    if (auto const* function_template =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(member)) {
      constructor = llvm::dyn_cast<clang::CXXConstructorDecl>(
          function_template->getTemplatedDecl());
    }
    if (constructor != nullptr && constructor->isExplicit() &&
        !constructor->isCopyOrMoveConstructor()) {
      return true;
    }
  }
  return false;
}

// A temporary of the declared type, `T(1, 2)` or `T{1, 2}`, whose arguments
// the declaration can take itself: `T x{1, 2}`.
struct Temporary {
  SourceLocation open;
  SourceLocation close;
  bool braced{false};
  Expr const* construct{nullptr};
};

std::optional<Temporary> AsTemporary(Expr const& expr, QualType type,
                                     ASTContext& context) {
  if (!context.hasSameUnqualifiedType(expr.getType(), type)) {
    return std::nullopt;
  }
  Temporary temporary;
  if (auto const* object = llvm::dyn_cast<clang::CXXTemporaryObjectExpr>(&expr)) {
    temporary.open = object->getParenOrBraceRange().getBegin();
    temporary.close = object->getParenOrBraceRange().getEnd();
    temporary.braced = object->isListInitialization();
    temporary.construct = object;
  } else if (auto const* cast =
                 llvm::dyn_cast<clang::CXXFunctionalCastExpr>(&expr)) {
    if (auto const* list =
            llvm::dyn_cast<clang::InitListExpr>(cast->getSubExpr())) {
      temporary.open = list->getLBraceLoc();
      temporary.close = list->getRBraceLoc();
      temporary.braced = true;
    } else {
      temporary.open = cast->getLParenLoc();
      temporary.close = cast->getRParenLoc();
    }
    temporary.construct = cast->getSubExpr();
  } else {
    return std::nullopt;
  }
  if (temporary.open.isInvalid() || temporary.close.isInvalid() ||
      temporary.open.isMacroID() || temporary.close.isMacroID()) {
    return std::nullopt;
  }
  return temporary;
}

// `char const kName[] = "font";`, a C array from a string literal: T-20 writes
// it with `=`, the one form modernize-avoid-c-arrays' AllowStringArrays lets
// through; braces would turn it into a finding of that check.
bool IsStringArray(QualType type, Expr const& init) {
  return type->isArrayType() &&
         llvm::isa<clang::StringLiteral>(init.IgnoreImplicit());
}

// `T x = value;`, `T x = {1, 2};` or `T x = T(1, 2);`, of a variable or of a
// data member's default; `name` is where its name is.
std::optional<Finding> CopyInit(Source const& source, ASTContext& context,
                                SourceLocation name, QualType type,
                                Expr const& init, Deduced deduced) {
  if (deduced == Deduced::kOther) {
    return std::nullopt;
  }
  // As written: in C++11, the elidable copy around it begins at the name.
  CharSourceRange const init_range{clang::Lexer::makeFileCharRange(
      CharSourceRange::getTokenRange(Unwrap(&init)->getSourceRange()),
      source.manager, source.lang)};
  if (init_range.isInvalid()) {
    return std::nullopt;
  }
  // The `=`, and the end of the token before it: what lies between that and
  // the initializer gives way to the brace.
  SourceLocation before_equal{source.EndOf(name)};
  SourceLocation equal;
  for (SourceLocation token = name; equal.isInvalid();) {
    std::optional<clang::Token> const next{source.After(token)};
    if (!next || next->getLocation().isMacroID() ||
        !source.manager.isBeforeInTranslationUnit(next->getLocation(),
                                                  init_range.getBegin())) {
      return std::nullopt;
    }
    if (next->is(clang::tok::equal)) {
      equal = next->getLocation();
    } else {
      before_equal = next->getEndLoc();
      token = next->getLocation();
    }
  }
  CharSourceRange const equal_to_init{
      CharSourceRange::getCharRange(before_equal, init_range.getBegin())};

  Finding finding;
  finding.where = equal;
  finding.written = "'='";
  if (source.Text(init_range.getBegin(),
                  init_range.getBegin().getLocWithOffset(1)) == "{") {
    // `= {1, 2}` initializes as `{1, 2}` does, only an explicit constructor
    // chosen makes it ill-formed; with `auto`, it is a std::initializer_list.
    if (deduced == Deduced::kAuto) {
      return std::nullopt;
    }
    finding.verdict = Verdict::kFix;
    finding.fixes.push_back(FixItHint::CreateRemoval(equal_to_init));
    return finding;
  }

  auto const* record = type->getAsCXXRecordDecl();
  if (deduced == Deduced::kAuto) {
    finding.verdict = Verdict::kFix;
  } else if (type->isDependentType()) {
    finding.verdict = Verdict::kDependent;
  } else if (record != nullptr) {
    Expr const* const inner = Unwrap(&init);
    if (std::optional<Temporary> const temporary{
            AsTemporary(*inner, type, context)}) {
      finding.verdict =
          temporary->braced
              ? Verdict::kFix
              : FromBraces(Classify(*temporary->construct, type, context));
      if (finding.verdict == Verdict::kFix) {
        finding.written = "'=' and a temporary";
        std::string const arguments{source.Text(
            source.EndOf(temporary->open), temporary->close)};
        finding.fixes.push_back(FixItHint::CreateReplacement(
            CharSourceRange::getCharRange(before_equal,
                                          source.EndOf(temporary->close)),
            "{" + arguments + "}"));
      }
      return finding;
    }
    finding.verdict = FromBraces(Classify(*inner, type, context));
    if (finding.verdict == Verdict::kFix && record->hasDefinition() &&
        HasExplicitConstructor(*record->getDefinition())) {
      finding.verdict = Verdict::kExplicit;
    }
  } else {
    finding.verdict = FromBraces(Classify(init, type, context));
  }
  if (finding.verdict == Verdict::kFix) {
    finding.fixes.push_back(
        FixItHint::CreateReplacement(equal_to_init, "{"));
    finding.fixes.push_back(
        FixItHint::CreateInsertion(init_range.getEnd(), "}"));
  }
  return finding;
}

// `T x(1, 2);`: the parentheses follow the name.
std::optional<Finding> CallInit(Source const& source, ASTContext& context,
                                SourceLocation name, QualType type,
                                Expr const& init, Deduced deduced) {
  if (deduced == Deduced::kOther) {
    return std::nullopt;
  }
  std::optional<clang::Token> const open{source.After(name)};
  if (!open || !open->is(clang::tok::l_paren) ||
      open->getLocation().isMacroID()) {
    return std::nullopt;
  }
  SourceLocation close;
  int depth = 0;
  for (std::optional<clang::Token> token{open}; close.isInvalid();
       token = source.After(token->getLocation())) {
    if (!token || token->is(clang::tok::eof) ||
        token->getLocation().isMacroID()) {
      return std::nullopt;
    }
    if (token->is(clang::tok::l_paren)) {
      ++depth;
    } else if (token->is(clang::tok::r_paren) && --depth == 0) {
      close = token->getLocation();
    }
  }

  Finding finding;
  finding.where = open->getLocation();
  finding.written = "parentheses";
  finding.verdict = deduced == Deduced::kAuto
                        ? Verdict::kFix
                        : FromBraces(Classify(init, type, context));
  if (finding.verdict == Verdict::kFix) {
    finding.fixes.push_back(FixItHint::CreateReplacement(
        CharSourceRange::getCharRange(open->getLocation(),
                                      open->getEndLoc()),
        "{"));
    finding.fixes.push_back(FixItHint::CreateReplacement(
        CharSourceRange::getCharRange(close, source.EndOf(close)), "}"));
  }
  return finding;
}

}  // namespace

void BracedVarInitCheck::registerMatchers(
    clang::ast_matchers::MatchFinder* finder) {
  using clang::ast_matchers::anyOf;
  using clang::ast_matchers::decompositionDecl;
  using clang::ast_matchers::expr;
  using clang::ast_matchers::fieldDecl;
  using clang::ast_matchers::hasInClassInitializer;
  using clang::ast_matchers::hasInitializer;
  using clang::ast_matchers::isExceptionVariable;
  using clang::ast_matchers::isImplicit;
  using clang::ast_matchers::isInstantiated;
  using clang::ast_matchers::isTemplateInstantiation;
  using clang::ast_matchers::parmVarDecl;
  using clang::ast_matchers::unless;
  using clang::ast_matchers::varDecl;
  // A template as written, not each of its instantiations, as
  // house-braced-member-init does.
  finder->addMatcher(
      varDecl(hasInitializer(expr()),
              unless(anyOf(parmVarDecl(), decompositionDecl(), isImplicit(),
                           isExceptionVariable(), isInstantiated(),
                           isTemplateInstantiation())))
          .bind("var"),
      this);
  finder->addMatcher(
      fieldDecl(hasInClassInitializer(expr()), unless(isInstantiated()))
          .bind("field"),
      this);
}

void BracedVarInitCheck::check(
    clang::ast_matchers::MatchFinder::MatchResult const& result) {
  Source const source{*result.SourceManager, getLangOpts()};
  ASTContext& context = *result.Context;
  clang::DeclaratorDecl const* decl = nullptr;
  std::optional<Finding> finding;
  if (auto const* var = result.Nodes.getNodeAs<clang::VarDecl>("var")) {
    decl = var;
    // A static data member's declaration in the class matches by the
    // initializer of its definition outside, which is reported there.
    if (var->getInit() == nullptr || var->isInitCapture() ||
        var->isCXXForRangeDecl() || var->getLocation().isMacroID() ||
        IsStringArray(var->getType(), *var->getInit())) {
      return;
    }
    switch (var->getInitStyle()) {
      case clang::VarDecl::CInit:
        finding = CopyInit(source, context, var->getLocation(), var->getType(),
                           *var->getInit(), DeducedOf(*var));
        break;
      case clang::VarDecl::CallInit:
      case clang::VarDecl::ParenListInit:
        finding = CallInit(source, context, var->getLocation(), var->getType(),
                           *var->getInit(), DeducedOf(*var));
        break;
      case clang::VarDecl::ListInit:
        break;
    }
  } else if (auto const* field =
                 result.Nodes.getNodeAs<clang::FieldDecl>("field")) {
    decl = field;
    if (field->getInClassInitStyle() != clang::ICIS_CopyInit ||
        field->getLocation().isMacroID() ||
        IsStringArray(field->getType(), *field->getInClassInitializer())) {
      return;
    }
    finding = CopyInit(source, context, field->getLocation(), field->getType(),
                       *field->getInClassInitializer(), Deduced::kNone);
  }
  if (!finding) {
    return;
  }

  std::string const name{decl->getNameAsString()};
  switch (finding->verdict) {
    case Verdict::kFix:
      diag(finding->where, "initialize '%0' with braces, not %1")
          << name << finding->written << finding->fixes;
      break;
    case Verdict::kNarrowing:
      diag(finding->where,
           "initialize '%0' with braces, not %1; an argument converts "
           "narrowing, which braces make an error: convert it explicitly")
          << name << finding->written;
      break;
    case Verdict::kDependent:
      diag(finding->where,
           "initialize '%0' with braces, not %1; no fix-it, as what braces "
           "choose depends on the template's arguments")
          << name << finding->written;
      break;
    case Verdict::kExplicit:
      diag(finding->where,
           "initialize '%0' with braces, not %1; no fix-it, as braces "
           "consider an explicit constructor, which '=' does not")
          << name << finding->written;
      break;
    case Verdict::kNone:
      break;
  }
}

}  // namespace house
