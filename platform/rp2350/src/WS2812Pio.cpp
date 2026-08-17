/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       WS2812Pio.cpp
 *      \brief      The four instructions that make the waveform, and the DMA that feeds them
 *
 *      \details    The program is written out as encoded instructions rather than
 *                  assembled from a .pio file. Two reasons, and the second is the one that
 *                  decided it: four instructions are short enough to read either way, and
 *                  the platform package installs a stand-in for pioasm rather than the
 *                  assembler itself, so a .pio source would build here and not on the next
 *                  machine.
 *
 *                  Each encoding is derived in the comment beside it, so the table can be
 *                  checked against the datasheet without reassembling anything.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of WS2812Pio.h, which reaches Arduino.h and its Serial macro. */
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/pio.h>

#include "Arduino.h"
#include "WS2812Pio.h"

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* The program, one bit of the strip per pass.
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
constexpr uint16_t Instructions[]{
    /* out x, 1 side 0 [8]: opcode 011, side 0 + delay 8 -> 01000, destination X 001,
       bit count 1 -> 00001. */
    0x6821u,
    /* jmp !x, 3 side 1 [7]: opcode 000, side 1 + delay 7 -> 10111, condition !X 001,
       address 3 -> 00011. */
    0x1723u,
    /* jmp 0 side 1 [7]: opcode 000, side 1 + delay 7 -> 10111, condition always 000,
       address 0 -> 00000. */
    0x1700u,
    /* nop side 0 [7], which is mov y, y: opcode 101, side 0 + delay 7 -> 00111,
       destination Y 010, operation none 00, source Y 010. */
    0xA742u,
};

constexpr uint InstructionCount{sizeof(Instructions) / sizeof(Instructions[0])};
constexpr uint WrapTargetOffset{0u};
constexpr uint WrapOffset{3u};

constexpr uint StateMachine{WS2812_PIO_STATE_MACHINE_INDEX};

PIO block() { return (WS2812_PIO_BLOCK_INDEX == 0u) ? pio0 : pio1; }

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Claims a state machine and a DMA channel for the strip's data pin
******************************************************************************************************************************************************/
void WS2812Pio::init(uint8_t Pin)
{
    if(IsReady) { return; }

    PIO Block = block();

    pio_program_t Program{};
    Program.instructions = Instructions;
    Program.length = InstructionCount;
    /* Anywhere the loader finds room, rather than a fixed address: nothing else in this
       firmware uses the block, but a fixed origin is a conflict waiting for the first
       library that does. */
    Program.origin = -1;

    const int Offset = pio_add_program(Block, &Program);
    if(Offset < 0) {
        Serial.println(F("Pixels: no room for the PIO program"));
        return;
    }

    DmaChannel = dma_claim_unused_channel(false);
    if(DmaChannel < 0) {
        Serial.println(F("Pixels: no free DMA channel"));
        return;
    }

    pio_sm_config Config = pio_get_default_sm_config();
    sm_config_set_wrap(&Config, Offset + WrapTargetOffset, Offset + WrapOffset);
    /* One bit, not optional and not a direction: every instruction carries it, which is
       what lets the four of them above describe the whole waveform. */
    sm_config_set_sideset(&Config, 1, false, false);
    sm_config_set_sideset_pins(&Config, Pin);
    /* Shifting left, so the most significant bit leaves first - the WS2812's order - and
       pulling automatically every 24 bits, which is one LED. */
    sm_config_set_out_shift(&Config, false, true, BitsPerPixel);
    /* The receive side is never read here, so its four words are worth more as depth on
       the side DMA fills. */
    sm_config_set_fifo_join(&Config, PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&Config, static_cast<float>(clock_get_hz(clk_sys)) / static_cast<float>(StateMachineClockHz));

    pio_gpio_init(Block, Pin);
    pio_sm_set_consecutive_pindirs(Block, StateMachine, Pin, 1, true);
    pio_sm_init(Block, StateMachine, Offset, &Config);
    pio_sm_set_enabled(Block, StateMachine, true);

    IsReady = true;
} /* init */


/******************************************************************************************************************************************************
  isBusy()
******************************************************************************************************************************************************/
bool WS2812Pio::isBusy() const
{
    if(!IsReady) { return false; }

    return dma_channel_is_busy(static_cast<uint>(DmaChannel));
} /* isBusy */


/******************************************************************************************************************************************************
  transmit()
******************************************************************************************************************************************************/
/*! \brief          Hands one frame to DMA and returns
 *  \details        Refuses rather than waits while the previous frame is still going out.
 *                  The caller ticks every 10 ms and a frame takes 3.3 ms, so a refusal
 *                  here means something is wrong rather than that the strip is busy - and
 *                  waiting would spend a third of a tick doing nothing.
 *
 *  \return         E_OK if DMA took the frame
******************************************************************************************************************************************************/
StdReturnType WS2812Pio::transmit(const WordType* Frame, size_t Count)
{
    if(!IsReady || (Frame == nullptr) || (Count == 0u)) { return E_NOT_OK; }
    if(isBusy()) { return E_NOT_OK; }

    dma_channel_config Configuration = dma_channel_get_default_config(static_cast<uint>(DmaChannel));
    channel_config_set_transfer_data_size(&Configuration, DMA_SIZE_32);
    channel_config_set_read_increment(&Configuration, true);
    channel_config_set_write_increment(&Configuration, false);
    /* Paced by the state machine's appetite rather than free-running, so the FIFO is
       refilled as it empties instead of being overrun in the first microsecond. */
    channel_config_set_dreq(&Configuration, pio_get_dreq(block(), StateMachine, true));

    dma_channel_configure(static_cast<uint>(DmaChannel), &Configuration,
                          &block()->txf[StateMachine], Frame, Count, true);

    return E_OK;
} /* transmit */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
