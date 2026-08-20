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
/**     \file       MsgCmdColorCycleParser.h
 *      \brief      Switches the colour cycle on and off, and says how fast it walks
 *
 *      \details    Two settings and one reading. The speed is the same kind of number the
 *                  animation speed is - scheduler ticks per step - so a clock's two speed
 *                  settings mean the same thing rather than each meaning something.
 *
 *                  The hue is answered but cannot be set. It is where the wheel happens to
 *                  be, which makes it worth reading back while somebody is watching a
 *                  colour they cannot name; setting it would be command 2's job, and that
 *                  one already sets a colour properly.
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_COLOR_CYCLE_PARSER_H_
#define _MSG_CMD_COLOR_CYCLE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "ColorCycle.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
#define MSG_CMD_COLOR_CYCLE_PARSER_PARAMETER_TABLE_SIZE         2u

/******************************************************************************************************************************************************
 *  C L A S S   M S G   C M D   C O L O R   C Y C L E   P A R S E R
******************************************************************************************************************************************************/
class MsgCmdColorCycleParser : public MsgParameterParser<MsgCmdColorCycleParser, MSG_CMD_COLOR_CYCLE_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char ActiveShortName{'A'};
    static constexpr char SpeedShortName{'S'};
    static constexpr char HueShortName{'H'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(ActiveShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(SpeedShortName,  MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    /* Collected rather than applied where it arrives, and applied after the speed: switching
       the cycle on puts a colour on the strip at once, and doing that before the speed of the
       same command has landed would start the wheel at the old rate for one step. */
    bool Active{false};
    bool ActiveGiven{false};

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) {
        UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length);
    }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == ActiveShortName) { Active = (Argument != 0u); ActiveGiven = true; }
        if(ParameterShortName == SpeedShortName)  { ColorCycle::getInstance().setTaskCycle(Argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdColorCycleParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdColorCycleParser() { }

    // methods
    void sendAnswer() const {
        const ColorCycle& colorCycle = ColorCycle::getInstance();

        sendAnswerParameter(ActiveShortName, static_cast<byte>(colorCycle.getIsActive() ? 1u : 0u));
        sendAnswerParameter(SpeedShortName, colorCycle.getTaskCycle());
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        sendAnswerParameter(HueShortName, colorCycle.getHue(), false);
    }

    void process() const {
        if(ActiveGiven) { ColorCycle::getInstance().setIsActive(Active); }
    }
};

#endif // _MSG_CMD_COLOR_CYCLE_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
