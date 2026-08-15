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
/**     \file       WS2812.h
 *      \brief      Pulse-shaping WS2812 transmitter for the AVR Dx
 *
 *      \details    The successor of the xmega E5 driver, and the same idea: the pulse widths
 *                  are made in hardware, so no cycle-counted code has to hold the timing and
 *                  an interrupt cannot corrupt a bit.
 *
 *                  The E5 built it from one XCL - a timer whose one-shot carried both widths
 *                  in PERCAPTL and CMPL, and a LUT that muxed them against the data line. The
 *                  AVR Dx has the LUT again, as CCL, but its TCBs carry one compare value
 *                  each. So the one XCL timer becomes two TCBs, one per width, and the LUT
 *                  picks between them:
 *
 *                      USART0 (MSPI host)  clocks the pixel bytes out at 800 kbit/s
 *                      XCK edge            starts both one-shots through the event system
 *                      TCB1                the short pulse, a zero bit
 *                      TCB2                the long pulse, a one bit
 *                      CCL LUT0            out = TXD ? TCB2 : TCB1, truth table 0xE4
 *
 *                  What the E5 had and this does not is the DMA: no AVR Dx has one. The bytes
 *                  are therefore fed by an interrupt, one per 10 us. That is affordable
 *                  precisely because the pulse shaping is in hardware - a late byte only
 *                  stretches the gap between bytes, it does not deform a pulse. The gap is
 *                  what has to be watched instead, see the reset time below.
 *
 *      \attention  The register set-up below is derived from the data sheet and compiles
 *                  against the device headers, but it has not been on an oscilloscope. Before
 *                  trusting it, check on hardware: the two pulse widths, that the data bit is
 *                  stable when the one-shots fire (UCPHA), and whether the CCL sees the TCB
 *                  waveforms without their pin outputs enabled.
 *
******************************************************************************************************************************************************/
#ifndef _WS2812_H_
#define _WS2812_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WS2812 configuration parameter */
/* The data line, driven by the CCL rather than by a port register: LUT0's own output pin.
   PA3 also carries USART0's XDIR, which stays undriven because RS485 mode is never
   enabled - the alternative, PA6, belongs to USART0's alternative pin block and would be
   just as free. */
#define WS2812_DATA_PIN                                 3u

/* WS2812 parameter */
/* The data sheet's pulse widths. Only the short one is critical: a zero that grows past
   about 550 ns is read as a one. */
#define WS2812_ZERO_PULSE_DURATION_NS                   350u
#define WS2812_ONE_PULSE_DURATION_NS                    900u
#define WS2812_ZERO_PULSE_MAX_DURATION_NS               550u

/* One bit per 1.25 us. This is the strip's rate, not a choice. */
#define WS2812_BIT_RATE_HZ                              800000uL

/* How long the line has to rest before the strip latches what it was sent. The gap between
   two bytes has to stay below this, or the frame is torn in half - which is the one budget
   every interrupt handler in this backend is measured against. */
#define WS2812_RESET_DURATION_US                        50u

/******************************************************************************************************************************************************
 *  C L A S S   W S 2 8 1 2
******************************************************************************************************************************************************/
class WS2812
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    WS2812() { }
    ~WS2812() { }

    static void initUsart();
    static void initEventSystem();
    static void initOneShotTimers();
    static void initLookUpTable();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WS2812& getInstance() {
        static WS2812 SingletonInstance;
        return SingletonInstance;
    }

    void init();

    /* Whether the previous frame is still on the wire. A frame started while one is running
       would interleave two of them, so the caller has to ask first. */
    bool isBusy() const;

    /* Hands one frame over and returns; the interrupt does the rest. The buffer has to
       outlive the transmission, which is why the caller keeps it rather than passing a
       temporary. */
    StdReturnType send(const byte* Frame, uint16_t Length);
};

#endif // _WS2812_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
