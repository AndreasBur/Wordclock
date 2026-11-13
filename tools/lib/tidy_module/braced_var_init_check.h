// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLS_TIDY_MODULE_BRACED_VAR_INIT_CHECK_H_
#define TOOLS_TIDY_MODULE_BRACED_VAR_INIT_CHECK_H_

#include "clang-tidy/ClangTidyCheck.h"

namespace house {

/*!
 * \brief house-braced-var-init: a variable or a data member's default is
 *        initialized with braces, `uint8_t count{0};`, not with `=` or
 *        parentheses.
 *
 * The style's braced initialization outside a constructor's member initializer
 * list, which house-braced-member-init checks: a local or global variable, a
 * `static` one, a static data member, the variable of a `for` or of a
 * condition, and the default of a data member. Not a parameter's default, the
 * variable of a range-based `for`, a structured binding or a lambda's capture,
 * nor a C array from a string literal, `char const kName[] = "font";`, which
 * T-20 writes with `=`, as modernize-avoid-c-arrays allows no other form.
 *
 * The fix-it writes braces where that keeps the meaning, and only there:
 *
 * - `T x = value;` and `T x(value);` become `T x{value};`, `T x = {1, 2};`
 *   becomes `T x{1, 2};`, and `T x = T(1, 2);` becomes `T x{1, 2};`.
 * - `auto` takes braces as well: with one expression, `auto x{value}` deduces
 *   the type of `value`, as compilers apply N3922 to C++11. `auto x = {1, 2};`
 *   is a `std::initializer_list` and stays.
 * - Not reported at all where braces would choose a `std::initializer_list`
 *   constructor that the initializer does not.
 * - Reported without a fix-it where an argument converts narrowing, where the
 *   type is a template's parameter, and where `T x = value;` initializes a
 *   class that has an `explicit` constructor, which braces consider and `=`
 *   does not.
 */
class BracedVarInitCheck : public clang::tidy::ClangTidyCheck {
 public:
  using ClangTidyCheck::ClangTidyCheck;

  bool isLanguageVersionSupported(
      clang::LangOptions const& lang_opts) const override {
    return lang_opts.CPlusPlus11;
  }
  void registerMatchers(clang::ast_matchers::MatchFinder* finder) override;
  void check(
      clang::ast_matchers::MatchFinder::MatchResult const& result) override;
};

}  // namespace house

#endif  // TOOLS_TIDY_MODULE_BRACED_VAR_INIT_CHECK_H_
