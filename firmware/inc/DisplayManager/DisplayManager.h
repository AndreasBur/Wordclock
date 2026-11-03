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
/**     \file       DisplayManager.h
 *      \brief      Decides what the display shows
 *
 *      \details    The Scheduler answers when something runs and the Display how it
 *                  reaches the LEDs; this decides what is on it. Its task arbitrates
 *                  between the clock, the animation that carries a word change and
 *                  an overlay that takes the display over for a while, and it keeps
 *                  the brightness the LEDs are driven with up to date.
 *
******************************************************************************************************************************************************/
#ifndef _DISPLAY_MANAGER_H_
#define _DISPLAY_MANAGER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Clock.h"
#include "Animations.h"
#include "Overlays.h"
#include "RealTimeClock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DisplayManager configuration parameter */

#define DISPLAY_MANAGER_TASK_CYCLE                  1u

/* DisplayManager parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y   M A N A G E R
******************************************************************************************************************************************************/
class DisplayManager
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockTime::HourType;
    using MinuteType = ClockTime::MinuteType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte kTaskCycle{DISPLAY_MANAGER_TASK_CYCLE};
    ClockWords last_clock_words_{};
    /* Rather than letting a default-constructed ClockWords stand for "nothing drawn
       yet": that only works as long as no real word set can equal it, which is a
       property of ClockWords the compiler would not defend. An own flag says so
       outright, and the first task then always draws the current time. */
    bool clock_words_initialized_{false};
    /* Whether an overlay held the display on the previous task, which is what makes the
       end of one detectable. */
    bool overlay_was_showing_{false};

    // functions
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

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static DisplayManager& GetInstance() {
        static DisplayManager singleton_instance;
        return singleton_instance;
    }

	// get methods
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

	// set methods

	// methods
    void Task();

    /* Draws the current time again, right now and without an animation. What asks for
       this is a display that no longer shows the clock at all - a test pattern, a pixel
       set by hand - and an animation would transition out of that rather than replace
       it. The latch is updated with it, so the redraw does not repeat on the next task.

       Refused while an overlay owns the display: the overlay would be overwritten for
       one task and redraw itself on the next, which looks like a glitch rather than
       like the refusal it is. */
    StdReturnType RefreshClock() {
        if(IsOverlayShowing()) { return kENotOk; }

        const ClockTime time = RealTimeClock::GetInstance().GetTime();
        UpdateClockWords(time.GetHour(), time.GetMinute());
        ShowClock(time.GetHour(), time.GetMinute());
        return kEOk;
    }

    /* Runs the selected animation on the current time, without waiting for the word
       change that would otherwise be the only trigger. Selecting the animation again
       with "9 -A<id>" shows it too, but it also means something else - it changes what
       runs from now on. */
    static StdReturnType StartAnimation() {
        if(IsOverlayShowing()) { return kENotOk; }

        const ClockTime time = RealTimeClock::GetInstance().GetTime();
        return Animations::GetInstance().SetTime(time.GetHour(), time.GetMinute());
    }

    /* Ends a running animation and puts the time back on the display. At a low speed an
       animation runs for minutes, and both a word change and a mode change are ignored
       while it does. */
    StdReturnType AbortAnimation() {
        Animations::GetInstance().Abort();
        return RefreshClock();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
