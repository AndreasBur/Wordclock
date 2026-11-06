// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the clock mode command.
 */

#ifndef _MSG_CMD_CLOCK_MODE_PARSER_H_
#define _MSG_CMD_CLOCK_MODE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Clock.h"

/*! \brief The number of options in the parameter table of the clock mode command. */
#define MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE           2u

/*!
 * \brief The parser of command 8, the clock mode command: sets the regional way of saying the time,
 *        and whether ES IST stands at every time.
 *
 * Its options:
 * - `-M<mode>`, uint8: sets the regional mode, Clock::ModeType. One from Clock::kNumberOfModes on
 *   is answered with `Error=4:M`, and the mode is left as it was.
 * - `-I<0|1>`, uint8: with any value but 0, ES IST stands at every time; with 0, only at the full
 *   and the half hour.
 *
 * An option the command does not carry is left alone, so `8` alone is a query. Neither option
 * draws anything: DisplayManager draws the new words, as it draws every change of the words.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies it at once. MsgCmdParser makes the parser and calls Parse(),
 * Process() and SendAnswer() in this order, so an error answered for an option stands in the line
 * before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdClockModeParser : public MsgParameterParser<MsgCmdClockModeParser, MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    /*! \brief A regional mode, as `-M` takes it: Clock::ModeType. */
    using ModeType = Clock::ModeType;

  private:
    friend class MsgParameterParser;

    static constexpr char kModeOptionShortName{'M'};
    static constexpr char kShowItIsOptionShortName{'I'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kModeOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kShowItIsOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kModeOptionShortName) {
            SetMode(static_cast<ModeType>(argument));
        }
        if(parameter_short_name == kShowItIsOptionShortName) {
            Clock::GetInstance().SetShowItIsPermanently(argument != 0u);
        }
    }

    void SetMode(ModeType mode) const
    {
        StdReturnType return_value = Clock::GetInstance().SetMode(mode);
        error_.CheckReturnValueAndSend(kModeOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void Show() const
    {
        StdReturnType return_value = Clock::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::ApiType::kClockShow, return_value, ErrorMessage::ErrorType::kDisplayPending);
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdClockModeParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdClockModeParser() = default;

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `M=<mode> I=<0|1>`: the regional mode, and whether ES IST stands at every time. A
     * space follows each field but the last; MsgCmdParser ends the line.
     */
    static void SendAnswer() {
        SendAnswerParameter(kModeOptionShortName, ToUnderlying(Clock::GetInstance().GetMode()));
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        SendAnswerParameter(kShowItIsOptionShortName, static_cast<byte>(Clock::GetInstance().GetShowItIsPermanently() ? 1u : 0u), false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It marks the display's buffer to be sent, through Clock::Show(). A refusal would be answered
     * as an error of ErrorMessage::ApiType::kClockShow, but no backend refuses.
     */
    void Process() const { Show(); }
};

#endif
