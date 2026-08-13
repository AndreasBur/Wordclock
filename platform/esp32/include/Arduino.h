/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       Arduino.h
 *      \brief      The core's Arduino.h, with Serial bound to WordclockSerial
 *
 *      \details    Everything the firmware uses of the Arduino API - byte, F(), PROGMEM,
 *                  pgm_read_byte, bitRead, itoa - comes from the real header, which this
 *                  one pulls in through include_next. Only Serial is replaced, so that a
 *                  web socket can reach the same port Communication reads its commands
 *                  from. The simulator's Arduino.h does exactly this.
 *
 *                  Order matters twice over. The real header has to come first, because it
 *                  declares the port this file then hides. And the port has to be captured
 *                  before it is hidden - hardwarePort() below - because after the macro
 *                  the name no longer refers to it. Capturing it through a function rather
 *                  than a reference variable is what makes it work whether the core made
 *                  Serial an object or, on a board that boots on USB CDC, a macro alias
 *                  for another one.
 *
 *                  The real header is reached through a path the build passes in, not
 *                  through include_next. include_next looks like the right tool and is
 *                  not: the firmware includes "Arduino.h" in quotes, so this file is found
 *                  relative to the includer rather than on the bracket path, and the
 *                  search then restarts at the front of that path - finding this file
 *                  again, where the include guard silently swallows it. The result
 *                  compiles until the first use of byte.
 *
 *                  A platform source that needs the framework's own headers - WiFi.h,
 *                  Preferences.h, esp_http_server.h - has to include them before this
 *                  file, or the macro reaches into them.
 *
******************************************************************************************************************************************************/
#ifndef _WORDCLOCK_ARDUINO_H_
#define _WORDCLOCK_ARDUINO_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* The real one, at the path scripts/firmware_includes.py found in the installed framework
   and passed in as a define. Not hardcoded, so a framework update moves it on its own; not
   include_next, for the reason given above. */
#ifndef WORDCLOCK_CORE_ARDUINO_H
# error "Arduino.h: WORDCLOCK_CORE_ARDUINO_H is not set. scripts/firmware_includes.py defines it; a build that bypasses that script cannot find the Arduino core's own header."
#endif

#include WORDCLOCK_CORE_ARDUINO_H

/* Proof that the line above really brought the core in. Without it everything below still
   parses, and the build fails much later on the first use of byte, in a firmware file that
   has nothing to do with the cause. */
static_assert(sizeof(byte) == 1u, "Arduino.h: the Arduino core's header did not come in through WORDCLOCK_CORE_ARDUINO_H");

#include "WordclockSerial.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Proof that this file, and not the core's, was the one that got included. WordclockSerial.cpp
   refuses to build without it: the alternative failure is silent - everything compiles, the
   firmware talks to the UART as before, and only the web console stays mute. */
#define WORDCLOCK_ARDUINO_SHIM                          1

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N S
******************************************************************************************************************************************************/
namespace WordclockPlatform {

/* Both of these have to be defined while Serial still means the core's port. */
inline Stream& hardwarePort() { return Serial; }
inline void hardwarePortBegin(unsigned long BaudRate) { Serial.begin(BaudRate); }

} // namespace WordclockPlatform

/******************************************************************************************************************************************************
 *  S E R I A L
******************************************************************************************************************************************************/
/* Undefined first, because the core defines Serial as a macro of its own - on this target
   it aliases whichever port the board boots its console on. Replacing it without saying so
   is a redefinition, and the compiler is right to warn; the alias is also exactly why the
   port above is captured through a function rather than a reference. */
#undef Serial
#define Serial                  WordclockSerial::getInstance()

#endif // _WORDCLOCK_ARDUINO_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
