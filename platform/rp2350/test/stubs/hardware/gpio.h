#ifndef _HARDWARE_GPIO_H_
#define _HARDWARE_GPIO_H_
#include <stdint.h>
typedef unsigned int uint;
typedef enum { GPIO_IN = 0, GPIO_OUT = 1 } gpio_dir_t;
void gpio_init(uint);
void gpio_set_dir(uint, bool);
void gpio_put(uint, bool);
#endif
