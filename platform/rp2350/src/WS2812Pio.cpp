// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The four instructions that make the waveform, and the DMA that feeds them.
 *
 * The program is written out as encoded instructions rather than assembled from a .pio file. Two
 * reasons, and the second is the one that decided it: four instructions are short enough to read
 * either way, and the platform package installs a stand-in for pioasm rather than the assembler
 * itself, so a .pio source would build here and not on the next machine.
 *
 * Each encoding is derived in the comment beside it, so the table can be checked against the
 * datasheet without reassembling anything.
 */

/* Ahead of WS2812Pio.h, which reaches Arduino.h and its Serial macro. */
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pio.h>

#include "Arduino.h"
#include "WS2812Pio.h"

namespace {
/*
 * The program, one bit of the strip per pass.
 *
 *      bitloop: out x, 1       side 0 [Phase3 - 1]
 *               jmp !x do_zero side 1 [Phase1 - 1]
 *      do_one:  jmp bitloop    side 1 [Phase2 - 1]
 *      do_zero: nop            side 0 [Phase2 - 1]
 *
 * A one is therefore high for Phase1 + Phase2 and low for Phase3; a zero is high for
 * Phase1 and low for Phase2 + Phase3. The low stretch at the top of the loop is the tail
 * of the bit before it, which costs one extra low phase before the very first bit and is
 * indistinguishable from the idle line that precedes it anyway.
 *
 * Field layout of an instruction, with one bit of side-set claimed: bits 15..13 the
 * opcode, bit 12 the side-set value, bits 11..8 the delay, and the remaining eight bits
 * the operands. The delay is what is left of the five-bit field, hence the four-bit limit
 * the header asserts against.
 */
constexpr uint16_t kInstructions[]{
    // out x, 1 side 0 [8]: opcode 011, side 0 + delay 8 -> 01000, destination X 001,
    // bit count 1 -> 00001.
    0x6821u,
    // jmp !x, 3 side 1 [7]: opcode 000, side 1 + delay 7 -> 10111, condition !X 001,
    // address 3 -> 00011.
    0x1723u,
    // jmp 0 side 1 [7]: opcode 000, side 1 + delay 7 -> 10111, condition always 000,
    // address 0 -> 00000.
    0x1700u,
    // nop side 0 [7], which is mov y, y: opcode 101, side 0 + delay 7 -> 00111,
    // destination Y 010, operation none 00, source Y 010.
    0xA742u,
};

constexpr uint kInstructionCount{sizeof(kInstructions) / sizeof(kInstructions[0])};
constexpr uint kWrapTargetOffset{0u};
constexpr uint kWrapOffset{3u};

constexpr uint kStateMachine{WS2812_PIO_STATE_MACHINE_INDEX};

PIO Block() { return (WS2812_PIO_BLOCK_INDEX == 0u) ? pio0 : pio1; }

} // namespace

void WS2812Pio::Init(uint8_t pin)
{
    if(is_ready_) { return; }

    PIO block = Block();

    // pio_sm_claim() panics on a state machine that is claimed already, so it is asked first.
    if(pio_sm_is_claimed(block, kStateMachine)) {
        Serial.println(F("Pixels: the PIO state machine is claimed already"));
        return;
    }
    pio_sm_claim(block, kStateMachine);

    pio_program_t program{};
    program.instructions = kInstructions;
    program.length = kInstructionCount;
    /*
     * Anywhere the loader finds room, rather than a fixed address: nothing else in this
     * firmware uses the block, but a fixed origin is a conflict waiting for the first
     * library that does.
     */
    program.origin = -1;

    const int offset = pio_add_program(block, &program);
    if(offset < 0) {
        pio_sm_unclaim(block, kStateMachine);
        Serial.println(F("Pixels: no room for the PIO program"));
        return;
    }

    dma_channel_ = dma_claim_unused_channel(false);
    if(dma_channel_ < 0) {
        // Given back, so that a call that gave up holds nothing that another one would need.
        pio_remove_program(block, &program, static_cast<uint>(offset));
        pio_sm_unclaim(block, kStateMachine);
        Serial.println(F("Pixels: no free DMA channel"));
        return;
    }

    pio_sm_config config = pio_get_default_sm_config();
    // Not negative any more: a negative offset returned above.
    const uint program_offset{static_cast<uint>(offset)};
    sm_config_set_wrap(&config, program_offset + kWrapTargetOffset, program_offset + kWrapOffset);
    // One bit, not optional and not a direction: every instruction carries it, which is
    // what lets the four of them above describe the whole waveform.
    sm_config_set_sideset(&config, 1, false, false);
    sm_config_set_sideset_pins(&config, pin);
    // Shifting left, so the most significant bit leaves first - the WS2812's order - and
    // pulling automatically every 24 bits, which is one LED.
    sm_config_set_out_shift(&config, false, true, kBitsPerPixel);
    // The receive side is never read here, so its four words are worth more as depth on
    // the side DMA fills.
    sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&config, static_cast<float>(clock_get_hz(clk_sys)) / static_cast<float>(kStateMachineClockHz));

    pio_gpio_init(block, pin);
    pio_sm_set_consecutive_pindirs(block, kStateMachine, pin, 1, true);
    pio_sm_init(block, kStateMachine, program_offset, &config);
    pio_sm_set_enabled(block, kStateMachine, true);

    is_ready_ = true;
}

bool WS2812Pio::IsBusy() const
{
    if(!is_ready_) { return false; }

    return dma_channel_is_busy(static_cast<uint>(dma_channel_));
}

// Not const, though nothing here writes a member: it drives the hardware, and a const
// would say it changes nothing.
// NOLINTBEGIN(readability-make-member-function-const)
StdReturnType WS2812Pio::Transmit(const WordType* frame, size_t count)
{
    if(!is_ready_ || (frame == nullptr) || (count == 0u)) { return kENotOk; }
    if(IsBusy()) { return kENotOk; }

    dma_channel_config configuration = dma_channel_get_default_config(static_cast<uint>(dma_channel_));
    channel_config_set_transfer_data_size(&configuration, DMA_SIZE_32);
    channel_config_set_read_increment(&configuration, true);
    channel_config_set_write_increment(&configuration, false);
    // Paced by the state machine's appetite rather than free-running, so the FIFO is
    // refilled as it empties instead of being overrun in the first microsecond.
    channel_config_set_dreq(&configuration, pio_get_dreq(Block(), kStateMachine, true));

    dma_channel_configure(static_cast<uint>(dma_channel_), &configuration,
                          &Block()->txf[kStateMachine], frame, static_cast<uint>(count), true);

    return kEOk;
}
// NOLINTEND(readability-make-member-function-const)
