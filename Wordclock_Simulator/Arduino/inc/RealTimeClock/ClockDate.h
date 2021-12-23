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
/**     \file       ClockDate.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _CLOCK_DATE_H_
#define _CLOCK_DATE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ClockDate configuration parameter */


/* ClockDate parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class ClockDate
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum WeekdayType {
        WEEKDAY_SUNDAY = 0u,
        WEEKDAY_MONDAY,
        WEEKDAY_TUESDAY,
        WEEKDAY_WEDNESDAY,
        WEEKDAY_THURSDAY,
        WEEKDAY_FRIDAY,
        WEEKDAY_SATURDAY,
    };

    enum MonthType {
        MONTH_JANUARY = 1u,
        MONTH_FEBRUARY,
        MONTH_MARCH,
        MONTH_APRIL,
        MONTH_MAY,
        MONTH_JUNE,
        MONTH_JULY,
        MONTH_AUGUST,
        MONTH_SEPTEMBER,
        MONTH_OCTOBER,
        MONTH_NOVEMBER,
        MONTH_DEZEMBER
    };

    using YearType = uint16_t;
    using MonthRawType = byte;
    //using MonthType = uint8_t;
    using DayType = byte;
    using RataDieDayType = uint32_t;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    YearType Year{YearMinValue};
    MonthType Month{MonthMinValue};
    DayType Day{DayMinValue};

    static constexpr YearType YearMinValue{2000u};
    static constexpr YearType YearMaxValue{2099u};
    static constexpr MonthType MonthMinValue{MONTH_JANUARY};
    static constexpr MonthType MonthMaxValue{MONTH_DEZEMBER};
    static constexpr DayType DayMinValue{1u};
    static constexpr DayType DayMaxValue{31u};

    // functions
    static constexpr DayType getWeekday(YearType Year, MonthType Month, DayType Day) {
        return (Day+=Month < 3 ? Year-- : Year-2, 23*Month/9+Day+4+Year/4-Year/100+Year/400)%7;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr ClockDate() { }
    constexpr ClockDate(YearType sYear, MonthType sMonth, DayType sDay)
    {
        setYear(sYear);
        setMonth(sMonth);
        setDay(sDay);
    }
    ~ClockDate() {}

	// get methods
    YearType getYear() const { return Year; }
    MonthType getMonth() const { return Month; }
    DayType getDay() const { return Day; }
    WeekdayType getWeekday() const {
        return static_cast<WeekdayType>(getWeekday(Year, Month, Day));
    }

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
    static constexpr bool isDayValid(DayType Day) { return (Day >= DayMinValue) && (Day <= DayMaxValue); }

    static RataDieDayType getRataDieDay(YearType Year, MonthType Month, DayType Day) {
        if (Month < 3) { Year--, Month += 12; }
        return 365*Year+Year/4-Year/100+Year/400+(153*Month-457)/5+Day-306;
    }

    static DayType getDaysOfMonth(MonthType Month, YearType Year) {
        MonthRawType monthRaw = static_cast<MonthRawType>(Month);

        if(monthRaw == 4 || monthRaw == 6 || monthRaw == 9 || monthRaw == 11) {
            return 30u;
        } else if(monthRaw == 2u) {
            if(Year % 4 == 0u) { return 29u; }
            else { return 28u; }
        } else if((monthRaw == 1u) || (monthRaw == 3u) || (monthRaw == 5u) ||
                (monthRaw == 7u) || (monthRaw == 8u) || (monthRaw == 12)) {
            return 31u;
        }
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
