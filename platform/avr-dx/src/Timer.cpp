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
/**     \file       Timer.cpp
 *      \brief      16-bit Timer/Counter type A as periodic interrupt and PWM source
 *
 *      \details    The member functions are instantiated here, once per timer enabled in Timer.h,
 *                  next to the overflow ISR of that timer.
 *
******************************************************************************************************************************************************/
#define _TIMER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Timer.h"
#include "Bit.h"
#include <avr/interrupt.h>
#include <util/atomic.h>

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* A type A timer counts 16 bit in the single-slope and dual-slope modes this driver uses */
#define TIMER_NUMBER_OF_BITS                        16u
#define TIMER_RESOLUTION                            (1uL << TIMER_NUMBER_OF_BITS)
#define TIMER_MAX_PRESCALER                         1024u
/* the longest period a 16-bit TOP reaches at the largest prescaler, counted up and down again */
#define TIMER_MAX_PERIOD                            ((TIMER_RESOLUTION / (F_CPU / 1000000uL)) * TIMER_MAX_PRESCALER * 2u)

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Init()
******************************************************************************************************************************************************/
/*! \brief          bring the timer up, stopped, with a period and optionally a callback
 *  \param[in]      microseconds                period of the timer overflow interrupt
 *  \param[in]      timer_overflow_callback      called from the overflow ISR, or nullptr for none
 *  \return         kEOk
 *                  kENotOk
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::Init(uint32_t microseconds, TimerIsrCallbackFVoid timer_overflow_callback)
{
    StdReturnType return_value{kENotOk};

    if(StateType::kNone == state_) {
        return_value = kEOk;
        state_ = StateType::kInit;

        /*
         * The Dx hands this timer over at its reset values rather than running, the way the
         * xmegaForArduino core used to - but RESET is kept anyway, so that a second Init() after a
         * DetachInterrupt() starts from the same place the first one did. The command is only
         * obeyed while the timer is disabled, hence the order.
         */
        SetRunning(false);
        GetTimer().CTRLESET = TCA_SINGLE_CMD_RESET_gc;

        // Away from the PORTA the route comes up on, before any channel can drive a pin: PA0 and
        // PA2 belong to the strip. See the traits in Timer.h.
        Bit<uint8_t>::WriteBitGroup(PORTMUX.TCAROUTEA, TimerTraits::kRouteMask, TimerTraits::kRouteGroup);

        /* dual-slope PWM, overflow interrupt at BOTTOM */
        Bit<uint8_t>::WriteBitGroup(GetTimer().CTRLB, TCA_SINGLE_WGMODE_gm, TCA_SINGLE_WGMODE_DSBOTTOM_gc);

        if(SetPeriod(microseconds) == kENotOk) return_value = kENotOk;
        if(timer_overflow_callback != nullptr) if(AttachInterrupt(timer_overflow_callback) == kENotOk) return_value = kENotOk;

        state_ = StateType::kReady;
    }
    return return_value;
} /* Init */


/******************************************************************************************************************************************************
  ToClockSelect()
******************************************************************************************************************************************************/
/*! \brief          The smallest prescaler that brings a period inside the counter
 *  \details        Smallest, because every step up halves the resolution of TOP and of the
 *                  compare values derived from it. Each test shifts the undivided count, so
 *                  no step depends on what the one before it did to it.
 *  \param[in]      timer_cycles                 the period in undivided timer cycles
 *  \return         the clock select bit group
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
typename Timer<TimerTraits>::TimerClockSelectType Timer<TimerTraits>::ToClockSelect(uint32_t timer_cycles)
{
    if(timer_cycles < TIMER_RESOLUTION)          { return TimerClockSelectType::kNoPrescaler; }
    if((timer_cycles >> 1u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale2; }
    if((timer_cycles >> 2u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale4; }
    if((timer_cycles >> 3u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale8; }
    if((timer_cycles >> 4u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale16; }
    if((timer_cycles >> 6u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale64; }
    if((timer_cycles >> 8u) < TIMER_RESOLUTION)  { return TimerClockSelectType::kPrescale256; }
    /* TIMER_MAX_PERIOD in SetPeriod() is what keeps this one inside 16 bit */
    return TimerClockSelectType::kPrescale1024;
} /* ToClockSelect */


/******************************************************************************************************************************************************
  SetPeriod()
******************************************************************************************************************************************************/
/*! \brief          set period of the timer overflow interrupt
 *  \details        this functions sets the period of the timer overflow interrupt therefore
 *                  prescaler and timer top value will be calculated
 *  \param[in]      microseconds                period of the timer overflow interrupt
 *  \return         kEOk
 *                  kENotOk
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::SetPeriod(uint32_t microseconds)
{
    StdReturnType return_value{kENotOk};

    /* was request out of bounds? An out of range period is refused and changes nothing */
    if((microseconds > 0u) && (microseconds < TIMER_MAX_PERIOD)) {
        return_value = kEOk;
        /* calculate timer cycles to reach timer period, counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2 */
        uint32_t timer_cycles = (F_CPU / 2000000uL) * microseconds;
        clock_select_bit_group_ = ToClockSelect(timer_cycles);
        timer_cycles >>= ToPrescalerShift(clock_select_bit_group_);

        /*
         * A 16-bit access goes through the timer's shared TEMP register, so an ISR touching another
         * 16-bit register of this timer must not get in between.
         * Through the buffer rather than PER: a TOP that lands below the counter mid-period is not
         * picked up until the counter has gone all the way round 0xFFFF, which is one period of
         * several hundred milliseconds in the wrong place. PERBUF is taken over at BOTTOM, where
         * the counter is below any legal TOP - and that holds for a stopped timer as much as for a
         * running one, since Resume() continues mid-period. A timer not yet started has its
         * registers loaded by Start().
         * The compare values go along, recomputed from their duty, so the PWM keeps its ratio
         * across the change and takes the new period at the same BOTTOM.
         */
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            top_ = static_cast<uint16_t>(timer_cycles);
            GetTimer().PERBUF = top_;
            for(uint8_t channel = 0u; channel < TimerTraits::kNumberOfChannels; channel++) {
                SetCompareBuffer(static_cast<TimerChannelType>(channel));
            }
        }

        if(StateType::kRunning == state_) { SetClockSelect(clock_select_bit_group_); }
    }
    return return_value;
} /* SetPeriod */


/******************************************************************************************************************************************************
  Start()
******************************************************************************************************************************************************/
/*! \brief          start the timer from zero
 *  \return         kEOk
 *                  kENotOk        timer was not initialized
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::Start()
{
    StdReturnType return_value{kENotOk};

    if(StateType::kReady == state_ || StateType::kRunning == state_ || StateType::kStopped == state_) {
        return_value = kEOk;
        SetRunning(false);
        LoadPeriodAndCompare();
        SetClockSelect(clock_select_bit_group_);
        /* RESTART clears the counter and the direction, so the first period is a whole one */
        GetTimer().CTRLESET = TCA_SINGLE_CMD_RESTART_gc;
        SetRunning(true);
        state_ = StateType::kRunning;
    }
    return return_value;
} /* Start */


/******************************************************************************************************************************************************
  Stop()
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::Stop()
{
    /* before Init() the timer is not this class's to stop */
    if(StateType::kNone != state_) {
        SetRunning(false);
        if(StateType::kRunning == state_) { state_ = StateType::kStopped; }
    }
} /* Stop */


/******************************************************************************************************************************************************
  Resume()
******************************************************************************************************************************************************/
/*! \brief          continue a stopped timer where it stopped
 *  \return         kEOk
 *                  kENotOk        timer was not stopped
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::Resume()
{
    StdReturnType return_value{kENotOk};

    if(StateType::kStopped == state_) {
        return_value = kEOk;
        SetRunning(true);
        state_ = StateType::kRunning;
    }
    return return_value;
} /* Resume */


/******************************************************************************************************************************************************
  AttachInterrupt()
******************************************************************************************************************************************************/
/*! \brief          install the callback the overflow ISR calls
 *  \details        The xmega put this interrupt on the PMIC's low level so that the core's millis()
 *                  could interrupt the callback. The Dx has no such pair of levels - it has one
 *                  CPUINT.LVL1VEC, and WS2812::Init() has given it to USART0_DRE, which is what
 *                  keeps the strip's bytes flowing inside its reset gap. Taking it away for this
 *                  would corrupt a frame, so the overflow stays at level 0 and the callback runs
 *                  without being interruptible. Keep it short.
 *  \param[in]      timer_overflow_callback      called from the overflow ISR
 *  \return         kEOk
 *                  kENotOk                    no callback given
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::AttachInterrupt(TimerIsrCallbackFVoid timer_overflow_callback)
{
    StdReturnType return_value{kENotOk};

    if(timer_overflow_callback != nullptr) {
        return_value = kEOk;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { timer_overflow_callback_ = timer_overflow_callback; }
        /*
         * The flag is set at every BOTTOM whether the interrupt is on or not, so one left over from
         * while it was off would call the new callback at once. Only then: with the interrupt
         * already on, a set flag is a real overflow about to be served.
         */
        if(!(GetTimer().INTCTRL & TCA_SINGLE_OVF_bm)) { GetTimer().INTFLAGS = TCA_SINGLE_OVF_bm; }
        Bit<uint8_t>::SetBits(GetTimer().INTCTRL, TCA_SINGLE_OVF_bm);
    }
    return return_value;
} /* AttachInterrupt */


/******************************************************************************************************************************************************
  DetachInterrupt()
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::DetachInterrupt()
{
    Bit<uint8_t>::ClearBits(GetTimer().INTCTRL, TCA_SINGLE_OVF_bm);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { timer_overflow_callback_ = nullptr; }
} /* DetachInterrupt */


/******************************************************************************************************************************************************
  Read()
******************************************************************************************************************************************************/
/*! \brief          read current timer value in microseconds
 *  \details        this function returns the time since the last overflow interrupt in microseconds
 *
 *  \param[out]     microseconds        current timer value
 *  \return         kEOk
 *                  kENotOk
 *  \pre            Timer has to be in RUNNING or STOPPED STATE
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::Read(uint32_t& microseconds)
{
    StdReturnType return_value{kENotOk};

    if(StateType::kRunning == state_ || StateType::kStopped == state_) {
        uint32_t counter_value{0u};
        uint32_t top_value{0u};
        TimerCountDirection direction{TimerCountDirection::kUp};
        return_value = kEOk;

        // counter and direction have to come from the same instant, or a turn at TOP in between
        // reads as a whole period too much
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            counter_value = GetTimer().CNT;
            top_value = GetTimer().PER;
            direction = GetCountingDirection();
        }
        /* if counter counting down, add top value to current value */
        if(direction == TimerCountDirection::kDown) { counter_value = (top_value - counter_value) + top_value; }

        microseconds = ToMicroseconds(counter_value, clock_select_bit_group_);
    }
    return return_value;
} /* Read */


/******************************************************************************************************************************************************
  HandleOverflowInterrupt()
******************************************************************************************************************************************************/
/*! \brief          called from the overflow ISR of this timer
 *  \details        The flag is not cleared by the vector executing, on this part no more than on the
 *                  xmega's TC4/TC5; without the explicit clear the ISR is entered again at once.
 *                  Writing the bit is what clears it here.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::HandleOverflowInterrupt()
{
    GetTimer().INTFLAGS = TCA_SINGLE_OVF_bm;
    if(timer_overflow_callback_ != nullptr) { timer_overflow_callback_(); }
} /* HandleOverflowInterrupt */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  GetCountingDirection
******************************************************************************************************************************************************/
/*! \details        CTRLESET and CTRLECLR are one register seen twice: a write sets or clears the
 *                  bits it names, and a read of either gives the state. Reading the SET half keeps
 *                  this looking like the xmega's CTRLGSET.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
typename Timer<TimerTraits>::TimerCountDirection Timer<TimerTraits>::GetCountingDirection()
{
    if(GetTimer().CTRLESET & TCA_SINGLE_DIR_bm) { return TimerCountDirection::kDown; }
    else { return TimerCountDirection::kUp; }
} /* GetCountingDirection */


/******************************************************************************************************************************************************
  SetClockSelect
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::SetClockSelect(TimerClockSelectType clock_select)
{
    Bit<uint8_t>::WriteBitGroup(GetTimer().CTRLA, TCA_SINGLE_CLKSEL_gm, static_cast<uint8_t>(clock_select));
} /* SetClockSelect */


/******************************************************************************************************************************************************
  SetRunning
******************************************************************************************************************************************************/
/*! \details        Where the xmega stopped its timer by selecting no clock source, this one keeps
 *                  the prescaler and loses ENABLE: the Dx CLKSEL has no OFF to select.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::SetRunning(bool running)
{
    if(running) { Bit<uint8_t>::SetBits(GetTimer().CTRLA, TCA_SINGLE_ENABLE_bm); }
    else        { Bit<uint8_t>::ClearBits(GetTimer().CTRLA, TCA_SINGLE_ENABLE_bm); }
} /* SetRunning */


/******************************************************************************************************************************************************
  SetCompareBuffer
******************************************************************************************************************************************************/
/*! \pre            called with interrupts off, the write goes through the TEMP register
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::SetCompareBuffer(TimerChannelType channel)
{
    // CMP0BUF..CMP2BUF are consecutive, which Timer.h asserts; the channel was range checked at
    // compile time
    (&GetTimer().CMP0BUF)[ToUnderlying(channel)] = ToCompareValue(top_, pwm_duty_[ToUnderlying(channel)]);
} /* SetCompareBuffer */


/******************************************************************************************************************************************************
  LoadPeriodAndCompare
******************************************************************************************************************************************************/
/*! \details        Straight into PER and CMPn, past the buffers, which is only safe while the timer is
 *                  disabled. Without it the first period after Start() would run on whatever the
 *                  registers held: the buffers are taken over at BOTTOM, and the first BOTTOM after
 *                  RESTART is a whole period away - with CMPn at the 0 Init()'s RESET left there.
 *                  Not a forced UPDATE before the RESTART instead: both are written to CTRLESET,
 *                  and an UPDATE still pending there ORed with RESTART reads as RESET.
 *  \pre            timer disabled
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::LoadPeriodAndCompare()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        GetTimer().PER = top_;
        for(uint8_t channel = 0u; channel < TimerTraits::kNumberOfChannels; channel++) {
            /* CMP0..CMP2 are consecutive like their buffers, which Timer.h asserts as well */
            (&GetTimer().CMP0)[channel] = ToCompareValue(top_, pwm_duty_[channel]);
        }
    }
} /* LoadPeriodAndCompare */


/******************************************************************************************************************************************************
  EnablePwmChannel
******************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::EnablePwmChannel(TimerChannelType channel, uint16_t duty)
{
    StdReturnType return_value = SetPwmDutyChannel(channel, duty);

    if(kEOk == return_value) {
        TimerTraits::GetPort().DIRSET = ToPinMask(channel);
        Bit<uint8_t>::SetBits(GetTimer().CTRLB, ToChannelEnableMask(channel));
    }
    return return_value;
} /* EnablePwmChannel */


/******************************************************************************************************************************************************
  DisablePwmChannel
******************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::DisablePwmChannel(TimerChannelType channel)
{
    Bit<uint8_t>::ClearBits(GetTimer().CTRLB, ToChannelEnableMask(channel));
    /* the pin stays an output, it now shows its PORT.OUT value instead of the waveform */
    return kEOk;
} /* DisablePwmChannel */


/******************************************************************************************************************************************************
  SetPwmDutyChannel
******************************************************************************************************************************************************/
/*! \brief          set duty of one compare channel, 0 .. kPwmDutyMax
 *  \details        Written to the buffer register, so the new compare value takes effect at the next
 *                  BOTTOM and a running PWM never sees half a period of each.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::SetPwmDutyChannel(TimerChannelType channel, uint16_t duty)
{
    StdReturnType return_value{kENotOk};

    if(duty <= kPwmDutyMax) {
        return_value = kEOk;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            pwm_duty_[ToUnderlying(channel)] = duty;
            SetCompareBuffer(channel);
        }
    }
    return return_value;
} /* SetPwmDutyChannel */


/******************************************************************************************************************************************************
 * I N S T A N C E S   A N D   I N T E R R U P T S
******************************************************************************************************************************************************/
#if (TIMER_USE_TCA0 == STD_ON)
template class Timer<TimerA0>;
ISR(TCA0_OVF_vect) { Timer<TimerA0>::GetInstance().HandleOverflowInterrupt(); }
#endif

#if (TIMER_USE_TCA1 == STD_ON)
template class Timer<TimerA1>;
ISR(TCA1_OVF_vect) { Timer<TimerA1>::GetInstance().HandleOverflowInterrupt(); }
#endif


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
