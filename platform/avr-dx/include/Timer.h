// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief 16-bit Timer/Counter type A as periodic interrupt and PWM source.
 *
 * The xmega E5's TC4/TC5 driver brought over to the Dx. One class for TCA0 and TCA1, picked by a
 * traits struct, which is what the xmega version did for TCC4 and TCC5. There the two kinds
 * differed in their register struct; here they do not - both are a TCA_SINGLE_t with three compare
 * channels - so the traits carry what actually differs on this part: which instance, and which port
 * its waveform outputs are routed to.
 *
 * The timer counts dual-slope and interrupts at BOTTOM, so a period spans 2 * PER timer cycles and
 * PWM is phase-correct. That much is unchanged from the xmega.
 *
 * \attention Nothing in this backend calls this yet. The clock's tick is polled off millis() in
 * main.cpp, and the strip's timing belongs to TCB1/TCB2 and the CCL - see WS2812.h. This is the
 * driver, not a new time base.
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "StandardTypes.h"
#include <avr/io.h>
#include <stddef.h>

/*
 * Timer configuration parameter: which timers get an instance and an overflow ISR.
 * Both are free here - unlike on the xmega, where the core owned TCD5 for millis() and drove
 * analogWrite() from TCC4 - so the choice is only about the flash an unused ISR costs. TCA1 is
 * fitted to the 48- and 64-pin derivatives alone.
 */
#define TIMER_USE_TCA0                          STD_ON
#define TIMER_USE_TCA1                          STD_OFF

/*
 * Which port carries the waveform outputs, named by its letter alone: WO0..WO2 are pins 0..2 of
 * that port, and both the PORT_t and the PORTMUX group config below are pasted from the letter.
 * One letter rather than the two of them written side by side, because they are a single decision
 * and two macros are two things that can be made to disagree.
 *
 * PORTB, and not the PORTA the route comes up on from reset: PA0 carries USART0 TXD into the CCL
 * and PA2 its XCK into the event system, which is how the strip gets its pulse widths (WS2812.h).
 * A timer left on its default route would drive both of them. PORTD is left alone because
 * PowerSwitch.h claims it, and PORTC and PORTF are the console and the two-wire bus.
 *
 * The 28- and 32-pin derivatives have no PORTB, and of the four ports they do have - A, C, D and
 * F - every one is spoken for already. So there is no default left to pick for them and they stop
 * here, with a sentence, rather than thirty lines further down in a handful of errors about names
 * that were never declared. Naming the letter beforehand is the way past it: PORTD once the supply
 * switch is known not to be fitted, or any port a particular board has kept free.
 */
#if (TIMER_USE_TCA0 == STD_ON) && !defined(TIMER_TCA0_ROUTE_PORT)
    #if defined(PORTB)
        #define TIMER_TCA0_ROUTE_PORT           B
    #else
        #error "Timer.h: this derivative has no PORTB for TCA0 to drive. Define TIMER_TCA0_ROUTE_PORT as the letter of a port this board keeps free, or set TIMER_USE_TCA0 to STD_OFF."
    #endif
#endif

#if (TIMER_USE_TCA1 == STD_ON)
    #if !defined(TCA1)
        #error "Timer.h: this derivative has no TCA1 - it is fitted to the 48- and 64-pin parts only. Set TIMER_USE_TCA1 to STD_OFF."
    #endif
    #if !defined(TIMER_TCA1_ROUTE_PORT)
        #define TIMER_TCA1_ROUTE_PORT           B
    #endif
#endif

/*
 * The letter above turned into the two names it stands for. Two steps each, because the letter is
 * itself a macro and has to be expanded before it is pasted rather than after.
 *
 * One route macro per timer, where one taking the timer as a second argument would read better:
 * an argument is expanded on the way through, and TCA0 is a macro of its own - so it arrives as
 * "(*(TCA_t *) 0x0A00)" and pasting PORTMUX_ onto an opening bracket is where it stops. Spelled
 * into the macro body instead, the timer is plain text and never expanded at all.
 */
#define TIMER_PORT(Letter)                      TIMER_PORT_PASTED(Letter)
#define TIMER_PORT_PASTED(Letter)               PORT##Letter
#define TIMER_ROUTE_TCA0(Letter)                TIMER_ROUTE_TCA0_PASTED(Letter)
#define TIMER_ROUTE_TCA0_PASTED(Letter)         PORTMUX_TCA0_PORT##Letter##_gc
#define TIMER_ROUTE_TCA1(Letter)                TIMER_ROUTE_TCA1_PASTED(Letter)
#define TIMER_ROUTE_TCA1_PASTED(Letter)         PORTMUX_TCA1_PORT##Letter##_gc

#if (TIMER_USE_TCA0 == STD_ON)
struct TimerA0
{
    using RegisterType = TCA_SINGLE_t;
    static RegisterType& GetTimer() { return TCA0.SINGLE; }
    static PORT_t& GetPort() { return TIMER_PORT(TIMER_TCA0_ROUTE_PORT); }
    static constexpr uint8_t kRouteMask{PORTMUX_TCA0_gm};
    static constexpr uint8_t kRouteGroup{TIMER_ROUTE_TCA0(TIMER_TCA0_ROUTE_PORT)};
    static constexpr uint8_t kNumberOfChannels{3u};
    static constexpr uint8_t kFirstPin{0u};
};
#endif

/*
 * Guarded, and not only the instantiation down in the .cpp: these member bodies are compiled wherever
 * the header is read, so a TCA1 the derivative has never heard of is an error on a part that was
 * never asked to use one. That is what the first cut of this file got wrong.
 *
 * TCA1 reaches PORTB and PORTC and nothing else, and PC0/PC1 are the console - so PORTB is its only
 * free route, the very one TCA0 is given above. Having both therefore means moving one of the two,
 * which is what the letters are for; that is why this is off rather than the pair being on.
 */
#if (TIMER_USE_TCA1 == STD_ON)
struct TimerA1
{
    using RegisterType = TCA_SINGLE_t;
    static RegisterType& GetTimer() { return TCA1.SINGLE; }
    static PORT_t& GetPort() { return TIMER_PORT(TIMER_TCA1_ROUTE_PORT); }
    static constexpr uint8_t kRouteMask{PORTMUX_TCA1_gm};
    static constexpr uint8_t kRouteGroup{TIMER_ROUTE_TCA1(TIMER_TCA1_ROUTE_PORT)};
    static constexpr uint8_t kNumberOfChannels{3u};
    static constexpr uint8_t kFirstPin{0u};
};
#endif

template <typename TimerTraits>
class Timer
{
  public:
    /* Timer ISR callback function */
    typedef void (*TimerIsrCallbackFVoid)(void);

    /* Type which describes the internal state of the TIMER */
    enum class StateType {
        kNone,
        kInit,
        kReady,
        kRunning,
        kStopped
    };

    /*
     * Type which includes the values of the Clock Select Bit Group.
     * One more step than the xmega offered: the Dx prescaler has a 16 between 8 and 64, which lets
     * a period that used to round up to the 64 keep four times the resolution. There is no OFF
     * here, though - this CLKSEL has no such group config, and the timer is stopped by taking
     * ENABLE away instead.
     */
    enum class TimerClockSelectType {
        kNoPrescaler = TCA_SINGLE_CLKSEL_DIV1_gc,
        kPrescale2 = TCA_SINGLE_CLKSEL_DIV2_gc,
        kPrescale4 = TCA_SINGLE_CLKSEL_DIV4_gc,
        kPrescale8 = TCA_SINGLE_CLKSEL_DIV8_gc,
        kPrescale16 = TCA_SINGLE_CLKSEL_DIV16_gc,
        kPrescale64 = TCA_SINGLE_CLKSEL_DIV64_gc,
        kPrescale256 = TCA_SINGLE_CLKSEL_DIV256_gc,
        kPrescale1024 = TCA_SINGLE_CLKSEL_DIV1024_gc
    };

    /*
     * Type which defines the compare channels, each drives one PWM pin.
     * Numbered, where the xmega lettered them A..D: the registers these reach are CMP0..CMP2 and
     * the pins WO0..WO2, so carrying the letters over would mean translating at every look into
     * the data sheet.
     */
    enum class TimerChannelType {
        kChannel0,
        kChannel1,
        kChannel2
    };

    enum class TimerCountDirection {
        kUp,
        kDown
    };

    /* PWM duty resolution, the same 10 bit TimerOne offers */
    static constexpr uint16_t kPwmDutyMax{1023u};

  private:
    /*
     * The following are relied on by the shift and the pointer arithmetic further down,
     * which is cheaper than a per-channel table but only holds while the registers stay laid out
     * the way they are. The xmega version had the equivalent pair of assertions about TC4 and TC5
     * sharing a bit layout.
     */
    static_assert(TCA_SINGLE_CMP1EN_bm == (TCA_SINGLE_CMP0EN_bm << 1u) &&
                  TCA_SINGLE_CMP2EN_bm == (TCA_SINGLE_CMP0EN_bm << 2u),
                  "compare enable bits are not consecutive");
    static_assert(offsetof(TCA_SINGLE_t, CMP1BUF) - offsetof(TCA_SINGLE_t, CMP0BUF) == sizeof(uint16_t) &&
                  offsetof(TCA_SINGLE_t, CMP2BUF) - offsetof(TCA_SINGLE_t, CMP1BUF) == sizeof(uint16_t),
                  "compare buffer registers are not consecutive");
    static_assert(offsetof(TCA_SINGLE_t, CMP1) - offsetof(TCA_SINGLE_t, CMP0) == sizeof(uint16_t) &&
                  offsetof(TCA_SINGLE_t, CMP2) - offsetof(TCA_SINGLE_t, CMP1) == sizeof(uint16_t),
                  "compare registers are not consecutive");

    /* SetPeriod() and Read() convert with whole timer cycles per half and per whole microsecond */
    static_assert(F_CPU % 2000000uL == 0u, "F_CPU has to be a multiple of 2 MHz");

    static constexpr uint8_t kPwmDutyBits{10u};

    StateType state_;
    TimerClockSelectType clock_select_bit_group_;
    TimerIsrCallbackFVoid timer_overflow_callback_;
    /*
     * What PER and the compare channels are meant to hold. Kept here rather than read back from the
     * registers, because a value on its way through a buffer register is not in them until BOTTOM,
     * and because a duty is a fraction of the period that has to be applied again when it changes.
     * Top starts at PER's reset value, which is what an Init() with a refused period leaves.
     */
    uint16_t top_;
    uint16_t pwm_duty_[TimerTraits::kNumberOfChannels];

    constexpr Timer() : state_(StateType::kNone), clock_select_bit_group_(TimerClockSelectType::kNoPrescaler), timer_overflow_callback_(nullptr), top_(0xFFFFu), pwm_duty_{} { }

    // functions
    static typename TimerTraits::RegisterType& GetTimer() { return TimerTraits::GetTimer(); }

    static constexpr uint8_t ToPinMask(TimerChannelType channel) {
        return static_cast<uint8_t>(1u << (TimerTraits::kFirstPin + ToUnderlying(channel)));
    }
    static constexpr uint8_t ToChannelEnableMask(TimerChannelType channel) {
        return static_cast<uint8_t>(TCA_SINGLE_CMP0EN_bm << ToUnderlying(channel));
    }
    static constexpr uint16_t ToCompareValue(uint16_t top, uint16_t duty) {
        return static_cast<uint16_t>((static_cast<uint32_t>(top) * duty) >> kPwmDutyBits);
    }
    /*
     * Shifted before it is divided: the other way round the truncation is scaled up by the
     * prescaler, and at 1024 Read() moved in steps of 1024 us where a tick is 43 us.
     * 2 * 0xFFFF << 10 is still well inside 32 bit.
     */
    static constexpr uint32_t ToMicroseconds(uint32_t timer_cycles, TimerClockSelectType clock_select) {
        return (timer_cycles << ToPrescalerShift(clock_select)) / (F_CPU / 1000000uL);
    }
    static constexpr uint8_t ToPrescalerShift(TimerClockSelectType clock_select) {
        switch(clock_select) {
            case TimerClockSelectType::kPrescale2:       return 1u;
            case TimerClockSelectType::kPrescale4:       return 2u;
            case TimerClockSelectType::kPrescale8:       return 3u;
            case TimerClockSelectType::kPrescale16:      return 4u;
            case TimerClockSelectType::kPrescale64:      return 6u;
            case TimerClockSelectType::kPrescale256:     return 8u;
            case TimerClockSelectType::kPrescale1024:    return 10u;
            default:                            return 0u;
        }
    }

    static TimerClockSelectType ToClockSelect(uint32_t);
    TimerCountDirection GetCountingDirection();
    void SetClockSelect(TimerClockSelectType);
    void SetRunning(bool);
    void SetCompareBuffer(TimerChannelType);
    void LoadPeriodAndCompare();
    StdReturnType EnablePwmChannel(TimerChannelType, uint16_t);
    StdReturnType DisablePwmChannel(TimerChannelType);
    StdReturnType SetPwmDutyChannel(TimerChannelType, uint16_t);

  public:
    // methods
    static Timer& GetInstance() {
        static Timer singleton_instance;
        return singleton_instance;
    }
    StdReturnType Init(uint32_t = 1000u, TimerIsrCallbackFVoid = nullptr);
    StdReturnType SetPeriod(uint32_t);
    StdReturnType Start();
    void Stop();
    StdReturnType Resume();
    StdReturnType AttachInterrupt(TimerIsrCallbackFVoid);
    void DetachInterrupt();
    StdReturnType Read(uint32_t&);
    void HandleOverflowInterrupt();

    // The channel is a template argument so that a channel the timer does not have fails to compile
    // instead of writing into reserved registers at run time
    template <TimerChannelType Channel> StdReturnType EnablePwm(uint16_t duty) {
        static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels, "this timer has no such compare channel");
        return EnablePwmChannel(Channel, duty);
    }
    template <TimerChannelType Channel> StdReturnType DisablePwm() {
        static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels, "this timer has no such compare channel");
        return DisablePwmChannel(Channel);
    }
    template <TimerChannelType Channel> StdReturnType SetPwmDuty(uint16_t duty) {
        static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels, "this timer has no such compare channel");
        return SetPwmDutyChannel(Channel, duty);
    }
};

#endif
