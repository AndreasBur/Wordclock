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
/**     \file       RealTimeClockTime.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _REAL_TIME_CLOCK_TIME_H_
#define _REAL_TIME_CLOCK_TIME_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* RealTimeClockTime configuration parameter */


/* RealTimeClockTime parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class RealTimeClockTime
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = uint8_t;
    using MinuteType = uint8_t;
    using SecondType = uint8_t;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    HourType Hour{0u};
    MinuteType Minute{0u};
    SecondType Second{0u};
    
    static constexpr HourType HourMinValue{0u};
    static constexpr HourType HourMaxValue{23u};
    static constexpr MinuteType MinuteMinValue{0u};
    static constexpr MinuteType MinuteMaxValue{59u};
    static constexpr SecondType SecondMinValue{0u};
    static constexpr SecondType SecondMaxValue{59u};
        
    // functions
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr RealTimeClockTime() { }
    constexpr RealTimeClockTime(HourType sHour, MinuteType sMinute, SecondType sSecond)
    {
        setHour(sHour);
        setMinute(sMinute);
        setSecond(sSecond);
    }
    ~RealTimeClockTime() {}

	// get methods
    HourType getHour() const { return Hour; }
    MinuteType getMinute() const { return Minute; }
    SecondType getSecond() const { return Second; }

	// set methods
    StdReturnType setHour(HourType sHour) 
    {
        if(isHourValid(sHour)) { 
            Hour = sHour; 
            return E_OK; 
        } else { 
            return E_NOT_OK; 
        }
    }
    StdReturnType setMinute(MinuteType sMinute)
    {
        if(isMinuteValid(sMinute)) {
            Minute = sMinute;
            return E_OK;
        } else {
            return E_NOT_OK; }
    }
    StdReturnType setSecond(SecondType sSecond)
    {
        if(isSecondValid(sSecond)) {
            Second = sSecond;
            return E_OK;
        } else {
            return E_NOT_OK; 
        }
    }

	// methods
    static constexpr bool isHourValid(HourType Hour) { return (Hour >= HourMinValue) && (Hour <= HourMaxValue); }
    static constexpr bool isMinuteValid(MinuteType Minute) { return (Minute >= MinuteMinValue) && (Minute <= MinuteMaxValue); }
    static constexpr bool isSecondValid(SecondType Second) { return (Second >= SecondMinValue) && (Second <= SecondMaxValue);  }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
