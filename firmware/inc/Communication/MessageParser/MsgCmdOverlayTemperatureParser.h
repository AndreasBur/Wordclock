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
/**     \file       MsgCmdOverlayTemperatureParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_
#define _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdOverlayTemperatureParser configuration parameter */


/* MsgCmdOverlayTemperatureParser parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdOverlayTemperatureParser : public MsgCmdOverlayParser<MsgCmdOverlayTemperatureParser>
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
    constexpr MsgCmdOverlayTemperatureParser(const char* Parameter) : MsgCmdOverlayParser(Parameter) { }
    ~MsgCmdOverlayTemperatureParser() { }

    // get methods

    // set methods

    // methods
    static MinuteType getPeriodInMinutes() { return Overlays::getInstance().getTemperaturePeriodInMinutes(); }
    static SecondType getEnduranceInSeconds() { return Overlays::getInstance().getTemperatureEnduranceInSeconds(); }
    static MonthType getMonth() { return Overlays::getInstance().getTemperatureMonth(); }
    static DayType getDay() { return Overlays::getInstance().getTemperatureDay(); }
    static DayType getValidInDays() { return Overlays::getInstance().getTemperatureValidInDays(); }
    static bool getIsActive() { return Overlays::getInstance().getTemperatureIsActive(); }
    static byte getSpeed() { return Overlays::getInstance().getTemperatureSpeed(); }
    static FontType getFont() {return Overlays::getInstance().getTemperatureFont(); }

    static StdReturnType setPeriodInMinutes(MinuteType PeriodInMinutes) { return Overlays::getInstance().setTemperaturePeriodInMinutes(PeriodInMinutes); }
    static StdReturnType setEnduranceInSeconds(SecondType EnduranceInSeconds) {return Overlays::getInstance().setTemperatureEnduranceInSeconds(EnduranceInSeconds); }
    static void setMonth(MonthType Month) { Overlays::getInstance().setTemperatureMonth(Month); }
    static void setDay(DayType Day) { Overlays::getInstance().setTemperatureDay(Day); }
    static void setValidInDays(DayType ValidInDays) { Overlays::getInstance().setTemperatureValidInDays(ValidInDays); }
    static void setSpeed(byte Speed) { Overlays::getInstance().setTemperatureSpeed(Speed); }
    static StdReturnType setFont(FontType Font) { return Overlays::getInstance().setTemperatureFont(Font); }
    static void setIsActive(bool IsActive) { Overlays::getInstance().setTemperatureIsActive(IsActive); }
    static void setText(const char* Text, LengthType Length) { UNUSED(Text); UNUSED(Length); }

    static void sendAnswerText(bool AppendSpace) { UNUSED(AppendSpace); }
    static void sendAnswerSpeedText(bool AppendSpace) { UNUSED(AppendSpace); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
