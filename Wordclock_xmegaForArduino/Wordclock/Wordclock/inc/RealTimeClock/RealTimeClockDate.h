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
/**     \file       Date.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _REAL_TIME_CLOCK_DATE_H_
#define _REAL_TIME_CLOCK_DATE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Date configuration parameter */


/* Date parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class RealTimeClockDate
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum WeekdayType {
        WEEKDAY_MONDAY = 1u,
        WEEKDAY_TUESDAY,
        WEEKDAY_WEDNESDAY,
        WEEKDAY_THURSDAY,
        WEEKDAY_FRIDAY,
        WEEKDAY_SATURDAY,
        WEEKDAY_SUNDAY
    };

    //enum MonthType {
        //MONTH_JANUARY = 1u,
        //MONTH_FEBRUARY,
        //MONTH_MARCH,
        //MONTH_APRIL,
        //MONTH_MAY,
        //MONTH_JUNE,
        //MONTH_JULY,
        //MONTH_AUGUST,
        //MONTH_SEPTEMBER,
        //MONTH_OCTOBER,
        //MONTH_NOVEMBER,
        //MONTH_DEZEMBER
    //};

    using YearType = uint16_t;
    using MonthType = uint8_t;
    using DayType = uint8_t;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    uint16_t Year{YearMinValue};
    uint8_t Month{MonthMinValue};
    uint8_t Day{DayMinValue};
    
    static constexpr YearType YearMinValue{2000u};
    static constexpr YearType YearMaxValue{2099u};
    static constexpr MonthType MonthMinValue{1u};
    static constexpr MonthType MonthMaxValue{12u};
    static constexpr DayType DayMinValue{1u};
    static constexpr DayType DayMaxValue{31u};
        
    // functions

  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr RealTimeClockDate() { }
    constexpr RealTimeClockDate(YearType sYear, MonthType sMonth, DayType sDay)
    {
        setYear(sYear);
        setMonth(sMonth);
        setDay(sDay);        
    }
    ~RealTimeClockDate() {}

	// get methods
    YearType getYear() const { return Year; }
    //MonthType getMonth() const { return static_cast<MonthType>(Month); }
    MonthType getMonth() const { return Month; }
    DayType getDay() const { return Day; }
    WeekdayType getWeekday() const {  }

	// set methods
    StdReturnType setYear(YearType sYear)
    {
        if(isYearValid(sYear)) { 
            Year = sYear; 
            return E_OK; 
        } else { 
            return E_NOT_OK; 
        }
    }
    StdReturnType setMonth(MonthType sMonth)
    {
        if(isMonthValid(sMonth)) {
            Month = sMonth;
            return E_OK;
        } else {
            return E_NOT_OK; }
    }
    StdReturnType setDay(DayType sDay)
    {
        if(isDayValid(sDay)) {
            Day = sDay;
            return E_OK;
        } else {
            return E_NOT_OK; 
        }
    }

	// methods
    static constexpr bool isYearValid(YearType Year) { return (Year >= YearMinValue) && (Year <= YearMaxValue); }
    static constexpr bool isMonthValid(MonthType Month) { return (Month >= MonthMinValue) && (Month <= MonthMaxValue); }
    static constexpr bool isDayValid(DayType Day) { return (Day >= DayMinValue) && (Day <= DayMaxValue);  }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
