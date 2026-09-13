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
    static MinuteType getPeriodInMinutes() { return Overlays::getInstance().getDatePeriodInMinutes(); }
    static SecondType getEnduranceInSeconds() { return Overlays::getInstance().getDateEnduranceInSeconds(); }
    static MonthType getMonth() { return Overlays::getInstance().getDateMonth(); }
    static DayType getDay() { return Overlays::getInstance().getDateDay(); }
    static DayType getValidInDays() { return Overlays::getInstance().getDateValidInDays(); }
    static bool getIsActive() { return Overlays::getInstance().getDateIsActive(); }
    static byte getSpeed() { return Overlays::getInstance().getDateSpeed(); }
    static FontType getFont() {return Overlays::getInstance().getDateFont(); }

    static StdReturnType setPeriodInMinutes(MinuteType PeriodInMinutes) { return Overlays::getInstance().setDatePeriodInMinutes(PeriodInMinutes); }
    static StdReturnType setEnduranceInSeconds(SecondType EnduranceInSeconds) { return Overlays::getInstance().setDateEnduranceInSeconds(EnduranceInSeconds); }
    static void setMonth(MonthType Month) { Overlays::getInstance().setDateMonth(Month); }
    static void setDay(DayType Day) { Overlays::getInstance().setDateDay(Day); }
    static void setValidInDays(DayType ValidInDays) { Overlays::getInstance().setDateValidInDays(ValidInDays); }
    static void setSpeed(byte Speed) { Overlays::getInstance().setDateSpeed(Speed); }
    static StdReturnType setFont(FontType Font) { return Overlays::getInstance().setDateFont(Font); }
    static void setIsActive(bool IsActive) { Overlays::getInstance().setDateIsActive(IsActive); }
    static void setText(const char* Text, LengthType Length) { UNUSED(Text); UNUSED(Length); }

    static void sendAnswerText(bool AppendSpace) { UNUSED(AppendSpace); }
    static void sendAnswerSpeedText(bool AppendSpace) { UNUSED(AppendSpace); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
