#ifndef _TASK_H_
#define _TASK_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include "FreeRTOS.h"
TickType_t xTaskGetTickCount();
void vTaskDelayUntil(TickType_t*, TickType_t);
// NOLINTEND(readability-identifier-naming)
#endif
