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
/**     \file       NightSwitch.h
 *      \brief      Takes the display down for the night and gives it back in the morning
 *
 *      \details    The clock lit the wall around the clock. This is the first setting that
 *                  is a time of day rather than a value, and the first that acts on its own
 *                  rather than when something asks.
 *
 *                  What it does at night is one number: the night brightness, where zero
 *                  means the display goes off altogether and anything else means it is
 *                  dimmed to that. One field rather than two settings, because "off" and
 *                  "very dim" are the same wish answered at different strengths.
 *
 *                  Dimming goes through the display's night level, which scales what the
 *                  setting, the gamma correction and the automatic arrived at. It does not
 *                  write the brightness setting, which matters twice: morning returns to
 *                  the brightness its owner chose rather than to one this module
 *                  remembered, and Persistence keeps storing that same chosen value instead
 *                  of whatever the clock happened to be showing at the moment it wrote.
 *
 *                  It acts on the crossing and not on the state. A clock switched on by
 *                  hand at two in the morning stays on until the window's next edge, rather
 *                  than going dark again a second later - which looks like a fault, and is
 *                  the behaviour that makes people stop using a timer.
 *
******************************************************************************************************************************************************/
#ifndef _NIGHT_SWITCH_H_
#define _NIGHT_SWITCH_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockTime.h"
#include "Display.h"
#include "RealTimeClock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* NightSwitch configuration parameter */
/* In scheduler ticks, so 100 * 10 ms = 1 s. A window's edge is a minute wide, so this is
   far finer than it needs to be - and still cheap, because a tick that is not an edge
   costs one comparison. */
#define NIGHT_SWITCH_TASK_CYCLE                         100u

/******************************************************************************************************************************************************
 *  C L A S S   N I G H T   S W I T C H
******************************************************************************************************************************************************/
class NightSwitch
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockTime::HourType;
    using MinuteType = ClockTime::MinuteType;

    /* Minutes since midnight, which is what the window is compared in: two numbers rather
       than four, and the wrap around midnight becomes one comparison instead of a case. */
    using TimeOfDayType = uint16_t;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{NIGHT_SWITCH_TASK_CYCLE};
    static constexpr TimeOfDayType MinutesPerHour{60u};
    static constexpr byte NightBrightnessOff{0u};
    /* What the display's night level is when nothing is dimming it. */
    static constexpr byte NightLevelFull{255u};

    static constexpr bool IsActiveInitValue{false};
    static constexpr HourType StartHourInitValue{23u};
    static constexpr MinuteType StartMinuteInitValue{0u};
    static constexpr HourType EndHourInitValue{6u};
    static constexpr MinuteType EndMinuteInitValue{0u};
    static constexpr byte NightBrightnessInitValue{NightBrightnessOff};

    bool IsActive{IsActiveInitValue};
    HourType StartHour{StartHourInitValue};
    MinuteType StartMinute{StartMinuteInitValue};
    HourType EndHour{EndHourInitValue};
    MinuteType EndMinute{EndMinuteInitValue};
    byte NightBrightness{NightBrightnessInitValue};

    /* What the last tick saw, so that only a change acts. Starts as day: a clock that comes
       up inside the window gets its first tick as an edge and switches straight away. */
    bool WasNight{false};

    NightSwitch() { }
    ~NightSwitch() { }

    // functions
    static constexpr TimeOfDayType toTimeOfDay(HourType Hour, MinuteType Minute) {
        return static_cast<TimeOfDayType>((Hour * MinutesPerHour) + Minute);
    }

    /* An empty window - one whose ends are the same minute - is no window at all rather
       than a whole day, because that is what somebody who has not configured it yet has. */
    static constexpr bool isWindowEmpty(TimeOfDayType Start, TimeOfDayType End) { return Start == End; }
    static constexpr bool isWindowCrossingMidnight(TimeOfDayType Start, TimeOfDayType End) { return Start > End; }

    static constexpr bool isWithinWindow(TimeOfDayType Now, TimeOfDayType Start, TimeOfDayType End) {
        if(isWindowEmpty(Start, End)) { return false; }
        /* The ordinary window is a span; the one that crosses midnight is everything
           outside the span between its ends, which is the same test read the other way. */
        if(isWindowCrossingMidnight(Start, End)) { return (Now >= Start) || (Now < End); }

        return (Now >= Start) && (Now < End);
    }

    static bool isDisplayOffAtNight(byte Brightness) { return Brightness == NightBrightnessOff; }

    void applyNight() const {
        if(isDisplayOffAtNight(NightBrightness)) { Display::getInstance().disable(); }
        else                                     { Display::getInstance().setBrightnessNightLevel(NightBrightness); }
    }

    /* Both undone rather than only the one that was applied: the night brightness may have
       been changed while the window was open, and then the morning has to put back
       whichever of the two the evening happened to use. */
    void applyDay() const {
        Display::getInstance().clearBrightnessNight();
        Display::getInstance().enable();
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static NightSwitch& getInstance() {
        static NightSwitch SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    bool getIsActive() const { return IsActive; }
    HourType getStartHour() const { return StartHour; }
    MinuteType getStartMinute() const { return StartMinute; }
    HourType getEndHour() const { return EndHour; }
    MinuteType getEndMinute() const { return EndMinute; }
    byte getNightBrightness() const { return NightBrightness; }

    /* Whether the clock considers it night now, which is what the answer reports - a
       window that is configured but switched off still has an answer to this. */
    bool isNight() const {
        const ClockTime Now = RealTimeClock::getInstance().getTime();

        return isWithinWindow(toTimeOfDay(Now.getHour(), Now.getMinute()),
                              toTimeOfDay(StartHour, StartMinute),
                              toTimeOfDay(EndHour, EndMinute));
    }

    // set methods
    void setStartHour(HourType sStartHour) { StartHour = sStartHour; }
    void setStartMinute(MinuteType sStartMinute) { StartMinute = sStartMinute; }
    void setEndHour(HourType sEndHour) { EndHour = sEndHour; }
    void setEndMinute(MinuteType sEndMinute) { EndMinute = sEndMinute; }
    void setNightBrightness(byte sNightBrightness) { NightBrightness = sNightBrightness; }

    /* Switching the whole thing off gives the display back rather than leaving it wherever
       the last edge put it - a clock left dark by a timer that is no longer running is a
       clock nobody can explain. */
    void setIsActive(bool sIsActive) {
        IsActive = sIsActive;
        if(!IsActive) { applyDay(); }
        WasNight = false;
    }

    // methods
    void resetToDefaults() {
        setIsActive(IsActiveInitValue);
        StartHour = StartHourInitValue;
        StartMinute = StartMinuteInitValue;
        EndHour = EndHourInitValue;
        EndMinute = EndMinuteInitValue;
        NightBrightness = NightBrightnessInitValue;
    }

    /* Acts on the crossing, not on the state: between two edges the display is whatever
       anybody last made it, so switching it on by hand at night keeps it on. */
    void task() {
        if(!IsActive) { return; }

        const bool IsNightNow = isNight();

        if(IsNightNow == WasNight) { return; }
        WasNight = IsNightNow;

        if(IsNightNow) { applyNight(); }
        else           { applyDay(); }
    }
};

#endif // _NIGHT_SWITCH_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
