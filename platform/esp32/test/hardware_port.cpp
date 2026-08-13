/* The hardware port itself, plus the two C functions the core would provide.

   Deliberately kept out of any file that includes the platform's Arduino.h: there the name
   Serial is the multiplexer, so an object of that name could not be defined. This file
   includes the stub directly, by the path the build passes in. */
#include WORDCLOCK_CORE_ARDUINO_H

#include <stdio.h>

Stream Serial;

void configTzTime(const char*, const char*, const char*) { }

char* itoa(int Value, char* Buffer, int) { snprintf(Buffer, 12, "%d", Value); return Buffer; }
