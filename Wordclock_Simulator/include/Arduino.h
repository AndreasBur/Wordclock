#ifndef _ARDUINO_H_
#define _ARDUINO_H_

typedef unsigned char byte;

#ifndef _WIN32
typedef bool boolean;
#endif // _WIN32

#define pgm_read_byte(x) *x
#define memcpy_P(x,y,z) memcpy(x,y,z)
#define PROGMEM

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

#endif
