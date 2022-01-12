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
    char Text[OVERLAY_TEXT_TEXT_SIZE];

    // functions
    void setStateToShow(ClockDate CurrentDate, ClockTime CurrentTime) { setText(); }
    void setStateToIdle(ClockDate CurrentDate, ClockTime CurrentTime) { Text::getInstance().stop(); }

    void showTask() { if(Text::getInstance().getState() == Text::STATE_IDLE) { setText(); } }
    void setText() { Text::getInstance().setTextWithShift(Text, getFont()); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayText() : Text{StringTools::NullCharacter} { }
    ~OverlayText() { }

	// get methods
    const char* getText() const { return Text; }

    // set methods

	// set methods
    void setText(const char* sText, LengthType Length) { StringTools::stringCopy(Text, sText, Length); }

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
