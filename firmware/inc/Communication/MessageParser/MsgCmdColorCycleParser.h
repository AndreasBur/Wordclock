// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Switches the colour cycle on and off, and says how fast it walks.
 *
 * Two settings and one reading. The speed is scheduler ticks per step - the kind of number every
 * animation keeps as its task cycle, but not the number the animation command takes: command 9
 * takes a speed that Scheduler::ConvertSpeedToTaskCycle() turns round, so there a higher number is
 * faster, and here it is slower.
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

/*! \brief The number of options in the parameter table of the colour cycle command. */
#define MSG_CMD_COLOR_CYCLE_PARSER_PARAMETER_TABLE_SIZE         2u

/*!
 * \brief The parser of command 15, the colour cycle command: starts and stops the walk of the
 *        display's colour round the wheel, and sets how fast it walks.
 *
 * Its options:
 * - `-A<0|1>`, uint8: with any value but 0, starts the cycle, which sets the display to the colour
 *   of the hue it has reached at once; with 0, stops it, which puts the colour of command 2 back
 *   at once.
 * - `-S<ticks>`, uint8: sets the scheduler ticks per step of hue; at 0 the cycle stands still on
 *   the hue it has reached. Unlike the speed of command 9, a higher number is slower.
 *
 * The hue is answered, `H`, but is not an option: `-H` is answered with `Error=3:H`. An option the
 * command does not carry is left alone, so `15` alone is a query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies `-S` at once and keeps `-A` for Process(). MsgCmdParser makes
 * the parser and calls Parse(), Process() and SendAnswer() in this order, so an error answered for
 * an option stands in the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
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

    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) {
        UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length);
    }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kActiveShortName) { active_ = (argument != 0u); active_given_ = true; }
        if(parameter_short_name == kSpeedShortName)  { ColorCycle::GetInstance().SetTaskCycle(argument); }
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdColorCycleParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdColorCycleParser() = default;

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `A=<0|1> S=<ticks> H=<hue>`: whether the cycle runs, its ticks per step, and the
     * hue it has reached, one of 256 steps round the wheel. A space follows each field but the
     * last; MsgCmdParser ends the line.
     */
    static void SendAnswer() {
        const ColorCycle& color_cycle = ColorCycle::GetInstance();

        SendAnswerParameter(kActiveShortName, static_cast<byte>(color_cycle.GetIsActive() ? 1u : 0u));
        SendAnswerParameter(kSpeedShortName, color_cycle.GetTaskCycle());
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        SendAnswerParameter(kHueShortName, color_cycle.GetHue(), false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It starts or stops the cycle as `-A` asks, after `-S` has been applied. Without `-A` it does
     * nothing.
     */
    void Process() const {
        if(active_given_) { ColorCycle::GetInstance().SetIsActive(active_); }
    }
};

#endif // _MSG_CMD_COLOR_CYCLE_PARSER_H_
