#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_GPIO_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_GPIO_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stdint.h>
typedef unsigned int uint;
typedef enum { GPIO_IN = 0, GPIO_OUT = 1 } gpio_dir_t;
void gpio_init(uint);
void gpio_set_dir(uint, bool);
void gpio_put(uint, bool);
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_GPIO_H_
