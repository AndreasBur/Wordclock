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
/**     \file       MsgCmdNightSwitchParser.h
 *      \brief      Configures when the clock takes the night off
 *
 *      \details    Five numbers and a switch: the two ends of the window, and how bright
 *                  the display is inside it, where zero means off altogether.
 *
 *                  Every option is optional, and what is not sent is left as it was - so
 *                  the end alone can be moved without repeating the start. The answer
 *                  reports the whole window whatever was sent, which is what makes a
 *                  half-remembered setting checkable in one line.
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_NIGHT_SWITCH_PARSER_H_
#define _MSG_CMD_NIGHT_SWITCH_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "NightSwitch.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
#define MSG_CMD_NIGHT_SWITCH_PARSER_PARAMETER_TABLE_SIZE        6u

/******************************************************************************************************************************************************
 *  C L A S S   M S G   C M D   N I G H T   S W I T C H   P A R S E R
******************************************************************************************************************************************************/
class MsgCmdNightSwitchParser : public MsgParameterParser<MsgCmdNightSwitchParser, MSG_CMD_NIGHT_SWITCH_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char kActiveShortName{'A'};
    static constexpr char kStartHourShortName{'H'};
    static constexpr char kStartMinuteShortName{'M'};
    static constexpr char kEndHourShortName{'E'};
    static constexpr char kEndMinuteShortName{'N'};
    static constexpr char kBrightnessShortName{'B'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kActiveShortName,      MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kStartHourShortName,   MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kStartMinuteShortName, MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kEndHourShortName,     MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kEndMinuteShortName,   MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kBrightnessShortName,  MsgParameter::kArgumentTypeUint8)
    };

    /* Collected rather than applied as they arrive, and the switch applied last: switching
       it on is what makes the next tick an edge, and an edge acting on half a window would
       use one end from this command and the other from the one before it. */
    bool active_{false};
    bool active_given_{false};

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) {
        UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length);
    }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        NightSwitch& night_switch = NightSwitch::GetInstance();

        if(parameter_short_name == kActiveShortName)      { active_ = (argument != 0u); active_given_ = true; }
        if(parameter_short_name == kStartHourShortName)   { night_switch.SetStartHour(argument); }
        if(parameter_short_name == kStartMinuteShortName) { night_switch.SetStartMinute(argument); }
        if(parameter_short_name == kEndHourShortName)     { night_switch.SetEndHour(argument); }
        if(parameter_short_name == kEndMinuteShortName)   { night_switch.SetEndMinute(argument); }
        if(parameter_short_name == kBrightnessShortName)  { night_switch.SetNightBrightness(argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdNightSwitchParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdNightSwitchParser() { }

    // methods
    static void SendAnswer() {
        const NightSwitch& night_switch = NightSwitch::GetInstance();

        SendAnswerParameter(kActiveShortName, static_cast<byte>(night_switch.GetIsActive() ? 1u : 0u));
        SendAnswerParameter(kStartHourShortName, night_switch.GetStartHour());
        SendAnswerParameter(kStartMinuteShortName, night_switch.GetStartMinute());
        SendAnswerParameter(kEndHourShortName, night_switch.GetEndHour());
        SendAnswerParameter(kEndMinuteShortName, night_switch.GetEndMinute());
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        SendAnswerParameter(kBrightnessShortName, night_switch.GetNightBrightness(), false);
    }

    void Process() const {
        if(active_given_) { NightSwitch::GetInstance().SetIsActive(active_); }
    }
};

#endif // _MSG_CMD_NIGHT_SWITCH_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
