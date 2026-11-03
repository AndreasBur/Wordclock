#ifndef _HARDWARE_CLOCKS_H_
#define _HARDWARE_CLOCKS_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stdint.h>
typedef enum { clk_sys = 0 } clock_handle_t;
uint32_t clock_get_hz(clock_handle_t);
// NOLINTEND(readability-identifier-naming)
#endif
