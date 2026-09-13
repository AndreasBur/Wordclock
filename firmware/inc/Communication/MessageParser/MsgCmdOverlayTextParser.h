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
    static MinuteType getPeriodInMinutes() { return Overlays::getInstance().getTextPeriodInMinutes(); }
    static SecondType getEnduranceInSeconds() { return Overlays::getInstance().getTextEnduranceInSeconds(); }
    static MonthType getMonth() { return Overlays::getInstance().getTextMonth(); }
    static DayType getDay() { return Overlays::getInstance().getTextDay(); }
    static DayType getValidInDays() { return Overlays::getInstance().getTextValidInDays(); }
    static byte getSpeed() { return Overlays::getInstance().getTextSpeed(); }
    static FontType getFont() {return Overlays::getInstance().getTextFont(); }
    static bool getIsActive() { return Overlays::getInstance().getTextIsActive(); }
    static const char* getText() { return Overlays::getInstance().getTextText(); }

    static StdReturnType setPeriodInMinutes(MinuteType PeriodInMinutes) { return Overlays::getInstance().setTextPeriodInMinutes(PeriodInMinutes); }
    static StdReturnType setEnduranceInSeconds(SecondType EnduranceInSeconds) { return Overlays::getInstance().setTextEnduranceInSeconds(EnduranceInSeconds); }
    static void setMonth(MonthType Month) { Overlays::getInstance().setTextMonth(Month); }
    static void setDay(DayType Day) { Overlays::getInstance().setTextDay(Day); }
    static void setValidInDays(DayType ValidInDays) { Overlays::getInstance().setTextValidInDays(ValidInDays); }
    static void setSpeed(byte Speed) { Overlays::getInstance().setTextSpeed(Speed); }
    static StdReturnType setFont(FontType Font) { return Overlays::getInstance().setTextFont(Font); }
    static void setIsActive(bool IsActive) { Overlays::getInstance().setTextIsActive(IsActive); }
    static void setText(const char* Text, LengthType Length) { Overlays::getInstance().setTextText(Text, Length); }

    static void sendAnswerText(bool AppendSpace) { sendAnswerParameter(TextOptionShortName, getText(), AppendSpace); }
    static void sendAnswerSpeed(bool AppendSpace) { sendAnswerParameter(SpeedOptionShortName, getSpeed(), AppendSpace); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
