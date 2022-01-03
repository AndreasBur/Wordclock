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

    enum MonthEnumType {
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
    using MonthType = byte;
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
    static constexpr MonthType MonthMinValue{1u};
    static constexpr MonthType MonthMaxValue{12u};
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
    MonthEnumType getMonthEnum() const { return static_cast<MonthEnumType>(Month); }
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

    static RataDieDayType getRataDieDay(YearType sYear, MonthType sMonth, DayType sDay) {
        if (sMonth < 3u) { sYear--, sMonth += 12u; }
        return static_cast<RataDieDayType>(365u)*sYear+sYear/4u-sYear/100u+sYear/400u+(153u*sMonth-457u)/5u+sDay-306u;
    }

    RataDieDayType getPassedDays(YearType sYear, MonthType sMonth, DayType sDay) {
        return labs(getRataDieDay(sYear, sMonth, sDay) - getRataDieDay(Year, Month, Day));
    }

    static DayType getDaysOfMonth(MonthType sMonth, YearType sYear) {
        if(sMonth == 4u || sMonth == 6u || sMonth == 9u || sMonth == 11u) {
            return 30u;
        } else if(sMonth == 2u) {
            if(sYear % 4u == 0u) { return 29u; }
            else { return 28u; }
        } else if((sMonth == 1u) || (sMonth == 3u) || (sMonth == 5u) ||
                (sMonth == 7u) || (sMonth == 8u) || (sMonth == 12)) {
            return 31u;
        }
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
