#ifndef _ARDUINO_H_
#define _ARDUINO_H_

/* ========================================================================== */
/*  Arduino compatibility shim for the wxWidgets simulator                    */
/* -------------------------------------------------------------------------- */
/*  Umbrella header that pulls the individual pieces together. Include order  */
/*  matters: arduino/Types.h defines `byte`, which sim/Pixels.h depends on,   */
/*  must come first. Serial is bound last because it needs the Pixels class.  */
/* ========================================================================== */

#include "arduino/Types.h"      // byte, boolean  (before Pixels.h!)

#include "sim/Pixels.h"

#include "arduino/Progmem.h"    // PROGMEM, F, pgm_read_byte, memcpy_P
#include "arduino/Bits.h"       // bitRead
#include "arduino/Itoa.h"       // itoa()

/* -------------------------------------------------------------------------- */
/*  Serial → simulator window                                                 */
/* -------------------------------------------------------------------------- */
/*  Serial I/O is routed to the wxWidgets Pixels window (print/read live on   */
/*  the Pixels singleton). Kept here rather than in a sub-header because it   */
/*  is specific to this simulator, not to the Arduino API.                    */
/* -------------------------------------------------------------------------- */
#define Serial                  Pixels::getInstance()

#endif // _ARDUINO_H_
