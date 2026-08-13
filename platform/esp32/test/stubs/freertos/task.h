#ifndef _TASK_H_
#define _TASK_H_
#include "FreeRTOS.h"
TickType_t xTaskGetTickCount();
void vTaskDelayUntil(TickType_t*, TickType_t);
#endif
