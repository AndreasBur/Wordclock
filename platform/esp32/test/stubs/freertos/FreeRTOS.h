#ifndef _FREERTOS_H_
#define _FREERTOS_H_
#include <stdint.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
#endif
