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
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _TIMER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Timer.h"
#include <Util/atomic.h>

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/
/* All Timers of XMEGA are 16 bit */
#define TIMER_NUMBER_OF_BITS                        16u
#define TIMER_RESOLUTION                            (1uL << TIMER_NUMBER_OF_BITS)
#define TIMER_MAX_PRESCALER                         1024u

/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of Timer
******************************************************************************************************************************************************/
Timer::Timer()
{

} /* Timer */


/******************************************************************************************************************************************************
  Destructor of Timer
******************************************************************************************************************************************************/
Timer::~Timer()
{

} /* ~Timer */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
StdReturnType Timer::init(uint32_t Microseconds, TimerIsrCallbackF_void sTimerOverflowCallback)
{
    StdReturnType returnValue{E_NOT_OK};

    if(TIMER_STATE_NONE == State) {
        returnValue = E_OK;
        State = TIMER_STATE_INIT;

        /* set mode 7: Dual-slope PWM */
        writeBitGroup(TIMER_TC.CTRLB, TC4_WGMODE_gm, TC_WGMODE_DSBOTTOM_gc);
        
        if(setPeriod(Microseconds) == E_NOT_OK) returnValue = E_NOT_OK;
        if(sTimerOverflowCallback != NULL) if(attachInterrupt(sTimerOverflowCallback) == E_NOT_OK) returnValue = E_NOT_OK;

        State = TIMER_STATE_READY;
    }
    return returnValue;
} /* init */


/******************************************************************************************************************************************************
  setPeriod()
******************************************************************************************************************************************************/
/*! \brief          set period of Timer1 overflow interrupt
 *  \details        this functions sets the period of the Timer1 overflow interrupt therefore 
 *                  prescaler and timer top value will be calculated
 *  \param[in]      Microseconds                period of the timer overflow interrupt
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
StdReturnType Timer::setPeriod(uint32_t Microseconds)
{
    StdReturnType returnValue{E_NOT_OK};
    
    /* was request out of bounds? */
    if(Microseconds <= ((TIMER_RESOLUTION / (F_CPU / 1000000uL)) * TIMER_MAX_PRESCALER * 2u)) {
        returnValue = E_OK;
        /* calculate timer cycles to reach timer period, counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2 */
        uint32_t timerCycles = (F_CPU / 2000000uL) * Microseconds;
        /* calculate timer prescaler */
        if(timerCycles < TIMER_RESOLUTION)              ClockSelectBitGroup = TIMER_REG_CS_NO_PRESCALER;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_2;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_4;
        else if((timerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_8;
        else if((timerCycles >>= 3) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_64;
        else if((timerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_256;
        else if((timerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_1024;
        else {
            /* request was out of bounds, set to maximum */
            timerCycles = TIMER_RESOLUTION - 1u;
            ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_1024;
            returnValue = E_NOT_OK;
        }
        /* Set TOP value of timer */
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { TIMER_TC.PER = timerCycles; }

        if(TIMER_STATE_RUNNING == State)
        {
            /* reset clock select register, and start the clock */
            //writeBitGroup(TIMER_TC.CTRLA, CLK_SCLKSEL_gm, ClockSelectBitGroup << CLK_SCLKSEL_gp);
            writeBitGroup(TIMER_TC.CTRLA, CLK_SCLKSEL_gm, CLK_SCLKSEL_gp, ClockSelectBitGroup, false);
        }
    }
    return returnValue;
} /* setPeriod */


/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          read current timer value in microseconds
 *  \details        this function returns the current timer value in microseconds
 *                  
 *  \param[out]     Microseconds        current timer value
 *  \return         E_OK
 *                  E_NOT_OK
 *  \pre            Timer has to be in RUNNING STATE
 *****************************************************************************************************************************************************/
StdReturnType Timer::read(uint32_t& Microseconds)
{
    StdReturnType returnValue{E_NOT_OK};

    if(TIMER_STATE_RUNNING == State || TIMER_STATE_STOPPED == State) {
        uint32_t counterValue{0u};
        byte prescaleShiftScale{0u};
        returnValue = E_OK;
        /* save current timer value */
        counterValue = TIMER_TC.CNT;

        switch (ClockSelectBitGroup)
        {
            case TIMER_REG_CS_NO_PRESCALER:
            prescaleShiftScale = 0u;
            break;
            case TIMER_REG_CS_PRESCALE_2:
            prescaleShiftScale = 1u;
            break;
            case TIMER_REG_CS_PRESCALE_4:
            prescaleShiftScale = 2u;
            break;
            case TIMER_REG_CS_PRESCALE_8:
            prescaleShiftScale = 3u;
            break;
            case TIMER_REG_CS_PRESCALE_64:
            prescaleShiftScale = 4u;
            break;
            case TIMER_REG_CS_PRESCALE_256:
            prescaleShiftScale = 8u;
            break;
            case TIMER_REG_CS_PRESCALE_1024:
            prescaleShiftScale = 10u;
            break;
            default:
            returnValue = E_NOT_OK;
        }
        /* if counter counting down, add top value to current value */
        if(getCountingDirection() == TIMER_COUNT_DIRECTION_DOWN) { 
            counterValue = static_cast<uint32_t>(TIMER_TC.PER - counterValue) + TIMER_TC.PER; 
        }
        /* transform counter value to microseconds in an efficient way */
        Microseconds = ((counterValue * 1000uL) / (F_CPU / 1000uL)) << prescaleShiftScale;
    }
    return returnValue;
} /* read */

/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getCountingDirection
******************************************************************************************************************************************************/
Timer::TimerCountDirection Timer::getCountingDirection()
{
        byte DIRValue = 0;

        if(isTimerOfType4(TIMER_TC)) DIRValue = readBitGroup(TIMER_TC.CTRLGSET, TC4_DIR_bm, TC4_DIR_bp, false);
        if(isTimerOfType5(TIMER_TC)) DIRValue = readBitGroup(TIMER_TC.CTRLGSET, TC5_DIR_bm, TC5_DIR_bp, false);
        
        if(DIRValue == TIMER_CTRLGSET_DIR_COUNT_UP) { return TIMER_COUNT_DIRECTION_UP; }
        else { return TIMER_COUNT_DIRECTION_DOWN; }
}


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 