#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include <stdint.h>
#include <string.h>

// Must be defined before Pixels.h (and its Pixel.h) are pulled in: those
// headers use `byte`. On Windows it came in transitively via <windows.h>;
// on GTK/Linux it did not, so define it up front here.
typedef uint8_t byte;
typedef bool boolean;

#include "Pixels.h"

#define F(x)                    x

#define pgm_read_byte(x)        *x
#define memcpy_P(x,y,z)         memcpy(x,y,z)
#define PROGMEM

#define bitRead(value, bit)     (((value) >> (bit)) & 0x01)

// The Arduino core provides itoa(); GCC/glibc does not. Minimal reimplementation
// matching Arduino's signature (value, buffer, base), including base-10 sign.
inline char* itoa(int Value, char* Buffer, int Base)
{
    char* p = Buffer;
    bool Negative = (Base == 10 && Value < 0);
    unsigned int u = Negative ? (unsigned int)(-Value) : (unsigned int)Value;

    char* start = p;
    do {
        int Digit = u % (unsigned int)Base;
        *p++ = (Digit < 10) ? char('0' + Digit) : char('a' + Digit - 10);
        u /= (unsigned int)Base;
    } while(u);
    if(Negative) *p++ = '-';
    *p = '\0';

    // reverse in place
    for(char* q = p - 1; start < q; ++start, --q) {
        char t = *start; *start = *q; *q = t;
    }
    return Buffer;
}

#define Serial                  Pixels::getInstance()

#endif
