// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The Arduino function itoa(), which GCC and glibc do not provide.
 *
 * A minimal reimplementation of the Arduino signature (value, buffer, base), including base-10
 * sign handling.
 */

#ifndef _ARDUINO_ITOA_H_
#define _ARDUINO_ITOA_H_

inline char* itoa(int value, char* buffer, int base) // NOLINT(readability-identifier-naming)
{
    char* p = buffer;
    bool negative = (base == 10 && value < 0);
    unsigned int u = negative ? (unsigned int)(-value) : (unsigned int)value;

    char* start = p;
    do {
        int digit = u % (unsigned int)base;
        *p++ = (digit < 10) ? char('0' + digit) : char('a' + digit - 10);
        u /= (unsigned int)base;
    } while(u);
    if(negative) *p++ = '-';
    *p = '\0';

    // reverse in place
    for(char* q = p - 1; start < q; ++start, --q) {
        char t = *start; *start = *q; *q = t;
    }
    return buffer;
}

#endif // _ARDUINO_ITOA_H_
