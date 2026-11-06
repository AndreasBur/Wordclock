// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the time command.
 */

#ifndef _MSG_CMD_TIME_PARSER_H_
#define _MSG_CMD_TIME_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "RealTimeClock.h"

/*! \brief The number of options in the parameter table of the time command. */
#define MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE           3u

/*!
 * \brief The parser of command 10, the time command: sets the time of the real-time clock.
 *
 * Its options:
 * - `-H<hour>`, uint8: the hour, 0 to 23.
 * - `-M<minute>`, uint8: the minute, 0 to 59.
 * - `-S<second>`, uint8: the second, 0 to 59.
 *
 * A value out of its range is answered with `Error=4:<option>`, and that part of the time is left
 * as it was. An option the command does not carry is left as it was too.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdTimeParser : public MsgParameterParser<MsgCmdTimeParser, MSG_CMD_TIME_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    /*! \brief An hour, as `-H` takes it: RealTimeClock::HourType. */
    using HourType = RealTimeClock::HourType;
    /*! \brief A minute, as `-M` takes it: RealTimeClock::MinuteType. */
    using MinuteType = RealTimeClock::MinuteType;
    /*! \brief A second, as `-S` takes it: RealTimeClock::SecondType. */
    using SecondType = RealTimeClock::SecondType;

  private:
    friend class MsgParameterParser;
    ClockTime time_;

    static constexpr char kHourOptionShortName{'H'};
    static constexpr char kMinuteOptionShortName{'M'};
    static constexpr char kSecondOptionShortName{'S'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kHourOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kMinuteOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kSecondOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kHourOptionShortName) {
            SetHour(argument);
        }
        if(parameter_short_name == kMinuteOptionShortName) {
            SetMinute(argument);
        }
        if(parameter_short_name == kSecondOptionShortName) {
            SetSecond(argument);
        }
    }

    void SendAnswerHour(bool append_space) const {
        SendAnswerParameter(kHourOptionShortName, time_.GetHour(), append_space);
    }
    void SendAnswerMinute(bool append_space) const {
        SendAnswerParameter(kMinuteOptionShortName, time_.GetMinute(), append_space);
    }
    void SendAnswerSecond(bool append_space) const {
        SendAnswerParameter(kSecondOptionShortName, time_.GetSecond(), append_space);
    }

    void SetHour(HourType hour) {
        StdReturnType return_value = time_.SetHour(hour);
        error_.CheckReturnValueAndSend(kHourOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetMinute(MinuteType minute) {
        StdReturnType return_value = time_.SetMinute(minute);
        error_.CheckReturnValueAndSend(kMinuteOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetSecond(SecondType second) {
        StdReturnType return_value = time_.SetSecond(second);
        error_.CheckReturnValueAndSend(kSecondOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * It takes the time of the real-time clock as it is now, which the options change part by
     * part.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    MsgCmdTimeParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) {
        time_ = RealTimeClock::GetInstance().GetTime();
    }

    /*! \brief Destroys the parser. */
    ~MsgCmdTimeParser() = default;

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `H=<hour> M=<minute> S=<second>`: the time of the real-time clock, as Process() has
     * read it back. A space follows each field but the last; MsgCmdParser ends the line.
     */
    void SendAnswer() const
    {
        SendAnswerHour(true);
        SendAnswerMinute(true);
        SendAnswerSecond(false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It writes the time to the real-time clock, and reads it back for the answer. It writes even
     * when the command carried no option, the time the parser was made with.
     */
    void Process()
    {
        RealTimeClock::GetInstance().SetTime(time_);
        time_ = RealTimeClock::GetInstance().GetTime();
    }
};

#endif
