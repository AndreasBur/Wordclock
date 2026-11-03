#ifndef _FREERTOS_H_
#define _FREERTOS_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stdint.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
// NOLINTEND(readability-identifier-naming)
#endif
