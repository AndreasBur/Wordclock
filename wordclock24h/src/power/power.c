/*-------------------------------------------------------------------------------------------------------------------------------------------
 * power.c - power routines for LED stripes
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
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
#include "power.h"
#include "log.h"

#if defined (STM32F401RE) || defined (STM32F411RE)              // STM32F401/STM32F411 Nucleo Board PC8
#define POWER_PERIPH_CLOCK_CMD  RCC_AHB1PeriphClockCmd
#define POWER_PERIPH            RCC_AHB1Periph_GPIOC
#define POWER_PORT              GPIOC
#define POWER_PIN               GPIO_Pin_8

#elif defined (STM32F103)
#define POWER_PERIPH_CLOCK_CMD  RCC_APB2PeriphClockCmd          // STM32F103 PB0
#define POWER_PERIPH            RCC_APB2Periph_GPIOB
#define POWER_PORT              GPIOB
#define POWER_PIN               GPIO_Pin_0

#else
#error STM32 unknown
#endif

#define POWER_ON                SET                             // 1 = Power on
#define POWER_OFF               RESET                           // 0 = Power off

POWER_GLOBALS                   power =
{
    0xFF                                                        // power.is_on
};

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * initialize power port
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
power_init (void)
{
    GPIO_InitTypeDef gpio;

    GPIO_StructInit (&gpio);
    POWER_PERIPH_CLOCK_CMD (POWER_PERIPH, ENABLE);     // enable clock for power port

    gpio.GPIO_Pin   = POWER_PIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

#if defined (STM32F10X)
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
#elif defined (STM32F4XX)
    gpio.GPIO_Mode  = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
#endif

    GPIO_Init(POWER_PORT, &gpio);
    log_message ("power_init() called");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * switch power on
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
power_on (void)
{
    if (power.is_on != 1)
    {
        GPIO_WriteBit(POWER_PORT, POWER_PIN, POWER_ON);
        log_message ("switching power on");
        power.is_on = 1;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * switch power off
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
power_off (void)
{
    if (power.is_on != 0)
    {
        GPIO_WriteBit(POWER_PORT, POWER_PIN, POWER_OFF);
        log_message ("switching power off");
        power.is_on = 0;
    }
}
