#ifndef _WS2812_H_
#define _WS2812_H_

#include "Simulator.h"
#include "Neopixel.h"

#define WS2812_NUMBER_OF_LEDS               110
#define WS2812_SUPPORT_DIMMING              STD_ON
#define WS2812_IS_SINGLETON                 STD_ON

/* Wrapper for WxWidgets */
typedef Simulator WS2812;

#endif
