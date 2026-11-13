// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

// The PIO and DMA calls, for the tests and the host process that do not inspect the frame.
// frame_test.cpp defines its own instead, so it can keep what was transmitted.
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pio.h>

pio_hw pio0_hw{};
pio_hw pio1_hw{};

int pio_add_program(PIO, pio_program_t const*) { return 0; }
void pio_remove_program(PIO, pio_program_t const*, uint) {}
bool pio_sm_is_claimed(PIO, uint) { return false; }
void pio_sm_claim(PIO, uint) {}
void pio_sm_unclaim(PIO, uint) {}
pio_sm_config pio_get_default_sm_config(void) { return {}; }
void sm_config_set_wrap(pio_sm_config*, uint, uint) {}
void sm_config_set_sideset(pio_sm_config*, uint, bool, bool) {}
void sm_config_set_sideset_pins(pio_sm_config*, uint) {}
void sm_config_set_out_shift(pio_sm_config*, bool, bool, uint) {}
void sm_config_set_fifo_join(pio_sm_config*, pio_fifo_join) {}
void sm_config_set_clkdiv(pio_sm_config*, float) {}
void pio_gpio_init(PIO, uint) {}
void pio_sm_set_consecutive_pindirs(PIO, uint, uint, uint, bool) {}
void pio_sm_init(PIO, uint, uint, pio_sm_config const*) {}
void pio_sm_set_enabled(PIO, uint, bool) {}
uint pio_get_dreq(PIO, uint, bool) { return 0u; }

uint32_t clock_get_hz(clock_handle_t) { return 150u * 1000u * 1000u; }

int dma_claim_unused_channel(bool) { return 0; }
dma_channel_config dma_channel_get_default_config(uint) { return {}; }
void channel_config_set_transfer_data_size(dma_channel_config*, dma_channel_transfer_size) {}
void channel_config_set_read_increment(dma_channel_config*, bool) {}
void channel_config_set_write_increment(dma_channel_config*, bool) {}
void channel_config_set_dreq(dma_channel_config*, uint) {}
void dma_channel_configure(uint, dma_channel_config const*, void volatile*, void const*, uint,
                           bool) {}
bool dma_channel_is_busy(uint) { return false; }
