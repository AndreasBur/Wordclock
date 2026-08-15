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
/**     \file       WS2812.cpp
 *      \brief      Pulse-shaping WS2812 transmitter for the AVR Dx, see WS2812.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <avr/interrupt.h>
#include <avr/io.h>

#include "Arduino.h"
#include "WS2812.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* The synchronous baud rate register: the host clock is the peripheral clock divided by
   twice this, in units of a sixty-fourth. */
#define WS2812_BAUD_REGISTER            static_cast<uint16_t>((64uL * F_CPU) / (2uL * WS2812_BIT_RATE_HZ))

/* Peripheral clock cycles per pulse. Rounded to nearest, because at 24 MHz a cycle is
   already 42 ns and truncating both widths would move them a full step towards zero. */
#define WS2812_CYCLES_PER_NS(Nanoseconds)   static_cast<uint16_t>(((F_CPU / 1000000uL) * (Nanoseconds) + 500uL) / 1000uL)

#define WS2812_ZERO_PULSE_CYCLES        WS2812_CYCLES_PER_NS(WS2812_ZERO_PULSE_DURATION_NS)
#define WS2812_ONE_PULSE_CYCLES         WS2812_CYCLES_PER_NS(WS2812_ONE_PULSE_DURATION_NS)

/* out = IN0 ? IN2 : IN1, with IN0 the data line and IN1/IN2 the two one-shots. Written out
   rather than left as a bare 0xE4 so the table can be checked against the wiring below. */
#define WS2812_LUT_TRUTH_TABLE          0xE4u

/* The truth table is what turns two timers into one data line, so a wrong width is worth
   catching here rather than on a strip that only shows the wrong colour. */
static_assert(WS2812_ZERO_PULSE_CYCLES > 0u, "WS2812: F_CPU is too low to time a zero pulse");
static_assert(WS2812_ONE_PULSE_CYCLES > WS2812_ZERO_PULSE_CYCLES, "WS2812: the two pulse widths collapsed into one");
static_assert(WS2812_CYCLES_PER_NS(WS2812_ZERO_PULSE_MAX_DURATION_NS) > WS2812_ZERO_PULSE_CYCLES,
              "WS2812: the zero pulse would be long enough to be read as a one");

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

/* What the interrupt is working through. Only ever set while the transmitter is idle, so
   the handler needs no guard against the frame changing under it. */
const byte* volatile FrameData{nullptr};
volatile uint16_t FrameLength{0u};
volatile uint16_t FramePosition{0u};

} // namespace

/******************************************************************************************************************************************************
 *  I N T E R R U P T   H A N D L E R S
******************************************************************************************************************************************************/
/*! \brief          Feeds the next pixel byte to the strip
 *  \details        Raised to the elevated interrupt level in init(), so it runs ahead of the
 *                  console's handlers. That is what keeps the gap between two bytes inside
 *                  the strip's reset time: the pulse widths themselves come from hardware and
 *                  do not care when this runs, but a byte held back longer than the reset
 *                  time makes the strip latch half a frame.
******************************************************************************************************************************************************/
ISR(USART0_DRE_vect)
{
    if(FramePosition >= FrameLength) {
        /* Disarmed rather than left to fire on an empty frame; the transmitter counts as
           busy until the shift register has drained, which STATUS reports. */
        USART0.CTRLA &= static_cast<uint8_t>(~USART_DREIE_bm);
        return;
    }

    USART0.TXDATAL = FrameData[FramePosition];
    FramePosition++;
}

/******************************************************************************************************************************************************
 *  P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  initUsart()
******************************************************************************************************************************************************/
/*! \brief          Puts USART0 into SPI host mode at the strip's bit rate
 *  \details        Master SPI mode rather than a plain USART because the strip needs a clock
 *                  edge per bit: that edge is what starts the one-shots, and it is the reason
 *                  the pulse widths need no software timing at all.
 *
 *                  Most significant bit first, because that is the order the WS2812 expects
 *                  its colour bytes in.
******************************************************************************************************************************************************/
void WS2812::initUsart()
{
    /* TXD and XCK are outputs; the data line the strip sees is neither of them, it is the
       look-up table's own pin. TXD only has to reach the table, XCK the event system. */
    PORTA.DIRSET = PIN0_bm | PIN2_bm;

    USART0.BAUD = WS2812_BAUD_REGISTER;
    USART0.CTRLC = USART_CMODE_MSPI_gc;
    USART0.CTRLB = USART_TXEN_bm;
} /* initUsart */


/******************************************************************************************************************************************************
  initEventSystem()
******************************************************************************************************************************************************/
/*! \brief          Routes the SPI clock to both one-shots
 *  \details        One channel for both timers, so the two pulses start on the very same
 *                  edge. Started apart they would drift against each other and the look-up
 *                  table would mux between two pulses that no longer share a rising edge.
******************************************************************************************************************************************************/
void WS2812::initEventSystem()
{
    EVSYS.CHANNEL0 = EVSYS_CHANNEL0_PORTA_PIN2_gc;
    EVSYS.USERTCB1CAPT = EVSYS_USER_CHANNEL0_gc;
    EVSYS.USERTCB2CAPT = EVSYS_USER_CHANNEL0_gc;
} /* initEventSystem */


/******************************************************************************************************************************************************
  initOneShotTimers()
******************************************************************************************************************************************************/
/*! \brief          Arms the two pulse widths
 *  \details        Single-shot mode: the event starts the counter, the output stays high for
 *                  the compare value and then falls. One timer per width is what replaces the
 *                  E5's single XCL, whose one-shot could carry both.
******************************************************************************************************************************************************/
void WS2812::initOneShotTimers()
{
    TCB1.CCMP = WS2812_ZERO_PULSE_CYCLES;
    TCB1.EVCTRL = TCB_CAPTEI_bm;
    TCB1.CTRLB = TCB_CNTMODE_SINGLE_gc;
    TCB1.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;

    TCB2.CCMP = WS2812_ONE_PULSE_CYCLES;
    TCB2.EVCTRL = TCB_CAPTEI_bm;
    TCB2.CTRLB = TCB_CNTMODE_SINGLE_gc;
    TCB2.CTRLA = TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;
} /* initOneShotTimers */


/******************************************************************************************************************************************************
  initLookUpTable()
******************************************************************************************************************************************************/
/*! \brief          Muxes the two pulses onto the data pin
 *  \details        The three inputs are peripherals rather than pins, which is why the port
 *                  around them stays free; the input positions are fixed by the device, and
 *                  a look-up table can only reach the USART and timers that carry its own
 *                  index or the neighbouring one.
******************************************************************************************************************************************************/
void WS2812::initLookUpTable()
{
    CCL.LUT0CTRLB = CCL_INSEL0_USART0_gc | CCL_INSEL1_TCB1_gc;
    CCL.LUT0CTRLC = CCL_INSEL2_TCB2_gc;
    CCL.TRUTH0 = WS2812_LUT_TRUTH_TABLE;
    CCL.LUT0CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;

    CCL.CTRLA = CCL_ENABLE_bm;
} /* initLookUpTable */


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          Builds the transmitter out of USART, event system, timers and look-up table
 *  \details        The order matters in one place only: the look-up table is enabled last, so
 *                  that it never drives the strip from inputs whose timers are not yet
 *                  running.
******************************************************************************************************************************************************/
void WS2812::init()
{
    initUsart();
    initEventSystem();
    initOneShotTimers();
    initLookUpTable();

    /* The one vector allowed to interrupt the others. The console's handlers are short, but
       "short" is measured against the strip's reset time here, and this removes the question
       rather than answering it per handler. */
    CPUINT.LVL1VEC = USART0_DRE_vect_num;
} /* init */


/******************************************************************************************************************************************************
  isBusy()
******************************************************************************************************************************************************/
/*! \brief          Whether a frame is still going out
 *  \details        Both halves are needed: the queue can be empty while the last byte is
 *                  still in the shift register, and starting a frame then would run the two
 *                  together without the reset gap between them.
******************************************************************************************************************************************************/
bool WS2812::isBusy() const
{
    if(FramePosition < FrameLength) { return true; }

    return (USART0.STATUS & USART_TXCIF_bm) == 0u;
} /* isBusy */


/******************************************************************************************************************************************************
  send()
******************************************************************************************************************************************************/
/*! \brief          Starts one frame
 *  \details        Returns as soon as the first byte is on its way; the rest follows from the
 *                  interrupt, which takes about 3.3 ms for a full display. The caller is free
 *                  in the meantime, and has to be - the tick this runs in also has a clock to
 *                  keep.
******************************************************************************************************************************************************/
StdReturnType WS2812::send(const byte* Frame, uint16_t Length)
{
    if((Frame == nullptr) || (Length == 0u)) { return E_NOT_OK; }
    if(isBusy()) { return E_NOT_OK; }

    FrameData = Frame;
    FrameLength = Length;
    FramePosition = 0u;

    /* Cleared by hand, because it is what isBusy() reads to tell a drained shift register
       from one that never started. */
    USART0.STATUS = USART_TXCIF_bm;
    USART0.CTRLA |= USART_DREIE_bm;

    return E_OK;
} /* send */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
