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

#ifndef WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_TIMER_H_
#define WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_TIMER_H_

#include <avr/io.h>
#include <stddef.h>

#include "StandardTypes.h"

/*!
 * \brief STD_ON gives TCA0 an instance, `Timer<TimerA0>`, and an overflow ISR.
 *
 * Which timers get an instance and an overflow ISR. Both are free here - unlike on the xmega, where
 * the core owned TCD5 for millis() and drove analogWrite() from TCC4 - so the choice is only about
 * the flash an unused ISR costs. TCA1 is fitted to the 48- and 64-pin derivatives alone.
 */
#define TIMER_USE_TCA0 STD_ON
/*! \brief STD_ON gives TCA1 an instance and an overflow ISR, as TIMER_USE_TCA0 does for TCA0. */
#define TIMER_USE_TCA1 STD_OFF

#if (TIMER_USE_TCA0 == STD_ON) && !defined(TIMER_TCA0_ROUTE_PORT)
#ifdef PORTB
/*!
 * \brief The letter of the port that carries TCA0's waveform outputs: B, unless the build defines
 *        another.
 *
 * Named by its letter alone: WO0..WO2 are pins 0..2 of that port, and both the PORT_t and
 * the PORTMUX group config are pasted from the letter. One letter rather than the two of
 * them written side by side, because they are a single decision and two macros are two
 * things that can be made to disagree.
 *
 * PORTB, and not the PORTA the route comes up on from reset: PA0 carries USART0 TXD into
 * the CCL and PA2 its XCK into the event system, which is how the strip gets its pulse
 * widths (WS2812.h). A timer left on its default route would drive both of them. PORTD is
 * left alone because PowerSwitch.h claims it, and PORTC and PORTF are the console and the
 * two-wire bus.
 *
 * The 28- and 32-pin derivatives have no PORTB, and of the four ports they do have - A, C,
 * D and F - every one is spoken for already. So there is no default left to pick for them
 * and they stop here, with a sentence, rather than thirty lines further down in a handful
 * of errors about names that were never declared. Naming the letter beforehand is the way
 * past it: PORTD once the supply switch is known not to be fitted, or any port a
 * particular board has kept free.
 */
#define TIMER_TCA0_ROUTE_PORT B
#else
#error \
    "Timer.h: this derivative has no PORTB for TCA0 to drive. Define TIMER_TCA0_ROUTE_PORT as the letter of a port this board keeps free, or set TIMER_USE_TCA0 to STD_OFF."  // NOLINT(whitespace/line_length)
#endif
#endif

#if (TIMER_USE_TCA1 == STD_ON)
#if !defined(TCA1)
#error \
    "Timer.h: this derivative has no TCA1 - it is fitted to the 48- and 64-pin parts only. Set TIMER_USE_TCA1 to STD_OFF."  // NOLINT(whitespace/line_length)
#endif
#if !defined(TIMER_TCA1_ROUTE_PORT)
/*!
 * \brief The letter of the port that carries TCA1's waveform outputs: B, unless the build defines
 *        another; see TIMER_TCA0_ROUTE_PORT.
 */
#define TIMER_TCA1_ROUTE_PORT B
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
/*!
 * \brief Returns the port that `Letter` names, as a `PORT_t`: `PORTB` for B.
 *
 * \param[in] Letter The letter of the port, or a macro that expands to it.
 */
#define TIMER_PORT(Letter) TIMER_PORT_PASTED(Letter)
/*!
 * \brief Pastes `Letter` onto `PORT`: the step of TIMER_PORT() after `Letter` is expanded.
 *
 * \param[in] Letter The letter of the port.
 */
#define TIMER_PORT_PASTED(Letter) PORT##Letter
/*!
 * \brief Returns the group config of `PORTMUX.TCAROUTEA` that routes TCA0 to the port `Letter`
 *        names.
 *
 * \param[in] Letter The letter of the port, or a macro that expands to it.
 */
#define TIMER_ROUTE_TCA0(Letter) TIMER_ROUTE_TCA0_PASTED(Letter)
/*!
 * \brief Pastes `Letter` into the name of TCA0's route: the step of TIMER_ROUTE_TCA0() after
 *        `Letter` is expanded.
 *
 * \param[in] Letter The letter of the port.
 */
#define TIMER_ROUTE_TCA0_PASTED(Letter) PORTMUX_TCA0_PORT##Letter##_gc
/*!
 * \brief Returns the group config of `PORTMUX.TCAROUTEA` that routes TCA1 to the port `Letter`
 *        names.
 *
 * \param[in] Letter The letter of the port, or a macro that expands to it.
 */
#define TIMER_ROUTE_TCA1(Letter) TIMER_ROUTE_TCA1_PASTED(Letter)
/*!
 * \brief Pastes `Letter` into the name of TCA1's route: the step of TIMER_ROUTE_TCA1() after
 *        `Letter` is expanded.
 *
 * \param[in] Letter The letter of the port.
 */
#define TIMER_ROUTE_TCA1_PASTED(Letter) PORTMUX_TCA1_PORT##Letter##_gc

#if (TIMER_USE_TCA0 == STD_ON)
/*!
 * \brief The traits of TCA0 for `Timer`: its registers, and the port and pins its waveform outputs
 *        are routed to.
 */
struct TimerA0 {
  using RegisterType = TCA_SINGLE_t; /*!< The registers of a type A timer in single mode. */
  /*! \brief Returns the registers of TCA0, in single mode. */
  static RegisterType& GetTimer() { return TCA0.SINGLE; }
  /*! \brief Returns the port that TIMER_TCA0_ROUTE_PORT names. */
  static PORT_t& GetPort() { return TIMER_PORT(TIMER_TCA0_ROUTE_PORT); }
  /*! \brief The bits of TCA0's route in `PORTMUX.TCAROUTEA`. */
  static constexpr uint8_t kRouteMask{PORTMUX_TCA0_gm};
  /*! \brief The route to the port that TIMER_TCA0_ROUTE_PORT names, within kRouteMask. */
  static constexpr uint8_t kRouteGroup{TIMER_ROUTE_TCA0(TIMER_TCA0_ROUTE_PORT)};
  static constexpr uint8_t kNumberOfChannels{3u}; /*!< The compare channels of the timer. */
  static constexpr uint8_t kFirstPin{0u}; /*!< The pin of WO0; WO1 and WO2 are the next two. */
};
#endif

/*
 * Guarded, and not only the instantiation down in the .cpp: these member bodies are compiled
 * wherever the header is read, so a TCA1 the derivative has never heard of is an error on a part
 * that was never asked to use one. That is what the first cut of this file got wrong.
 *
 * TCA1 reaches PORTB and PORTC and nothing else, and PC0/PC1 are the console - so PORTB is its only
 * free route, the very one TCA0 is given above. Having both therefore means moving one of the two,
 * which is what the letters are for; that is why this is off rather than the pair being on.
 */
#if (TIMER_USE_TCA1 == STD_ON)
/*!
 * \brief The traits of TCA1 for `Timer`: its registers, and the port and pins its waveform outputs
 *        are routed to.
 */
struct TimerA1 {
  using RegisterType = TCA_SINGLE_t; /*!< The registers of a type A timer in single mode. */
  /*! \brief Returns the registers of TCA1, in single mode. */
  static RegisterType& GetTimer() { return TCA1.SINGLE; }
  /*! \brief Returns the port that TIMER_TCA1_ROUTE_PORT names. */
  static PORT_t& GetPort() { return TIMER_PORT(TIMER_TCA1_ROUTE_PORT); }
  /*! \brief The bits of TCA1's route in `PORTMUX.TCAROUTEA`. */
  static constexpr uint8_t kRouteMask{PORTMUX_TCA1_gm};
  /*! \brief The route to the port that TIMER_TCA1_ROUTE_PORT names, within kRouteMask. */
  static constexpr uint8_t kRouteGroup{TIMER_ROUTE_TCA1(TIMER_TCA1_ROUTE_PORT)};
  static constexpr uint8_t kNumberOfChannels{3u}; /*!< The compare channels of the timer. */
  static constexpr uint8_t kFirstPin{0u}; /*!< The pin of WO0; WO1 and WO2 are the next two. */
};
#endif

/*!
 * \brief A type A timer: a periodic overflow interrupt, and a phase-correct PWM on each of its
 *        compare channels.
 *
 * There is one instance per timer, GetInstance(). Init() brings the timer up stopped, with a period
 * and a callback, and Start() runs it from zero. It counts dual-slope, from BOTTOM up to TOP and
 * down again, and interrupts at BOTTOM.
 *
 * EnablePwm() puts a channel's PWM on its pin: WO0, WO1 and WO2 are `TimerTraits::kFirstPin` and
 * the two pins after it of `TimerTraits::GetPort()`. F_CPU has to be a multiple of 2 MHz, which the
 * class asserts.
 *
 * \tparam TimerTraits The timer and the port its outputs are routed to: TimerA0 or TimerA1.
 */
template <typename TimerTraits>
class Timer {
 public:
  /*! \brief A function the overflow ISR calls: no arguments, no result. */
  using TimerIsrCallbackFVoid = void (*)();

  /*! \brief The state of the timer, which Init(), Start(), Stop(), Resume() and Read() check. */
  enum class StateType {
    kNone,    /*!< Before Init(). */
    kInit,    /*!< While Init() runs. */
    kReady,   /*!< Initialised, and not started yet. */
    kRunning, /*!< Counting, since Start() or Resume(). */
    kStopped  /*!< Stopped by Stop() while it ran. */
  };

  /*!
   * \brief The prescaler: what the peripheral clock is divided by to clock the timer.
   *
   * One more step than the xmega offered: the Dx prescaler has a 16 between 8 and 64, which lets
   * a period that used to round up to the 64 keep four times the resolution. There is no OFF
   * here, though - this CLKSEL has no such group config, and the timer is stopped by taking
   * ENABLE away instead.
   */
  enum class TimerClockSelectType {
    kNoPrescaler = TCA_SINGLE_CLKSEL_DIV1_gc,    /*!< The peripheral clock, undivided. */
    kPrescale2 = TCA_SINGLE_CLKSEL_DIV2_gc,      /*!< The peripheral clock divided by 2. */
    kPrescale4 = TCA_SINGLE_CLKSEL_DIV4_gc,      /*!< The peripheral clock divided by 4. */
    kPrescale8 = TCA_SINGLE_CLKSEL_DIV8_gc,      /*!< The peripheral clock divided by 8. */
    kPrescale16 = TCA_SINGLE_CLKSEL_DIV16_gc,    /*!< The peripheral clock divided by 16. */
    kPrescale64 = TCA_SINGLE_CLKSEL_DIV64_gc,    /*!< The peripheral clock divided by 64. */
    kPrescale256 = TCA_SINGLE_CLKSEL_DIV256_gc,  /*!< The peripheral clock divided by 256. */
    kPrescale1024 = TCA_SINGLE_CLKSEL_DIV1024_gc /*!< The peripheral clock divided by 1024. */
  };

  /*!
   * \brief A compare channel, each of which drives one PWM pin.
   *
   * Numbered, where the xmega lettered them A..D: the registers these reach are CMP0..CMP2 and
   * the pins WO0..WO2, so carrying the letters over would mean translating at every look into
   * the data sheet.
   */
  enum class TimerChannelType {
    kChannel0, /*!< CMP0, which drives WO0. */
    kChannel1, /*!< CMP1, which drives WO1. */
    kChannel2  /*!< CMP2, which drives WO2. */
  };

  /*! \brief The direction the counter runs in. */
  enum class TimerCountDirection {
    kUp,  /*!< From BOTTOM up to TOP. */
    kDown /*!< From TOP down to BOTTOM. */
  };

  /*! \brief The largest duty: a duty has 10 bit, the same resolution TimerOne offers. */
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
  static_assert(offsetof(TCA_SINGLE_t, CMP1BUF) - offsetof(TCA_SINGLE_t, CMP0BUF) ==
                        sizeof(uint16_t) &&
                    offsetof(TCA_SINGLE_t, CMP2BUF) - offsetof(TCA_SINGLE_t, CMP1BUF) ==
                        sizeof(uint16_t),
                "compare buffer registers are not consecutive");
  static_assert(offsetof(TCA_SINGLE_t, CMP1) - offsetof(TCA_SINGLE_t, CMP0) == sizeof(uint16_t) &&
                    offsetof(TCA_SINGLE_t, CMP2) - offsetof(TCA_SINGLE_t, CMP1) == sizeof(uint16_t),
                "compare registers are not consecutive");

  /* SetPeriod() and Read() convert with whole timer cycles per half and per whole microsecond */
  static_assert(F_CPU % 2000000uL == 0u, "F_CPU has to be a multiple of 2 MHz");

  static constexpr uint8_t kPwmDutyBits{10u};

  StateType state_;
  TimerClockSelectType clock_select_bit_group_;
  TimerIsrCallbackFVoid timer_overflow_callback_{nullptr};
  /*
   * What PER and the compare channels are meant to hold. Kept here rather than read back from the
   * registers, because a value on its way through a buffer register is not in them until BOTTOM,
   * and because a duty is a fraction of the period that has to be applied again when it changes.
   * Top starts at PER's reset value, which is what an Init() with a refused period leaves.
   */
  uint16_t top_{0xFFFFu};
  uint16_t pwm_duty_[TimerTraits::kNumberOfChannels]{};

  constexpr Timer()
      : state_(StateType::kNone), clock_select_bit_group_(TimerClockSelectType::kNoPrescaler) {}

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
  static constexpr uint32_t ToMicroseconds(uint32_t timer_cycles,
                                           TimerClockSelectType clock_select) {
    return (timer_cycles << ToPrescalerShift(clock_select)) / (F_CPU / 1000000uL);
  }
  static constexpr uint8_t ToPrescalerShift(TimerClockSelectType clock_select) {
    switch (clock_select) {
      case TimerClockSelectType::kPrescale2:
        return 1u;
      case TimerClockSelectType::kPrescale4:
        return 2u;
      case TimerClockSelectType::kPrescale8:
        return 3u;
      case TimerClockSelectType::kPrescale16:
        return 4u;
      case TimerClockSelectType::kPrescale64:
        return 6u;
      case TimerClockSelectType::kPrescale256:
        return 8u;
      case TimerClockSelectType::kPrescale1024:
        return 10u;
      default:
        return 0u;
    }
  }

  static TimerClockSelectType ToClockSelect(uint32_t timer_cycles);
  TimerCountDirection GetCountingDirection();
  void SetClockSelect(TimerClockSelectType clock_select);
  void SetRunning(bool running);
  void SetCompareBuffer(TimerChannelType channel);
  void LoadPeriodAndCompare();
  StdReturnType EnablePwmChannel(TimerChannelType channel, uint16_t duty);
  StdReturnType DisablePwmChannel(TimerChannelType channel);
  StdReturnType SetPwmDutyChannel(TimerChannelType channel, uint16_t duty);

 public:
  /*! \brief Returns the one timer that `TimerTraits` names. */
  static Timer& GetInstance() {
    static Timer singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Brings the timer up, stopped, with a period and optionally a callback.
   *
   * It disables the timer, resets its registers, routes its waveform outputs to
   * `TimerTraits::GetPort()` and sets dual-slope counting with the overflow at BOTTOM. Then it
   * sets the period as SetPeriod() does and, unless `timer_overflow_callback` is `nullptr`,
   * attaches the callback as AttachInterrupt() does. The timer counts once Start() is called.
   *
   * Only the first call acts: nothing brings the timer back to the state before Init().
   *
   * \param[in] microseconds The period of the overflow interrupt in microseconds, as SetPeriod()
   *                         takes it.
   * \param[in] timer_overflow_callback The function the overflow ISR calls, or `nullptr` for none.
   * \return kEOk, or kENotOk in two cases. If the timer was initialised already, nothing is
   *         changed. If SetPeriod() refused `microseconds`, the timer is initialised all the same,
   *         with TOP at 0xFFFF and no prescaler, and the callback is attached.
   */
  StdReturnType Init(uint32_t microseconds = 1000u,
                     TimerIsrCallbackFVoid timer_overflow_callback = nullptr);

  /*!
   * \brief Sets the period of the overflow interrupt.
   *
   * It takes the smallest prescaler under which half the period fits into the 16 bits of TOP,
   * because every step up halves the resolution of TOP and of the compare values derived from it,
   * and rounds TOP down to whole timer cycles. TOP and the compare values, recomputed from each
   * channel's duty, go into the buffer registers with interrupts off; the timer takes them over
   * at the next BOTTOM, and Start() loads them directly. While the timer runs, the prescaler is
   * written to `CTRLA` at once.
   *
   * It does not check the state, so it acts before Init() as well.
   *
   * \param[in] microseconds The period in microseconds: above 0, and below what the 16 bits reach
   *                         at the prescaler 1024, counted up and down; at 24 MHz the longest is
   *                         5591039.
   * \return kEOk, or kENotOk if `microseconds` is out of that range; the period is then left as it
   *         was.
   * \warning A stopped timer keeps the prescaler it ran on, and Resume() does not write a new one.
   *          After Stop(), a SetPeriod() that changes the prescaler, and Resume(), the timer counts
   *          the new TOP at the old prescaler, and Read() converts at the new one. Start() writes
   *          the new prescaler.
   */
  StdReturnType SetPeriod(uint32_t microseconds);

  /*!
   * \brief Starts the timer from zero.
   *
   * It loads TOP and the compare values straight into `PER` and `CMPn`, past their buffers,
   * writes the prescaler and restarts the counter, so the first period is a whole one of the
   * period set. A running timer starts over.
   *
   * \return kEOk, or kENotOk before Init(); the timer is then left as it was.
   */
  StdReturnType Start();

  /*!
   * \brief Stops the timer where it is.
   *
   * It takes ENABLE away and keeps the counter, the prescaler and the interrupt. A running timer
   * is then stopped, and Resume() continues it; a timer not yet started stays ready. Before
   * Init() it does nothing.
   */
  void Stop();

  /*!
   * \brief Continues a stopped timer where it stopped.
   *
   * \return kEOk, or kENotOk if Stop() has not stopped a running timer; the timer is then left as
   *         it was.
   * \warning The timer continues on the prescaler it stopped on, see SetPeriod().
   */
  StdReturnType Resume();

  /*!
   * \brief Installs the callback the overflow ISR calls, and enables the overflow interrupt.
   *
   * An overflow flag left set from while the interrupt was off is cleared first, so the new
   * callback is not called at once for an overflow from before. It does not check the state.
   *
   * The xmega put this interrupt on the PMIC's low level so that the core's millis() could
   * interrupt the callback. The Dx has no such pair of levels - it has one `CPUINT.LVL1VEC`, and
   * WS2812::Init() has given it to `USART0_DRE`, which is what keeps the strip's bytes flowing
   * inside its reset gap. Taking it away for this would corrupt a frame, so the overflow stays at
   * level 0, and no other handler of level 0 - the millisecond time base and the console among
   * them - can run while the callback does. Keep it short.
   *
   * \param[in] timer_overflow_callback The function the overflow ISR calls.
   * \return kEOk, or kENotOk if `timer_overflow_callback` is `nullptr`; the callback and the
   *         interrupt are then left as they were.
   */
  StdReturnType AttachInterrupt(TimerIsrCallbackFVoid timer_overflow_callback);

  /*! \brief Disables the overflow interrupt and removes the callback; the timer counts on. */
  void DetachInterrupt();

  /*!
   * \brief Reads the time since the last overflow, at BOTTOM, in microseconds.
   *
   * The counter, `PER` and the direction are read with interrupts off, so that a turn at TOP in
   * between cannot add a whole period. On the way down, the time is TOP plus what the counter
   * has come down from it. The result is rounded down to whole microseconds.
   *
   * \param[out] microseconds The time since the last overflow, in microseconds.
   * \return kEOk, or kENotOk if the timer is neither running nor stopped; `microseconds` is then
   *         left as it was.
   */
  StdReturnType Read(uint32_t& microseconds);

  /*!
   * \brief Serves the overflow interrupt: clears its flag, then calls the callback if there is one.
   *
   * Called from the overflow ISR of this timer, and by nothing else. The flag is not cleared by
   * the vector executing, on this part no more than on the xmega's TC4/TC5; without the explicit
   * clear the ISR is entered again at once. Writing the bit is what clears it here.
   */
  void HandleOverflowInterrupt();

  /*!
   * \brief Sets the duty of a compare channel and puts the channel's PWM on its pin.
   *
   * It sets the duty as SetPwmDuty() does, then makes the pin an output and enables the channel's
   * compare output. The channel is a template argument so that a channel the timer does not have
   * fails to compile instead of writing into reserved registers at run time.
   *
   * \tparam Channel The compare channel.
   * \param[in] duty The duty, from 0 to kPwmDutyMax.
   * \return kEOk, or kENotOk if `duty` is above kPwmDutyMax; the channel is then left as it was.
   * \pre Init() has routed the outputs away from PORTA, where PA0 and PA2 carry the strip; this
   *      does not check the state.
   */
  template <TimerChannelType Channel>
  StdReturnType EnablePwm(uint16_t duty) {
    static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels,
                  "this timer has no such compare channel");
    return EnablePwmChannel(Channel, duty);
  }

  /*!
   * \brief Takes the PWM off a compare channel's pin.
   *
   * The pin stays an output, and shows its `PORT.OUT` value instead of the waveform. The duty is
   * kept, and SetPeriod() still recomputes the channel's compare value from it.
   *
   * \tparam Channel The compare channel; one the timer does not have fails to compile.
   * \return kEOk, always.
   */
  template <TimerChannelType Channel>
  StdReturnType DisablePwm() {
    static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels,
                  "this timer has no such compare channel");
    return DisablePwmChannel(Channel);
  }

  /*!
   * \brief Sets the duty of a compare channel.
   *
   * Written to the buffer register, with interrupts off, so the new compare value takes effect at
   * the next BOTTOM and a running PWM never sees half a period of each. The compare value is TOP
   * times `duty` over 1024, rounded down, so the one of kPwmDutyMax stays just below TOP. A
   * channel whose PWM is not enabled takes the duty as well, and starts with it.
   *
   * \tparam Channel The compare channel; one the timer does not have fails to compile.
   * \param[in] duty The duty, from 0 to kPwmDutyMax.
   * \return kEOk, or kENotOk if `duty` is above kPwmDutyMax; the duty is then left as it was.
   */
  template <TimerChannelType Channel>
  StdReturnType SetPwmDuty(uint16_t duty) {
    static_assert(ToUnderlying(Channel) < TimerTraits::kNumberOfChannels,
                  "this timer has no such compare channel");
    return SetPwmDutyChannel(Channel, duty);
  }
};

#endif  // WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_TIMER_H_
