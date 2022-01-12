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
/**     \file       OverlayDate.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _OVERLAY_DATE_H_
#define _OVERLAY_DATE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"
#include <stdlib.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* OverlayDate configuration parameter */


/* OverlayDate parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y   D A T E
******************************************************************************************************************************************************/
class OverlayDate : public Overlay<OverlayDate>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte DateStringLength{10u + 1u};
    friend class Overlay;
    char DateString[DateStringLength]{0u};

    // functions
    void setStateToShow(ClockDate CurrentDate, ClockTime CurrentTime) {
        setDateString(CurrentDate);
        setText();
    }
    void setStateToIdle(ClockDate CurrentDate, ClockTime CurrentTime) { Text::getInstance().stop(); }

    void showTask() { if(Text::getInstance().getState() == Text::STATE_IDLE) { setText(); } }
    void setText() { Text::getInstance().setTextWithShift(DateString, getFont()); }

    void setDateString(ClockDate CurrentDate) {
        char* tmp = DateString;
        dayToString(CurrentDate.getDay(), tmp);
        tmp = checkDigitsAndAppendDot(CurrentDate.getDay(), tmp);
        monthToString(CurrentDate.getMonth(), tmp);
        tmp = checkDigitsAndAppendDot(CurrentDate.getMonth(), tmp);
        yearToString(CurrentDate.getYear(), tmp);
    }

    char* dayToString(DayType Day, char* DayString) { return itoa(Day, DayString, 10u); }
    char* monthToString(MonthType Month, char* MonthString) { return itoa(Month, MonthString, 10u); }
    char* yearToString(YearType Year, char* YearString) { return itoa(Year, YearString, 10u); }
    char* appendDot(char* String) { String[0u] = '.'; return &String[1u]; }
    char* checkDigitsAndAppendDot(uint16_t Value, char* String) { return appendDot(&String[digitsOfNumber(Value)]); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayDate() { }
    ~OverlayDate() { }

	// get methods


	// set methods

	// methods
    SecondType task(SecondType ShowTimerInSeconds, ClockDate CurrentDate, ClockTime CurrentTime) {
        if(State == STATE_SHOW) { showTask(); }
        return Overlay::task(ShowTimerInSeconds, CurrentDate, CurrentTime);
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
