// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Switches the colour cycle on and off, and says how fast it walks.
 *
 * Two settings and one reading. The speed is the same kind of number the animation speed is -
 * scheduler ticks per step - so a clock's two speed settings mean the same thing rather than each
 * meaning something.
 *
 * The hue is answered but cannot be set. It is where the wheel happens to be, which makes it worth
 * reading back while somebody is watching a colour they cannot name; setting it would be command
 * 2's job, and that one already sets a colour properly.
 */

#ifndef _MSG_CMD_COLOR_CYCLE_PARSER_H_
#define _MSG_CMD_COLOR_CYCLE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "ColorCycle.h"

#define MSG_CMD_COLOR_CYCLE_PARSER_PARAMETER_TABLE_SIZE         2u

class MsgCmdColorCycleParser : public MsgParameterParser<MsgCmdColorCycleParser, MSG_CMD_COLOR_CYCLE_PARSER_PARAMETER_TABLE_SIZE>
{
  private:
    friend class MsgParameterParser;

    static constexpr char kActiveShortName{'A'};
    static constexpr char kSpeedShortName{'S'};
    static constexpr char kHueShortName{'H'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kActiveShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kSpeedShortName,  MsgParameter::ArgumentTypeType::kUint8)
    };

    /*
     * Collected rather than applied where it arrives, and applied after the speed: switching
     * the cycle on puts a colour on the strip at once, and doing that before the speed of the
     * same command has landed would start the wheel at the old rate for one step.
     */
    bool active_{false};
    bool active_given_{false};

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) {
        UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length);
    }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kActiveShortName) { active_ = (argument != 0u); active_given_ = true; }
        if(parameter_short_name == kSpeedShortName)  { ColorCycle::GetInstance().SetTaskCycle(argument); }
    }

  public:
    constexpr MsgCmdColorCycleParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdColorCycleParser() { }

    // methods
    static void SendAnswer() {
        const ColorCycle& color_cycle = ColorCycle::GetInstance();

        SendAnswerParameter(kActiveShortName, static_cast<byte>(color_cycle.GetIsActive() ? 1u : 0u));
        SendAnswerParameter(kSpeedShortName, color_cycle.GetTaskCycle());
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        SendAnswerParameter(kHueShortName, color_cycle.GetHue(), false);
    }

    void Process() const {
        if(active_given_) { ColorCycle::GetInstance().SetIsActive(active_); }
    }
};

#endif // _MSG_CMD_COLOR_CYCLE_PARSER_H_
