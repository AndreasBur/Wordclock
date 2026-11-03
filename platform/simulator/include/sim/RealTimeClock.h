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
    ClockDateTime date_time_;
    std::chrono::steady_clock::time_point last_update_{std::chrono::steady_clock::now()};

    // functions
    RealTimeClock() {}
    ~RealTimeClock() {}

    void UpdateDateTime(const wxDateTime& value) {
        date_time_.SetDateYear(value.GetYear());
        date_time_.SetDateMonth(value.GetMonth() + 1u);
        date_time_.SetDateDay(value.GetDay());
        date_time_.SetTimeHour(value.GetHour());
        date_time_.SetTimeMinute(value.GetMinute());
        date_time_.SetTimeSecond(value.GetSecond());
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static RealTimeClock& GetInstance() {
        static RealTimeClock singleton_instance;
        return singleton_instance;
    }

    // get methods
    ClockDateTime GetDateTime() const { return date_time_; }
    ClockTime GetTime() const { return date_time_.GetTime(); }
    ClockDate GetDate() const { return date_time_.GetDate(); }

    // set methods
    void SetDateTime(ClockDateTime date_time) {
        date_time_ = date_time;
        last_update_ = std::chrono::steady_clock::now();
    }
    void SetTime(ClockTime time) {
        date_time_.SetTime(time);
        last_update_ = std::chrono::steady_clock::now();
    }
    void SetDate(ClockDate date) {
        date_time_.SetDate(date);
        last_update_ = std::chrono::steady_clock::now();
    }

    // methods
    void Task() {
        const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_);
        if(elapsed_seconds.count() == 0) { return; }

        wxDateTime current(date_time_.GetDateDay(),
                             static_cast<wxDateTime::Month>(date_time_.GetDateMonth() - 1u),
                             date_time_.GetDateYear(),
                             date_time_.GetTimeHour(),
                             date_time_.GetTimeMinute(),
                             date_time_.GetTimeSecond());
        current += wxTimeSpan::Seconds(elapsed_seconds.count());
        UpdateDateTime(current);
        last_update_ += elapsed_seconds;
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
