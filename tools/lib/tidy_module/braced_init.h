// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later
//
// What braces would do in place of an initializer written otherwise: the part
// that house-braced-member-init and house-braced-var-init share.

#ifndef TOOLS_TIDY_MODULE_BRACED_INIT_H_
#define TOOLS_TIDY_MODULE_BRACED_INIT_H_

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"

namespace house {

// What braces in place of the initializer would do.
enum class Braces {
  kSame,         // the same: reported with a fix-it
  kNarrowing,    // an argument narrows, an error in braces: reported without one
  kDependent,    // depends on the instantiation: reported without one
  kInitializer,  // chooses a std::initializer_list constructor: not reported
};

// Whether the class has a constructor, or a constructor template, whose first
// parameter is a std::initializer_list and whose others have a default: one
// that braces prefer.
bool HasInitializerListConstructor(clang::CXXRecordDecl const& record);

// Whether `argument` converts narrowing to `target` ([dcl.init.list]): an error
// in braces. Where a type depends on a template's parameter, it cannot be
// known, and is taken as not: the instantiation that narrows fails to compile,
// which does not go unnoticed.
bool IsNarrowing(clang::Expr const& argument, clang::QualType target,
                 clang::ASTContext& context);

// What braces would do in place of the parentheses or the `=` of `init`, the
// initializer of something of type `type`, as the AST holds it.
Braces Classify(clang::Expr const& init, clang::QualType type,
                clang::ASTContext& context);

}  // namespace house

#endif  // TOOLS_TIDY_MODULE_BRACED_INIT_H_
