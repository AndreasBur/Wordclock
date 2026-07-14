#ifndef _ARDUINO_TYPES_H_
#define _ARDUINO_TYPES_H_

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/*  Arduino core types                                                        */
/* -------------------------------------------------------------------------- */
/*  Must be visible before Pixels.h (and its Pixel.h) are pulled in: those    */
/*  headers use `byte`. On Windows it came in transitively via <windows.h>;   */
/*  on GTK/Linux it did not, so define it up front here. Arduino.h therefore  */
/*  includes this header first.                                               */
/* -------------------------------------------------------------------------- */
typedef uint8_t byte;
typedef bool    boolean;

#endif // _ARDUINO_TYPES_H_
