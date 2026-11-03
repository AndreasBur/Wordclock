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
    friend class Overlay;
    static constexpr byte kDateStringLength{10u + 1u};
    char date_string_[kDateStringLength]{0u};

    // functions
    void SetStateToShow(ClockDate current_date, ClockTime current_time) {
        SetDateString(current_date);
        SetText();
        UNUSED(current_time);
    }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) {
        Text::GetInstance().Stop();
        UNUSED(current_date);
        UNUSED(current_time);
    }

    /* The date is always there to be shown; only the temperature has to wait for a
       reading. Answered here rather than defaulted in the base, so that adding an overlay
       is a decision about this and not a silent inheritance of it. */
    static bool IsReady() { return true; }

    /* Nothing of its own: the string it shows is built from the current date on every
       start, so there is no configuration here for a reset to undo. */
    static void ResetOwnToDefaults() { }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::StateType::kIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(date_string_, GetFont()); }

    void SetDateString(ClockDate current_date) {
        char* tmp = date_string_;
        itoa(current_date.GetDay(), tmp, 10u);
        tmp = CheckDigitsAndAppendDot(current_date.GetDay(), tmp);
        itoa(current_date.GetMonth(), tmp, 10u);
        tmp = CheckDigitsAndAppendDot(current_date.GetMonth(), tmp);
        itoa(current_date.GetYear(), tmp, 10u);
    }

    static char* AppendDot(char* string) { string[0u] = '.'; return &string[1u]; }
    static char* CheckDigitsAndAppendDot(uint16_t value, char* string) { return AppendDot(&string[DigitsOfNumber(value)]); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayDate() { }
    ~OverlayDate() { }

	// get methods


	// set methods

	// methods
    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == StateType::kShow) { ShowTask(); }
        return Overlay::Task(show_timer_in_seconds, current_date, current_time);
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
