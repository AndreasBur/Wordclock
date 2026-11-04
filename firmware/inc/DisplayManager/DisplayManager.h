// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Decides what the display shows.
 *
 * The Scheduler answers when something runs and the Display how it reaches the LEDs; this decides
 * what is on it. Its task arbitrates between the clock, the animation that carries a word change
 * and an overlay that takes the display over for a while, and it keeps the brightness the LEDs are
 * driven with up to date.
 */

#ifndef _DISPLAY_MANAGER_H_
#define _DISPLAY_MANAGER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Clock.h"
#include "Animations.h"
#include "Overlays.h"
#include "RealTimeClock.h"

/*! \brief The cycle of the display manager's task, in scheduler ticks. */
#define DISPLAY_MANAGER_TASK_CYCLE                  1u

/*!
 * \brief Decides what the display shows: the clock, the animation that carries a word change, or an
 *        overlay that takes the display over for a while.
 *
 * It also keeps the brightness the LEDs are driven with up to date.
 */
class DisplayManager
{
  public:
    using HourType = ClockTime::HourType;     /*!< The hour of a time, as ClockTime holds it. */
    using MinuteType = ClockTime::MinuteType; /*!< The minute of a time, as ClockTime holds it. */

  private:
    static constexpr byte kTaskCycle{DISPLAY_MANAGER_TASK_CYCLE};
    ClockWords last_clock_words_{};
    /*
     * Rather than letting a default-constructed ClockWords stand for "nothing drawn
     * yet": that only works as long as no real word set can equal it, which is a
     * property of ClockWords the compiler would not defend. An own flag says so
     * outright, and the first task then always draws the current time.
     */
    bool clock_words_initialized_{false};
    // Whether an overlay held the display on the previous task, which is what makes the
    // end of one detectable.
    bool overlay_was_showing_{false};

    constexpr DisplayManager() { }
    ~DisplayManager() { }

    static bool IsOverlayShowing() {
        return Overlays::GetInstance().GetState() == Overlays::OverlayType::StateType::kShow;
    }

    static void ShowClock(HourType hour, MinuteType minute) {
        Display::GetInstance().Clear();
        Clock::GetInstance().SetTime(hour, minute);
        Display::GetInstance().Show();
    }

    static void TaskClockWordsChange(HourType hour, MinuteType minute) {
        Animations& animations = Animations::GetInstance();
        if(animations.GetAnimation() == Animations::AnimationIdType::kNone) {
            ShowClock(hour, minute);
        } else {
            animations.SetTime(hour, minute);
        }
    }

    bool UpdateClockWords(HourType hour, MinuteType minute) {
        ClockWords clock_words;
        if(Clock::GetInstance().GetClockWords(hour, minute, clock_words) == kENotOk) {
            return false;
        }

        if(clock_words_initialized_ && clock_words == last_clock_words_) {
            return false;
        }

        last_clock_words_ = clock_words;
        clock_words_initialized_ = true;
        return true;
    }

  public:
    /*! \brief Returns the one display manager of the firmware. */
    static DisplayManager& GetInstance() {
        static DisplayManager singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the ticks between two calls of Task(), DISPLAY_MANAGER_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*!
     * \brief Decides what the display shows, and brings the brightness the LEDs are driven with up
     *        to date.
     *
     * While an overlay shows, the display is left to it. When the overlay has ended, the current
     * time is drawn at once, without an animation. Otherwise the time is drawn when its words
     * change, and on the first call: through the selected animation, or directly if none is
     * selected.
     */
    void Task();

    /*!
     * \brief Draws the current time again, right now and without an animation.
     *
     * What asks for this is a display that no longer shows the clock at all - a test pattern, a
     * pixel set by hand - and an animation would transition out of that rather than replace it.
     * The latch of the words last drawn is updated with it, so the redraw does not repeat on the
     * next task.
     *
     * Refused while an overlay owns the display: the overlay would be overwritten for one task and
     * redraw itself on the next, which looks like a glitch rather than like the refusal it is.
     *
     * \return kEOk, or kENotOk while an overlay shows; the display is then left as it was.
     */
    StdReturnType RefreshClock() {
        if(IsOverlayShowing()) { return kENotOk; }

        const ClockTime time = RealTimeClock::GetInstance().GetTime();
        UpdateClockWords(time.GetHour(), time.GetMinute());
        ShowClock(time.GetHour(), time.GetMinute());
        return kEOk;
    }

    /*!
     * \brief Runs the selected animation on the current time, without waiting for the word change
     *        that would otherwise be the only trigger.
     *
     * Selecting the animation again with `9 -A<id>` shows it too, but it also means something
     * else - it changes what runs from now on.
     *
     * \return kENotOk while an overlay shows, with nothing started; otherwise what
     *         Animations::SetTime() returns.
     */
    static StdReturnType StartAnimation() {
        if(IsOverlayShowing()) { return kENotOk; }

        const ClockTime time = RealTimeClock::GetInstance().GetTime();
        return Animations::GetInstance().SetTime(time.GetHour(), time.GetMinute());
    }

    /*!
     * \brief Ends a running animation and puts the time back on the display.
     *
     * At a low speed an animation runs for minutes, and both a word change and a mode change are
     * ignored while it does.
     *
     * \return kEOk, or kENotOk while an overlay shows; the animation is ended all the same, but the
     *         time is not drawn.
     */
    StdReturnType AbortAnimation() {
        Animations::GetInstance().Abort();
        return RefreshClock();
    }
};

#endif
