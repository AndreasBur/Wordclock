// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESP_ERR_H_
#define WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESP_ERR_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
typedef int esp_err_t;
#define ESP_OK 0
/* The framework's own generic failure, which is what an absent header answers. */
#define ESP_FAIL -1
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESP_ERR_H_
