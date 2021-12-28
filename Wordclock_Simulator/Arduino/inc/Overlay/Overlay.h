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

    using YearType = ClockDate::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using HourType = ClockDateTime::HourType;
    using MinuteType = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    StateType State{STATE_DISABLED};
    // functions
    constexpr Overlay() { }
    ~Overlay() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr SecondType SecondToStartShow{30u};
    MinuteType PeriodInMinutes{1u};
    SecondType EnduranceInSeconds{1u};
    MonthType Month{0u};
    DayType Day{0u};
    DayType ValidInDays{0u};

    // functions
    bool isShowTimerExpired(SecondType ShowTimerInSeconds) const { return ShowTimerInSeconds == 0u; }
    SecondType decrementShowTimer(SecondType ShowTimerInSeconds) {
        if(ShowTimerInSeconds > 0) { return ShowTimerInSeconds - 1u; }
        else { return 0u; }
    }

    bool isPeriodValid(MinuteType Period) const { return Period > 0u; }
    bool isEnduranceValid(SecondType Endurance) const { return Endurance > 0u; }
    bool isDateSet() const { return (Month != 0u) || (Day != 0u); }
    bool isDayAndMonthSet() const { return isDaySet() && isMonthSet(); }
    bool isMonthMatching(MonthType CurrentMonth) const { return CurrentMonth == Month; }
    bool isDayMatching(DayType CurrentDay) const { return CurrentDay == Day; }
    bool isDateMatchingDayAndMonthSet(MonthType CurrentMonth, DayType CurrentDay) const { return isMonthMatching(CurrentMonth) && isDayMatching(CurrentDay); }
    bool isDateMatchingMonthSet(MonthType CurrentMonth) const { return CurrentMonth == Month; }
    bool isDateMatchingDaySet(DayType CurrentDay) const { return CurrentDay == Day; }
    bool isMinuteMatching(MinuteType CurrentMinute) const { return !(CurrentMinute % PeriodInMinutes); }
    bool isSecondMatching(SecondType CurrentSecond) const { return CurrentSecond == SecondToStartShow; }
    bool isDaySet() const { return Day != 0u; }
    bool isMonthSet() const { return Month != 0u; }
    bool isNextYear(MonthType CurrentMonth) const { return CurrentMonth < Month; }

    bool isTimeMatching(MinuteType CurrentMinute, SecondType CurrentSecond) const { return isMinuteMatching(CurrentMinute) && isSecondMatching(CurrentSecond); }
    bool isDateMachtching(MonthType CurrentMonth, DayType CurrentDay) const {
        if(isDayAndMonthSet()) { return isDateMatchingDayAndMonthSet(CurrentMonth, CurrentDay); }
        if(isMonthSet()) { return isDateMatchingMonthSet(CurrentMonth); }
        if(isDaySet()) { return isDateMatchingDaySet(CurrentDay); }
    }

    bool isDateValid(ClockDate CurrentDate) const {
        YearType year = CurrentDate.getYear();
        if(isNextYear(CurrentDate.getMonth())) { year--;}
        return CurrentDate.getRataDieDay(year, Month, Day) < ValidInDays;
    }

    SecondType setStateToShow() {
        State = STATE_SHOW;
        return EnduranceInSeconds;
    }

    SecondType showTimerTask(SecondType ShowTimerInSeconds) {
        return decrementShowTimer(ShowTimerInSeconds);
    }

    SecondType checkTimeAndSetStateToShow(SecondType ShowTimerInSeconds, ClockTime CurrentTime) {
        if(isTimeMatching(CurrentTime.getMinute(), CurrentTime.getSecond())) {
            return setStateToShow();
        }
        return ShowTimerInSeconds;
    }

    SecondType idleTask(SecondType ShowTimerInSeconds, ClockDate CurrentDate, ClockTime CurrentTime) {
        if(isDateSet()) {
            if(isDateValid(CurrentDate)) {
                return checkTimeAndSetStateToShow(ShowTimerInSeconds, CurrentTime);
            }
        } else {
            return checkTimeAndSetStateToShow(ShowTimerInSeconds, CurrentTime);
        }
        return ShowTimerInSeconds;
    }

    SecondType showTask(SecondType ShowTimerInSeconds) {
        if(isShowTimerExpired(ShowTimerInSeconds)) { State = STATE_IDLE; }
        return showTimerTask(ShowTimerInSeconds);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
	// get methods
	StateType getState() const { return State; }
    MinuteType getPeriodInMinutes() const { return PeriodInMinutes; }
    SecondType getEnduranceInSeconds() const { return EnduranceInSeconds; }
    MonthType getMonth() const { return Month; }
    DayType getDay() const { return Day; }
    DayType getValidInDays() const { return ValidInDays; }
    bool getIsActive() const { return State != STATE_DISABLED; }

	// set methods
    StdReturnType setPeriodInMinutes(MinuteType sPeriodInMinutes) {
        if(isPeriodValid(sPeriodInMinutes)) {
            PeriodInMinutes = sPeriodInMinutes;
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }
    StdReturnType setEnduranceInSeconds(SecondType sEnduranceInSeconds) {
        if(isEnduranceValid(sEnduranceInSeconds)) {
            EnduranceInSeconds = sEnduranceInSeconds;
            return E_OK;
        } else {
            return E_NOT_OK;
        }
    }
    void setMonth(MonthType sMonth) { Month = sMonth; }
    void setDay(DayType sDay) { Day = sDay; }
    void setValidInDays(DayType sValidInDays) { ValidInDays = sValidInDays; }
    void setIsActive(bool sIsActive) { if(sIsActive) { enableIsActive(); } else { disableIsActive(); } }

	// methods
    void enableIsActive() { if(State == STATE_DISABLED) { State = STATE_IDLE; } }
    void disableIsActive() { State = STATE_DISABLED; }

    SecondType task(SecondType ShowTimerInSeconds, ClockDate Date, ClockTime Time) {
        if(State == STATE_IDLE) { return idleTask(ShowTimerInSeconds, Date, Time); }
        if(State == STATE_SHOW) { return showTask(ShowTimerInSeconds); }
        return 0u;
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
