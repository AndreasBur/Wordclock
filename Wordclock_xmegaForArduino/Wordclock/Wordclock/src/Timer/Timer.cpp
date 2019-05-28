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
#define TIMER_NUMBER_OF_BITS						16
#define TIMER_RESOLUTION							(1UL << TIMER_NUMBER_OF_BITS)
#define TIMER_MAX_PRESCALER                         1024

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
/*! \brief          Timer Constructor
 *  \details        Instantiation of the Timer library
 *
 *  \return         -
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
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Timer::init(uint32_t Microseconds, TimerIsrCallbackF_void sTimerOverflowCallback)
{
	stdReturnType ReturnValue = E_NOT_OK;

	if(TIMER_STATE_NONE == State) {
    	ReturnValue = E_OK;
    	State = TIMER_STATE_INIT;

        /* set mode 7: Dual-slope PWM*/
        writeBitGroup(TCC4_CTRLB, TC4_WGMODE_gm, TC_WGMODE_DSBOTTOM_gc);
    	
    	if(setPeriod(Microseconds) == E_NOT_OK) ReturnValue = E_NOT_OK;
    	//if(sTimerOverflowCallback != NULL) if(attachInterrupt(sTimerOverflowCallback) == E_NOT_OK) ReturnValue = E_NOT_OK;

    	State = TIMER_STATE_READY;
	}
	return ReturnValue;
} /* init */


/******************************************************************************************************************************************************
  setPeriod()
******************************************************************************************************************************************************/
/*! \brief          set period of Timer1 overflow interrupt
 *  \details        this functions sets the period of the Timer1 overflow interrupt therefore 
 *                  prescaler and timer top value will be calculated
 *  \param[in]      Microseconds				period of the timer overflow interrupt
 *  \return         E_OK
 *                  E_NOT_OK
 *****************************************************************************************************************************************************/
stdReturnType Timer::setPeriod(uint32_t Microseconds)
{
	stdReturnType ReturnValue = E_NOT_OK;
	unsigned long TimerCycles;
    
    /* was request out of bounds? */
    if(Microseconds <= ((TIMER_RESOLUTION / (F_CPU / 1000000)) * TIMER_MAX_PRESCALER * 2)) {
        ReturnValue = E_OK;
        /* calculate timer cycles to reach timer period, counter runs backwards after TOP, interrupt is at BOTTOM so divide microseconds by 2 */
        TimerCycles = (F_CPU / 2000000) * Microseconds;
        /* calculate timer prescaler */
        if(TimerCycles < TIMER_RESOLUTION)              ClockSelectBitGroup = TIMER_REG_CS_NO_PRESCALER;
        else if((TimerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_2;
        else if((TimerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_4;
        else if((TimerCycles >>= 1) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_8;
        else if((TimerCycles >>= 3) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_64;
        else if((TimerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_256;
        else if((TimerCycles >>= 2) < TIMER_RESOLUTION) ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_1024;
        else {
            /* request was out of bounds, set as maximum */
            TimerCycles = TIMER_RESOLUTION - 1;
            ClockSelectBitGroup = TIMER_REG_CS_PRESCALE_1024;
            ReturnValue = E_NOT_OK;
        }
        /* Set TOP value of timer */
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { TCC4_PER = TimerCycles; }

        if(TIMER_STATE_RUNNING == State)
        {
            /* reset clock select register, and starts the clock */
            writeBitGroup(TCC4_CTRLA, CLK_SCLKSEL_gm, ClockSelectBitGroup << CLK_SCLKSEL_gp);
        }
    }
	return ReturnValue;
} /* setPeriod */

/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 