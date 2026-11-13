// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino function itoa(), which GCC and glibc do not provide.
 *
 * A minimal reimplementation of the Arduino signature (value, buffer, base), including base-10
 * sign handling.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_ITOA_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_ITOA_H_

/*!
 * \brief Writes `value` into `buffer` in base `base`, as a terminated string, as Arduino's itoa()
 *        does.
 *
 * Digits above 9 are lowercase letters. In base 10 a negative `value` gets a minus sign; in any
 * other base its bits are written as those of an unsigned int. An int has 32 bits here and 16 on
 * an AVR, so a negative value in another base has more digits here than on an AVR.
 *
 * \param[in] value The number.
 * \param[out] buffer Where the string goes.
 * \param[in] base The base.
 * \return `buffer`.
 * \pre `base` is from 2 to 36, and `buffer` holds every digit, a minus sign and the terminator;
 *      neither is checked.
 */
inline char* itoa(int value, char* buffer, int base)  // NOLINT(readability-identifier-naming)
{
  char* p = buffer;
  bool negative = (base == 10 && value < 0);
  unsigned int u = negative ? static_cast<unsigned int>(-value) : static_cast<unsigned int>(value);

  char* start = p;
  do {
    unsigned int const digit = u % static_cast<unsigned int>(base);
    *p++ = (digit < 10) ? char('0' + digit) : char('a' + digit - 10);
    u /= static_cast<unsigned int>(base);
  } while (u);
  if (negative) *p++ = '-';
  *p = '\0';

  // reverse in place
  for (char* q = p - 1; start < q; ++start, --q) {
    char t = *start;
    *start = *q;
    *q = t;
  }
  return buffer;
}

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_ARDUINO_ITOA_H_
