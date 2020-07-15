/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ldr.c - ldr functions
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

#include "adc.h"
#include "ldr.h"
#include "delay.h"
#include "eeprom.h"
#include "eeprom-data.h"

LDR_GLOBALS      ldr =
{
    0,              // ldr_value
    0,              // ldr_raw_value
    0,              // ldr_min_value
    0,              // ldr_max_value
};

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * start conversion
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
ldr_start_conversion (void)
{
    adc_start_single_conversion ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * poll LDR value
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
ldr_poll_brightness  (void)
{
    uint_fast8_t    rtc;

    rtc = adc_poll_conversion_value (&ldr.ldr_raw_value);

    if (rtc)
    {
        if (ldr.ldr_raw_value < ldr.ldr_min_value)
        {
            ldr.ldr_raw_value = ldr.ldr_min_value;
        }

        if (ldr.ldr_raw_value > ldr.ldr_max_value)
        {
            ldr.ldr_raw_value = ldr.ldr_max_value;
        }

        if (ldr.ldr_max_value > ldr.ldr_min_value)
        {
            ldr.ldr_value = (32 * (ldr.ldr_raw_value - ldr.ldr_min_value)) / (ldr.ldr_max_value - ldr.ldr_min_value);

            if (ldr.ldr_value > 31)                         // if (ldr_raw_value == ldr.ldr_max_value) then ldr.ldr_value is 32
            {
                ldr.ldr_value = 31;
            }
        }
        else
        {
            ldr.ldr_value = 31;
        }
    }
    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read configuration from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
ldr_read_config_from_eeprom (uint32_t eeprom_version)
{
    ldr.ldr_min_value = 0;
    ldr.ldr_max_value = 4095;

    if (eeprom_is_up)
    {
        if (eeprom_version >= EEPROM_VERSION_2_1)
        {
            eeprom_read (EEPROM_DATA_OFFSET_LDR_MIN_VALUE, (uint8_t *) &ldr.ldr_min_value, EEPROM_DATA_SIZE_LDR_MIN_VALUE);
            eeprom_read (EEPROM_DATA_OFFSET_LDR_MAX_VALUE, (uint8_t *) &ldr.ldr_max_value, EEPROM_DATA_SIZE_LDR_MAX_VALUE);

            if (ldr.ldr_min_value == 0xFFFF)
            {
                ldr.ldr_min_value = 0;
            }

            if (ldr.ldr_max_value == 0xFFFF)
            {
                ldr.ldr_max_value = 1023;
            }
        }
    }

    return;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write configuration to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
ldr_write_config_to_eeprom (void)
{
    uint_fast8_t            rtc = 0;

    if (eeprom_is_up)
    {
        if (eeprom_write (EEPROM_DATA_OFFSET_LDR_MIN_VALUE, (uint8_t *) &ldr.ldr_min_value, EEPROM_DATA_SIZE_LDR_MIN_VALUE) &&
            eeprom_write (EEPROM_DATA_OFFSET_LDR_MAX_VALUE, (uint8_t *) &ldr.ldr_max_value, EEPROM_DATA_SIZE_LDR_MAX_VALUE))

        {
            rtc = 1;
        }
    }

    return rtc;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * set minimum value
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast16_t
ldr_set_min_value (void)
{
    uint16_t        adc_value;
    uint_fast8_t    cnt = 0;

    adc_start_single_conversion ();

    while (! adc_poll_conversion_value (&adc_value))
    {
        delay_msec (10);
        cnt++;

        if (cnt >= 10)
        {
            break;
        }
    }

    if (cnt < 10)
    {
        ldr.ldr_min_value = adc_value;
        ldr_write_config_to_eeprom ();
    }

    return ldr.ldr_min_value;
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------
 * set maximum value
 *-----------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast16_t
ldr_set_max_value (void)
{
    uint16_t        adc_value;
    uint_fast8_t    cnt = 0;

    adc_start_single_conversion ();

    while (! adc_poll_conversion_value (&adc_value))
    {
        delay_msec (10);
        cnt++;

        if (cnt >= 10)
        {
            break;
        }
    }

    if (cnt < 10)
    {
        ldr.ldr_max_value = adc_value;
        ldr_write_config_to_eeprom ();
    }

    return ldr.ldr_max_value;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize LDR
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
ldr_init (void)
{
    adc_init ();
}
