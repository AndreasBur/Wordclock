#ifndef _ARDUINO_PROGMEM_H_
#define _ARDUINO_PROGMEM_H_

#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Flash-memory (PROGMEM) shims                                              */
/* -------------------------------------------------------------------------- */
/*  On the simulator everything lives in normal RAM, so the flash accessors   */
/*  collapse to plain memory access.                                          */
/* -------------------------------------------------------------------------- */
#define PROGMEM
#define F(x)                    x
#define pgm_read_byte(x)        (*(x))
#define memcpy_P(x, y, z)       memcpy((x), (y), (z))

#endif // _ARDUINO_PROGMEM_H_
