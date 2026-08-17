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
/**     \file       WS2812Pio.h
 *      \brief      WS2812 waveform out of a PIO state machine, fed by DMA
 *
 *      \details    The counterpart of the ESP32 backend's RMT channel and of the AVR-Dx
 *                  backend's timer-and-CCL arrangement: the pulse widths come out of
 *                  hardware, and the processor only hands over a frame.
 *
 *                  Split out of Pixels for the same reason the AVR-Dx backend splits its
 *                  WS2812 out - the buffer logic above is identical on all three
 *                  platforms, and keeping the waveform beside it would hide that.
 *
 *                  A frame is handed to DMA and the call returns. Nothing here waits for
 *                  the strip: 110 LEDs take 3.3 ms, a third of the scheduler's tick, and
 *                  blocking for that would push every other module's timing around.
 *
******************************************************************************************************************************************************/
#ifndef _WS2812_PIO_H_
#define _WS2812_PIO_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WS2812Pio configuration parameter */
/* One state machine of one block. The RP2350 has twelve of them across three blocks, so
   claiming one costs nothing that anything else here wants. */
#define WS2812_PIO_BLOCK_INDEX                          0u
#define WS2812_PIO_STATE_MACHINE_INDEX                  0u

/******************************************************************************************************************************************************
 *  C L A S S   W S 2 8 1 2   P I O
******************************************************************************************************************************************************/
class WS2812Pio
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* One LED per word rather than one colour per byte, because that is the shape the
       state machine reads: the three bytes sit in bits 31..8 and are shifted out from the
       top, which is the WS2812's most-significant-first order. */
    using WordType = uint32_t;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* The three phases of one bit, in state machine cycles. A zero is high for Phase1 and
       low for the other two; a one is high for Phase1 + Phase2 and low for Phase3. At the
       clock below that is 400 ns and 800 ns of high time against a 1250 ns period - the
       datasheet's nominal values, not the 300/900 the vendor examples use. Those also
       work, but they spend the whole tolerance on one side, and it is the far end of a
       long strip with a slow edge that eats into what is left. */
    static constexpr uint32_t Phase1Cycles{8u};
    static constexpr uint32_t Phase2Cycles{8u};
    static constexpr uint32_t Phase3Cycles{9u};
    static constexpr uint32_t CyclesPerBit{Phase1Cycles + Phase2Cycles + Phase3Cycles};

    /* 50 ns a cycle, which makes all three phases whole numbers and keeps every delay
       inside the four bits an instruction has for it once one bit is spent on side-set. */
    static constexpr uint32_t StateMachineClockHz{20u * 1000u * 1000u};

    static constexpr uint32_t BitsPerPixel{24u};

    static_assert(CyclesPerBit * (1000000000u / StateMachineClockHz) == 1250u,
                  "WS2812Pio: the three phases do not add up to the WS2812's 1.25 us bit period");
    static_assert((Phase1Cycles <= 16u) && (Phase2Cycles <= 16u) && (Phase3Cycles <= 16u),
                  "WS2812Pio: a delay of more than 16 cycles does not fit an instruction's four delay bits");

    bool IsReady{false};
    int DmaChannel{-1};

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WS2812Pio& getInstance() {
        static WS2812Pio SingletonInstance;
        return SingletonInstance;
    }

    /* Claims the state machine and a DMA channel for the strip's data pin. Reports and
       gives up rather than retrying: the contract's init() returns nothing, and a clock
       whose console still answers is more useful for finding a miswired pin than one that
       reboots. */
    void init(uint8_t Pin);

    bool isReady() const { return IsReady; }

    /* True while DMA is still fetching the previous frame. The caller has to check this
       before handing over the next one - two frames back to back would lose the low gap
       the strip latches on. */
    bool isBusy() const;

    /* Hands one frame to DMA and returns. The buffer has to outlive the transfer, which
       is why Pixels keeps a frame of its own rather than passing its pixel buffer. */
    StdReturnType transmit(const WordType* Frame, size_t Count);
};

#endif // _WS2812_PIO_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
