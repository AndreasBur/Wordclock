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

    // functions

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdOverlayTextParser(const char* Parameter) : MsgCmdOverlayParser(Parameter) { }
    ~MsgCmdOverlayTextParser() { }

    // get methods

    // set methods

    // methods
    MinuteType getPeriodInMinutes() const { return Overlays::getInstance().getTextPeriodInMinutes(); }
    SecondType getEnduranceInSeconds() const { return Overlays::getInstance().getTextEnduranceInSeconds(); }
    MonthType getMonth() const { return Overlays::getInstance().getTextMonth(); }
    DayType getDay() const { return Overlays::getInstance().getTextDay(); }
    DayType getValidInDays() const { return Overlays::getInstance().getTextValidInDays(); }
    byte getSpeed() const { return Overlays::getInstance().getTextSpeed(); }
    FontType getFont() const {return Overlays::getInstance().getTextFont(); }
    bool getIsActive() const { return Overlays::getInstance().getTextIsActive(); }
    const char* getText() const { return Overlays::getInstance().getTextText(); }

    StdReturnType setPeriodInMinutes(MinuteType PeriodInMinutes) { return Overlays::getInstance().setTextPeriodInMinutes(PeriodInMinutes); }
    StdReturnType setEnduranceInSeconds(SecondType EnduranceInSeconds) { return Overlays::getInstance().setTextEnduranceInSeconds(EnduranceInSeconds); }
    void setMonth(MonthType Month) { Overlays::getInstance().setTextMonth(Month); }
    void setDay(DayType Day) { Overlays::getInstance().setTextDay(Day); }
    void setValidInDays(DayType ValidInDays) { Overlays::getInstance().setTextValidInDays(ValidInDays); }
    void setSpeed(byte Speed) { Overlays::getInstance().setTextSpeed(Speed); }
    StdReturnType setFont(FontType Font) { return Overlays::getInstance().setTextFont(Font); }
    void setIsActive(bool IsActive) { Overlays::getInstance().setTextIsActive(IsActive); }
    void setText(const char* Text, LengthType Length) { Overlays::getInstance().setTextText(Text, Length); }

    void sendAnswerText(bool AppendSpace) const { sendAnswerParameter(TextOptionShortName, getText(), AppendSpace); }
    void sendAnswerSpeed(bool AppendSpace) const { sendAnswerParameter(SpeedOptionShortName, getSpeed(), AppendSpace); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
