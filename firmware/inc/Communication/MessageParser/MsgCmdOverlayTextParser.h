// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the text overlay command.
 */

#ifndef _MSG_CMD_OVERLAY_TEXT_PARSER_H_
#define _MSG_CMD_OVERLAY_TEXT_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"
#include "Overlays.h"

/*!
 * \brief The parser of command 7, the text overlay command: sets the text that scrolls over the
 *        clock, and when and how it does.
 *
 * It takes the options of MsgCmdOverlayParser and applies them to the text overlay of Overlays.
 * `-T` sets the text, and the answer has a `T` field with it.
 *
 * MsgCmdParser takes the command only while OVERLAYS_SUPPORT_TEXT is STD_ON. The number holds
 * while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is STD_OFF moves the
 * commands behind it down by one.
 */
class MsgCmdOverlayTextParser : public MsgCmdOverlayParser<MsgCmdOverlayTextParser>
{
  public:

  private:
    friend class MsgCmdOverlayParser;

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdOverlayTextParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdOverlayTextParser() = default;

    /*! \brief Returns the period of the text overlay, in minutes, for the `P` field. */
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetTextPeriodInMinutes(); }

    /*! \brief Returns the endurance of the text overlay, in seconds, for the `E` field. */
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetTextEnduranceInSeconds(); }

    /*! \brief Returns the month of the text overlay, 0 for none, for the `M` field. */
    static MonthType GetMonth() { return Overlays::GetInstance().GetTextMonth(); }

    /*! \brief Returns the day of the text overlay, 0 for none, for the `D` field. */
    static DayType GetDay() { return Overlays::GetInstance().GetTextDay(); }

    /*! \brief Returns the valid-in-days of the text overlay, for the `V` field. */
    static DayType GetValidInDays() { return Overlays::GetInstance().GetTextValidInDays(); }

    /*! \brief Returns the speed the text overlay's text scrolls at, for the `S` field. */
    static byte GetSpeed() { return Overlays::GetInstance().GetTextSpeed(); }

    /*! \brief Returns the font of the text overlay's text, for the `F` field. */
    static FontType GetFont() {return Overlays::GetInstance().GetTextFont(); }

    /*! \brief Returns whether the text overlay is switched on, for the `A` field. */
    static bool GetIsActive() { return Overlays::GetInstance().GetTextIsActive(); }

    /*! \brief Returns the text of the text overlay, for the `T` field. */
    static const char* GetText() { return Overlays::GetInstance().GetTextText(); }

    /*!
     * \brief Sets the period of the text overlay, for `-P`.
     *
     * \param[in] period_in_minutes The period, in minutes.
     * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
     */
    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetTextPeriodInMinutes(period_in_minutes); }

    /*!
     * \brief Sets the endurance of the text overlay, for `-E`.
     *
     * \param[in] endurance_in_seconds The endurance, in seconds.
     * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it
     *         was.
     */
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) { return Overlays::GetInstance().SetTextEnduranceInSeconds(endurance_in_seconds); }

    /*!
     * \brief Limits the text overlay to `month`, or lifts the limit with 0, for `-M`.
     *
     * \param[in] month The month, 1 to 12, or 0 for none; it is not checked.
     */
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetTextMonth(month); }

    /*!
     * \brief Limits the text overlay to `day` of the month, or lifts the limit with 0, for `-D`.
     *
     * \param[in] day The day of the month, 1 to 31, or 0 for none; it is not checked.
     */
    static void SetDay(DayType day) { Overlays::GetInstance().SetTextDay(day); }

    /*!
     * \brief Sets on how many days after its date the text overlay goes on starting, for `-V`.
     *
     * \param[in] valid_in_days The days after the date, 0 for the date only.
     */
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetTextValidInDays(valid_in_days); }

    /*!
     * \brief Sets the speed the text overlay's text scrolls at, for `-S`.
     *
     * \param[in] speed The speed, 1 the slowest to 255 the fastest; at 0 the text does not move.
     */
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetTextSpeed(speed); }

    /*!
     * \brief Sets the font of the text overlay's text, for `-F`.
     *
     * \param[in] font The font.
     * \return kEOk, or kENotOk if `font` is not one of Text::FontType; the font is then left as it
     *         was.
     */
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetTextFont(font); }

    /*!
     * \brief Switches the text overlay on or off, for `-A`.
     *
     * \param[in] is_active True to switch it on, false to switch it off.
     */
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetTextIsActive(is_active); }

    /*!
     * \brief Sets the text of the text overlay to the text of `-T`: the first `length` - 1
     *        characters of `text`.
     *
     * The text runs up to the next `-` or the end of the line, so it cannot hold a `-`, and it
     * keeps the space before a following option: `7 -THallo -A1` sets `Hallo `.
     *
     * \param[in] text The text, in the parameter string, where it is not terminated.
     * \param[in] length The number of its characters, plus one.
     * \note `length` is not checked against OVERLAY_TEXT_TEXT_SIZE, which Overlays::SetTextText()
     *       requires. What keeps it within is the message: Message holds fewer characters than
     *       OVERLAY_TEXT_TEXT_SIZE, the command number and `-T` included.
     */
    static void SetText(const char* text, LengthType length) { Overlays::GetInstance().SetTextText(text, length); }

    /*!
     * \brief Sends the `T` field of the answer: the text of the text overlay.
     *
     * \param[in] append_space True to send a space after the field.
     */
    static void SendAnswerText(bool append_space) { SendAnswerParameter(kTextOptionShortName, GetText(), append_space); }

    /*!
     * \brief Sends the `S` field of the answer, and is not called: MsgCmdOverlayParser sends the
     *        `S` field itself.
     *
     * \param[in] append_space True to send a space after the field.
     */
    static void SendAnswerSpeed(bool append_space) { SendAnswerParameter(kSpeedOptionShortName, GetSpeed(), append_space); }

};

#endif
