#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include <string.h>
#include "WS2812.h"

typedef unsigned char byte;

#define F(x)    x

#define pgm_read_byte(x) *x
#define memcpy_P(x,y,z) memcpy(x,y,z)
#define PROGMEM

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#define Serial  WS2812::getInstance()

#endif
