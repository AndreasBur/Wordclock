// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Main header file of standard types library.
 *
 * Library with standard types.
 */

#ifndef _STANDARD_TYPES_H_
#define _STANDARD_TYPES_H_

#include "limits"
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <type_traits>

/* standard type for configuration */
#define STD_ON                  1u
#define STD_OFF                 0u

#define STD_NULL_CHARACTER      '\0'

/* assembler NOPs */
#define STD_NOP_1               "nop      \n\t"
#define STD_NOP_2               "rjmp .+0 \n\t"
#define STD_NOP_4               STD_NOP_2 STD_NOP_2
#define STD_NOP_8               STD_NOP_4 STD_NOP_4
#define STD_NOP_16              STD_NOP_8 STD_NOP_8

/* Dummy statement to prevent compiler warnings unused parameter */
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

/* standard return type for functions */
enum StdReturnType {
    kEOk = 0u,
    kENotOk = 1u
};

template <typename T> uint8_t DigitsOfNumber(T number, unsigned = 10);

/*
 * An enumerator as the number it is stored as - to index an array with, count with or print.
 * An enum class does not turn into a number by itself, which is the point of it; this is the
 * one place it is asked to. std::to_underlying, which is C++23.
 */
template <typename EnumType>
constexpr std::underlying_type_t<EnumType> ToUnderlying(EnumType value)
{
    return static_cast<std::underlying_type_t<EnumType>>(value);
}

template <typename T>
uint8_t DigitsOfNumber(T number, unsigned base)
{
    T number_of_digits = 0u;

    do { number /= base; ++number_of_digits; }
    while (number != 0u);

    return number_of_digits;
}

#endif
