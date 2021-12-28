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
/**     \file       MsgCmdOverlayDateParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_OVERLAY_DATE_PARSER_H_
#define _MSG_CMD_OVERLAY_DATE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdOverlayDateParser configuration parameter */


/* MsgCmdOverlayDateParser parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdOverlayDateParser : public MsgCmdOverlayParser<MsgCmdOverlayDateParser>
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
    constexpr MsgCmdOverlayDateParser(const char* Parameter) : MsgCmdOverlayParser(Parameter) { }
    ~MsgCmdOverlayDateParser() { }

    // get methods

    // set methods

    // methods
    byte getPeriodInMinutes() const { return Overlays::getInstance().getDatePeriodInMinutes(); }
    byte getEnduranceInSeconds() const { return Overlays::getInstance().getDateEnduranceInSeconds(); }
    byte getMonth() const { return Overlays::getInstance().getDateMonth(); }
    byte getDay() const { return Overlays::getInstance().getDateDay(); }
    byte getValidInDays() const { return Overlays::getInstance().getDateValidInDays(); }
    byte getIsActive() const { return Overlays::getInstance().getDateIsActive(); }

    StdReturnType setPeriodInMinutes(byte PeriodInMinutes) { return Overlays::getInstance().setDatePeriodInMinutes(PeriodInMinutes); }
    StdReturnType setEnduranceInSeconds(byte EnduranceInSeconds) { return Overlays::getInstance().setDateEnduranceInSeconds(EnduranceInSeconds); }
    void setMonth(byte Month) { Overlays::getInstance().setDateMonth(Month); }
    void setDay(byte Day) { Overlays::getInstance().setDateDay(Day); }
    void setValidInDays(byte ValidInDays) { Overlays::getInstance().setDateValidInDays(ValidInDays); }
    void setIsActive(bool IsActive) { Overlays::getInstance().setDateIsActive(IsActive); }
    void setText(const char* Text, LengthType Length) { }
    void setTextSpeed(byte Speed) { }

    void sendAnswerText(bool AppendSpace) const {}
    void sendAnswerSpeedText(bool AppendSpace) const {}

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
