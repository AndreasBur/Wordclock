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
/**     \file       OverlayText.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _OVERLAY_TEXT_H_
#define _OVERLAY_TEXT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* OverlayText configuration parameter */
#define OVERLAY_TEXT_TEXT_SIZE              50u

/* OverlayText parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y   T E X T
******************************************************************************************************************************************************/
class OverlayText : public Overlay<OverlayText>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using LengthType = StringTools::LengthType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class Overlay;
    /*
     * Initialised like the other two overlays' string buffers rather than left to the
     * singleton's static storage to zero. Persistence reads this one before anything has
     * written it, and a terminator that holds only because of where the object lives is
     * one refactor away from not holding.
     */
    char text_[OVERLAY_TEXT_TEXT_SIZE]{0u};

    // functions
    void SetStateToShow(ClockDate current_date, ClockTime current_time) { SetText(); UNUSED(current_date); UNUSED(current_time); }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) { Text::GetInstance().Stop(); UNUSED(current_date); UNUSED(current_time); }

    /*
     * An empty text is still a text - it is what was configured, and shortening the
     * overlay because of it would hide the mistake rather than show it. Only the
     * temperature can have nothing to show.
     */
    static bool IsReady() { return true; }

    /*
     * The only overlay with anything of its own to reset. An empty text rather than the
     * previous one, so a reset clock shows nothing rather than whatever the last owner
     * configured.
     */
    void ResetOwnToDefaults() { text_[0u] = STD_NULL_CHARACTER; }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::StateType::kIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(text_, GetFont()); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayText() : text_{StringTools::kNullCharacter} { }
    ~OverlayText() { }

	// get methods
    const char* GetText() const { return text_; }

    // set methods

	// set methods
    void SetText(const char* text, LengthType length) { StringTools::StringCopy(text_, text, length); }

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
