// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser the date, temperature and text overlay commands share.
 */

#ifndef _MSG_CMD_OVERLAY_PARSER_H_
#define _MSG_CMD_OVERLAY_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "MsgCmdBaseOverlayParser.h"
#include "ClockDateTime.h"
#include "Text.h"

/*!
 * \brief The parser the date, temperature and text overlay commands share: applies their options to
 *        the overlay of `Derived`, and answers its settings.
 *
 * Its options, those of MsgCmdBaseOverlayParser:
 * - `-P<period>`, uint8: sets the period; the overlay starts in each minute of the hour that the
 *   period divides. 0 is answered with `Error=4:P`.
 * - `-E<endurance>`, uint8: sets the endurance, the seconds the overlay shows for once it has
 *   started. 0 is answered with `Error=4:E`.
 * - `-M<month>` and `-D<day>`, uint8: limit the overlay to a month and to a day of the month, 0
 *   lifting the limit. Neither is checked.
 * - `-V<days>`, uint8: sets on how many days after the date of `-M` and `-D` the overlay goes on
 *   starting, 0 for the date only. It is not checked.
 * - `-A<0|1>`, uint8: switches the overlay on with any value but 0, and off with 0.
 * - `-T<text>`, string: hands the text to `Derived`, which only the text overlay takes.
 * - `-S<speed>`, uint8: sets the speed the text scrolls at, from 1 the slowest to 255 the fastest;
 *   at 0 the text does not move.
 * - `-F<id>`, uint8: sets the font of the text, Text::FontType.
 *
 * An option that is refused leaves its setting as it was. An option the command does not carry is
 * left alone, so a command without options is a query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which applies it at once. MsgCmdParser makes the parser and calls Parse(),
 * Process() and SendAnswer() in this order, so an error answered for an option stands in the line
 * before the fields of the answer.
 *
 * Each overlay parser derives from this class with itself as `Derived`, makes this class its
 * friend, and provides the public hooks this class calls to reach its overlay in Overlays: a getter
 * and a setter for each setting, `SetText()` for `-T` and `SendAnswerText()` for the `T` field.
 *
 * A font that is not one of Text::FontType, such as `5 -F9`, is answered with `Error=4:F`, and
 * the font is left as it was.
 *
 * \tparam Derived The overlay parser that derives from this class.
 */
template <typename Derived> class MsgCmdOverlayParser
    : public MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>, public MsgCmdBaseOverlayParser
{
  public:
    /*! \brief The parameter parser this class derives from. */
    using MsgParameterParserType = MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
    /*! \brief A position in the options, MsgParameterParser::PositionType. */
    using PositionType = typename MsgParameterParserType::PositionType;
    /*! \brief The length of the text of `-T`, its terminator included, StringTools::LengthType. */
    using LengthType = StringTools::LengthType;

    /*! \brief A year in full, ClockDate::YearType. */
    using YearType = ClockDate::YearType;
    /*! \brief A month, 1 for January to 12 for December, as `-M` takes it. */
    using MonthType = ClockDateTime::MonthType;
    /*! \brief A day of the month or a number of days, as `-D` and `-V` take it. */
    using DayType = ClockDateTime::DayType;
    /*! \brief An hour, ClockDateTime::HourType. */
    using HourType = ClockDateTime::HourType;
    /*! \brief A number of minutes, as `-P` takes it. */
    using MinuteType = ClockDateTime::MinuteType;
    /*! \brief A number of seconds, as `-E` takes it. */
    using SecondType = ClockDateTime::SecondType;
    /*! \brief A font of the text output, as `-F` takes it. */
    using FontType = Text::FontType;

  protected:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
    constexpr MsgCmdOverlayParser(const char* parameter) : MsgParameterParserType(kParameterTable, parameter) {
        /*
         * Protected rather than private with the derived class as a friend, which is what
         * clang-tidy asks for: a friend would open this base's whole private side to it, where
         * protected opens only what is meant for it. What the private constructor would
         * prevent - somebody inheriting this template with another class's Derived - the
         * assertion catches instead, at the first point where Derived is a complete type.
         */
        static_assert(std::is_base_of<MsgCmdOverlayParser, Derived>::value,
                      "MsgCmdOverlayParser is a CRTP base: Derived has to be the class that inherits it");
    }

    /*! \brief Destroys the parser. */
    ~MsgCmdOverlayParser() = default;

  private:
    friend class MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;

    Derived& Underlying() { return static_cast<Derived&>(*this); }
    Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

    void HandleParameter(char parameter_short_name, byte argument) {
        if(parameter_short_name == kPeriodOptionShortName) { SetPeriodInMinutes(argument); }
        if(parameter_short_name == kEnduranceOptionShortName) { SetEnduranceInSeconds(argument); }
        if(parameter_short_name == kMonthOptionShortName) { Underlying().SetMonth(argument); }
        if(parameter_short_name == kDayOptionShortName) { Underlying().SetDay(argument); }
        if(parameter_short_name == kValidOptionShortName) { Underlying().SetValidInDays(argument); }
        if(parameter_short_name == kActiveOptionShortName) { Underlying().SetIsActive(static_cast<bool>(argument)); }
        if(parameter_short_name == kSpeedOptionShortName) { Underlying().SetSpeed(argument); }
        // Through SetFont() below, which answers a refused font; called on the overlay directly,
        // as it was, 5 -F9 left the font as it was without a word.
        if(parameter_short_name == kFontOptionShortName) { SetFont(static_cast<FontType>(argument)); }
    }

    void HandleParameter(char parameter_short_name, const char* argument, PositionType length) {
        if(parameter_short_name == kTextOptionShortName) { Underlying().SetText(argument, length); }
    }

    void SetPeriodInMinutes(MinuteType period_in_minutes) {
        StdReturnType return_value = Underlying().SetPeriodInMinutes(period_in_minutes);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kPeriodOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetEnduranceInSeconds(SecondType endurance_in_seconds) {
        StdReturnType return_value = Underlying().SetEnduranceInSeconds(endurance_in_seconds);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kEnduranceOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }
    void SetFont(FontType font) {
        StdReturnType return_value = Underlying().SetFont(font);
        MsgParameterParserType::error_.CheckReturnValueAndSend(kFontOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);

    }

    void SendAnswerPeriod(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kPeriodOptionShortName,  Underlying().GetPeriodInMinutes(), append_space); }
    void SendAnswerEndurance(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kEnduranceOptionShortName, Underlying().GetEnduranceInSeconds(), append_space); }
    void SendAnswerMonth(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kMonthOptionShortName, Underlying().GetMonth(), append_space); }
    void SendAnswerDay(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kDayOptionShortName, Underlying().GetDay(), append_space); }
    void SendAnswerValid(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kValidOptionShortName, Underlying().GetValidInDays(), append_space); }
    void SendAnswerSpeed(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kSpeedOptionShortName, Underlying().GetSpeed(), append_space); }
    void SendAnswerFont(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kFontOptionShortName, ToUnderlying(Underlying().GetFont()), append_space); }
    void SendAnswerActive(bool append_space) const { MsgParameterParserType::SendAnswerParameter(kActiveOptionShortName, Underlying().GetIsActive(), append_space); }
    void SendAnswerText(bool append_space) const { Underlying().SendAnswerText(append_space); }

  public:

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `P=<period> E=<endurance> M=<month> D=<day> V=<days> T=<text> S=<speed> F=<id>
     * A=<0|1>`, the settings of the overlay once the options are applied. The `T` field is what
     * `Derived::SendAnswerText()` sends, and only the text overlay sends one. A space follows each
     * field but the last; MsgCmdParser ends the line.
     */
    void SendAnswer() const
    {
        SendAnswerPeriod(true);
        SendAnswerEndurance(true);
        SendAnswerMonth(true);
        SendAnswerDay(true);
        SendAnswerValid(true);
        SendAnswerText(true);
        SendAnswerSpeed(true);
        SendAnswerFont(true);
        SendAnswerActive(false);
    }

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * Nothing is left to do: every option is applied as Parse() reads it.
     */
    static void Process() {  }

};

#endif
