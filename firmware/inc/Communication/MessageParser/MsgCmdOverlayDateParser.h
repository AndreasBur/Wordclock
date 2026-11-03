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
    constexpr MsgCmdOverlayDateParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }
    ~MsgCmdOverlayDateParser() { }

    // get methods

    // set methods

    // methods
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetDatePeriodInMinutes(); }
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetDateEnduranceInSeconds(); }
    static MonthType GetMonth() { return Overlays::GetInstance().GetDateMonth(); }
    static DayType GetDay() { return Overlays::GetInstance().GetDateDay(); }
    static DayType GetValidInDays() { return Overlays::GetInstance().GetDateValidInDays(); }
    static bool GetIsActive() { return Overlays::GetInstance().GetDateIsActive(); }
    static byte GetSpeed() { return Overlays::GetInstance().GetDateSpeed(); }
    static FontType GetFont() {return Overlays::GetInstance().GetDateFont(); }

    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetDatePeriodInMinutes(period_in_minutes); }
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) { return Overlays::GetInstance().SetDateEnduranceInSeconds(endurance_in_seconds); }
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetDateMonth(month); }
    static void SetDay(DayType day) { Overlays::GetInstance().SetDateDay(day); }
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetDateValidInDays(valid_in_days); }
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetDateSpeed(speed); }
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetDateFont(font); }
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetDateIsActive(is_active); }
    static void SetText(const char* text, LengthType length) { UNUSED(text); UNUSED(length); }

    static void SendAnswerText(bool append_space) { UNUSED(append_space); }
    static void SendAnswerSpeedText(bool append_space) { UNUSED(append_space); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
