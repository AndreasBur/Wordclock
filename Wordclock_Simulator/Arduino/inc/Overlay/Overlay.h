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
        STATE_SHOW_ACTIVE,
        STATE_SHOW_IDLE
    };

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
    StateType State{STATE_DISABLED};
    byte PeriodInMinutes{1u};
    byte EnduranceInSeconds{1u};
    byte Month{0u};
    byte Day{0u};
    byte ValidInDays{0u};
    ClockDateTime DT;
    byte ShowTimerInSeconds;

    // functions
    bool isShowTimerExpired() { return ShowTimerInSeconds == 0u; }
    bool setShowTimer(byte Seconds) { ShowTimerInSeconds = Seconds; }

    bool isDateSet() const { return (Month != 0u) || (Day != 0u); }
    bool isDayAndMonthSet() const { return isDaySet() && isMonthSet(); }
    bool isMonthMatching(byte currentMonth) const { return currentMonth == Month; }
    bool isDayMatching(byte currentDay) const { return currentDay == Day; }
    bool isDateMatchingDayAndMonthSet(byte currentMonth, byte currentDay) const { return isMonthMatching(currentMonth) && isDayMatching(currentDay); }
    bool isDateMatchingMonthSet(byte currentMonth) const { return currentMonth == Month; }
    bool isDateMatchingDaySet(byte currentDay) const { return currentDay == Day; }
    isDaySet() const { return Day != 0u; }
    isMonthSet() const { return Month != 0u; }

    isDateMachtching(byte currentMonth, byte currentDay) const {
        if(isDayAndMonthSet()) { return isDateMatchingDayAndMonthSet(currentMonth, currentDay); }
        if(isMonthSet()) { return isDateMatchingMonthSet(currentMonth); }
        if(isDaySet()) { return isDateMatchingDaySet(currentDay); }
    }

    bool isDayStillValid(byte currentMonth, byte currentDay) {
        if(isMonthMatching(currentMonth)) { return (Day + ValidInDays) <= currentDay; }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:

	// get methods
    byte getPeriodInMinutes() const { return PeriodInMinutes; }
    byte getEnduranceInSeconds() const { return EnduranceInSeconds; }
    byte getMonth() const { return Month; }
    byte getDay() const { return Day; }
    byte getValidInDays() const { return ValidInDays; }
    bool getIsActive() const { return State != STATE_DISABLED; }

	// set methods
    void setPeriodInMinutes(byte sPeriodInMinutes) { PeriodInMinutes = sPeriodInMinutes; }
    void setEnduranceInSeconds(byte sEnduranceInSeconds) { EnduranceInSeconds = sEnduranceInSeconds; }
    void setMonth(byte sMonth) { Month = sMonth; }
    void setDay(byte sDay) { Day = sDay; }
    void setValidInDays(byte sValidInDays) { ValidInDays = sValidInDays; }
    void setIsActive(bool sIsActive) { if(sIsActive) { enableIsActive(); } else { disableIsActive(); } }

	// methods
    void enableIsActive() { if(State == STATE_DISABLED) { State = STATE_IDLE; } }
    void disableIsActive() { State = STATE_DISABLED; }

    task() {
        byte currentDay = DT.getDateDay();
        byte currentMonth = DT.getDateMonth();


    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
