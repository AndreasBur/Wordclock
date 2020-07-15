/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main.c - main routines of wclock24h
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * System Clocks configured on STM32F103 Mini development board:
 *
 *    External 8MHz crystal
 *    SYSCLK:       72MHz ((HSE_VALUE / HSE_Prediv) * PLL_Mul) = ((8MHz / 1) * 9)  = 72MHz
 *    AHB clock:    72MHz (AHB  Prescaler = 1)
 *    APB1 clock:   36MHz (APB1 Prescaler = 2)
 *    APB2 clock:   72MHz (APB2 Prescaler = 1)
 *    Timer clock:  72MHz (APB1 Prescaler = 2, Timer Multiplier 2)
 *
 * System Clocks configured on STM32F401 and STM32F411 Nucleo Board (see changes in system_stm32f4xx.c):
 *
 *    External 8MHz crystal
 *    Main clock:   84MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_P = ((8MHz / 8) * 336) / 4 = 84MHz
 *    AHB clock:    84MHz (AHB  Prescaler = 1)
 *    APB1 clock:   42MHz (APB1 Prescaler = 2)
 *    APB2 clock:   84MHz (APB2 Prescaler = 1)
 *    Timer clock:  84MHz
 *    SDIO clock:   48MHz ((HSE_VALUE / PLL_M) * PLL_N) / PLL_Q = ((8MHz / 8) * 336) / 7 = 48MHz
 *
 * On STM32F401/STM32F411 Nucleo Board, make sure that
 *
 *    - SB54 and SB55 are OFF
 *    - SB16 and SB50 are OFF
 *    - R35 and R37 are soldered (0R or simple wire)
 *    - C33 and C34 are soldered with 22pF capacitors
 *    - X3 is soldered with 8MHz crystal
 *
 * Internal devices used:
 *
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | Device                  | STM32F4x1 Nucleo                  | STM32F103C8T6                     |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | User button             | GPIO:   PC13                      | GPIO:   PA6                       |
 *    | Board LED               | GPIO:   PA5                       | GPIO:   PC13                      |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *
 * External devices:
 *
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | Device                  | STM32F4x1 Nucleo                  | STM32F103C8T6                     |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | TSOP31238 (IRMP)        | GPIO:      PC10                   | GPIO:      PB3                    |
 *    | DS18xx (OneWire)        | GPIO:      PD2                    | GPIO:      PB5                    |
 *    | Logger (Nucleo: USB)    | USART2:    TX=PA2  RX=PA3         | USART1:    TX=PA9  RX=PA10        |
 *    | WPS button              | GPIO:      PC5                    | GPIO:      PA7                    |
 *    | ESP8266 USART           | USART6:    TX=PA11 RX=PA12        | USART2:    TX=PA2  RX=PA3         |
 *    | ESP8266 RST/CH_PD       | GPIO:      RST=PA7 CH_PD=PA6      | GPIO:      RST=PA0 CH_PD=PA1      |
 *    | ESP8266 GPIO0           | GPIO:      FLASH=PA4              | GPIO:      FLASH=PA4              |
 *    | ESP8266 GPIO13/GPIO15   | USART1:    GPIO13=PA9 GPIO15=PA10 | USART1:    GPIO13=PA9 GPIO15=PA10 |
 *    | ESP8266 GPIO14          | GPIO:      GPIO14=RESET           | GPIO:      GPIO14=RESET           |
 *    | ESP8266 GPIO4           | GPIO:      GPIO4=BOOT0            | GPIO:      GPIO4=BOOT0            |
 *    | DCF77                   | GPIO:      DATA=PC11 PON=PC12     | GPIO:      DATA=PB8  PON=PB9      |
 *    | I2C DS3231 & EEPROM     | I2C3:      SCL=PA8 SDA=PC9        | I2C1:      SCL=PB6 SDA=PB7        |
 *    | LDR                     | ADC:       ADC1_IN14=PC4          | ADC:       ADC12_IN5=PA5          |
 *    | WS2812 / SK6812         | TIM3/DMA1: PC6                    | TIM1/DMA1: PA8                    |
 *    | APA102                  | SPI2/DMA1: SCK=PB13 MOSI=PB15     | SPI2/DMA1: SCK=PB13 MOSI=PB15     |
 *    | Power switch            | GPIO:      PC8                    | GPIO:      PB0                    |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | Free Port A             | PA0 PA1 PA15                      | PA11 PA12 PA15                    |
 *    | Free Port B             | PB0 PB2 PB4..PB7 PB10...PB12 PB14 | PB1  PB4  PB10 PB11 PB12 PB14     |
 *    | Free Port C             | PC3 PC7                 PC14 PC15 | PC14 PC15                         |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *
 * Timers:
 *
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | Device                  | STM32F4x1 Nucleo                  | STM32F103C8T6                     |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *    | General (IRMP etc.)     | TIM2                              | TIM2                              |
 *    | WS2812                  | TIM3                              | TIM1                              |
 *    | DS18xx (OneWire)        | Systick (see delay.c)             | Systick (see delay.c)             |
 *    +-------------------------+-----------------------------------+-----------------------------------+
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wclock24h-config.h"
#include "base.h"
#include "display.h"
#include "dcf77.h"
#include "timeserver.h"
#include "esp8266.h"
#include "board-led.h"
#include "power.h"
#include "irmp.h"
#include "remote-ir.h"
#include "button.h"
#include "wpsbutton.h"
#include "eeprom.h"
#include "eeprom-data.h"
#include "tempsensor.h"
#include "ds18xx.h"
#include "rtc.h"
#include "ldr.h"
#include "delay.h"
#include "night.h"
#include "weather.h"
#include "vars.h"
#if WCLOCK24H == 1
#include "tables.h"
#else
#include "tables12h.h"
#endif

#include "tetris.h"
#include "log.h"
#include "main.h"

#define DEFAULT_UPDATE_HOST     "uclock.de"
#define DEFAULT_UPDATE_PATH     "update"

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global public variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
MAIN_GLOBALS                    gmain;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global private variables
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t             esp8266_is_online           = 0;
static uint32_t                 eeprom_version              = 0xFFFFFFFF;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * global private variables, modified by timer ISR
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static volatile uint_fast8_t    animation_flag              = 0;        // flag: animate LEDs
static volatile uint_fast8_t    seconds_flag                = 0;        // flag: seconds changed
static volatile uint_fast8_t    ds3231_flag                 = 0;        // flag: read date/time from RTC DS3231
static volatile uint_fast8_t    net_time_flag               = 0;        // flag: read date/time from time server
static volatile uint_fast16_t   net_time_countdown          = 3800;     // counter: if it counts to 0, then net_time_flag will be triggered
static volatile uint_fast8_t    ldr_conversion_flag         = 0;        // flag: read LDR value
static volatile uint_fast8_t    measure_temperature_flag    = 0;        // flag: measure temperature from DS18XX
static volatile uint_fast8_t    read_temperature_flag       = 0;        // flag: read temperature from RTC & DS18XX
static volatile uint_fast8_t    read_rtc_temperature_flag   = 0;        // flag: read temperature from RTC
static volatile uint_fast8_t    show_time_flag              = 0;        // flag: update time on display, set every full minute
static volatile uint_fast8_t    half_minute_flag            = 0;        // flag: it is hh:mm:30
volatile uint32_t               uptime                      = 0;        // uptime in seconds
static volatile uint_fast8_t    wday                        = 0;        // current weekday, 0=Sunday
static volatile uint_fast8_t    hour                        = 0;        // current hour
static volatile uint_fast8_t    minute                      = 0;        // current minute
static volatile uint_fast8_t    second                      = 0;        // current second

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timer definitions:
 *
 *      F_INTERRUPTS    = TIM_CLK / (TIM_PRESCALER + 1) / (TIM_PERIOD + 1)
 * <==> TIM_PRESCALER   = TIM_CLK / F_INTERRUPTS / (TIM_PERIOD + 1) - 1
 *
 * STM32F4x1:
 *      TIM_PERIOD      =   8 - 1 =   7
 *      TIM_PRESCALER   = 700 - 1 = 699
 *      F_INTERRUPTS    = 84000000 / 700 / 8 = 15000
 * STM32F103:
 *      TIM_PERIOD      =   6 - 1 =   5
 *      TIM_PRESCALER   = 800 - 1 = 799
 *      F_INTERRUPTS    = 72000000 / 800 / 6 = 15000
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#if defined (STM32F401RE) || defined (STM32F411RE)                          // STM32F401/STM32F411 Nucleo Board PC13
#define TIM_CLK                 84000000L                                   // timer clock, 84MHz on STM32F401/411 Nucleo Board
#define TIM_PERIOD              7

#elif defined (STM32F103)
#define TIM_CLK                 72000000L                                   // timer clock, 72MHz on STM32F103
#define TIM_PERIOD              5
#else
#error STM32 unknown
#endif

#define TIM_PRESCALER           ((TIM_CLK / F_INTERRUPTS) / (TIM_PERIOD + 1) - 1)

#define STATUS_LED_FLASH_TIME   50                                          // status LED: time of flash


/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize timer2
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
timer2_init (void)
{
    TIM_TimeBaseInitTypeDef     tim;
    NVIC_InitTypeDef            nvic;

    TIM_TimeBaseStructInit (&tim);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);

    tim.TIM_ClockDivision   = TIM_CKD_DIV1;
    tim.TIM_CounterMode     = TIM_CounterMode_Up;
    tim.TIM_Period          = TIM_PERIOD;
    tim.TIM_Prescaler       = TIM_PRESCALER;
    TIM_TimeBaseInit (TIM2, &tim);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    nvic.NVIC_IRQChannel                    = TIM2_IRQn;
    nvic.NVIC_IRQChannelCmd                 = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority  = 0x0F;
    nvic.NVIC_IRQChannelSubPriority         = 0x0F;
    NVIC_Init (&nvic);

    TIM_Cmd(TIM2, ENABLE);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timer2 IRQ handler for IRMP, soft clock, dcf77, animations, several timeouts
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
extern void TIM2_IRQHandler (void);                                     // keep compiler happy

void
TIM2_IRQHandler (void)
{
    static uint_fast8_t  last_minute_of_ds3231_flag = 0xFF;
    static uint_fast16_t ldr_cnt;
    static uint_fast16_t animation_cnt;
    static uint_fast16_t clk_cnt;
    static uint_fast16_t dcf77_cnt;
    static uint_fast16_t net_time_cnt;
    static uint_fast16_t eeprom_cnt;

    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    (void) irmp_ISR ();                                                 // call irmp ISR

    if (uptime > 1)
    {
        ldr_cnt++;

        if (ldr_cnt == F_INTERRUPTS / 4)                                // start LDR conversion every 1/4 seconds
        {
            ldr_conversion_flag = 1;
            ldr_cnt = 0;
        }
    }

    animation_cnt++;

    if (animation_cnt == F_INTERRUPTS / 64)                             // set animation_flag every 1/64 of a second
    {
        animation_flag = 1;
        animation_cnt = 0;
    }

    dcf77_cnt++;

    if (dcf77_cnt == F_INTERRUPTS / 100)                                // call dcf77_tick every 1/100 of a second
    {
        dcf77_cnt = 0;
        dcf77_tick ();
    }

    net_time_cnt++;

    if (net_time_cnt == F_INTERRUPTS / 100)                             // set esp8266_ten_ms_tick every 1/100 of a second
    {
        esp8266_ten_ms_tick = 1;
        net_time_cnt = 0;
    }

    eeprom_cnt++;

    if (eeprom_cnt == F_INTERRUPTS / 1000)                              // set eeprom_ms_tick every 1/1000 of a second
    {
        eeprom_ms_tick = 1;
        eeprom_cnt = 0;
    }

    clk_cnt++;

    if (clk_cnt == F_INTERRUPTS)                                        // increment internal clock every second
    {
        clk_cnt = 0;
        seconds_flag = 1;

        uptime++;
        second++;

        if (second == 45)                                       // get rtc time at hh:mm:45, but not twice in same minute
        {
            if (last_minute_of_ds3231_flag != minute)
            {
                last_minute_of_ds3231_flag = minute;
                ds3231_flag = 1;
            }
        }
        else if (second == 49)
        {
            measure_temperature_flag = 1;                       // start ADC conversion of DS18xx
        }
        else if (second == 50)
        {
            read_temperature_flag = 1;                          // read temperature data of DS18xx
        }
        else if (second == 51)
        {
            read_rtc_temperature_flag = 1;                      // read temperature data of RTC
        }
        else if (second == 60)
        {
            second = 0;
            minute++;

            show_time_flag = 1;

            if (minute == 60)
            {
                minute = 0;
                hour++;

                if (hour == 24)
                {
                    hour = 0;
                    wday++;

                    if (wday == 7)
                    {
                        wday = 0;
                    }
                }
            }
        }
        else if (second == 30)
        {
            half_minute_flag = 1;
        }

        if (net_time_countdown)
        {
            net_time_countdown--;

            if (net_time_countdown == 0)                                // trigger net time update
            {
                net_time_flag = 1;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read version from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_version_from_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_read (EEPROM_DATA_OFFSET_VERSION, (uint8_t *) &eeprom_version, sizeof(uint32_t));
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write version to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_version_to_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_write (EEPROM_DATA_OFFSET_VERSION, (uint8_t *) &eeprom_version, sizeof(uint32_t));
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read update host from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_update_host_from_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_read (EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, (uint8_t *) gmain.update_host, EEPROM_DATA_SIZE_UPDATE_HOSTNAME);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write update host to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_update_host_to_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_write (EEPROM_DATA_OFFSET_UPDATE_HOSTNAME, (uint8_t *) gmain.update_host, EEPROM_DATA_SIZE_UPDATE_HOSTNAME);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read update path from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_update_path_from_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_read (EEPROM_DATA_OFFSET_UPDATE_PATH, (uint8_t *) gmain.update_path, EEPROM_DATA_SIZE_UPDATE_PATH);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write update path to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_update_path_to_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        rtc = eeprom_write (EEPROM_DATA_OFFSET_UPDATE_PATH, (uint8_t *) gmain.update_path, EEPROM_DATA_SIZE_UPDATE_PATH);
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * read main parameters from EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
read_main_parameters_from_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        if (eeprom_version >= EEPROM_VERSION_2_6)
        {
            if (read_update_host_from_eeprom () &&
                read_update_path_from_eeprom ())
            {
                if (gmain.update_host[0] >= 'a' && gmain.update_host[0] <= 'z' &&
                    gmain.update_path[0] >= 'a' && gmain.update_path[0] <= 'z')
                {                                                                   // eeprom correctly initialized?
                    rtc = 1;
                }
            }
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * write main parameters to EEPROM
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
write_main_parameters_to_eeprom (void)
{
    uint_fast8_t    rtc = 0;

    if (eeprom_is_up)
    {
        if (write_update_host_to_eeprom () &&
            write_update_path_to_eeprom ())
        {
            rtc = 1;
        }
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * toggle power
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
toggle_power (uint_fast8_t do_sync)
{
    uint_fast8_t            display_clock_flag;

    display.power_is_on = ! display.power_is_on;
    display_set_ambilight_power(display.power_is_on);

    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;

    if (display.power_is_on)
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_ON;
    }
    else
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_OFF;
    }

    if (do_sync)
    {
        var_send_display_power ();
    }

    return display_clock_flag;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set power
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
set_power (uint_fast8_t new_power_is_on)
{
    uint_fast8_t            display_clock_flag;

    display.power_is_on = new_power_is_on;
    display_set_ambilight_power(display.power_is_on);

    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;

    if (display.power_is_on)
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_ON;
    }
    else
    {
        display_clock_flag |= DISPLAY_CLOCK_FLAG_POWER_OFF;
    }

    return display_clock_flag;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * log eeprom write error
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
log_eeprom_error (const char * str)
{
    log_printf ("Writing %s to EEPROM failed\r\n", str);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main function
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int
main (void)
{
    static uint_fast16_t    last_ldr_raw_value          = 0xFFFF;
    static uint_fast8_t     last_ldr_value              = 0xFF;
    uint_fast8_t            esp8266_is_up               = 0;
    IRMP_DATA               irmp_data;
    uint32_t                stop_time;
    uint_fast8_t            cmd;
    uint_fast8_t            status_led_cnt              = 0;
    uint_fast8_t            display_clock_flag          = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
    uint_fast8_t            time_changed                = 0;
    uint_fast16_t           ldr_raw_value;
    uint_fast8_t            ldr_value;
    uint32_t                local_uptime;
    uint32_t                ap_pressed                  = 0;
    uint32_t                wps_pressed                 = 0;                // timestamp when wps button has been pressed
    uint32_t                clockspeed                  = 100000;           // default clock speed for i2c bus
    uint_fast8_t            msg_rtc;
    uint_fast8_t            ds18xx_temperature_index    = 0xFF;
    uint_fast8_t            rtc_temperature_index       = 0xFF;
    uint_fast8_t            show_overlay_idx            = 0xFF;
    uint32_t                show_icon_stop_time         = 0;

    SystemInit ();
    SystemCoreClockUpdate();                                                // needed for Nucleo board

#if defined (STM32F103)                                                     // disable JTAG to get back PB3, PB4, PA13, PA14, PA15
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);                    // turn on clock for the alternate function register
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);                // disable the JTAG, enable the SWJ interface
#endif

    gmain.hardware_configuration = 0;
#if defined (STM32F103)
    gmain.hardware_configuration |= HW_STM32_F103;
#elif defined (STM32F401RE)
    gmain.hardware_configuration |= HW_STM32_F401;
#elif defined (STM32F411RE)
    gmain.hardware_configuration |= HW_STM32_F411;
#else
# error unknown STM32 hardware
#endif

#if WCLOCK24H == 1
    gmain.hardware_configuration |= HW_WC_24H;
#else
    gmain.hardware_configuration |= HW_WC_12H;
#endif

#if DSP_USE_WS2812_GRB == 1
    gmain.hardware_configuration |= HW_LED_WS2812_GRB_LED;
#elif DSP_USE_WS2812_RGB == 1
    gmain.hardware_configuration |= HW_LED_WS2812_RGB_LED;
#elif DSP_USE_APA102 == 1
    gmain.hardware_configuration |= HW_LED_APA102_RGB_LED;
#elif DSP_USE_SK6812_RGB == 1
    gmain.hardware_configuration |= HW_LED_SK6812_RGB_LED;
#elif DSP_USE_SK6812_RGBW == 1
    gmain.hardware_configuration |= HW_LED_SK6812_RGBW_LED;
#else
# error unknown LED type
#endif

    strcpy (gmain.update_host, DEFAULT_UPDATE_HOST);
    strcpy (gmain.update_path, DEFAULT_UPDATE_PATH);

    log_init (115200);                                                      // initilize logger on uart

    irmp_init ();                                                           // initialize IRMP

    power_init ();                                                          // initialize power port pin
    power_on ();                                                            // switch power on
    delay_init (DELAY_RESOLUTION_5_US);                                     // initialize delay functions with granularity of 5 us
    timer2_init ();                                                         // initialize timer2 for IRMP, DCF77, EEPROM etc.
    board_led_init ();                                                      // initialize GPIO for board LED
    button_init ();                                                         // initialize GPIO for user button
    wpsbutton_init ();                                                      // initialize GPIO for WPS button

    if (button_pressed ())                                                  // set ESP8266 into flash mode
    {
        board_led_on ();
        esp8266_flash ();
    }

    log_message ("\r\nWelcome to WordClock Logger!");
    log_message ("----------------------------");
    log_puts ("Version: ");
    log_message (VERSION);

    rtc_init (clockspeed);                                                  // initialize I2C RTC
    eeprom_init (clockspeed);                                               // initialize I2C EEPROM

    if (grtc.rtc_is_up)
    {
        log_message ("rtc is online");
    }
    else
    {
        log_message ("rtc is offline, trying to reduce clock speed...");
        clockspeed /= 2;

        rtc_init (clockspeed);                                                  // initialize I2C RTC again
        eeprom_init (clockspeed);                                               // initialize I2C EEPROM again

        if (grtc.rtc_is_up)
        {
            log_message ("rtc is now online");
        }
        else
        {
            log_message ("rtc remains offline");
        }
    }

    display_init ();                                                        // initialize display

    if (eeprom_is_up)
    {
        log_message ("eeprom is online");
        read_version_from_eeprom ();
        log_printf ("current eeprom version: 0x%08x\r\n", eeprom_version);

        if ((eeprom_version & 0xFF0000FF) == 0x00000000 &&
            eeprom_version >= EEPROM_VERSION_1_5 && eeprom_version <= EEPROM_VERSION)
        {                                                               // Upper and Lower Byte must be 0x00
            gmain.eeprom_version[0] = ((eeprom_version >> 16) & 0xFF) + '0';
            gmain.eeprom_version[1] = '.';
            gmain.eeprom_version[2] = ((eeprom_version >>  8) & 0xFF) + '0';
            gmain.eeprom_version[4] = '\0';

            if (eeprom_version >= EEPROM_VERSION_1_5)
            {
                log_message ("reading ir codes from eeprom");
                remote_ir_read_codes_from_eeprom ();

                debug_log_message ("reading display configuration from eeprom");
                display_read_config_from_eeprom (eeprom_version);

                debug_log_message ("reading timeserver data from eeprom");
                timeserver_read_data_from_eeprom ();
            }

            debug_log_message ("reading night timers from eeprom");
            night_read_data_from_eeprom (eeprom_version);

            if (eeprom_version >= EEPROM_VERSION_2_1)
            {
                debug_log_message ("reading temp configuration from eeprom");
                temp_read_config_from_eeprom (eeprom_version);
                rtc_read_config_from_eeprom (eeprom_version);

                debug_log_message ("reading LDR configuration from eeprom");
                ldr_read_config_from_eeprom (eeprom_version);
            }

            if (eeprom_version >= EEPROM_VERSION_2_2)
            {
                debug_log_message ("reading weather configuration from eeprom");
                weather_read_config_from_eeprom ();
            }

            debug_log_message ("reading update host/path configuration from eeprom");
            if (! read_main_parameters_from_eeprom ())
            {
                strcpy (gmain.update_host, DEFAULT_UPDATE_HOST);
                strcpy (gmain.update_path, DEFAULT_UPDATE_PATH);
                write_main_parameters_to_eeprom ();
            }

        }
    }
    else
    {
        log_message ("eeprom is offline");
    }

    ldr_init ();                                                            // initialize LDR (ADC)
    dcf77_init ();                                                          // initialize DCF77
    night_init ();                                                          // initialize night time routines
    temp_init ();                                                           // initialize DS18xx

    display_reset_led_states ();

    if (eeprom_is_up)
    {
        if (eeprom_version != EEPROM_VERSION)
        {
            log_printf ("updating EEPROM to version 0x%08x... ", EEPROM_VERSION);

            eeprom_version = EEPROM_VERSION;

            if (! write_version_to_eeprom ())
            {
                log_eeprom_error ("version");
            }

            if (! remote_ir_write_codes_to_eeprom ())
            {
                log_eeprom_error ("IR codes");
            }

            if (! display_write_config_to_eeprom ())
            {
                log_eeprom_error ("display configuration");
            }

            if (! timeserver_write_data_to_eeprom ())
            {
                log_eeprom_error ("timeserver configuration");
            }

            if (! night_write_data_to_eeprom ())
            {
                log_eeprom_error ("night time configuration");
            }

            if (! temp_write_config_to_eeprom ())
            {
                log_eeprom_error ("temperature configuration");
            }

            if (! rtc_write_config_to_eeprom ())
            {
                log_eeprom_error ("RTC configuration");
            }

            if (! ldr_write_config_to_eeprom ())
            {
                log_eeprom_error ("LDR configuration");
            }

            if (! weather_write_config_to_eeprom ())
            {
                log_eeprom_error ("weather configuration");
            }

            if (! write_main_parameters_to_eeprom ())
            {
                log_eeprom_error ("update main parameters configuration");
            }

            log_message ("done");
            eeprom_version = EEPROM_VERSION;
        }
    }

    ds3231_flag = 1;

    stop_time = uptime + 3;                                                 // wait 3 seconds for IR signal...
    display_set_status_led (1, 1, 1);                                       // show white status LED

    while (uptime < stop_time)
    {
        if (irmp_get_data (&irmp_data))                                     // got IR signal?
        {
            display_set_status_led (1, 0, 0);                               // yes, show red status LED
            delay_sec (1);                                                  // and wait 1 second
            (void) irmp_get_data (&irmp_data);                              // flush input of IRMP now
            display_set_status_led (0, 0, 0);                               // and switch status LED off

            debug_log_message ("calling IR learn function");
            if (remote_ir_learn ())                                         // learn IR commands
            {
                remote_ir_write_codes_to_eeprom ();                         // if successful, save them in EEPROM
            }
            break;                                                          // and break the loop
        }
    }

    display_set_status_led (0, 0, 0);                                       // switch off status LED

    esp8266_init ();

    if (ds18xx.is_up)
    {
        temp_read_temp_index ();
    }

    if (grtc.rtc_is_up)
    {
        rtc_get_temperature_index ();
    }

    while (1)
    {
        local_uptime = uptime;                                                          // cache volatile variable in local variable

        if (esp8266_is_up)                                                              // if user pressed user button, set ESP8266 to AP mode
        {
            if (local_uptime - ap_pressed > 5 && button_pressed ())
            {
                ap_pressed = local_uptime;                                              // debounce here
                log_message ("user button pressed: configuring esp8266 as access point");
                esp8266.is_online = 0;
                esp8266.ipaddress[0] = '\0';
                esp8266_accesspoint ("wordclock", "1234567890");
            }
            else if (local_uptime - wps_pressed > 5 && wpsbutton_pressed ())            // if user pressed wps button, send WPS command to ESP8266
            {
                wps_pressed = local_uptime;                                             // debounce here
                log_message ("wps button pressed: sending esp8266 wps command");
                esp8266.is_online = 0;
                esp8266.ipaddress[0] = '\0';
                esp8266_wps ();
            }
        }

        if (status_led_cnt)
        {
            status_led_cnt--;

            if (! status_led_cnt)
            {
                display_set_status_led (0, 0, 0);
            }
        }

        msg_rtc = esp8266_get_message ();

        if (msg_rtc == ESP8266_CMD)
        {
            char *              parameters;
            uint_fast8_t        cmd_code;
            uint_fast8_t        var_idx;

            parameters = esp8266.cmd;
            cmd_code = *parameters++;

            switch (cmd_code)
            {
                case 'R':                                                   // remote procedure call: Rii
                {
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (var_idx)
                    {
                        case LDR_MIN_VALUE_RPC_VAR:
                        {
                            debug_log_message ("rpc: set LDR value as minimum");
                            ldr_set_min_value ();
                            var_send_ldr_min_value ();
                            break;
                        }

                        case LDR_MAX_VALUE_RPC_VAR:
                        {
                            debug_log_message ("rpc: set LDR value as maximum");
                            ldr_set_max_value ();
                            var_send_ldr_max_value ();
                            break;
                        }

                        case LEARN_IR_RPC_VAR:
                        {
                            debug_log_message ("rpc: learn IR codes");

                            if (remote_ir_learn ())
                            {
                                remote_ir_write_codes_to_eeprom ();
                            }
                            break;
                        }

                        case GET_NET_TIME_RPC_VAR:
                        {
                            net_time_flag = 1;
                            debug_log_message ("rpc: start net time request");
                            break;
                        }

                        case DISPLAY_TEMPERATURE_RPC_VAR:
                        {
                            show_overlay_idx = OVERLAY_TEMPERATURE_IDX;
                            debug_log_message ("rpc: show temperature");
                            break;
                        }

                        case TEST_DISPLAY_RPC_VAR:
                        {
                            debug_log_message ("rpc: start display test");
                            display_test ();
                            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            break;
                        }

                        case GET_WEATHER_RPC_VAR:
                        {
                            weather_query ();
                            debug_log_message ("rpc: get weather");
                            break;
                        }
                    }
                    break;
                }

                case 'N':                                                   // numeric variable: Niillhh
                {
                    uint_fast8_t    lo;
                    uint_fast8_t    hi;
                    uint_fast16_t   val;

                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    lo      = htoi (parameters, 2);
                    parameters += 2;
                    hi      = htoi (parameters, 2);
                    parameters += 2;

                    val     = (hi << 8) | lo;

                    switch (var_idx)
                    {
                        case EEPROM_IS_UP_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set eeprom_is_up = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case HARDWARE_CONFIGURATION_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set hardware_configuration = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case RTC_IS_UP_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set rtc_is_up = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case DISPLAY_POWER_NUM_VAR:
                        {
                            if (display.power_is_on != val)
                            {
                                display_clock_flag = set_power (val);
                                debug_log_printf ("cmd: set power_is_on = %d\r\n", val);
                            }
                            break;
                        }

                        case DISPLAY_AMBILIGHT_POWER_NUM_VAR:
                        {
                            if (display.ambilight_power_is_on != val)
                            {
                                display_set_ambilight_power (val);
                                debug_log_printf ("cmd: set power_is_on = %d\r\n", val);
                            }
                            break;
                        }

                        case DISPLAY_MODE_NUM_VAR:
                        {
                            if (display.display_mode != val)
                            {
                                display_set_display_mode (val, FALSE);
                                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                                debug_log_printf ("cmd: set display_mode = %d\r\n", val);
                            }
                            break;
                        }

                        case MAX_DISPLAY_MODES_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set TABLE_MODDES_COUNT = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case DISPLAY_BRIGHTNESS_NUM_VAR:
                        {
                            if (display.automatic_brightness)
                            {
                                last_ldr_value = 0xFF;
                                display_set_automatic_brightness (0, FALSE);
                            }
                            display_set_display_brightness (val, FALSE, TRUE);
                            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                            debug_log_printf ("cmd: set display_brightness = %d, disable autmomatic brightness control per LDR\r\n", val);
                            break;
                        }

                        case DISPLAY_FLAGS_NUM_VAR:
                        {
                            display_set_display_flags (val);
                            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            debug_log_printf ("cmd: set display_flags = 0x%02x\r\n", val);
                            break;
                        }

                        case DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR:
                        {
                            last_ldr_value = 0xFF;

                            if (val)
                            {
                                display_set_automatic_brightness (1, FALSE);
                            }
                            else
                            {
                                display_set_automatic_brightness (0, FALSE);
                            }

                            debug_log_printf ("cmd: set automatic_brightness = %d\r\n", val);
                            break;
                        }

                        case DISPLAY_TEMPERATURE_INTERVAL_NUM_VAR:
                        {
                            display_set_overlay_interval (OVERLAY_TEMPERATURE_IDX, val);
                            debug_log_printf ("cmd: set temperature overlay interval = %d\r\n", val);
                            break;
                        }

                        case DISPLAY_HEART_INTERVAL_NUM_VAR:
                        {
                            display_set_overlay_interval (OVERLAY_HEART_IDX, val);
                            debug_log_printf ("cmd: set heart overlay interval = %d\r\n", val);
                            break;
                        }

                        case DISPLAY_XMAS_TREE_INTERVAL_NUM_VAR:
                        {
                            display_set_overlay_interval (OVERLAY_XMAS_TREE_IDX, val);
                            debug_log_printf ("cmd: set xmas tree overlay interval = %d\r\n", val);
                            break;
                        }

                        case DISPLAY_DATE_INTERVAL_NUM_VAR:
                        {
                            display_set_overlay_interval (OVERLAY_DATE_IDX, val);
                            debug_log_printf ("cmd: set date overlay interval = %d\r\n", val);
                            break;
                        }

                        case DISPLAY_FIREWORKS_INTERVAL_NUM_VAR:
                        {
                            display_set_overlay_interval (OVERLAY_FIREWORKS_IDX, val);
                            debug_log_printf ("cmd: set fireworks overlay interval = %d\r\n", val);
                            break;
                        }

                        case ANIMATION_MODE_NUM_VAR:
                        {
                            if (display.animation_mode != val)
                            {
                                display_set_animation_mode (val, FALSE);
                                animation_flag = 0;
                                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            }
                            debug_log_printf ("cmd: set animation_mode = %d\r\n", val);
                            break;
                        }

                        case AMBILIGHT_MODE_NUM_VAR:
                        {
                            if (display.ambilight_mode != val)
                            {
                                display_set_ambilight_mode (val, FALSE);
                                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            }
                            debug_log_printf ("cmd: set ambilight_mode = %d\r\n", val);
                            break;
                        }

                        case AMBILIGHT_LEDS_NUM_VAR:
                        {
                            display_set_number_of_ambilight_leds (val);
                            debug_log_printf ("cmd: set number of ambilight leds = %d\r\n", val);
                            break;
                        }

                        case AMBILIGHT_OFFSET_NUM_VAR:
                        {
                            display_set_ambilight_led_offset (val);
                            debug_log_printf ("cmd: set ambilight led offset = %d\r\n", val);
                            break;
                        }

                        case AMBILIGHT_BRIGHTNESS_NUM_VAR:
                        {
                            display_set_ambilight_brightness (val, FALSE, TRUE);
                            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                            debug_log_printf ("cmd: set ambilight brightness = %d\r\n", val);
                            break;
                        }

                        case COLOR_ANIMATION_MODE_NUM_VAR:
                        {
                            if (display.color_animation_mode != val)
                            {
                                display_set_color_animation_mode (val, FALSE);
                            }
                            debug_log_printf ("cmd: set color animation_mode = %d\r\n", display.color_animation_mode);
                            break;
                        }

                        case LDR_RAW_VALUE_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set ldr_raw_value = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case LDR_MIN_VALUE_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set ldr_min_value = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case LDR_MAX_VALUE_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set ldr_max_value = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case TIMEZONE_NUM_VAR:
                        {
                            int_fast16_t tz = val & 0xFF;

                            if (val & 0x100)
                            {
                                tz = -tz;
                            }

                            debug_log_printf ("cmd: set timezone = %d\r\n", tz);
                            timeserver_set_timezone (tz);
                            break;
                        }

                        case DS18XX_IS_UP_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set ds18xx_is_up = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case RTC_TEMP_INDEX_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set rtc_temp_index = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case RTC_TEMP_CORRECTION_NUM_VAR:
                        {
                            rtc_set_temp_correction (val);
                            read_rtc_temperature_flag = 1;
                            debug_log_printf ("cmd: set rtc_temp_correction = %d\r\n", val);
                            break;
                        }

                        case DS18XX_TEMP_INDEX_NUM_VAR:
                        {
                            debug_log_printf ("cmd: set ds18xx_temp_index = %d, but it's readonly\r\n", val);
                            break;
                        }

                        case DS18XX_TEMP_CORRECTION_NUM_VAR:
                        {
                            temp_set_temp_correction (val);
                            measure_temperature_flag = 1;                                           // measure & read
                            read_temperature_flag = 1;
                            debug_log_printf ("cmd: set ds18xx_temp_correction = %d\r\n", val);
                            break;
                        }

                        case TICKER_DECELRATION_NUM_VAR:
                        {
                            display_set_ticker_deceleration (val);
                            debug_log_printf ("cmd: set ticker_deceleration = %d\r\n", val);
                            break;
                        }
                    }
                    break;
                }

                case 'n':                                                   // numeric array: Niinnbb
                {
                    uint_fast8_t    n;
                    uint_fast8_t    val;

                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    n = htoi (parameters, 2);
                    parameters += 2;

                    val = htoi (parameters, 2);

                    switch (var_idx)
                    {
                        case DISPLAY_DIMMED_COLORS:
                        {
                            if (n < sizeof (display.dimmed_colors))
                            {
                                if (val <= MAX_BRIGHTNESS)
                                {
                                    display_set_dimmed_color (n, val);
                                    debug_log_printf ("cmd: set dimmed_colors[%d] = %d\r\n", n, val);
                                }
                            }
                            else
                            {
                                debug_log_printf ("cmd: set dimmed_colors[%d] = %d, but n is too high\r\n", n, val);
                            }
                            break;
                        }
                    }
                    break;
                }

                case 'S':                                               // string variable: Siissssssss...
                {
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (var_idx)
                    {
                        case TICKER_TEXT_STR_VAR:
                        {
                            display_set_ticker ((unsigned char *) parameters, 1);
                            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            debug_log_printf ("cmd: print ticker: '%s'\r\n", parameters);
                            break;
                        }

                        case VERSION_STR_VAR:
                        {
                            debug_log_printf ("cmd: set VERSION = '%s', but it's readonly\r\n", parameters);
                            break;
                        }

                        case EEPROM_VERSION_STR_VAR:
                        {
                            debug_log_printf ("cmd: set eeprom_version = '%s', but it's readonly\r\n", parameters);
                            break;
                        }

                        case ESP8266_VERSION_STR_VAR:
                        {
                            debug_log_printf ("cmd: set esp8266_version = '%s', but it's readonly\r\n", parameters);
                            break;
                        }

                        case TIMESERVER_STR_VAR:
                        {
                            timeserver_set_timeserver (parameters);
                            debug_log_printf ("cmd: set timeserver = '%s'\r\n", parameters);
                            break;
                        }

                        case WEATHER_APPID_STR_VAR:
                        {
                            weather_set_appid (parameters);
                            debug_log_printf ("cmd: set weather appid = '%s'\r\n", parameters);
                            break;
                        }

                        case WEATHER_CITY_STR_VAR:
                        {
                            weather_set_city (parameters);
                            debug_log_printf ("cmd: set weather city = '%s'\r\n", parameters);
                            break;
                        }

                        case WEATHER_LON_STR_VAR:
                        {
                            weather_set_lon (parameters);
                            debug_log_printf ("cmd: set weather lon = '%s'\r\n", parameters);
                            break;
                        }

                        case WEATHER_LAT_STR_VAR:
                        {
                            weather_set_lat (parameters);
                            debug_log_printf ("cmd: set weather lat = '%s'\r\n", parameters);
                            break;
                        }

                        case UPDATE_HOST_VAR:
                        {
                            strncpy (gmain.update_host, parameters, EEPROM_MAX_HOSTNAME_LEN - 1);
                            write_update_host_to_eeprom ();
                            debug_log_printf ("cmd: set update host = '%s'\r\n", parameters);
                            break;
                        }

                        case UPDATE_PATH_VAR:
                        {
                            strncpy (gmain.update_path, parameters, EEPROM_MAX_UPDATE_PATH_LEN - 1);
                            write_update_path_to_eeprom ();
                            debug_log_printf ("cmd: set update path = '%s'\r\n", parameters);
                            break;
                        }
                    }
                    break;
                }

                case 'T':
                {
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (var_idx)
                    {
                        case CURRENT_TM_VAR:
                        {
                            // struct tm tmtemp;
                            gmain.tm.tm_year = 1000 * (parameters[0]  - '0') +
                                                100 * (parameters[1]  - '0') +
                                                 10 * (parameters[2]  - '0') +
                                                  1 * (parameters[3]  - '0');
                            gmain.tm.tm_mon  =   10 * (parameters[4]  - '0') +
                                                  1 * (parameters[5]  - '0');
                            gmain.tm.tm_mday =   10 * (parameters[6]  - '0') +
                                                  1 * (parameters[7]  - '0');
                            gmain.tm.tm_hour =   10 * (parameters[8]  - '0') +
                                                  1 * (parameters[9]  - '0');
                            gmain.tm.tm_min  =   10 * (parameters[10] - '0') +
                                                  1 * (parameters[11] - '0');
                            gmain.tm.tm_sec =    10 * (parameters[12] - '0') +
                                                  1 * (parameters[13] - '0');

                            gmain.tm.tm_wday = dayofweek (gmain.tm.tm_mday, gmain.tm.tm_mon + 1, gmain.tm.tm_year + 1900);

                            if (grtc.rtc_is_up)
                            {
                                rtc_set_date_time (&(gmain.tm));
                            }

                            if (hour != (uint_fast8_t) gmain.tm.tm_hour || minute != (uint_fast8_t) gmain.tm.tm_min)
                            {
                                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            }

                            wday   = gmain.tm.tm_wday;
                            hour   = gmain.tm.tm_hour;
                            minute = gmain.tm.tm_min;
                            second = gmain.tm.tm_sec;

                            debug_log_printf ("cmd: set time = %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                                                wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                                                gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
                            break;
                        }
                    }
                    break;
                }

                case 'D':                                               // display Dxii...
                {
                    cmd_code = *parameters++;
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (cmd_code)
                    {
                        case 'N':                                       // DN: Display mode Name
                        {
                            debug_log_printf ("cmd: set display_mode_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
                            break;
                        }

                        case 'C':                                       // DC: Display Color
                        {
                            DSP_COLORS rgb;

                            rgb.red = htoi (parameters, 2);
                            parameters += 2;
                            rgb.green = htoi (parameters, 2);
                            parameters += 2;
                            rgb.blue = htoi (parameters, 2);
                            parameters += 2;
#if DSP_USE_SK6812_RGBW == 1
                            rgb.white = htoi (parameters, 2);
                            parameters += 2;
#endif

                            if (var_idx == DISPLAY_DSP_COLOR_VAR)
                            {
                                display_set_display_colors (&(rgb));
#if DSP_USE_SK6812_RGBW == 1
                                debug_log_printf ("cmd: set display_colors = %d %d %d %d\r\n", rgb.red, rgb.green, rgb.blue, rgb.white);
#else
                                debug_log_printf ("cmd: set display_colors = %d %d %d\r\n", rgb.red, rgb.green, rgb.blue);
#endif
                            }
                            else if (var_idx == AMBILIGHT_DSP_COLOR_VAR)
                            {
                                display_set_ambilight_colors (&(rgb));
#if DSP_USE_SK6812_RGBW == 1
                                debug_log_printf ("cmd: set ambilight_colors = %d %d %d %d\r\n", rgb.red, rgb.green, rgb.blue, rgb.white);
#else
                                debug_log_printf ("cmd: set ambilight_colors = %d %d %d\r\n", rgb.red, rgb.green, rgb.blue);
#endif
                            }
                            else
                            {
                                debug_log_printf ("DC: invalid var_idx: %d\r\n", var_idx);
                            }
                            break;
                        }
                    }
                    break;
                }

                case 'A':                                                               // animation
                {
                    cmd_code = *parameters++;
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (cmd_code)
                    {
                        case 'N':                                                       // AN: Animation mode Name
                        {
                            debug_log_printf ("cmd: set animation_mode_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
                            break;
                        }

                        case 'D':                                                       // AD: Animation Deceleration
                        {
                            uint_fast8_t deceleration = htoi (parameters, 2);
                            parameters += 2;
                            display_set_animation_deceleration (var_idx, deceleration);
                            debug_log_printf ("cmd: set animation_deceleration[%d] = %d\r\n", var_idx, deceleration);
                            break;
                        }

                        case 'E':                                                       // AE: Animation default deceleration
                        {
                            debug_log_message ("cmd: set animation_default_deceleration, but it's readonly\r\n");
                            break;
                        }

                        case 'F':                                                       // AF: Animation Flags
                        {
                            uint_fast8_t flags = htoi (parameters, 2);
                            parameters += 2;
                            display_set_animation_flags (var_idx, flags);
                            debug_log_printf ("cmd: set animation_flags[%d] = 0x%02x\r\n", var_idx, flags);
                            break;
                        }
                    }
                    break;
                }

                case 'C':                                                               // Color animation
                {
                    cmd_code = *parameters++;
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (cmd_code)
                    {
                        case 'N':                                                       // CNiis: Color animation Name
                        {
                            debug_log_printf ("cmd: set color_animation_name[%d] = '%s', but it's readonly\r\n", var_idx, parameters);
                            break;
                        }

                        case 'D':                                                       // CDiid: Color animation Deceleration
                        {
                            uint_fast8_t deceleration = htoi (parameters, 2);
                            parameters += 2;
                            display_set_color_animation_deceleration (var_idx, deceleration);
                            debug_log_printf ("cmd: set color_animation_deceleration[%d] = %d\r\n", var_idx, deceleration);
                            break;
                        }

                        case 'E':                                                       // CEiid: Color animation default deceleration
                        {
                            debug_log_message ("cmd: set color_animation_default_deceleration, but it's readonly\r\n");
                            break;
                        }

                        case 'F':                                                       // CFiif: Color animation Flags
                        {
                            debug_log_message ("cmd: set color_animation_flags, but it's readonly");
                            break;
                        }
                    }
                    break;
                }

                case 'M':                                                               // Ambilight mode
                {
                    cmd_code = *parameters++;
                    var_idx = htoi (parameters, 2);
                    parameters += 2;

                    switch (cmd_code)
                    {
                        case 'N':                                                       // MN: Ambilight mode Name
                        {
                            debug_log_printf ("cmd: set ambilight_mode_name[%d] = '%s', but it's readonly", var_idx, parameters);
                            break;
                        }

                        case 'D':                                                       // MD: Ambilight mode Deceleration
                        {
                            uint_fast8_t deceleration = htoi (parameters, 2);
                            parameters += 2;
                            display_set_ambilight_mode_deceleration (var_idx, deceleration);
                            debug_log_printf ("cmd: set ambilight_mode_deceleration[%d] = %d\r\n", var_idx, deceleration);
                            break;
                        }

                        case 'E':                                                       // ME: Ambilight mode default deceleration
                        {
                            debug_log_message ("cmd: set ambilight_mode_default_deceleration, but it's readonly\r\n");
                            break;
                        }

                        case 'F':                                                       // MF: Ambilight mode Flags
                        {
                            uint_fast8_t flags = htoi (parameters, 2);
                            parameters += 2;
                            display_set_ambilight_mode_flags (var_idx, flags);
                            debug_log_printf ("cmd: set ambilight_mode_flags[%d] = %d\r\n", var_idx, flags);
                            break;
                        }
                    }
                    break;
                }

                case 't':                                                               // tiimmff night table minutes + flags
                {
                    uint_fast16_t minutes;
                    uint_fast8_t flags;

                    var_idx = htoi (parameters, 2);
                    parameters += 2;
                    minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
                    parameters += 4;
                    flags = htoi (parameters, 2);
                    parameters += 2;

                    night_time[var_idx].minutes = minutes;
                    night_time[var_idx].flags = flags;
                    night_write_data_to_eeprom ();
                    debug_log_printf ("cmd: set night_time[%d]: minutes = %d, flags = 0x%02x\r\n", var_idx, minutes, flags);
                    break;
                }

                case 'a':                                                               // aiimmff night table minutes + flags
                {
                    uint_fast16_t minutes;
                    uint_fast8_t flags;

                    var_idx = htoi (parameters, 2);
                    parameters += 2;
                    minutes = htoi (parameters, 2) + (htoi (parameters + 2, 2) << 8);
                    parameters += 4;
                    flags = htoi (parameters, 2);
                    parameters += 2;

                    ambilight_night_time[var_idx].minutes = minutes;
                    ambilight_night_time[var_idx].flags = flags;
                    night_write_data_to_eeprom ();
                    debug_log_printf ("cmd: set ambilight night_time[%d]: minutes = %d, flags = 0x%02x\r\n", var_idx, minutes, flags);
                    break;
                }

                case 'G':                                                               // Gx Games
                {
                    switch (*parameters++)
                    {
                        case 'T':
                        {
                            if (*parameters == 's')
                            {
                                tetris ();                                              // GTs: Start Tetris
                                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                            }
                        }
                    }
                    break;
                }
            }
        }
        else if (msg_rtc == ESP8266_TIME)
        {
            char *          endptr;
            uint32_t        seconds_since_1900;

            seconds_since_1900  = strtoul (esp8266.time, &endptr, 10);
            timeserver_convert_time (&gmain.tm, seconds_since_1900);

            if (grtc.rtc_is_up)
            {
                rtc_set_date_time (&gmain.tm);
            }

            if (hour != (uint_fast8_t) gmain.tm.tm_hour || minute != (uint_fast8_t) gmain.tm.tm_min)
            {
                var_send_tm ();
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }

            wday   = gmain.tm.tm_wday;
            hour   = gmain.tm.tm_hour;
            minute = gmain.tm.tm_min;
            second = gmain.tm.tm_sec;

            debug_log_printf ("cmd: set time to %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                                wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                                gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
            net_time_countdown = 3600 + 180 + 30 - second;                      // calc next timeserver call 1h3m later at hh:mm:30
        }
        else if (msg_rtc == ESP8266_IPADDRESS)                                  // ESP8266 got new ip address
        {
            unsigned char buf[32];

            var_send_all_variables ();
            sprintf ((char *) buf, "IP %s", esp8266.ipaddress);
            log_printf ("info: ip address = %s\r\n", esp8266.ipaddress);

            display_set_ticker (buf, 1);
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
        }
        else if (msg_rtc == ESP8266_ACCESSPOINT)
        {
            debug_log_message ("info: got ESP8266_ACCESSPOINT");
        }
        else if (msg_rtc == ESP8266_MODE)
        {
            if (esp8266.mode == ESP8266_AP_MODE)
            {
                debug_log_message ("info: got ESP8266_AP_MODE");
            }
            else if (esp8266.mode == ESP8266_CLIENT_MODE)
            {
                debug_log_message ("info: got ESP8266_CLIENT_MODE");
            }
            else
            {
                debug_log_message ("info: got invalid ESP8266 mode");
            }
        }
        else if (msg_rtc == ESP8266_FIRMWARE)
        {
            debug_log_message ("info: got ESP8266_FIRMWARE");
        }
        else if (msg_rtc == ESP8266_WEATHER)
        {
            log_printf ("info: weather = %s\r\n", esp8266.weather);
            display_set_ticker ((unsigned char *) esp8266.weather, 1);
            display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
        }

        if (display.animation_stop_flag && display.automatic_brightness && ldr_poll_brightness ())
        {
            ldr_raw_value = ldr.ldr_raw_value;

            if (ldr_raw_value + 16 < last_ldr_raw_value || ldr_raw_value > last_ldr_raw_value + 16)         // difference greater than 16
            {
                debug_log_printf ("ldr: old raw brightnes: %d new raw brightness: %d\r\n", last_ldr_raw_value, ldr_raw_value);
                last_ldr_raw_value = ldr_raw_value;
                var_send_ldr_raw_value ();
            }

            ldr_value = ldr.ldr_value;                                                                      // ldr_value is 0...31

            if (ldr_value + 1 < last_ldr_value || ldr_value > last_ldr_value + 1)                           // difference greater than 2
            {
                last_ldr_value = ldr_value;                                                                 // store value 0...31
                ldr_value /= 2;                                                                             // set ldr_value to 0...15
                debug_log_printf ("ldr: old brightnes: %d new brightness: %d\r\n", last_ldr_value / 2, ldr_value);
                display_set_display_brightness (ldr_value, FALSE, FALSE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
            }
        }

        if (!esp8266_is_up)                                                 // esp8266 up yet?
        {
            if (esp8266.is_up)
            {
                esp8266_is_up = 1;
                log_message ("esp8266 now up");
            }
        }
        else
        {                                                                   // esp8266 is up...
            if (! esp8266_is_online)                                        // but not online yet...
            {
                if (esp8266.is_online)                                      // now online?
                {
                    esp8266_is_online = 1;
                    log_message ("esp8266 now online");
                    net_time_flag = 1;
                }
            }
        }

        if (dcf77_time(&gmain.tm))
        {
            display_set_status_led (1, 1, 0);                               // got DCF77 time, light yellow = green + red LED

            status_led_cnt = 50;

            if (grtc.rtc_is_up)
            {
                rtc_set_date_time (&gmain.tm);
            }

            if (hour != (uint_fast8_t) gmain.tm.tm_hour || minute != (uint_fast8_t) gmain.tm.tm_min)
            {
                var_send_tm ();
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
            }

            wday        = gmain.tm.tm_wday;
            hour        = gmain.tm.tm_hour;
            minute      = gmain.tm.tm_min;
            second      = gmain.tm.tm_sec;

            log_printf ("dcf77: %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                         wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                         gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
        }

        if (ds3231_flag)
        {
            if (grtc.rtc_is_up && rtc_get_date_time (&gmain.tm))
            {
                if (hour != (uint_fast8_t) gmain.tm.tm_hour || minute != (uint_fast8_t) gmain.tm.tm_min)
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                }

                wday        = gmain.tm.tm_wday;
                hour        = gmain.tm.tm_hour;
                minute      = gmain.tm.tm_min;
                second      = gmain.tm.tm_sec;

                log_printf ("read rtc: %s %4d-%02d-%02d %02d:%02d:%02d\r\n",
                            wdays_en[gmain.tm.tm_wday], gmain.tm.tm_year + 1900, gmain.tm.tm_mon + 1, gmain.tm.tm_mday,
                            gmain.tm.tm_hour, gmain.tm.tm_min, gmain.tm.tm_sec);
            }

            ds3231_flag = 0;
        }

        if (display.animation_stop_flag && display.automatic_brightness && ldr_conversion_flag)
        {
            ldr_start_conversion ();
            ldr_conversion_flag = 0;
        }

        if (net_time_flag)
        {
            if (esp8266.is_online)
            {
                display_set_status_led (0, 0, 1);                           // light blue status LED
                status_led_cnt = STATUS_LED_FLASH_TIME;
                timeserver_start_timeserver_request ();                     // start a timeserver request, answer follows...
            }

            net_time_flag = 0;
            net_time_countdown = 3800;                                      // next net time after 3800 sec
        }

        if (show_time_flag)                                                 // set every full minute
        {
            show_time_flag = 0;

            gmain.tm.tm_hour = hour;
            gmain.tm.tm_min  = minute;
            gmain.tm.tm_sec  = second;
            gmain.tm.tm_wday = wday;

            var_send_tm ();

            // display_clock_flag = 0;                                      // don't reset flag

            if (night_check_night_times (0, display.power_is_on, wday, hour * 60 + minute))
            {
                display_clock_flag = toggle_power (TRUE);
                log_printf ("Found Timer: %s at %02d:%02d\r\n", display.power_is_on ? "on" : "off", hour, minute);
            }

            if (night_check_night_times (1, display.ambilight_power_is_on, wday, hour * 60 + minute))
            {
                display_set_ambilight_power (! display.ambilight_power_is_on);
                log_printf ("Found Timer: ambilight %s at %02d:%02d\r\n", display.ambilight_power_is_on ? "on" : "off", hour, minute);
            }

            if (display_clock_flag == 0)                                // no night time found
            {
#if WCLOCK24H == 1
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
#else
                if (minute % 5)
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_MINUTES; // only update minute LEDs
                }
                else
                {
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                }
#endif
            }
        }

        if (half_minute_flag)
        {
            uint_fast8_t overlay_idx;
            uint_fast8_t overlay_max_interval;

            half_minute_flag        = 0;
            overlay_max_interval    = 0;

            for (overlay_idx = 0; overlay_idx < MAX_OVERLAYS; overlay_idx++)
            {
                if (display.overlays.intervals[overlay_idx] > overlay_max_interval &&
                    (minute % display.overlays.intervals[overlay_idx]) == 0)
                {
                    overlay_max_interval = display.overlays.intervals[overlay_idx];
                    show_overlay_idx = overlay_idx;
                }
            }
        }

        if (measure_temperature_flag)
        {
            measure_temperature_flag = 0;

            if (ds18xx.is_up)
            {
                temp_start_conversion (read_temperature_flag);          // start conversion, wait if read_temperature_flag is also set
            }
        }

        if (read_temperature_flag)
        {
            read_temperature_flag = 0;

            if (ds18xx.is_up)
            {
                ds18xx_temperature_index = temp_read_temp_index ();
                log_printf ("DS18xxx temperature: %d%s\r\n", ds18xx_temperature_index / 2, (ds18xx_temperature_index % 2) ? ".5" : "");
                var_send_ds18xx_temp_index ();
            }
            else
            {
                ds18xx_temperature_index = 0xFF;
            }
        }

        if (read_rtc_temperature_flag)
        {
            read_rtc_temperature_flag = 0;

            if (grtc.rtc_is_up)
            {
                rtc_temperature_index = rtc_get_temperature_index ();
                log_printf ("RTC temperature: %d%s\r\n", rtc_temperature_index / 2, (rtc_temperature_index % 2) ? ".5" : "");
                var_send_rtc_temp_index ();
            }
            else
            {
                rtc_temperature_index = 0xFF;
            }
        }

        if (show_overlay_idx != 0xFF)                                                       // icon to show?
        {
            if (display.overlays.icon_map[show_overlay_idx] < MAX_ICONS)                    // yes, it's a real icon
            {
                log_printf ("icon: %d\r\n", display.overlays.icon_map[show_overlay_idx]);
                display.display_icon = display.overlays.icon_map[show_overlay_idx];         // indicate displaying icon
                show_icon_stop_time = local_uptime + 5;                                     // stop in 5 seconds
            }
            else if (show_overlay_idx == OVERLAY_TEMPERATURE_IDX)
            {
                if (ds18xx_temperature_index != 0xFF)
                {
                    display_temperature (ds18xx_temperature_index);
                    show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
                }
                else if (rtc_temperature_index != 0xFF)
                {
                    display_temperature (rtc_temperature_index);
                    show_icon_stop_time = 0xFFFFFFFF;                                       // flag: wait for end of animation
                }
            }
            else if (show_overlay_idx == OVERLAY_DATE_IDX)
            {
                char datebuf[11];
                log_message ("show date as ticker");
                sprintf (datebuf, "%02d.%02d.%d", gmain.tm.tm_mday, gmain.tm.tm_mon + 1, gmain.tm.tm_year + 1900);
                display_set_ticker ((unsigned char *) datebuf, 1);                          // display date and wait
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;                         // update display after ticker
            }

            show_overlay_idx = 0xFF;
        }
        else if (show_icon_stop_time > 0)
        {
            if (show_icon_stop_time == 0xFFFFFFFF)                                      // waiting for end of animation?
            {
                 if (! display.animation_start_flag && display.animation_stop_flag)     // animation running?
                 {                                                                      // no, ...
                    show_icon_stop_time = local_uptime + 5;                             // show temperature 5 seconds long
                 }
            }
            else if (local_uptime >= show_icon_stop_time)                               // 5 seconds gone?
            {                                                                           // yes, ...
                show_icon_stop_time = 0;                                                // reset flag
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;                     // and force update of display
            }
        }

        if (display_clock_flag)                                                         // refresh display (time/mode changed)
        {
            debug_log_message ("update display");

#if WCLOCK24H == 1
            if (display.display_mode == MODES_COUNT - 1)                                // temperature
            {
                uint_fast8_t temperature_index;

                if (ds18xx.is_up)
                {
                    temperature_index = temp_read_temp_index ();
                    log_printf ("got temperature from DS18xxx: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                }
                else if (grtc.rtc_is_up)
                {
                    temperature_index = rtc_get_temperature_index ();
                    log_printf ("got temperature from RTC: %d%s\r\n", temperature_index / 2, (temperature_index % 2) ? ".5" : "");
                }
                else
                {
                    temperature_index = 0x00;
                    debug_log_message ("no temperature available");
                }

                display_clock (0, temperature_index - 20, display_clock_flag);          // show temperature
            }
            else
            {
                display_clock (hour, minute, display_clock_flag);                       // show new time
            }
#else
            display_clock (hour, minute, display_clock_flag);                           // show new time
#endif
            display_clock_flag = DISPLAY_CLOCK_FLAG_NONE;
        }

        if (animation_flag)
        {
            animation_flag = 0;
            display_animation ();
        }

        if (seconds_flag)
        {
            seconds_flag = 0;
            display_seconds (second);
        }

        cmd = remote_ir_get_cmd ();                                                     // get IR command

        if (cmd != REMOTE_IR_CMD_INVALID)                                               // got IR command, light green LED
        {
            display_set_status_led (1, 0, 0);
            status_led_cnt = STATUS_LED_FLASH_TIME;
        }

        switch (cmd)
        {
            case REMOTE_IR_CMD_POWER:
            {
                display_clock_flag = toggle_power (TRUE);
                debug_log_message ("IRMP: POWER key");
                break;
            }

            case REMOTE_IR_CMD_OK:
            {
                debug_log_message ("IRMP: OK key, not used anymore");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_DISPLAY_MODE:                                  // decrement display mode
            {
                display_decrement_display_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: decrement display mode");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_DISPLAY_MODE:                                  // increment display mode
            {
                display_increment_display_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: increment display mode");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_ANIMATION_MODE:                                // decrement display mode
            {
                display_decrement_animation_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: decrement animation mode");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_ANIMATION_MODE:                                // increment display mode
            {
                display_increment_animation_mode (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                debug_log_message ("IRMP: increment animation mode");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_HOUR:                                          // decrement hour
            {
                if (hour > 0)
                {
                    hour--;
                }
                else
                {
                    hour = 23;
                }

                second              = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: decrement hour");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_HOUR:                                          // increment hour
            {
                if (hour < 23)
                {
                     hour++;
                }
                else
                {
                    hour =  0;
                }

                second              = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: increment hour");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_MINUTE:                                        // decrement minute
            {
                if (minute > 0)
                {
                    minute--;
                }
                else
                {
                    minute = 59;
                }

                second              = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: decrement minute");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_MINUTE:                                        // increment minute
            {
                if (minute < 59)
                {
                    minute++;
                }
                else
                {
                    minute = 0;
                }

                second              = 0;
                display_clock_flag  = DISPLAY_CLOCK_FLAG_UPDATE_ALL;
                time_changed        = 1;
                debug_log_message ("IRMP: increment minute");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_RED:                                // decrement red brightness
            {
                display_decrement_display_color_red (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement red brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_RED:                                // increment red brightness
            {
                display_increment_display_color_red (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment red brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_GREEN:                              // decrement green brightness
            {
                display_decrement_display_color_green (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement green brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_GREEN:                              // increment green brightness
            {
                display_increment_display_color_green (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment green brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS_BLUE:                               // decrement blue brightness
            {
                display_decrement_display_color_blue (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement blue brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS_BLUE:                               // increment blue brightness
            {
                display_increment_display_color_blue (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment blue brightness");
                break;
            }

            case REMOTE_IR_CMD_AUTO_BRIGHTNESS_CONTROL:                                 // toggle auto brightness
            {
                uint_fast8_t new_auto = display.automatic_brightness;
                new_auto = ! new_auto;
                last_ldr_value = 0xFF;
                display_set_automatic_brightness (new_auto, TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: toggle auto brightness");
                break;
            }

            case REMOTE_IR_CMD_DECREMENT_BRIGHTNESS:                                    // decrement brightness
            {
                if (display.automatic_brightness)
                {
                    last_ldr_value = 0xFF;
                    display_set_automatic_brightness (0, TRUE);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                }

                display_decrement_display_brightness (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: decrement brightness");
                break;
            }

            case REMOTE_IR_CMD_INCREMENT_BRIGHTNESS:                                    // increment brightness
            {
                if (display.automatic_brightness)
                {
                    last_ldr_value = 0xFF;
                    display_set_automatic_brightness (0, TRUE);
                    display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                }

                display_increment_display_brightness (TRUE);
                display_clock_flag = DISPLAY_CLOCK_FLAG_UPDATE_NO_ANIMATION;
                debug_log_message ("IRMP: increment brightness");
                break;
            }

            case REMOTE_IR_CMD_GET_TEMPERATURE:                                         // get temperature
            {
                show_overlay_idx = OVERLAY_TEMPERATURE_IDX;
                debug_log_message ("IRMP: get temperature");
                break;
            }

            default:
            {
                break;
            }
        }

        if (time_changed)
        {
            if (grtc.rtc_is_up)
            {
                gmain.tm.tm_hour = hour;
                gmain.tm.tm_min  = minute;
                gmain.tm.tm_sec  = second;
                rtc_set_date_time (&gmain.tm);
            }

            time_changed = 0;
        }
    }

    return 0;
}
