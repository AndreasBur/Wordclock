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
/**     \file       MsgCmdOverlayTextParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_OVERLAY_TEXT_PARSER_H_
#define _MSG_CMD_OVERLAY_TEXT_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"
#include "Overlays.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdOverlayTextParser configuration parameter */


/* MsgCmdOverlayTextParser parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdOverlayTextParser : public MsgCmdOverlayParser<MsgCmdOverlayTextParser>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgCmdOverlayParser;


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdOverlayTextParser(const char* Parameter) : MsgCmdOverlayParser(Parameter) { }
    ~MsgCmdOverlayTextParser() { }

    // get methods

    // set methods

    // methods
    byte getPeriodInMinutes() const { return Overlays::getInstance().getTextPeriodInMinutes(); }
    byte getEnduranceInSeconds() const { return Overlays::getInstance().getTextEnduranceInSeconds(); }
    byte getMonth() const { return Overlays::getInstance().getTextMonth(); }
    byte getDay() const { return Overlays::getInstance().getTextDay(); }
    byte getValidInDays() const { return Overlays::getInstance().getTextValidInDays(); }
    byte getIsActive() const { return Overlays::getInstance().getTextIsActive(); }

    void setPeriodInMinutes(byte PeriodInMinutes) { Overlays::getInstance().setTextPeriodInMinutes(PeriodInMinutes); }
    void setEnduranceInSeconds(byte EnduranceInSeconds) {Overlays::getInstance().setTextEnduranceInSeconds(EnduranceInSeconds); }
    void setMonth(byte Month) { Overlays::getInstance().setTextMonth(Month); }
    void setDay(byte Day) { Overlays::getInstance().setTextDay(Day); }
    void setValidInDays(byte ValidInDays) { Overlays::getInstance().setTextValidInDays(ValidInDays); }
    void setIsActive(bool IsActive) { Overlays::getInstance().setTextIsActive(IsActive); }
    void setText(const char* Text, PositionType Length) { }

    void sendAnswerText(bool AppendSpace) {}

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
