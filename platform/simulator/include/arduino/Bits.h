#ifndef _ARDUINO_BITS_H_
#define _ARDUINO_BITS_H_

/* -------------------------------------------------------------------------- */
/*  Bit helpers                                                               */
/* -------------------------------------------------------------------------- */
#define bitRead(value, bit)     (((value) >> (bit)) & 0x01) // NOLINT(readability-identifier-naming)

#endif // _ARDUINO_BITS_H_
