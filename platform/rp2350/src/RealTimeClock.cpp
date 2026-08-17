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
 *      \brief      RP2350 real-time clock, backed by the system clock
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

/* The chip keeps UTC, so the two conversions for it leave the zone rule out: gmtime_r and
   timegm are what localtime_r and mktime are for the display's local time. */
void toDateTimeUtc(const time_t& Seconds, ClockDateTime& DateTime)
{
    struct tm Utc{};

    gmtime_r(&Seconds, &Utc);
    toDateTime(Utc, DateTime);
}

constexpr int32_t SecondsPerMinute{60};
constexpr int32_t SecondsPerHour{60 * SecondsPerMinute};
constexpr int32_t SecondsPerDay{24 * SecondsPerHour};

/* Days from 1970-01-01 to a civil date, by Howard Hinnant's days_from_civil: the year is
   shifted so that it begins in March, which puts the leap day at the end of it and makes
   the month lengths a repeating pattern rather than a table.

   Written out rather than left to timegm(), which mktime's counterpart would be: newlib
   declares it on neither of the two cores this firmware is built against, so writing it
   out is what keeps one source serving both. The other way round - a local time to seconds - keeps using mktime, which is
   there and which has to apply the zone rule anyway. */
constexpr int32_t daysFromCivil(int32_t Year, int32_t Month, int32_t Day)
{
    Year -= (Month <= 2) ? 1 : 0;

    const int32_t Era = ((Year >= 0) ? Year : (Year - 399)) / 400;
    const int32_t YearOfEra = Year - (Era * 400);
    const int32_t DayOfYear = (((153 * (Month + ((Month > 2) ? -3 : 9))) + 2) / 5) + Day - 1;
    const int32_t DayOfEra = (YearOfEra * 365) + (YearOfEra / 4) - (YearOfEra / 100) + DayOfYear;

    return (Era * 146097) + DayOfEra - 719468;
}

/* The epoch itself, the leap day that the shifted year is built around, and both ends of
   what ClockDate covers. */
static_assert(daysFromCivil(1970, 1, 1) == 0, "the epoch must be day zero");
static_assert(daysFromCivil(2000, 3, 1) == 11017, "the day after a leap day must line up");
static_assert(daysFromCivil(2026, 8, 14) == 20679, "an ordinary day must line up");
static_assert(daysFromCivil(2099, 12, 31) == 47481, "the last day ClockDate accepts must line up");

time_t toSecondsUtc(const ClockDateTime& DateTime)
{
    const int32_t Days = daysFromCivil(static_cast<int32_t>(DateTime.getDateYear()),
                                       static_cast<int32_t>(DateTime.getDateMonth()),
                                       static_cast<int32_t>(DateTime.getDateDay()));

    return static_cast<time_t>((Days * SecondsPerDay) +
                               (static_cast<int32_t>(DateTime.getTimeHour()) * SecondsPerHour) +
                               (static_cast<int32_t>(DateTime.getTimeMinute()) * SecondsPerMinute) +
                               static_cast<int32_t>(DateTime.getTimeSecond()));
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
    /* A time set by hand is worth keeping over the next power cut as much as one from the
       network, so the chip is written again on the next task rather than at the turn of
       the hour it would otherwise wait for. */
    HourWrittenToChip = NoHourWritten;
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
 *
 *                  The clock chip is the source of last resort rather than a second one:
 *                  it is read only while the system clock holds nothing, and written back
 *                  from the system clock afterwards. Which way round matters - the chip
 *                  drifts a few seconds a month and SNTP does not, so a chip that could
 *                  overwrite a synchronised clock would make the clock worse.
******************************************************************************************************************************************************/
void RealTimeClock::task()
{
    ClockDateTime Current;

    /* An unset clock is left alone rather than shown. Before the first SNTP answer the
       system clock sits in 1970, which ClockDate rejects outright - so the display would
       otherwise keep whatever the failed setters left behind. */
    if(!readSystemDateTime(Current)) {
        readChip();
        return;
    }

    DateTime = Current;
    writeChip();
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
  readChip()
******************************************************************************************************************************************************/
/*! \brief          Puts what the clock chip kept into the system clock
 *  \details        Only ever called while the system clock holds nothing, so there is
 *                  nothing here that could overwrite a synchronised time. Retried once a
 *                  second until it works, which is what picks up a chip that answers late -
 *                  and what keeps the retry off a bus the light sensor shares.
 *
 *  \return         E_OK if the chip had a time and the system clock took it
******************************************************************************************************************************************************/
StdReturnType RealTimeClock::readChip()
{
    if(ChipReadCountdown > 0u) { ChipReadCountdown--; return E_NOT_OK; }

    ChipReadCountdown = ChipReadInterval;

    ClockDateTime Utc;

    if(Chip.getDateTime(Utc) == E_NOT_OK) { return E_NOT_OK; }

    const struct timeval Time{toSecondsUtc(Utc), 0};
    if(settimeofday(&Time, nullptr) != 0) { return E_NOT_OK; }

    /* Not written back at once: what was just read is what the chip already holds, and the
       hour it belongs to is what the next write is measured against. */
    HourWrittenToChip = Utc.getTimeHour();
    return E_OK;
} /* readChip */


/******************************************************************************************************************************************************
  writeChip()
******************************************************************************************************************************************************/
/*! \brief          Keeps the clock chip in step with the system clock
 *  \details        Once an hour, and once more whenever the time was set by hand. Once an
 *                  hour because the chip is the more accurate of the two and needs no
 *                  correcting - what the write is for is the case where SNTP has moved the
 *                  system clock and the chip would otherwise keep an old time for the next
 *                  power cut. The hour is remembered rather than a countdown kept, so a
 *                  clock that was off for a while writes on its first hour and not a
 *                  whole period later.
 *
 *                  A failed write is not retried within the hour: the next one comes
 *                  anyway, and a chip that is not answering would otherwise be talked to
 *                  on every single tick.
******************************************************************************************************************************************************/
void RealTimeClock::writeChip()
{
    if(DateTime.getTimeHour() == HourWrittenToChip) { return; }

    ClockDateTime Utc;
    toDateTimeUtc(time(nullptr), Utc);

    Chip.setDateTime(Utc);
    HourWrittenToChip = DateTime.getTimeHour();
} /* writeChip */


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
