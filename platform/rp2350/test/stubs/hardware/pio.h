#ifndef _HARDWARE_PIO_H_
#define _HARDWARE_PIO_H_
#include <stdint.h>
#include <stddef.h>
typedef unsigned int uint;
typedef struct pio_hw { volatile uint32_t txf[4]; }* PIO;
extern pio_hw pio0_hw;
extern pio_hw pio1_hw;
#define pio0 (&pio0_hw)
#define pio1 (&pio1_hw)
typedef enum { PIO_FIFO_JOIN_NONE = 0, PIO_FIFO_JOIN_TX = 1 } pio_fifo_join;
typedef struct { const uint16_t* instructions; uint8_t length; int8_t origin;
                 uint8_t pio_version; uint8_t used_gpio_ranges; } pio_program_t;
typedef struct { uint32_t clkdiv; uint32_t execctrl; uint32_t shiftctrl; uint32_t pinctrl; } pio_sm_config;
int pio_add_program(PIO, const pio_program_t*);
pio_sm_config pio_get_default_sm_config(void);
void sm_config_set_wrap(pio_sm_config*, uint, uint);
void sm_config_set_sideset(pio_sm_config*, uint, bool, bool);
void sm_config_set_sideset_pins(pio_sm_config*, uint);
void sm_config_set_out_shift(pio_sm_config*, bool, bool, uint);
void sm_config_set_fifo_join(pio_sm_config*, pio_fifo_join);
void sm_config_set_clkdiv(pio_sm_config*, float);
void pio_gpio_init(PIO, uint);
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool);
void pio_sm_init(PIO, uint, uint, const pio_sm_config*);
void pio_sm_set_enabled(PIO, uint, bool);
uint pio_get_dreq(PIO, uint, bool);
#endif
