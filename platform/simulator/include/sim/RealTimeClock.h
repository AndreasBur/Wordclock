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
/**     \file       RealTimeClock.h
 *      \brief      Simulator stub of the real-time clock driver
 *
 *      \details    Mirrors the public API of the hardware RTC driver (date/time
 *                  getters and setters, singleton access) so the shared firmware
 *                  core builds against the simulator. Time advances from a monotonic
 *                  host clock after it has been initialized, just like an RTC would.
 *
******************************************************************************************************************************************************/
#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"
#include <chrono>
#include <wx/datetime.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* RealTimeClock configuration parameter */


/* RealTimeClock parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   R E A L   T I M E   C L O C K
******************************************************************************************************************************************************/
class RealTimeClock
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

    using YearType = ClockDateTime::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using WeekdayType = ClockDateTime::WeekdayType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockDateTime DateTime;
    std::chrono::steady_clock::time_point LastUpdate{std::chrono::steady_clock::now()};

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    void updateDateTime(const wxDateTime& Value) {
        DateTime.setDateYear(Value.GetYear());
        DateTime.setDateMonth(Value.GetMonth() + 1u);
        DateTime.setDateDay(Value.GetDay());
        DateTime.setTimeHour(Value.GetHour());
        DateTime.setTimeMinute(Value.GetMinute());
        DateTime.setTimeSecond(Value.GetSecond());
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static RealTimeClock& getInstance() {
        static RealTimeClock singletonInstance;
        return singletonInstance;
    }

    // get methods
    ClockDateTime getDateTime() const { return DateTime; }
    ClockTime getTime() const { return DateTime.getTime(); }
    ClockDate getDate() const { return DateTime.getDate(); }

    // set methods
    void setDateTime(ClockDateTime sDateTime) {
        DateTime = sDateTime;
        LastUpdate = std::chrono::steady_clock::now();
    }
    void setTime(ClockTime Time) {
        DateTime.setTime(Time);
        LastUpdate = std::chrono::steady_clock::now();
    }
    void setDate(ClockDate Date) {
        DateTime.setDate(Date);
        LastUpdate = std::chrono::steady_clock::now();
    }

    // methods
    void task() {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        const auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - LastUpdate);
        if(elapsedSeconds.count() == 0) { return; }

        wxDateTime current(DateTime.getDateDay(),
                             static_cast<wxDateTime::Month>(DateTime.getDateMonth() - 1u),
                             DateTime.getDateYear(),
                             DateTime.getTimeHour(),
                             DateTime.getTimeMinute(),
                             DateTime.getTimeSecond());
        current += wxTimeSpan::Seconds(elapsedSeconds.count());
        updateDateTime(current);
        LastUpdate += elapsedSeconds;
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
