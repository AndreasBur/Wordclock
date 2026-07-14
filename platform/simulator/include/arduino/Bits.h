#ifndef _ARDUINO_BITS_H_
#define _ARDUINO_BITS_H_

/* -------------------------------------------------------------------------- */
/*  Bit helpers                                                               */
/* -------------------------------------------------------------------------- */
#define bitRead(value, bit)     (((value) >> (bit)) & 0x01)

#endif // _ARDUINO_BITS_H_
