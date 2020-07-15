/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * base.h - declarations of base routines
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef BASE_H
#define BASE_H

#include <string.h>
#include <stdint.h>
#include <time.h>
#include "esp8266.h"

extern const char * wdays_en[7];
extern const char * wdays_de[7];

extern int          dayofweek (int, int, int);
extern int          is_holiday (int, int, int);
extern uint16_t     htoi (char *, uint8_t);
extern void         strsubst (char *, int, int);
#endif
