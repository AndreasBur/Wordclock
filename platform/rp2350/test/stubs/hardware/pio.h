#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_PIO_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_PIO_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stdint.h>
#include <stddef.h>
typedef unsigned int uint;
typedef struct pio_hw {
  uint32_t volatile txf[4];
}* PIO;
extern pio_hw pio0_hw;
extern pio_hw pio1_hw;
#define pio0 (&pio0_hw)
#define pio1 (&pio1_hw)
typedef enum { PIO_FIFO_JOIN_NONE = 0, PIO_FIFO_JOIN_TX = 1 } pio_fifo_join;
typedef struct {
  uint16_t const* instructions;
  uint8_t length;
  int8_t origin;
  uint8_t pio_version;
  uint8_t used_gpio_ranges;
} pio_program_t;
typedef struct {
  uint32_t clkdiv;
  uint32_t execctrl;
  uint32_t shiftctrl;
  uint32_t pinctrl;
} pio_sm_config;
int pio_add_program(PIO, pio_program_t const*);
void pio_remove_program(PIO, pio_program_t const*, uint);
bool pio_sm_is_claimed(PIO, uint);
void pio_sm_claim(PIO, uint);
void pio_sm_unclaim(PIO, uint);
pio_sm_config pio_get_default_sm_config(void);
void sm_config_set_wrap(pio_sm_config*, uint, uint);
void sm_config_set_sideset(pio_sm_config*, uint, bool, bool);
void sm_config_set_sideset_pins(pio_sm_config*, uint);
void sm_config_set_out_shift(pio_sm_config*, bool, bool, uint);
void sm_config_set_fifo_join(pio_sm_config*, pio_fifo_join);
void sm_config_set_clkdiv(pio_sm_config*, float);
void pio_gpio_init(PIO, uint);
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool);
void pio_sm_init(PIO, uint, uint, pio_sm_config const*);
void pio_sm_set_enabled(PIO, uint, bool);
uint pio_get_dreq(PIO, uint, bool);
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_HARDWARE_PIO_H_
