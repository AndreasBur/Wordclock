// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_FREERTOS_TASK_H_
#define WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_FREERTOS_TASK_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include "FreeRTOS.h"
TickType_t xTaskGetTickCount();
void vTaskDelayUntil(TickType_t*, TickType_t);
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_FREERTOS_TASK_H_
