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
#include "Text.h"

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
class OverlayText : public Overlay
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
    char Text[OVERLAY_TEXT_TEXT_SIZE];

    // functions
    byte convertSpeedToTaskCycle(byte Speed) const { return UINT8_MAX - Speed; }
    byte convertTaskCycleToSpeed(byte TaskCylce) const { return UINT8_MAX - TaskCylce; }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayText() : Text{StringTools::NullCharacter} { }
    ~OverlayText() { }

	// get methods
    const char* getText() const { return Text; }
    byte getSpeed() const { return convertTaskCycleToSpeed(Text::getInstance().getTaskCycle()); }

    // set methods

	// set methods
    void setText(const char* sText, LengthType Length) { StringTools::stringCopy(Text, sText, Length); }
    void setSpeed(byte Cycle) { Text::getInstance().setTaskCycle(convertSpeedToTaskCycle(Cycle)); }

	// methods
    SecondType task(SecondType ShowTimerInSeconds, ClockDate Date, ClockTime Time) {
        StateType stateOld = Overlay::getState();
        ShowTimerInSeconds = Overlay::task(ShowTimerInSeconds, Date, Time);
        if(State == STATE_SHOW && stateOld == STATE_IDLE) {
            Text::getInstance().setTextWithShift(Text, Text::FONT_10X10);
        }
        return ShowTimerInSeconds;
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
