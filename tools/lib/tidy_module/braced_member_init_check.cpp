// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "braced_member_init_check.h"

#include <string>

#include "braced_init.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

namespace house {
namespace {

using clang::CXXCtorInitializer;
using clang::QualType;

// The name a diagnostic gives the initializee: the member's, or the type's of a
// base or of a delegating constructor.
std::string NameOf(CXXCtorInitializer const& init,
                   clang::PrintingPolicy const& policy) {
  if (clang::FieldDecl const* member = init.getAnyMember()) {
    return member->getNameAsString();
  }
  return init.getTypeSourceInfo()->getType().getAsString(policy);
}

}  // namespace

void BracedMemberInitCheck::registerMatchers(
    clang::ast_matchers::MatchFinder* finder) {
  using clang::ast_matchers::cxxConstructorDecl;
  using clang::ast_matchers::cxxCtorInitializer;
  using clang::ast_matchers::forEachConstructorInitializer;
  using clang::ast_matchers::isDefinition;
  using clang::ast_matchers::isImplicit;
  using clang::ast_matchers::isInstantiated;
  using clang::ast_matchers::isWritten;
  using clang::ast_matchers::unless;
  // A template as written, not each of its instantiations: an initializer is
  // reported once, at the place it is written, and a template that no file
  // instantiates is checked as well.
  finder->addMatcher(
      cxxConstructorDecl(isDefinition(), unless(isImplicit()),
                         unless(isInstantiated()),
                         forEachConstructorInitializer(
                             cxxCtorInitializer(isWritten()).bind("init"))),
      this);
}

void BracedMemberInitCheck::check(
    clang::ast_matchers::MatchFinder::MatchResult const& result) {
  auto const* init = result.Nodes.getNodeAs<CXXCtorInitializer>("init");
  clang::SourceManager const& sources = *result.SourceManager;
  clang::SourceLocation const left = init->getLParenLoc();
  clang::SourceLocation const right = init->getRParenLoc();
  // Written in a macro, the parentheses are the macro's, which the fix-it must
  // not change.
  if (left.isInvalid() || right.isInvalid() || left.isMacroID() ||
      right.isMacroID()) {
    return;
  }
  // A braced initializer has its braces where the parentheses would be.
  clang::CharSourceRange const left_range{
      clang::CharSourceRange::getCharRange(left, left.getLocWithOffset(1))};
  if (clang::Lexer::getSourceText(left_range, sources, getLangOpts()) != "(") {
    return;
  }

  QualType const type{init->getAnyMember() != nullptr
                          ? init->getAnyMember()->getType()
                          : init->getTypeSourceInfo()->getType()};
  Braces const braces = Classify(*init->getInit(), type, *result.Context);
  if (braces == Braces::kInitializer) {
    return;
  }
  std::string const name{NameOf(*init, result.Context->getPrintingPolicy())};
  switch (braces) {
    case Braces::kSame:
      diag(left, "initialize '%0' with braces, not parentheses")
          << name << clang::FixItHint::CreateReplacement(left_range, "{")
          << clang::FixItHint::CreateReplacement(
                 clang::CharSourceRange::getCharRange(
                     right, right.getLocWithOffset(1)),
                 "}");
      break;
    case Braces::kNarrowing:
      diag(left,
           "initialize '%0' with braces, not parentheses; an argument converts "
           "narrowing, which "
           "braces make an error: convert it explicitly")
          << name;
      break;
    case Braces::kDependent:
      diag(left,
           "initialize '%0' with braces, not parentheses; no fix-it, as what "
           "braces choose "
           "depends on the template's arguments")
          << name;
      break;
    case Braces::kInitializer:
      break;
  }
}

}  // namespace house
