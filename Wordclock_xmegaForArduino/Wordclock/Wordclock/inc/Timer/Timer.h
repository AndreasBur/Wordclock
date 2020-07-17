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
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _TIMER_H_
#define _TIMER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Timer configuration parameter */
#define TIMER_TC                    TCC4

/* Timer parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/
#define TIMER_CTRLGSET_DIR_COUNT_UP             0u
#define TIMER_CTRLGSET_DIR_COUNT_DOWN           1u

/******************************************************************************************************************************************************
 *  C L A S S   T I M E R
******************************************************************************************************************************************************/
class Timer
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  /* Timer ISR callback function */
    typedef void (*TimerIsrCallbackF_void)(void);

    /* Type which describes the Timer peripheral */
    //typedef enum {
        //TIMER_TCC4,
        //TIMER_TCC5,
        //TIMER_TCD5
    //} TimerType;

    /* Type which describes the internal state of the TIMER */
    typedef enum {
        TIMER_STATE_NONE,
        TIMER_STATE_INIT,
        TIMER_STATE_READY,
        TIMER_STATE_RUNNING,
        TIMER_STATE_STOPPED
    } TimerStateType;

    /* Type which includes the values of the Clock Select Bit Group */
    typedef enum {
        TIMER_REG_CS_NO_CLOCK,
        TIMER_REG_CS_NO_PRESCALER,
        TIMER_REG_CS_PRESCALE_2,
        TIMER_REG_CS_PRESCALE_4,
        TIMER_REG_CS_PRESCALE_8,
        TIMER_REG_CS_PRESCALE_64,
        TIMER_REG_CS_PRESCALE_256,
        TIMER_REG_CS_PRESCALE_1024
    } TimerClockSelectType;

    /* Type which includes the Pwm Pins */
    typedef enum {
        TEST
        //TIMER_PWM_PIN_9 = TIMER_A_ARDUINO_PIN,
        //TIMER_PWM_PIN_10 = TIMER_B_ARDUINO_PIN
    } TimerPwmPinType;

    typedef enum {
        TIMER_COUNT_DIRECTION_UP = TIMER_CTRLGSET_DIR_COUNT_UP,
        TIMER_COUNT_DIRECTION_DOWN = TIMER_CTRLGSET_DIR_COUNT_DOWN
    } TimerCountDirection;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:

    // functions
    template <typename TimerType> bool isTimerOfType5(TimerType Timer) { return (uint16_t)&Timer & 0x40u ? true : false; }
    template <typename TimerType> bool isTimerOfType4(TimerType Timer) { return (uint16_t)&Timer & 0x40u ? false : true; }

    TimerCountDirection getCountingDirection();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Timer();
    ~Timer();
    Timer(const Timer&);
    TimerStateType State;
    TimerClockSelectType ClockSelectBitGroup;
    uint32_t PwmPeriod;

    // get methods


    // set methods

    // methods
    static Timer& getInstance();
    TimerIsrCallbackF_void TimerOverflowCallback;
    StdReturnType init(uint32_t = 1000u, TimerIsrCallbackF_void = NULL);
    StdReturnType setPeriod(uint32_t);
    StdReturnType enablePwm(TimerPwmPinType, uint16_t);
    StdReturnType disablePwm(TimerPwmPinType);
    StdReturnType setPwmDuty(TimerPwmPinType, uint16_t);
    StdReturnType start();
    void stop();
    StdReturnType resume();
    StdReturnType attachInterrupt(TimerIsrCallbackF_void);
    void detachInterrupt();
    StdReturnType read(uint32_t&);
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
