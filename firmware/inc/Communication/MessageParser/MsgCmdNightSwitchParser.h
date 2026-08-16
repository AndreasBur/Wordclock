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

    static constexpr char ActiveShortName{'A'};
    static constexpr char StartHourShortName{'H'};
    static constexpr char StartMinuteShortName{'M'};
    static constexpr char EndHourShortName{'E'};
    static constexpr char EndMinuteShortName{'N'};
    static constexpr char BrightnessShortName{'B'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(ActiveShortName,      MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(StartHourShortName,   MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(StartMinuteShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(EndHourShortName,     MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(EndMinuteShortName,   MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(BrightnessShortName,  MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    /* Collected rather than applied as they arrive, and the switch applied last: switching
       it on is what makes the next tick an edge, and an edge acting on half a window would
       use one end from this command and the other from the one before it. */
    bool Active{false};
    bool ActiveGiven{false};

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) {
        UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length);
    }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        NightSwitch& nightSwitch = NightSwitch::getInstance();

        if(ParameterShortName == ActiveShortName)      { Active = (Argument != 0u); ActiveGiven = true; }
        if(ParameterShortName == StartHourShortName)   { nightSwitch.setStartHour(Argument); }
        if(ParameterShortName == StartMinuteShortName) { nightSwitch.setStartMinute(Argument); }
        if(ParameterShortName == EndHourShortName)     { nightSwitch.setEndHour(Argument); }
        if(ParameterShortName == EndMinuteShortName)   { nightSwitch.setEndMinute(Argument); }
        if(ParameterShortName == BrightnessShortName)  { nightSwitch.setNightBrightness(Argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdNightSwitchParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdNightSwitchParser() { }

    // methods
    void sendAnswer() const {
        const NightSwitch& nightSwitch = NightSwitch::getInstance();

        sendAnswerParameter(ActiveShortName, static_cast<byte>(nightSwitch.getIsActive() ? 1u : 0u));
        sendAnswerParameter(StartHourShortName, nightSwitch.getStartHour());
        sendAnswerParameter(StartMinuteShortName, nightSwitch.getStartMinute());
        sendAnswerParameter(EndHourShortName, nightSwitch.getEndHour());
        sendAnswerParameter(EndMinuteShortName, nightSwitch.getEndMinute());
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        sendAnswerParameter(BrightnessShortName, nightSwitch.getNightBrightness(), false);
    }

    void process() const {
        if(ActiveGiven) { NightSwitch::getInstance().setIsActive(Active); }
    }
};

#endif // _MSG_CMD_NIGHT_SWITCH_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
