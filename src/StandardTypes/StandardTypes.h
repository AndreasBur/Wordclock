// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Main header file of standard types library.
 *
 * Library with standard types.
 */

#ifndef WORDCLOCK_SRC_STANDARDTYPES_STANDARDTYPES_H_
#define WORDCLOCK_SRC_STANDARDTYPES_STANDARDTYPES_H_

/*
 * Re-exported, every one of them, for the same reason as the backends' Arduino.h: what includes
 * this header wants the fixed-width integers, `size_t`, `CHAR_BIT` or `std::numeric_limits` as
 * much as anything defined here. Without the pragma an include-cleaner credits those to the inner
 * header and offers to drop this one from a file that uses nothing else of it.
 */
// IWYU pragma: begin_exports
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include <limits>
#include <type_traits>
// IWYU pragma: end_exports

#define STD_ON 1u  /*!< A configuration switch that is on. */
#define STD_OFF 0u /*!< A configuration switch that is off. */

#define STD_NULL_CHARACTER '\0' /*!< The char that ends a string. */

/*! \brief Inline assembler for the AVR that waits one cycle: `nop`. */
#define STD_NOP_1 "nop      \n\t"
/*! \brief Inline assembler for the AVR that waits two cycles: a jump to the next instruction. */
#define STD_NOP_2 "rjmp .+0 \n\t"
/*! \brief Inline assembler for the AVR that waits 4 cycles. */
#define STD_NOP_4 STD_NOP_2 STD_NOP_2
/*! \brief Inline assembler for the AVR that waits 8 cycles. */
#define STD_NOP_8 STD_NOP_4 STD_NOP_4
/*! \brief Inline assembler for the AVR that waits 16 cycles. */
#define STD_NOP_16 STD_NOP_8 STD_NOP_8

/*!
 * \brief Uses `x` without effect, so that the compiler does not warn about an unused parameter.
 *
 * \param[in] x The parameter that is not used.
 */
#define UNUSED(x) (void)(x)

/* binary to decimal */
/*
#define B(x) ( \
(0##x >>  0 & 0001) | \
(0##x >>  2 & 0002) | \
(0##x >>  4 & 0004) | \
(0##x >>  6 & 0010) | \
(0##x >>  8 & 0020) | \
(0##x >> 10 & 0040) | \
(0##x >> 12 & 0100) | \
(0##x >> 14 & 0200) )
*/

/*! \brief What a function returns that can fail. */
enum StdReturnType {
  kEOk = 0u,   /*!< It did what it was asked to. */
  kENotOk = 1u /*!< It did not; the function says what it then left as it was. */
};

/*!
 * \brief Counts the digits of `number` in `base`.
 *
 * \param[in] number The number.
 * \param[in] base The base the digits are counted in.
 * \return The number of digits; 0 has one.
 * \pre `number` is not negative. A minus sign is not a digit, so it is not counted - where itoa()
 *      wrote one, the string is a character longer than this says.
 */
template <typename T>
uint8_t DigitsOfNumber(T number, unsigned base = 10);

/*!
 * \brief Returns an enumerator as the number it is stored as - to index an array with, count with
 *        or print.
 *
 * An enum class does not turn into a number by itself, which is the point of it; this is the one
 * place it is asked to. It does what `std::to_underlying()` does, which is C++23.
 *
 * \param[in] value The enumerator.
 * \return The enumerator's value, in the underlying type of its enum.
 */
template <typename EnumType>
constexpr std::underlying_type_t<EnumType> ToUnderlying(EnumType value) {
  return static_cast<std::underlying_type_t<EnumType>>(value);
}

template <typename T>
uint8_t DigitsOfNumber(T number, unsigned base) {
  uint8_t number_of_digits{0u};

  // Divided in T, not in the unsigned of `base`: a signed T would otherwise be converted to
  // unsigned first.
  do {
    number = static_cast<T>(number / static_cast<T>(base));
    ++number_of_digits;
  } while (number != T{0});

  return number_of_digits;
}

#endif  // WORDCLOCK_SRC_STANDARDTYPES_STANDARDTYPES_H_
