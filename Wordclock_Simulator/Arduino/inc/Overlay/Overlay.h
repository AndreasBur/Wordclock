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
/**     \file       Overlay.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _OVERLAY_H_
#define _OVERLAY_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockDateTime.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Overlay configuration parameter */


/* Overlay parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y
******************************************************************************************************************************************************/
class Overlay
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        STATE_DISABLED,
        STATE_IDLE,
        STATE_SHOW
    };

    using MonthType = ClockDateTime::MonthType;
    using MonthRawType = ClockDateTime::MonthRawType;
    using DayType = ClockDateTime::DayType;
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:

    // functions
    constexpr Overlay() { }
    ~Overlay() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr SecondType SecondToStartShow{30u};
    StateType State{STATE_DISABLED};
    MinuteType PeriodInMinutes{1u};
    SecondType EnduranceInSeconds{1u};
    MonthRawType Month{0u};
    DayType Day{0u};
    DayType ValidInDays{0u};
    ClockDateTime DT;
    SecondType ShowTimerInSeconds{0u};
    SecondType SecondLast{0u};

    // functions
    bool isShowTimerExpired() const { return ShowTimerInSeconds == 0u; }
    void decrementShowTimer() { if(ShowTimerInSeconds > 0) { ShowTimerInSeconds--; } }

    bool isDateSet() const { return (Month != 0u) || (Day != 0u); }
    bool isDayAndMonthSet() const { return isDaySet() && isMonthSet(); }
    bool isMonthMatching(MonthRawType CurrentMonth) const { return CurrentMonth == Month; }
    bool isDayMatching(DayType CurrentDay) const { return CurrentDay == Day; }
    bool isDateMatchingDayAndMonthSet(MonthRawType CurrentMonth, DayType CurrentDay) const { return isMonthMatching(CurrentMonth) && isDayMatching(CurrentDay); }
    bool isDateMatchingMonthSet(MonthRawType CurrentMonth) const { return CurrentMonth == Month; }
    bool isDateMatchingDaySet(DayType CurrentDay) const { return CurrentDay == Day; }
    bool isMinuteMatching(MinuteType CurrentMinute) const { return CurrentMinute % PeriodInMinutes; }
    bool isSecondMatching(SecondType CurrentSecond) const { return CurrentSecond == SecondToStartShow; }
    bool isDaySet() const { return Day != 0u; }
    bool isMonthSet() const { return Month != 0u; }
    //bool isNextYear() const { return CurrentMonth > Month; }

    bool isTimeMatching(MinuteType CurrentMinute, SecondType CurrentSecond) const { return isMinuteMatching(CurrentMinute) && isSecondMatching(CurrentSecond); }
    bool isDateMachtching(MonthType CurrentMonth, DayType CurrentDay) const {
        if(isDayAndMonthSet()) { return isDateMatchingDayAndMonthSet(CurrentMonth, CurrentDay); }
        if(isMonthSet()) { return isDateMatchingMonthSet(CurrentMonth); }
        if(isDaySet()) { return isDateMatchingDaySet(CurrentDay); }
    }

    //bool isDayValid(DayType CurrentDay) const { return (Day + ValidInDays) <= CurrentDay; }

    bool isDateValid(ClockDate CurrentDate) const {
        //if(isMonthMatching(CurrentMonth)) {
        //    return isDayValid(CurrentDay);
        //} else
    }

    void setStateToShow() {
        ShowTimerInSeconds = EnduranceInSeconds;
        State = STATE_SHOW;
    }

    //SecondsType getPassedSeconds()
    void showTimerTask(SecondType CurrentSecond) {
        if(CurrentSecond != SecondLast) {
            SecondLast = CurrentSecond;
            decrementShowTimer();
        }
    }

    void idleTask(ClockDate CurrentDate, ClockTime CurrentTime) {
        if(isDateSet()) {
            if(isDateValid(CurrentDate)) {
                if(isTimeMatching(CurrentTime.getMinute(), CurrentTime.getSecond())) { setStateToShow(); }
            }
        } else { if(isTimeMatching(CurrentTime.getMinute(), CurrentTime.getSecond())) { setStateToShow(); } }
    }

    void showTask(ClockTime CurrentTime) {
        showTimerTask(CurrentTime.getSecond());
        if(isShowTimerExpired()) { State = STATE_IDLE; }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
	// get methods
    MinuteType getPeriodInMinutes() const { return PeriodInMinutes; }
    SecondType getEnduranceInSeconds() const { return EnduranceInSeconds; }
    MonthRawType getMonth() const { return Month; }
    DayType getDay() const { return Day; }
    DayType getValidInDays() const { return ValidInDays; }
    bool getIsActive() const { return State != STATE_DISABLED; }

	// set methods
    void setPeriodInMinutes(MinuteType sPeriodInMinutes) { PeriodInMinutes = sPeriodInMinutes; }
    void setEnduranceInSeconds(SecondType sEnduranceInSeconds) { EnduranceInSeconds = sEnduranceInSeconds; }
    void setMonth(MonthRawType sMonth) { Month = sMonth; }
    void setDay(DayType sDay) { Day = sDay; }
    void setValidInDays(DayType sValidInDays) { ValidInDays = sValidInDays; }
    void setIsActive(bool sIsActive) { if(sIsActive) { enableIsActive(); } else { disableIsActive(); } }

	// methods
    void enableIsActive() { if(State == STATE_DISABLED) { State = STATE_IDLE; } }
    void disableIsActive() { State = STATE_DISABLED; }

    task() {
        ClockTime time = DT.getTime();
        ClockDate date = DT.getDate();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
