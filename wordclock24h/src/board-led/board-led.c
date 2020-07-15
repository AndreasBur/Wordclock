/*-------------------------------------------------------------------------------------------------------------------------------------------
 * board-led.c - LED routines
 *
 * Copyright (c) 2014-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */

#if defined (STM32F10X)
#include "stm32f10x.h"
#elif defined (STM32F4XX)
#include "stm32f4xx.h"
#endif
#include "board-led.h"

#if defined (STM32F407VG)                                       // STM32F4 Discovery Board PD12
#define BOARD_LED_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define BOARD_LED_PERIPH            RCC_AHB1Periph_GPIOD
#define BOARD_LED_PORT              GPIOD
#define BOARD_LED_LED               GPIO_Pin_12
#define BOARD_LED_ON                SET
#define BOARD_LED_OFF               RESET

#elif defined (STM32F401RE) || defined (STM32F411RE)            // STM32F401/STM32F411 Nucleo Board PA5
#define BOARD_LED_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define BOARD_LED_PERIPH            RCC_AHB1Periph_GPIOA
#define BOARD_LED_PORT              GPIOA
#define BOARD_LED_LED               GPIO_Pin_5
#define BOARD_LED_ON                SET
#define BOARD_LED_OFF               RESET

#elif defined (STM32F103)
#define BOARD_LED_PERIPH_CLOCK_CMD  RCC_APB2PeriphClockCmd
#define BOARD_LED_PERIPH            RCC_APB2Periph_GPIOC
#define BOARD_LED_PORT              GPIOC
#define BOARD_LED_LED               GPIO_Pin_13
#define BOARD_LED_ON                RESET
#define BOARD_LED_OFF               SET

#else
#error STM32 unknown
#endif

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize LED port
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
board_led_init (void)
{
    GPIO_InitTypeDef gpio;

    GPIO_StructInit (&gpio);
    BOARD_LED_PERIPH_CLOCK_CMD (BOARD_LED_PERIPH, ENABLE);     // enable clock for LED Port

    gpio.GPIO_Pin   = BOARD_LED_LED;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

#if defined (STM32F10X)
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
#elif defined (STM32F4XX)
    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
#endif

    GPIO_Init(BOARD_LED_PORT, &gpio);
    board_led_off ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * green LED on
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
board_led_on (void)
{
    GPIO_WriteBit(BOARD_LED_PORT, BOARD_LED_LED, BOARD_LED_ON);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * green LED off
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
board_led_off (void)
{
    GPIO_WriteBit(BOARD_LED_PORT, BOARD_LED_LED, BOARD_LED_OFF);
}
