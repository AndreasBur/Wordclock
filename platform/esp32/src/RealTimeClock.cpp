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
/**     \file       RealTimeClock.cpp
 *      \brief      ESP32 real-time clock, backed by the system clock
 *
 *      \details    All conversions go through the C library's local time, so the zone
 *                  and the daylight-saving rule that the application installed with
 *                  configTzTime() apply in both directions.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "RealTimeClock.h"

#include <sys/time.h>
#include <time.h>

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
namespace {

/* tm counts years from 1900 and months from zero, ClockDate counts neither. */
constexpr int TmYearOffset{1900};
constexpr int TmMonthOffset{1};

void toDateTime(const struct tm& Local, ClockDateTime& DateTime)
{
    DateTime.setDateYear(static_cast<ClockDateTime::YearType>(Local.tm_year + TmYearOffset));
    DateTime.setDateMonth(static_cast<ClockDateTime::MonthType>(Local.tm_mon + TmMonthOffset));
    DateTime.setDateDay(static_cast<ClockDateTime::DayType>(Local.tm_mday));
    DateTime.setTimeHour(static_cast<ClockDateTime::HourType>(Local.tm_hour));
    DateTime.setTimeMinute(static_cast<ClockDateTime::MinuteType>(Local.tm_min));
    DateTime.setTimeSecond(static_cast<ClockDateTime::SecondType>(Local.tm_sec));
}

void toTm(const ClockDateTime& DateTime, struct tm& Local)
{
    Local = {};
    Local.tm_year = static_cast<int>(DateTime.getDateYear()) - TmYearOffset;
    Local.tm_mon = static_cast<int>(DateTime.getDateMonth()) - TmMonthOffset;
    Local.tm_mday = static_cast<int>(DateTime.getDateDay());
    Local.tm_hour = static_cast<int>(DateTime.getTimeHour());
    Local.tm_min = static_cast<int>(DateTime.getTimeMinute());
    Local.tm_sec = static_cast<int>(DateTime.getTimeSecond());
    /* Unknown rather than off, so mktime() decides from the zone rule whether the wall
       clock time it was handed is in summer or winter time. Hardcoding zero would move
       the clock by an hour for half the year. */
    Local.tm_isdst = -1;
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  isSystemTimeSet()
******************************************************************************************************************************************************/
/*! \brief          Tells whether the system clock holds a real date yet
 *  \details        Asked as "is the year one ClockDate accepts" rather than by querying
 *                  SNTP: that answers the question the firmware actually has, and it
 *                  stays true for a time that arrived from a command instead of the net.
 *
 *  \return         true once the system clock has been set
******************************************************************************************************************************************************/
bool RealTimeClock::isSystemTimeSet()
{
    ClockDateTime Unused;
    return readSystemDateTime(Unused);
} /* isSystemTimeSet */


/******************************************************************************************************************************************************
  setDateTime()
******************************************************************************************************************************************************/
void RealTimeClock::setDateTime(ClockDateTime sDateTime)
{
    DateTime = sDateTime;
    writeSystemDateTime(DateTime);
} /* setDateTime */


/******************************************************************************************************************************************************
  setTime()
******************************************************************************************************************************************************/
void RealTimeClock::setTime(ClockTime Time)
{
    ClockDateTime Value = DateTime;
    Value.setTime(Time);
    setDateTime(Value);
} /* setTime */


/******************************************************************************************************************************************************
  setDate()
******************************************************************************************************************************************************/
void RealTimeClock::setDate(ClockDate Date)
{
    ClockDateTime Value = DateTime;
    Value.setDate(Date);
    setDateTime(Value);
} /* setDate */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          Takes the current local time from the system clock
 *  \details        Nothing is counted forward here, unlike on the simulator: SNTP keeps
 *                  the system clock right on its own, so counting would only add a
 *                  second source of truth that drifts away from it.
******************************************************************************************************************************************************/
void RealTimeClock::task()
{
    ClockDateTime Current;

    /* An unset clock is left alone rather than shown. Before the first SNTP answer the
       system clock sits in 1970, which ClockDate rejects outright - so the display would
       otherwise keep whatever the failed setters left behind. */
    if(!readSystemDateTime(Current)) { return; }

    DateTime = Current;
} /* task */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  readSystemDateTime()
******************************************************************************************************************************************************/
/*! \brief          Reads the system clock as local date and time
 *
 *  \return         true if the system clock holds a year ClockDate accepts
******************************************************************************************************************************************************/
bool RealTimeClock::readSystemDateTime(ClockDateTime& Value)
{
    const time_t Now = time(nullptr);
    struct tm Local{};

    if(localtime_r(&Now, &Local) == nullptr) { return false; }
    if(!ClockDate::isYearValid(static_cast<ClockDate::YearType>(Local.tm_year + TmYearOffset))) { return false; }

    toDateTime(Local, Value);
    return true;
} /* readSystemDateTime */


/******************************************************************************************************************************************************
  writeSystemDateTime()
******************************************************************************************************************************************************/
/*! \brief          Puts a date and time into the system clock
 *
 *  \return         E_OK if the system clock took the value
******************************************************************************************************************************************************/
StdReturnType RealTimeClock::writeSystemDateTime(const ClockDateTime& Value)
{
    struct tm Local{};
    toTm(Value, Local);

    const time_t Seconds = mktime(&Local);
    if(Seconds == static_cast<time_t>(-1)) { return E_NOT_OK; }

    const struct timeval Time{Seconds, 0};
    if(settimeofday(&Time, nullptr) != 0) { return E_NOT_OK; }

    return E_OK;
} /* writeSystemDateTime */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
