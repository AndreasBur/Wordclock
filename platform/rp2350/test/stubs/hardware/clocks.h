#ifndef _HARDWARE_CLOCKS_H_
#define _HARDWARE_CLOCKS_H_
#include <stdint.h>
typedef enum { clk_sys = 0 } clock_handle_t;
uint32_t clock_get_hz(clock_handle_t);
#endif
