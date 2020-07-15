/*-------------------------------------------------------------------------------------------------------------------------------------------
 * eeprom.c - EEPROM routines
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "eeprom.h"
#include "i2c.h"

#define EEPROM_FIRST_ADDR       0xA0
#define EEPROM_WAITSTATES       15                              // we have to wait 15ms after each write cycle

uint_fast8_t                    eeprom_is_up = 0;
volatile uint_fast8_t           eeprom_ms_tick;                 // should be set every 1 ms by IRQ, see main.c

static  uint_fast8_t            eeprom_addr;


/*--------------------------------------------------------------------------------------------------------------------------------------
 * eeprom_waitstates() - wait 15 ms after each write cycle
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
static void
eeprom_waitstates (void)
{
    uint_fast16_t  cnt = 0;

    while (1)
    {
        if (eeprom_ms_tick)
        {
            eeprom_ms_tick = 0;

            cnt++;

            if (cnt > EEPROM_WAITSTATES)
            {
                break;
            }
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize EEPROM functions
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_init (uint32_t clockspeed)
{
    uint_fast8_t    i;
    uint8_t         value;

    i2c_init (clockspeed);

    for (i = 0; i < 8; i++)                                     // test read 0xA0, 0xA2, 0xA4, 0xA6 .. 0xAE
    {
        eeprom_addr = EEPROM_FIRST_ADDR | (i << 1);

        if (i2c_read (eeprom_addr, 0x00, 1, &value, 1) == I2C_OK)
        {
            eeprom_is_up = 1;
            break;
        }
    }

    return eeprom_is_up;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get address of EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_get_address (void)
{
    return eeprom_addr;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_read (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt)
{
    uint_fast8_t    rtc;

    if (eeprom_is_up)
    {
        if (i2c_read (eeprom_addr, start_addr, 1, buffer, cnt) == I2C_OK)
        {
            rtc = 1;
        }
        else
        {
            rtc = 0;
        }
    }
    else
    {
        rtc = 0;
    }
    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
eeprom_write (uint_fast16_t start_addr, uint8_t * buffer, uint_fast16_t cnt)
{
    uint_fast8_t rtc;

    if (eeprom_is_up)
    {
        // we must write every single byte, because we have to wait 15ms every cycle
        while (cnt--)
        {
            if (i2c_write (eeprom_addr, start_addr, 1, buffer, 1) == I2C_OK)
            {
                rtc = 1;
            }
            else
            {
                rtc = 0;
                break;
            }
            eeprom_waitstates ();
            start_addr++;
            buffer++;
        }
    }
    else
    {
        rtc = 0;
    }
    return rtc;
}
