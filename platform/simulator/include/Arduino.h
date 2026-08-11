#ifndef _ARDUINO_H_
#define _ARDUINO_H_

/* ========================================================================== */
/*  Arduino compatibility shim for the wxWidgets simulator                    */
/* -------------------------------------------------------------------------- */
/*  Umbrella header that pulls the individual pieces together. Include order  */
/*  matters: arduino/Types.h defines `byte`, which the sim headers depend on,  */
/*  so it comes first. Serial is bound last because it needs SerialShim.      */
/* ========================================================================== */

#include "arduino/Types.h"      // byte, boolean  (before the sim headers!)

#include "sim/SerialShim.h"

#include "arduino/Progmem.h"    // PROGMEM, F, pgm_read_byte, memcpy_P
#include "arduino/Bits.h"       // bitRead
#include "arduino/Itoa.h"       // itoa()

/* -------------------------------------------------------------------------- */
/*  Serial → simulator window                                                 */
/* -------------------------------------------------------------------------- */
/*  Serial I/O goes to SerialShim, which writes it into the text controls of  */
/*  the Pixels window and hands typed lines back. Kept here rather than in a  */
/*  sub-header because it is specific to this simulator, not to the Arduino   */
/*  API.                                                                      */
/* -------------------------------------------------------------------------- */
#define Serial                  SerialShim::getInstance()

#endif // _ARDUINO_H_
