#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include <string.h>

typedef unsigned char byte;

#define pgm_read_byte(x) *x
#define memcpy_P(x,y,z) memcpy(x,y,z)
#define PROGMEM

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#endif
