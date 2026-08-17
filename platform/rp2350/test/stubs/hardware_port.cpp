/* The hardware port itself, plus the one C function the core would provide.

   Deliberately kept out of any file that includes the platform's Arduino.h: there the name
   Serial is the multiplexer, so an object of that name could not be defined. This file
   includes the stub directly, by the path the build passes in. */
#include WORDCLOCK_CORE_ARDUINO_H

#include <stdio.h>

Stream Serial;

char* itoa(int Value, char* Buffer, int) { snprintf(Buffer, 12, "%d", Value); return Buffer; }
