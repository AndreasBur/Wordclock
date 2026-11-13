// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the temperature overlay command.
 */

#ifndef _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_
#define _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"
#include "Overlays.h"

/*!
 * \brief The parser of command 6, the temperature overlay command: sets when the temperature
 *        scrolls over the clock, and how.
 *
 * It takes the options of MsgCmdOverlayParser and applies them to the temperature overlay of
 * Overlays. `-T` is in the parameter table, but the overlay shows the temperature and drops it,
 * and the answer has no `T` field.
 *
 * MsgCmdParser takes the command only while OVERLAYS_SUPPORT_TEMPERATURE is STD_ON. The number
 * holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is STD_OFF moves
 * the commands behind it down by one.
 */
class MsgCmdOverlayTemperatureParser : public MsgCmdOverlayParser<MsgCmdOverlayTemperatureParser>
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
    constexpr MsgCmdOverlayTemperatureParser(const char* parameter) : MsgCmdOverlayParser(parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdOverlayTemperatureParser() = default;

    /*! \brief Returns the period of the temperature overlay, in minutes, for the `P` field. */
    static MinuteType GetPeriodInMinutes() { return Overlays::GetInstance().GetTemperaturePeriodInMinutes(); }

    /*! \brief Returns the endurance of the temperature overlay, in seconds, for the `E` field. */
    static SecondType GetEnduranceInSeconds() { return Overlays::GetInstance().GetTemperatureEnduranceInSeconds(); }

    /*! \brief Returns the month of the temperature overlay, 0 for none, for the `M` field. */
    static MonthType GetMonth() { return Overlays::GetInstance().GetTemperatureMonth(); }

    /*! \brief Returns the day of the temperature overlay, 0 for none, for the `D` field. */
    static DayType GetDay() { return Overlays::GetInstance().GetTemperatureDay(); }

    /*! \brief Returns the valid-in-days of the temperature overlay, for the `V` field. */
    static DayType GetValidInDays() { return Overlays::GetInstance().GetTemperatureValidInDays(); }

    /*! \brief Returns whether the temperature overlay is switched on, for the `A` field. */
    static bool GetIsActive() { return Overlays::GetInstance().GetTemperatureIsActive(); }

    /*! \brief Returns the speed the temperature overlay's text scrolls at, for the `S` field. */
    static byte GetSpeed() { return Overlays::GetInstance().GetTemperatureSpeed(); }

    /*! \brief Returns the font of the temperature overlay's text, for the `F` field. */
    static FontType GetFont() {return Overlays::GetInstance().GetTemperatureFont(); }

    /*!
     * \brief Sets the period of the temperature overlay, for `-P`.
     *
     * \param[in] period_in_minutes The period, in minutes.
     * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
     */
    static StdReturnType SetPeriodInMinutes(MinuteType period_in_minutes) { return Overlays::GetInstance().SetTemperaturePeriodInMinutes(period_in_minutes); }

    /*!
     * \brief Sets the endurance of the temperature overlay, for `-E`.
     *
     * \param[in] endurance_in_seconds The endurance, in seconds.
     * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it
     *         was.
     */
    static StdReturnType SetEnduranceInSeconds(SecondType endurance_in_seconds) {return Overlays::GetInstance().SetTemperatureEnduranceInSeconds(endurance_in_seconds); }

    /*!
     * \brief Limits the temperature overlay to `month`, or lifts the limit with 0, for `-M`.
     *
     * \param[in] month The month, 1 to 12, or 0 for none; it is not checked.
     */
    static void SetMonth(MonthType month) { Overlays::GetInstance().SetTemperatureMonth(month); }

    /*!
     * \brief Limits the temperature overlay to `day` of the month, or lifts the limit with 0, for
     *        `-D`.
     *
     * \param[in] day The day of the month, 1 to 31, or 0 for none; it is not checked.
     */
    static void SetDay(DayType day) { Overlays::GetInstance().SetTemperatureDay(day); }

    /*!
     * \brief Sets on how many days after its date the temperature overlay goes on starting, for
     *        `-V`.
     *
     * \param[in] valid_in_days The days after the date, 0 for the date only.
     */
    static void SetValidInDays(DayType valid_in_days) { Overlays::GetInstance().SetTemperatureValidInDays(valid_in_days); }

    /*!
     * \brief Sets the speed the temperature overlay's text scrolls at, for `-S`.
     *
     * \param[in] speed The speed, 1 the slowest to 255 the fastest; at 0 the text does not move.
     */
    static void SetSpeed(byte speed) { Overlays::GetInstance().SetTemperatureSpeed(speed); }

    /*!
     * \brief Sets the font of the temperature overlay's text, for `-F`.
     *
     * \param[in] font The font.
     * \return kEOk, or kENotOk if `font` is not one of Text::FontType; the font is then left as it
     *         was.
     */
    static StdReturnType SetFont(FontType font) { return Overlays::GetInstance().SetTemperatureFont(font); }

    /*!
     * \brief Switches the temperature overlay on or off, for `-A`.
     *
     * \param[in] is_active True to switch it on, false to switch it off.
     */
    static void SetIsActive(bool is_active) { Overlays::GetInstance().SetTemperatureIsActive(is_active); }

    /*!
     * \brief Drops the text of `-T`: the temperature overlay shows the temperature.
     *
     * \param[in] text The text; it is not read.
     * \param[in] length The number of its characters, plus one; it is not read.
     */
    static void SetText(const char* text, LengthType length) { UNUSED(text); UNUSED(length); }

    /*!
     * \brief Sends nothing: the answer of the temperature overlay command has no `T` field.
     *
     * \param[in] append_space Not used.
     */
    static void SendAnswerText(bool append_space) { UNUSED(append_space); }

    /*!
     * \brief Sends nothing, and is not called: MsgCmdOverlayParser sends the `S` field itself.
     *
     * \param[in] append_space Not used.
     */
    static void SendAnswerSpeedText(bool append_space) { UNUSED(append_space); }

};

#endif
