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
/**     \file       Timer.h
 *      \brief      16-bit Timer/Counter type A as periodic interrupt and PWM source
 *
 *      \details    The xmega E5's TC4/TC5 driver brought over to the Dx. One class for TCA0 and
 *                  TCA1, picked by a traits struct, which is what the xmega version did for TCC4
 *                  and TCC5. There the two kinds differed in their register struct; here they do
 *                  not - both are a TCA_SINGLE_t with three compare channels - so the traits carry
 *                  what actually differs on this part: which instance, and which port its waveform
 *                  outputs are routed to.
 *
 *                  The timer counts dual-slope and interrupts at BOTTOM, so a period spans 2 * PER
 *                  timer cycles and PWM is phase-correct. That much is unchanged from the xmega.
 *
 *      \attention  Nothing in this backend calls this yet. The clock's tick is polled off millis()
 *                  in main.cpp, and the strip's timing belongs to TCB1/TCB2 and the CCL - see
 *                  WS2812.h. This is the driver, not a new time base.
 *
******************************************************************************************************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include <avr/io.h>
#include <stddef.h>


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Timer configuration parameter: which timers get an instance and an overflow ISR.
 * Both are free on this part - unlike on the xmega, where the core owned TCD5 for millis() and
 * drove analogWrite() from TCC4 - so the choice here is only about the flash an unused ISR costs. */
#define TIMER_USE_TCA0                          STD_ON
#define TIMER_USE_TCA1                          STD_OFF


/******************************************************************************************************************************************************
 *  T I M E R   T R A I T S
******************************************************************************************************************************************************/
/* The waveform outputs WO0..WO2 are pins 0..2 of whichever port PORTMUX.TCAROUTEA names, so the
 * port and the route below are one decision written twice and have to agree.
 *
 * PORTB, and not the PORTA the route defaults to from reset: PA0 carries USART0 TXD into the CCL
 * and PA2 its XCK into the event system, which is how the strip gets its pulse widths (WS2812.h).
 * A timer that came up on its default route would drive both of them. PORTD is left alone because
 * PowerSwitch.h claims it, and PORTC and PORTF are the console and the two-wire bus. */
struct TimerA0
{
    using RegisterType = TCA_SINGLE_t;
    static RegisterType& getTimer() { return TCA0.SINGLE; }
    static PORT_t& getPort() { return PORTB; }
    static constexpr uint8_t RouteMask{PORTMUX_TCA0_gm};
    static constexpr uint8_t RouteGroup{PORTMUX_TCA0_PORTB_gc};
    static constexpr uint8_t NumberOfChannels{3u};
    static constexpr uint8_t FirstPin{0u};
};

/* TCA1 reaches PORTB and PORTC and nothing else, and PC0/PC1 are the console - so PORTB is its only
 * free route, the same one TCA0 is given above. Turning both on therefore means moving TCA0 to
 * PORTE first; that is why TIMER_USE_TCA1 is off rather than the pair being enabled by default. */
struct TimerA1
{
    using RegisterType = TCA_SINGLE_t;
    static RegisterType& getTimer() { return TCA1.SINGLE; }
    static PORT_t& getPort() { return PORTB; }
    static constexpr uint8_t RouteMask{PORTMUX_TCA1_gm};
    static constexpr uint8_t RouteGroup{PORTMUX_TCA1_PORTB_gc};
    static constexpr uint8_t NumberOfChannels{3u};
    static constexpr uint8_t FirstPin{0u};
};


/******************************************************************************************************************************************************
 *  C L A S S   T I M E R
******************************************************************************************************************************************************/
template <typename TimerTraits>
class Timer
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* Timer ISR callback function */
    typedef void (*TimerIsrCallbackF_void)(void);

    /* Type which describes the internal state of the TIMER */
    typedef enum {
        STATE_NONE,
        STATE_INIT,
        STATE_READY,
        STATE_RUNNING,
        STATE_STOPPED
    } StateType;

    /* Type which includes the values of the Clock Select Bit Group.
     * One more step than the xmega offered: the Dx prescaler has a 16 between 8 and 64, which lets
     * a period that used to round up to the 64 keep four times the resolution. There is no OFF
     * here, though - this CLKSEL has no such group config, and the timer is stopped by taking
     * ENABLE away instead. */
    typedef enum {
        TIMER_REG_CS_NO_PRESCALER = TCA_SINGLE_CLKSEL_DIV1_gc,
        TIMER_REG_CS_PRESCALE_2 = TCA_SINGLE_CLKSEL_DIV2_gc,
        TIMER_REG_CS_PRESCALE_4 = TCA_SINGLE_CLKSEL_DIV4_gc,
        TIMER_REG_CS_PRESCALE_8 = TCA_SINGLE_CLKSEL_DIV8_gc,
        TIMER_REG_CS_PRESCALE_16 = TCA_SINGLE_CLKSEL_DIV16_gc,
        TIMER_REG_CS_PRESCALE_64 = TCA_SINGLE_CLKSEL_DIV64_gc,
        TIMER_REG_CS_PRESCALE_256 = TCA_SINGLE_CLKSEL_DIV256_gc,
        TIMER_REG_CS_PRESCALE_1024 = TCA_SINGLE_CLKSEL_DIV1024_gc
    } TimerClockSelectType;

    /* Type which defines the compare channels, each drives one PWM pin.
     * Numbered, where the xmega lettered them A..D: the registers these reach are CMP0..CMP2 and
     * the pins WO0..WO2, so carrying the letters over would mean translating at every look into
     * the data sheet. */
    typedef enum {
        TIMER_CHANNEL_0,
        TIMER_CHANNEL_1,
        TIMER_CHANNEL_2
    } TimerChannelType;

    typedef enum {
        TIMER_COUNT_DIRECTION_UP,
        TIMER_COUNT_DIRECTION_DOWN
    } TimerCountDirection;

    /* PWM duty resolution, the same 10 bit TimerOne offers */
    static constexpr uint16_t PwmDutyMax{1023u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Both of the following are relied on by the shift and the pointer arithmetic further down,
     * which is cheaper than a per-channel table but only holds while the registers stay laid out
     * the way they are. The xmega version had the equivalent pair of assertions about TC4 and TC5
     * sharing a bit layout. */
    static_assert(TCA_SINGLE_CMP1EN_bm == (TCA_SINGLE_CMP0EN_bm << 1u) &&
                  TCA_SINGLE_CMP2EN_bm == (TCA_SINGLE_CMP0EN_bm << 2u),
                  "compare enable bits are not consecutive");
    static_assert(offsetof(TCA_SINGLE_t, CMP1BUF) - offsetof(TCA_SINGLE_t, CMP0BUF) == sizeof(uint16_t) &&
                  offsetof(TCA_SINGLE_t, CMP2BUF) - offsetof(TCA_SINGLE_t, CMP1BUF) == sizeof(uint16_t),
                  "compare buffer registers are not consecutive");

    StateType State;
    TimerClockSelectType ClockSelectBitGroup;
    TimerIsrCallbackF_void TimerOverflowCallback;

    constexpr Timer() : State(STATE_NONE), ClockSelectBitGroup(TIMER_REG_CS_NO_PRESCALER), TimerOverflowCallback(nullptr) { }

    // functions
    static typename TimerTraits::RegisterType& getTimer() { return TimerTraits::getTimer(); }

    static constexpr uint8_t toPinMask(TimerChannelType Channel) {
        return static_cast<uint8_t>(1u << (TimerTraits::FirstPin + Channel));
    }
    static constexpr uint8_t toChannelEnableMask(TimerChannelType Channel) {
        return static_cast<uint8_t>(TCA_SINGLE_CMP0EN_bm << Channel);
    }
    static constexpr uint8_t toPrescalerShift(TimerClockSelectType ClockSelect) {
        switch(ClockSelect) {
            case TIMER_REG_CS_PRESCALE_2:       return 1u;
            case TIMER_REG_CS_PRESCALE_4:       return 2u;
            case TIMER_REG_CS_PRESCALE_8:       return 3u;
            case TIMER_REG_CS_PRESCALE_16:      return 4u;
            case TIMER_REG_CS_PRESCALE_64:      return 6u;
            case TIMER_REG_CS_PRESCALE_256:     return 8u;
            case TIMER_REG_CS_PRESCALE_1024:    return 10u;
            default:                            return 0u;
        }
    }

    TimerCountDirection getCountingDirection();
    void setClockSelect(TimerClockSelectType);
    void setRunning(bool);
    StdReturnType enablePwmChannel(TimerChannelType, uint16_t);
    StdReturnType disablePwmChannel(TimerChannelType);
    StdReturnType setPwmDutyChannel(TimerChannelType, uint16_t);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    // methods
    static Timer& getInstance() {
        static Timer SingletonInstance;
        return SingletonInstance;
    }
    StdReturnType init(uint32_t = 1000u, TimerIsrCallbackF_void = nullptr);
    StdReturnType setPeriod(uint32_t);
    StdReturnType start();
    void stop();
    StdReturnType resume();
    StdReturnType attachInterrupt(TimerIsrCallbackF_void);
    void detachInterrupt();
    StdReturnType read(uint32_t&);
    void handleOverflowInterrupt();

    /* The channel is a template argument so that a channel the timer does not have fails to compile
     * instead of writing into reserved registers at run time */
    template <TimerChannelType Channel> StdReturnType enablePwm(uint16_t Duty) {
        static_assert(Channel < TimerTraits::NumberOfChannels, "this timer has no such compare channel");
        return enablePwmChannel(Channel, Duty);
    }
    template <TimerChannelType Channel> StdReturnType disablePwm() {
        static_assert(Channel < TimerTraits::NumberOfChannels, "this timer has no such compare channel");
        return disablePwmChannel(Channel);
    }
    template <TimerChannelType Channel> StdReturnType setPwmDuty(uint16_t Duty) {
        static_assert(Channel < TimerTraits::NumberOfChannels, "this timer has no such compare channel");
        return setPwmDutyChannel(Channel, Duty);
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
