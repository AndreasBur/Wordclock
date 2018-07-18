#ifndef _WS2812_H_
#define _WS2812_H_

#include "Simulator.h"

#define WS2812_NUMBER_OF_LEDS               110

/* Wrapper for WxWidgets */
#define Pixels() Pixels(0L, _("Wordclock Simulator"))

typedef Simulator WS2812;

#endif
