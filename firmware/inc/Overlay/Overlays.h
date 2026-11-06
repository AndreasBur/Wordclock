// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The date, temperature and text overlays together, and which of them holds the display.
 */

#ifndef _OVERLAYS_H_
#define _OVERLAYS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"

#define OVERLAYS_SUPPORT_DATE               STD_ON /*!< Whether the date overlay is built in. */
/*! \brief Whether the temperature overlay is built in. */
#define OVERLAYS_SUPPORT_TEMPERATURE        STD_ON
#define OVERLAYS_SUPPORT_TEXT               STD_ON /*!< Whether the text overlay is built in. */

/*!
 * \brief The scheduler ticks between two calls of Overlays::Task().
 *
 * Task() acts only when the second of the real-time clock has changed, so the cycle decides how
 * soon after that change it acts, not how often.
 */
#define OVERLAYS_TASK_CYCLE                 1u

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
# include "OverlayDate.h"
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
# include "OverlayTemperature.h"
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
# include "OverlayText.h"
#endif

/*!
 * \brief The overlays facade: holds the date, temperature and text overlays, starts them in their
 *        periods and decides which of them holds the display.
 *
 * An overlay that is switched on starts in its period: in every minute of the hour that is a
 * multiple of its period in minutes, minute 0 included, at second 30. Its month and day limit the
 * days it starts on. With neither set, it starts on every day; with a month alone, on every day of
 * that month; with a day alone, on that day of every month; and with both, on that day. With both
 * set, a valid-in-days adds the days up to that many after that day, across the turn of the year as
 * well. The temperature overlay starts only while Temperature has a reading.
 *
 * Once started, an overlay shifts its text across the display through Text, at its speed and in
 * its font, and starts the text again whenever it has gone through. It holds the display for its
 * endurance in seconds and goes back to idle on the last second of it: an endurance of 3 started
 * at second 30 ends at second 33. ShowDateNow(), ShowTemperatureNow() and ShowTextNow() start an
 * overlay at once instead of at its period, and Abort() ends the one that shows.
 *
 * Only one overlay holds the display at a time, as one show timer serves all three. When several
 * are due in the same second, the first of date, temperature and text, in this order, starts, and
 * the others wait for their next period.
 *
 * Overlays draws nothing itself: Text draws the letters and shows the display in its own task.
 * While an overlay shows, DisplayManager leaves the display to it and starts no animation; once
 * the overlay has ended, DisplayManager draws the current time at once, without an animation.
 *
 * An overlay whose `OVERLAYS_SUPPORT_*` switch is STD_OFF is not built in, and neither are its
 * accessors and its "show now" procedure.
 *
 * An overlay that starts, by its period or now, ends a running animation through
 * Animations::Abort(): both would draw on the same display.
 */
class Overlays
{
  public:
    /*!
     * \brief The base of the three overlays, Overlay, named for its types: StateType and the date
     *        and time types below come from it.
     *
     * No overlay is of this type; each is an Overlay of its own class.
     */
    using OverlayType = Overlay<Overlays>;
    using MonthType = OverlayType::MonthType;   /*!< A month, as ClockDate counts it. */
    using DayType = OverlayType::DayType;       /*!< A day of the month, or a number of days. */
    using HourType = OverlayType::HourType;     /*!< An hour of the day, as ClockTime counts it. */
    using MinuteType = OverlayType::MinuteType; /*!< A minute, or a number of minutes. */
    using SecondType = OverlayType::SecondType; /*!< A second, or a number of seconds. */
    /*! \brief The state of an overlay, and of the overlays as a whole as GetState() returns it. */
    using StateType = OverlayType::StateType;
    using FontType = OverlayType::FontType;     /*!< A font of Text. */

#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    /*! \brief The size of the buffer SetTextText() copies into, its terminator included. */
    using LengthType = OverlayText::LengthType;
#endif

    /*!
     * \brief The overlays, one value for each overlay that is built in.
     *
     * An overlay whose `OVERLAYS_SUPPORT_*` switch is STD_OFF has no value here, and the values
     * behind it move down by one.
     */
    enum class OverlayIdType {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        kDate, /*!< The date overlay. */
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        kTemperature, /*!< The temperature overlay. */
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        kText /*!< The text overlay. */
#endif
    };

  private:
    static constexpr byte kTaskCycle{OVERLAYS_TASK_CYCLE};
    // The seconds the showing overlay has left. One timer for all three overlays, which is
    // why only one of them may show at a time.
    SecondType show_timer_in_seconds_{0u};
    // The second Task() last acted on, so that it acts once per second of the clock.
    SecondType last_second_{0u};
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    OverlayDate date_;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    OverlayTemperature temperature_;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    OverlayText text_;
#endif

    constexpr Overlays() { }
    ~Overlays() { }

    bool IsShow() const;
    bool IsIdle() const;

    void TaskIdle(ClockDate date, ClockTime time);
    void TaskShow(ClockDate date, ClockTime time);
    static void AbortAnimation();

    template <typename OverlayClassType>
    StdReturnType ShowNow(OverlayClassType& overlay);

  public:
    /*! \brief Returns the one overlays facade. */
    static Overlays& GetInstance() {
        static Overlays singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the scheduler ticks between two calls of Task(), OVERLAYS_TASK_CYCLE. */
    static byte GetTaskCycle() { return kTaskCycle; }

    /*!
     * \brief Returns the state of the overlays as a whole.
     *
     * Showing beats idle beats disabled, so that the one overlay holding the display decides, and
     * an armed one is not hidden behind the others that are switched off.
     *
     * \return StateType::kShow while an overlay shows, StateType::kIdle while none shows and at
     *         least one is switched on, and StateType::kDisabled while all are switched off.
     */
    StateType GetState() const;

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    /*! \brief Returns the period of the date overlay, in minutes. */
    MinuteType GetDatePeriodInMinutes() const { return date_.GetPeriodInMinutes(); }

    /*! \brief Returns the endurance of the date overlay, in seconds. */
    SecondType GetDateEnduranceInSeconds() const { return date_.GetEnduranceInSeconds(); }

    /*! \brief Returns the month the date overlay starts in, 0 for every month. */
    MonthType GetDateMonth() const { return date_.GetMonth(); }

    /*! \brief Returns the day of the month the date overlay starts on, 0 for every day. */
    DayType GetDateDay() const { return date_.GetDay(); }

    /*! \brief Returns the valid-in-days of the date overlay, 0 for none. */
    DayType GetDateValidInDays() const { return date_.GetValidInDays(); }

    /*! \brief Returns the speed the date overlay shifts its text at, 0 for none. */
    byte GetDateSpeed() const { return date_.GetSpeed(); }

    /*! \brief Returns the font the date overlay draws in. */
    Text::FontType GetDateFont() const { return date_.GetFont(); }

    /*! \brief Returns whether the date overlay is switched on. */
    bool GetDateIsActive() const { return date_.GetIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    /*! \brief Returns the period of the temperature overlay, in minutes. */
    MinuteType GetTemperaturePeriodInMinutes() const { return temperature_.GetPeriodInMinutes(); }

    /*! \brief Returns the endurance of the temperature overlay, in seconds. */
    SecondType GetTemperatureEnduranceInSeconds() const { return temperature_.GetEnduranceInSeconds(); }

    /*! \brief Returns the month the temperature overlay starts in, 0 for every month. */
    MonthType GetTemperatureMonth() const { return temperature_.GetMonth(); }

    /*! \brief Returns the day of the month the temperature overlay starts on, 0 for every day. */
    DayType GetTemperatureDay() const { return temperature_.GetDay(); }

    /*! \brief Returns the valid-in-days of the temperature overlay, 0 for none. */
    DayType GetTemperatureValidInDays() const { return temperature_.GetValidInDays(); }

    /*! \brief Returns the speed the temperature overlay shifts its text at, 0 for none. */
    byte GetTemperatureSpeed() const { return temperature_.GetSpeed(); }

    /*! \brief Returns the font the temperature overlay draws in. */
    Text::FontType GetTemperatureFont() const { return temperature_.GetFont(); }

    /*!
     * \brief Returns the text of the temperature overlay: the reading as it took it at its last
     *        start, with the degree sign.
     *
     * It is empty until the overlay has started once. The degree sign is the Latin-1 byte 0xB0
     * the font tables use, which Temperature::GetTemperatureString() leaves out.
     */
    const char* GetTemperatureString() const { return temperature_.GetTemperatureString(); }

    /*! \brief Returns whether the temperature overlay is switched on. */
    bool GetTemperatureIsActive() const { return temperature_.GetIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    /*! \brief Returns the period of the text overlay, in minutes. */
    MinuteType GetTextPeriodInMinutes() const { return text_.GetPeriodInMinutes(); }

    /*! \brief Returns the endurance of the text overlay, in seconds. */
    SecondType GetTextEnduranceInSeconds() const { return text_.GetEnduranceInSeconds(); }

    /*! \brief Returns the month the text overlay starts in, 0 for every month. */
    MonthType GetTextMonth() const { return text_.GetMonth(); }

    /*! \brief Returns the day of the month the text overlay starts on, 0 for every day. */
    DayType GetTextDay() const { return text_.GetDay(); }

    /*! \brief Returns the valid-in-days of the text overlay, 0 for none. */
    DayType GetTextValidInDays() const { return text_.GetValidInDays(); }

    /*! \brief Returns the speed the text overlay shifts its text at, 0 for none. */
    byte GetTextSpeed() const { return text_.GetSpeed(); }

    /*! \brief Returns the font the text overlay draws in. */
    Text::FontType GetTextFont() const { return text_.GetFont(); }

    /*! \brief Returns whether the text overlay is switched on. */
    bool GetTextIsActive() const { return text_.GetIsActive(); }

    /*! \brief Returns the text the text overlay shows, empty until one is set. */
    const char* GetTextText() const { return text_.GetText(); }
#endif

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    /*!
     * \brief Sets the period of the date overlay: it starts in every minute of the hour that is a
     *        multiple of `period_in_minutes`.
     *
     * \param[in] period_in_minutes The period in minutes; 60 or more starts it at minute 0 only.
     * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
     */
    StdReturnType SetDatePeriodInMinutes(MinuteType period_in_minutes) { return date_.SetPeriodInMinutes(period_in_minutes); }

    /*!
     * \brief Sets the endurance of the date overlay: the seconds it holds the display for.
     *
     * A date overlay that shows keeps the endurance it started with.
     *
     * \param[in] endurance_in_seconds The endurance in seconds.
     * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it
     *         was.
     */
    StdReturnType SetDateEnduranceInSeconds(SecondType endurance_in_seconds) { return date_.SetEnduranceInSeconds(endurance_in_seconds); }

    /*!
     * \brief Sets the month the date overlay starts in.
     *
     * \param[in] month The month, 1 to 12, or 0 for every month; it is not checked.
     */
    void SetDateMonth(MonthType month) { date_.SetMonth(month); }

    /*!
     * \brief Sets the day of the month the date overlay starts on.
     *
     * \param[in] day The day, 1 to 31, or 0 for every day; it is not checked.
     */
    void SetDateDay(DayType day) { date_.SetDay(day); }

    /*!
     * \brief Sets the valid-in-days of the date overlay: the days after its day and month it starts
     *        on as well, when both are set.
     *
     * \param[in] valid_in_days The number of days, 0 for the day alone.
     */
    void SetDateValidInDays(DayType valid_in_days) { date_.SetValidInDays(valid_in_days); }

    /*!
     * \brief Sets the speed the date overlay shifts its text at.
     *
     * It takes effect at the next start, which sets the task cycle of Text through
     * Scheduler::ConvertSpeedToTaskCycle().
     *
     * \param[in] speed The speed, 255 for the fastest and 1 for the slowest; 0 stops the text,
     *                  while the overlay still holds the display for its endurance.
     */
    void SetDateSpeed(byte speed) { date_.SetSpeed(speed); }

    /*!
     * \brief Sets the font the date overlay draws in.
     *
     * It takes effect the next time the overlay starts its text. A font that is not built in is
     * taken, and draws nothing.
     *
     * \param[in] font The font.
     * \return kEOk, or kENotOk if `font` is not a font of Text::FontType, Text::IsFontValid(); the
     *         font is then left as it was.
     */
    StdReturnType SetDateFont(FontType font) { return date_.SetFont(font); }

    /*!
     * \brief Switches the date overlay on or off.
     *
     * Switching on an overlay that is on leaves it as it is. Switching off ends a show at once.
     *
     * \param[in] is_active True to switch it on, false to switch it off.
     */
    void SetDateIsActive(bool is_active) { date_.SetIsActive(is_active); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    /*!
     * \brief Sets the period of the temperature overlay: it starts in every minute of the hour that
     *        is a multiple of `period_in_minutes`.
     *
     * \param[in] period_in_minutes The period in minutes; 60 or more starts it at minute 0 only.
     * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
     */
    StdReturnType SetTemperaturePeriodInMinutes(MinuteType period_in_minutes) { return temperature_.SetPeriodInMinutes(period_in_minutes); }

    /*!
     * \brief Sets the endurance of the temperature overlay: the seconds it holds the display for.
     *
     * A temperature overlay that shows keeps the endurance it started with.
     *
     * \param[in] endurance_in_seconds The endurance in seconds.
     * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it
     *         was.
     */
    StdReturnType SetTemperatureEnduranceInSeconds(SecondType endurance_in_seconds) { return temperature_.SetEnduranceInSeconds(endurance_in_seconds); }

    /*!
     * \brief Sets the month the temperature overlay starts in.
     *
     * \param[in] month The month, 1 to 12, or 0 for every month; it is not checked.
     */
    void SetTemperatureMonth(MonthType month) { temperature_.SetMonth(month); }

    /*!
     * \brief Sets the day of the month the temperature overlay starts on.
     *
     * \param[in] day The day, 1 to 31, or 0 for every day; it is not checked.
     */
    void SetTemperatureDay(DayType day) { temperature_.SetDay(day); }

    /*!
     * \brief Sets the valid-in-days of the temperature overlay: the days after its day and month it
     *        starts on as well, when both are set.
     *
     * \param[in] valid_in_days The number of days, 0 for the day alone.
     */
    void SetTemperatureValidInDays(DayType valid_in_days) { temperature_.SetValidInDays(valid_in_days); }

    /*!
     * \brief Sets the speed the temperature overlay shifts its text at.
     *
     * It takes effect at the next start, which sets the task cycle of Text through
     * Scheduler::ConvertSpeedToTaskCycle().
     *
     * \param[in] speed The speed, 255 for the fastest and 1 for the slowest; 0 stops the text,
     *                  while the overlay still holds the display for its endurance.
     */
    void SetTemperatureSpeed(byte speed) { temperature_.SetSpeed(speed); }

    /*!
     * \brief Sets the font the temperature overlay draws in.
     *
     * It takes effect the next time the overlay starts its text. A font that is not built in is
     * taken, and draws nothing.
     *
     * \param[in] font The font.
     * \return kEOk, or kENotOk if `font` is not a font of Text::FontType, Text::IsFontValid(); the
     *         font is then left as it was.
     */
    StdReturnType SetTemperatureFont(FontType font) { return temperature_.SetFont(font); }

    /*!
     * \brief Switches the temperature overlay on or off.
     *
     * Switching on an overlay that is on leaves it as it is. Switching off ends a show at once.
     *
     * \param[in] is_active True to switch it on, false to switch it off.
     */
    void SetTemperatureIsActive(bool is_active) { temperature_.SetIsActive(is_active); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    /*!
     * \brief Sets the period of the text overlay: it starts in every minute of the hour that is a
     *        multiple of `period_in_minutes`.
     *
     * \param[in] period_in_minutes The period in minutes; 60 or more starts it at minute 0 only.
     * \return kEOk, or kENotOk if `period_in_minutes` is 0; the period is then left as it was.
     */
    StdReturnType SetTextPeriodInMinutes(MinuteType period_in_minutes) { return text_.SetPeriodInMinutes(period_in_minutes); }

    /*!
     * \brief Sets the endurance of the text overlay: the seconds it holds the display for.
     *
     * A text overlay that shows keeps the endurance it started with.
     *
     * \param[in] endurance_in_seconds The endurance in seconds.
     * \return kEOk, or kENotOk if `endurance_in_seconds` is 0; the endurance is then left as it
     *         was.
     */
    StdReturnType SetTextEnduranceInSeconds(SecondType endurance_in_seconds) { return text_.SetEnduranceInSeconds(endurance_in_seconds); }

    /*!
     * \brief Sets the month the text overlay starts in.
     *
     * \param[in] month The month, 1 to 12, or 0 for every month; it is not checked.
     */
    void SetTextMonth(MonthType month) { text_.SetMonth(month); }

    /*!
     * \brief Sets the day of the month the text overlay starts on.
     *
     * \param[in] day The day, 1 to 31, or 0 for every day; it is not checked.
     */
    void SetTextDay(DayType day) { text_.SetDay(day); }

    /*!
     * \brief Sets the valid-in-days of the text overlay: the days after its day and month it starts
     *        on as well, when both are set.
     *
     * \param[in] valid_in_days The number of days, 0 for the day alone.
     */
    void SetTextValidInDays(DayType valid_in_days) { text_.SetValidInDays(valid_in_days); }

    /*!
     * \brief Switches the text overlay on or off.
     *
     * Switching on an overlay that is on leaves it as it is. Switching off ends a show at once.
     *
     * \param[in] is_active True to switch it on, false to switch it off.
     */
    void SetTextIsActive(bool is_active) { text_.SetIsActive(is_active); }

    /*!
     * \brief Sets the speed the text overlay shifts its text at.
     *
     * It takes effect at the next start, which sets the task cycle of Text through
     * Scheduler::ConvertSpeedToTaskCycle().
     *
     * \param[in] speed The speed, 255 for the fastest and 1 for the slowest; 0 stops the text,
     *                  while the overlay still holds the display for its endurance.
     */
    void SetTextSpeed(byte speed) { text_.SetSpeed(speed); }

    /*!
     * \brief Sets the font the text overlay draws in.
     *
     * It takes effect the next time the overlay starts its text. A font that is not built in is
     * taken, and draws nothing.
     *
     * \param[in] font The font.
     * \return kEOk, or kENotOk if `font` is not a font of Text::FontType, Text::IsFontValid(); the
     *         font is then left as it was.
     */
    StdReturnType SetTextFont(FontType font) { return text_.SetFont(font); }

    /*!
     * \brief Sets the text the text overlay shows.
     *
     * It copies at most `length` - 1 characters of `text`, up to its terminator, and terminates
     * the copy.
     *
     * \param[in] text The text, terminated.
     * \param[in] length The size of the buffer to fill, its terminator included.
     * \pre `length` is at most OVERLAY_TEXT_TEXT_SIZE; it is not checked, and a larger one writes
     *      past the text of the overlay. A `length` of 0 leaves it as it was.
     */
    void SetTextText(const char* text, LengthType length) { text_.SetText(text, length); }
#endif

    /*!
     * \brief Puts every overlay back to how a new clock has it: switched off, a period of 1 minute,
     *        an endurance of 1 second, no month, day or valid-in-days, speed 1, the font
     *        Text::FontType::kFont10x10, and an empty text.
     *
     * Every overlay back to what it starts with, the same way Display, Clock, Animations and
     * Illuminance answer Persistence::Reset(). Here since the overlays are stored - before that a
     * reset had nothing of theirs to undo.
     *
     * An overlay that shows ends its show first, as Abort() would end it.
     */
    void ResetToDefaults();

    /*!
     * \brief Starts the overlays in their periods, keeps the one that shows going and ends it when
     *        its endurance has run out.
     *
     * It acts only when the second of RealTimeClock differs from the one it last acted on, so once
     * per second, however often the scheduler calls it. While an overlay shows, it counts the show
     * timer down and starts the overlay's text again whenever Text has gone through it; on the
     * second that runs the timer out, it ends the overlay. While none shows, it starts the first
     * one that is due, in the order date, temperature, text. It draws and shows nothing itself.
     */
	void Task();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    /*!
     * \brief Starts the date overlay now instead of at its next period.
     *
     * It starts whatever the minute, the second, its month and its day, and holds the display for
     * its endurance as if its period had started it. Without the "show now" procedures an overlay
     * can only be watched by waiting for its period, which is also what makes one hard to develop
     * against. RpcIdType::kOverlayDateShow calls this.
     *
     * \return kEOk, or kENotOk if the date overlay is switched off or an overlay shows already,
     *         this one included; nothing is started then.
     */
    StdReturnType ShowDateNow();
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    /*!
     * \brief Starts the temperature overlay now instead of at its next period.
     *
     * It starts as ShowDateNow() starts the date overlay. RpcIdType::kOverlayTemperatureShow calls
     * this.
     *
     * \return kEOk, or kENotOk if the temperature overlay is switched off, Temperature has no
     *         reading, or an overlay shows already, this one included; nothing is started then.
     */
    StdReturnType ShowTemperatureNow();
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    /*!
     * \brief Starts the text overlay now instead of at its next period.
     *
     * It starts as ShowDateNow() starts the date overlay, an empty text included, which then holds
     * the display for the endurance with nothing to shift. RpcIdType::kOverlayTextShow calls this.
     *
     * \return kEOk, or kENotOk if the text overlay is switched off or an overlay shows already,
     *         this one included; nothing is started then.
     */
    StdReturnType ShowTextNow();
#endif

    /*!
     * \brief Ends the overlay that shows.
     *
     * Through the same transition the expired show timer takes: the overlay stops its text where
     * the shift stands and goes back to idle, to start again at its next period. What brings the
     * clock back afterwards is DisplayManager, which redraws once it sees that no overlay is
     * showing any more. RpcIdType::kOverlayAbort calls this.
     *
     * \return kEOk, or kENotOk if no overlay shows; nothing is changed then.
     */
    StdReturnType Abort();
};

#endif
