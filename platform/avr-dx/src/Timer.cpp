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
  init()
******************************************************************************************************************************************************/
/*! \brief          bring the timer up, stopped, with a period and optionally a callback
 *  \param[in]      Microseconds                period of the timer overflow interrupt
 *  \param[in]      sTimerOverflowCallback      called from the overflow ISR, or nullptr for none
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::init(uint32_t Microseconds, TimerIsrCallbackF_void sTimerOverflowCallback)
{
    StdReturnType returnValue{E_NOT_OK};

    if(STATE_NONE == State) {
        returnValue = E_OK;
        State = STATE_INIT;

        /* The Dx hands this timer over at its reset values rather than running, the way the
           xmegaForArduino core used to - but RESET is kept anyway, so that a second init() after a
           detachInterrupt() starts from the same place the first one did. The command is only
           obeyed while the timer is disabled, hence the order. */
        setRunning(false);
        getTimer().CTRLESET = TCA_SINGLE_CMD_RESET_gc;

        /* Away from the PORTA the route comes up on, before any channel can drive a pin: PA0 and
           PA2 belong to the strip. See the traits in Timer.h. */
        writeBitGroup(PORTMUX.TCAROUTEA, TimerTraits::RouteMask, TimerTraits::RouteGroup);

        /* dual-slope PWM, overflow interrupt at BOTTOM */
        writeBitGroup(getTimer().CTRLB, TCA_SINGLE_WGMODE_gm, TCA_SINGLE_WGMODE_DSBOTTOM_gc);

        if(setPeriod(Microseconds) == E_NOT_OK) returnValue = E_NOT_OK;
        if(sTimerOverflowCallback != nullptr) if(attachInterrupt(sTimerOverflowCallback) == E_NOT_OK) returnValue = E_NOT_OK;

        State = STATE_READY;
    }
    return returnValue;
} /* init */


/******************************************************************************************************************************************************
  setPeriod()
******************************************************************************************************************************************************/
/*! \brief          set period of the timer overflow interrupt
 *  \details        this functions sets the period of the timer overflow interrupt therefore
 *                  prescaler and timer top value will be calculated
 *  \param[in]      Microseconds                period of the timer overflow interrupt
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::setPeriod(uint32_t Microseconds)
{
    StdReturnType returnValue{E_NOT_OK};

    /* was request out of bounds? An out of range period is refused and changes nothing */
    if((Microseconds > 0u) && (Microseconds < TIMER_MAX_PERIOD)) {
        returnValue = E_OK;
        /* calculate timer cycles to reach timer period, counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2 */
        uint32_t timerCycles = (F_CPU / 2000000uL) * Microseconds;
        /* calculate timer prescaler */
        if(timerCycles < TIMER_RESOLUTION)              ClockSelectBitGroup = TIMER_REG_CS_NO_PRESCALER;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_2;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_4;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_8;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_16;
        else if((timerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_64;
        else if((timerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_256;
        /* TIMER_MAX_PERIOD above is what keeps this one inside 16 bit */
        else { timerCycles >>= 2;                         ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_1024; }

        /* A 16-bit access goes through the timer's shared TEMP register, so an ISR touching another
           16-bit register of this timer must not get in between.
           Through the buffer rather than PER: a TOP that lands below the counter mid-period is not
           picked up until the counter has gone all the way round 0xFFFF, which is one period of
           several hundred milliseconds in the wrong place. PERBUF is taken over at BOTTOM, where
           the counter is below any legal TOP - and that holds for a stopped timer as much as for a
           running one, since resume() continues mid-period. A timer not yet started has its
           registers loaded by start().
           The compare values go along, recomputed from their duty, so the PWM keeps its ratio
           across the change and takes the new period at the same BOTTOM. */
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            Top = static_cast<uint16_t>(timerCycles);
            getTimer().PERBUF = Top;
            for(uint8_t Channel = 0u; Channel < TimerTraits::NumberOfChannels; Channel++) {
                setCompareBuffer(static_cast<TimerChannelType>(Channel));
            }
        }

        if(STATE_RUNNING == State) { setClockSelect(ClockSelectBitGroup); }
    }
    return returnValue;
} /* setPeriod */


/******************************************************************************************************************************************************
  start()
******************************************************************************************************************************************************/
/*! \brief          start the timer from zero
 *  \return         E_OK
 *                  E_NOT_OK        timer was not initialized
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::start()
{
    StdReturnType returnValue{E_NOT_OK};

    if(STATE_READY == State || STATE_RUNNING == State || STATE_STOPPED == State) {
        returnValue = E_OK;
        setRunning(false);
        loadPeriodAndCompare();
        setClockSelect(ClockSelectBitGroup);
        /* RESTART clears the counter and the direction, so the first period is a whole one */
        getTimer().CTRLESET = TCA_SINGLE_CMD_RESTART_gc;
        setRunning(true);
        State = STATE_RUNNING;
    }
    return returnValue;
} /* start */


/******************************************************************************************************************************************************
  stop()
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::stop()
{
    /* before init() the timer is not this class's to stop */
    if(STATE_NONE != State) {
        setRunning(false);
        if(STATE_RUNNING == State) { State = STATE_STOPPED; }
    }
} /* stop */


/******************************************************************************************************************************************************
  resume()
******************************************************************************************************************************************************/
/*! \brief          continue a stopped timer where it stopped
 *  \return         E_OK
 *                  E_NOT_OK        timer was not stopped
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::resume()
{
    StdReturnType returnValue{E_NOT_OK};

    if(STATE_STOPPED == State) {
        returnValue = E_OK;
        setRunning(true);
        State = STATE_RUNNING;
    }
    return returnValue;
} /* resume */


/******************************************************************************************************************************************************
  attachInterrupt()
******************************************************************************************************************************************************/
/*! \brief          install the callback the overflow ISR calls
 *  \details        The xmega put this interrupt on the PMIC's low level so that the core's millis()
 *                  could interrupt the callback. The Dx has no such pair of levels - it has one
 *                  CPUINT.LVL1VEC, and WS2812::init() has given it to USART0_DRE, which is what
 *                  keeps the strip's bytes flowing inside its reset gap. Taking it away for this
 *                  would corrupt a frame, so the overflow stays at level 0 and the callback runs
 *                  without being interruptible. Keep it short.
 *  \param[in]      sTimerOverflowCallback      called from the overflow ISR
 *  \return         E_OK
 *                  E_NOT_OK                    no callback given
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::attachInterrupt(TimerIsrCallbackF_void sTimerOverflowCallback)
{
    StdReturnType returnValue{E_NOT_OK};

    if(sTimerOverflowCallback != nullptr) {
        returnValue = E_OK;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { TimerOverflowCallback = sTimerOverflowCallback; }
        /* The flag is set at every BOTTOM whether the interrupt is on or not, so one left over from
           while it was off would call the new callback at once. Only then: with the interrupt
           already on, a set flag is a real overflow about to be served. */
        if(!(getTimer().INTCTRL & TCA_SINGLE_OVF_bm)) { getTimer().INTFLAGS = TCA_SINGLE_OVF_bm; }
        getTimer().INTCTRL |= TCA_SINGLE_OVF_bm;
    }
    return returnValue;
} /* attachInterrupt */


/******************************************************************************************************************************************************
  detachInterrupt()
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::detachInterrupt()
{
    getTimer().INTCTRL &= static_cast<uint8_t>(~TCA_SINGLE_OVF_bm);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { TimerOverflowCallback = nullptr; }
} /* detachInterrupt */


/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          read current timer value in microseconds
 *  \details        this function returns the time since the last overflow interrupt in microseconds
 *
 *  \param[out]     Microseconds        current timer value
 *  \return         E_OK
 *                  E_NOT_OK
 *  \pre            Timer has to be in RUNNING or STOPPED STATE
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::read(uint32_t& Microseconds)
{
    StdReturnType returnValue{E_NOT_OK};

    if(STATE_RUNNING == State || STATE_STOPPED == State) {
        uint32_t counterValue{0u};
        uint32_t topValue{0u};
        TimerCountDirection direction{TIMER_COUNT_DIRECTION_UP};
        returnValue = E_OK;

        /* counter and direction have to come from the same instant, or a turn at TOP in between
           reads as a whole period too much */
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            counterValue = getTimer().CNT;
            topValue = getTimer().PER;
            direction = getCountingDirection();
        }
        /* if counter counting down, add top value to current value */
        if(direction == TIMER_COUNT_DIRECTION_DOWN) { counterValue = (topValue - counterValue) + topValue; }

        Microseconds = toMicroseconds(counterValue, ClockSelectBitGroup);
    }
    return returnValue;
} /* read */


/******************************************************************************************************************************************************
  handleOverflowInterrupt()
******************************************************************************************************************************************************/
/*! \brief          called from the overflow ISR of this timer
 *  \details        The flag is not cleared by the vector executing, on this part no more than on the
 *                  xmega's TC4/TC5; without the explicit clear the ISR is entered again at once.
 *                  Writing the bit is what clears it here.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::handleOverflowInterrupt()
{
    getTimer().INTFLAGS = TCA_SINGLE_OVF_bm;
    if(TimerOverflowCallback != nullptr) { TimerOverflowCallback(); }
} /* handleOverflowInterrupt */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getCountingDirection
******************************************************************************************************************************************************/
/*! \details        CTRLESET and CTRLECLR are one register seen twice: a write sets or clears the
 *                  bits it names, and a read of either gives the state. Reading the SET half keeps
 *                  this looking like the xmega's CTRLGSET.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
typename Timer<TimerTraits>::TimerCountDirection Timer<TimerTraits>::getCountingDirection()
{
    if(getTimer().CTRLESET & TCA_SINGLE_DIR_bm) { return TIMER_COUNT_DIRECTION_DOWN; }
    else { return TIMER_COUNT_DIRECTION_UP; }
} /* getCountingDirection */


/******************************************************************************************************************************************************
  setClockSelect
******************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::setClockSelect(TimerClockSelectType ClockSelect)
{
    writeBitGroup(getTimer().CTRLA, TCA_SINGLE_CLKSEL_gm, ClockSelect);
} /* setClockSelect */


/******************************************************************************************************************************************************
  setRunning
******************************************************************************************************************************************************/
/*! \details        Where the xmega stopped its timer by selecting no clock source, this one keeps
 *                  the prescaler and loses ENABLE: the Dx CLKSEL has no OFF to select.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::setRunning(bool Running)
{
    if(Running) { getTimer().CTRLA |= TCA_SINGLE_ENABLE_bm; }
    else        { getTimer().CTRLA &= static_cast<uint8_t>(~TCA_SINGLE_ENABLE_bm); }
} /* setRunning */


/******************************************************************************************************************************************************
  setCompareBuffer
******************************************************************************************************************************************************/
/*! \pre            called with interrupts off, the write goes through the TEMP register
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::setCompareBuffer(TimerChannelType Channel)
{
    /* CMP0BUF..CMP2BUF are consecutive, which Timer.h asserts; the channel was range checked at
       compile time */
    (&getTimer().CMP0BUF)[Channel] = toCompareValue(Top, PwmDuty[Channel]);
} /* setCompareBuffer */


/******************************************************************************************************************************************************
  loadPeriodAndCompare
******************************************************************************************************************************************************/
/*! \details        Straight into PER and CMPn, past the buffers, which is only safe while the timer is
 *                  disabled. Without it the first period after start() would run on whatever the
 *                  registers held: the buffers are taken over at BOTTOM, and the first BOTTOM after
 *                  RESTART is a whole period away - with CMPn at the 0 init()'s RESET left there.
 *                  Not a forced UPDATE before the RESTART instead: both are written to CTRLESET,
 *                  and an UPDATE still pending there ORed with RESTART reads as RESET.
 *  \pre            timer disabled
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
void Timer<TimerTraits>::loadPeriodAndCompare()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        getTimer().PER = Top;
        for(uint8_t Channel = 0u; Channel < TimerTraits::NumberOfChannels; Channel++) {
            /* CMP0..CMP2 are consecutive like their buffers, which Timer.h asserts as well */
            (&getTimer().CMP0)[Channel] = toCompareValue(Top, PwmDuty[Channel]);
        }
    }
} /* loadPeriodAndCompare */


/******************************************************************************************************************************************************
  enablePwmChannel
******************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::enablePwmChannel(TimerChannelType Channel, uint16_t Duty)
{
    StdReturnType returnValue = setPwmDutyChannel(Channel, Duty);

    if(E_OK == returnValue) {
        TimerTraits::getPort().DIRSET = toPinMask(Channel);
        getTimer().CTRLB |= toChannelEnableMask(Channel);
    }
    return returnValue;
} /* enablePwmChannel */


/******************************************************************************************************************************************************
  disablePwmChannel
******************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::disablePwmChannel(TimerChannelType Channel)
{
    getTimer().CTRLB &= static_cast<uint8_t>(~toChannelEnableMask(Channel));
    /* the pin stays an output, it now shows its PORT.OUT value instead of the waveform */
    return E_OK;
} /* disablePwmChannel */


/******************************************************************************************************************************************************
  setPwmDutyChannel
******************************************************************************************************************************************************/
/*! \brief          set duty of one compare channel, 0 .. PwmDutyMax
 *  \details        Written to the buffer register, so the new compare value takes effect at the next
 *                  BOTTOM and a running PWM never sees half a period of each.
 *****************************************************************************************************************************************************/
template <typename TimerTraits>
StdReturnType Timer<TimerTraits>::setPwmDutyChannel(TimerChannelType Channel, uint16_t Duty)
{
    StdReturnType returnValue{E_NOT_OK};

    if(Duty <= PwmDutyMax) {
        returnValue = E_OK;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            PwmDuty[Channel] = Duty;
            setCompareBuffer(Channel);
        }
    }
    return returnValue;
} /* setPwmDutyChannel */


/******************************************************************************************************************************************************
 * I N S T A N C E S   A N D   I N T E R R U P T S
******************************************************************************************************************************************************/
#if (TIMER_USE_TCA0 == STD_ON)
template class Timer<TimerA0>;
ISR(TCA0_OVF_vect) { Timer<TimerA0>::getInstance().handleOverflowInterrupt(); }
#endif

#if (TIMER_USE_TCA1 == STD_ON)
template class Timer<TimerA1>;
ISR(TCA1_OVF_vect) { Timer<TimerA1>::getInstance().handleOverflowInterrupt(); }
#endif


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
