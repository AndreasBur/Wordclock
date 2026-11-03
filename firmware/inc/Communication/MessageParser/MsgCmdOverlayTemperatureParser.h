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
    constexpr MsgCmdOverlayTemperatureParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }
    ~MsgCmdOverlayTemperatureParser() { }

    // get methods

    // set methods

    // methods
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetTemperaturePeriodInMinutes(); }
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetTemperatureEnduranceInSeconds(); }
    static MonthType GetMonth() { return Overlays::GetInstance().GetTemperatureMonth(); }
    static DayType GetDay() { return Overlays::GetInstance().GetTemperatureDay(); }
    static DayType GetValidInDays() { return Overlays::GetInstance().GetTemperatureValidInDays(); }
    static bool GetIsActive() { return Overlays::GetInstance().GetTemperatureIsActive(); }
    static byte GetSpeed() { return Overlays::GetInstance().GetTemperatureSpeed(); }
    static FontType GetFont() {return Overlays::GetInstance().GetTemperatureFont(); }

    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetTemperaturePeriodInMinutes(period_in_minutes); }
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) {return Overlays::GetInstance().SetTemperatureEnduranceInSeconds(endurance_in_seconds); }
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetTemperatureMonth(month); }
    static void SetDay(DayType day) { Overlays::GetInstance().SetTemperatureDay(day); }
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetTemperatureValidInDays(valid_in_days); }
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetTemperatureSpeed(speed); }
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetTemperatureFont(font); }
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetTemperatureIsActive(is_active); }
    static void SetText(const char* text, LengthType length) { UNUSED(text); UNUSED(length); }

    static void SendAnswerText(bool append_space) { UNUSED(append_space); }
    static void SendAnswerSpeedText(bool append_space) { UNUSED(append_space); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
