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
 *                  A night brightness of zero also takes the strip's supply away where there
 *                  is a switch to do it with. That is the state the switch was built for:
 *                  a display that is off still draws its quiescent current, some 110 mA for
 *                  this strip, and a night is the longest stretch in which nobody is looking
 *                  at what it buys. Power is asked rather than the port written, because the
 *                  cut is a sequence over several ticks and not a write.
 *
 *                  It acts on the crossing and not on the state. A clock switched on by
 *                  hand at two in the morning stays on until the window's next edge, rather
 *                  than going dark again a second later - which looks like a fault, and is
 *                  the behaviour that makes people stop using a timer.
 *
 *                  The supply is the one exception, and it is one because a cut rail turns
 *                  "on" into a switch that does nothing: the display would be enabled, the
 *                  data line gated and the wall still dark. So a hand that switches the
 *                  clock on inside the window gets the supply back, which is the crossing
 *                  rule kept rather than broken - the display stays where the hand put it.
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
#include "Power.h"
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

    /* Whether it was the night that took the supply down. What it guards is the tick that
       watches for a hand: without it a supply cut by hand at noon would be handed straight
       back on the next tick, because the display is enabled the whole time. A window's edge
       is another matter and gives it back either way - the same rule that already re-enables
       a display somebody switched off. */
    bool SupplyCutForNight{false};

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

    /* Power is asked rather than the port written: it darkens the strip over the data line
       first and waits for that frame to leave the wire, so the rail is still up when this
       returns and the sequence finishes on the ticks after it.

       Where no switch is fitted the display is darkened the way it always was, and the strip
       keeps its current because on that board it cannot do otherwise - which is the same
       answer the procedures give, rather than a night that quietly does less than it says. */
    void cutSupplyForTheNight() {
        if(!Power::isSwitchFitted()) { Display::getInstance().disable(); return; }

        Power::getInstance().switchSupplyOff();
        SupplyCutForNight = true;
    }

    /* Ahead of the enable and not after it, because the rail has to be up before a frame
       goes out - Power keeps the data line gated until it is, and releases it a tick later. */
    void restoreSupplyAfterTheNight() {
        if(!SupplyCutForNight) { return; }

        Power::getInstance().switchSupplyOn();
        SupplyCutForNight = false;
    }

    void applyNight() {
        if(isDisplayOffAtNight(NightBrightness)) { cutSupplyForTheNight(); }
        else                                     { Display::getInstance().setBrightnessNightLevel(NightBrightness); }
    }

    /* Both undone rather than only the one that was applied: the night brightness may have
       been changed while the window was open, and then the morning has to put back
       whichever of the two the evening happened to use. */
    void applyDay() {
        Display::getInstance().clearBrightnessNight();
        restoreSupplyAfterTheNight();
        Display::getInstance().enable();
    }

    /* The one thing watched as a state rather than as an edge, and only because a cut rail
       makes it a different question. Between two crossings the display is whatever anybody
       last made it - but where the night took the supply away, switching the clock on reaches
       a gated data line and an unpowered strip, so the wall stays dark and the switch reads
       as broken. Giving the rail back is what makes that hand mean something at two in the
       morning; it is the crossing rule kept rather than an exception to it.

       Nothing here cuts the supply again. A display switched off by hand a minute later keeps
       its rail until the window's next edge, for the same reason the rest of this module acts
       on crossings: a clock that undid what a hand just did would read as a fault. */
    void followTheDisplayBackOn() {
        if(!SupplyCutForNight) { return; }
        if(!Display::getInstance().isEnabled()) { return; }

        restoreSupplyAfterTheNight();
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
       anybody last made it, so switching it on by hand at night keeps it on - which is what
       the tick between two edges is for, since a hand that switched it on has a supply to
       get back. */
    void task() {
        if(!IsActive) { return; }

        const bool IsNightNow = isNight();

        if(IsNightNow == WasNight) { followTheDisplayBackOn(); return; }
        WasNight = IsNightNow;

        if(IsNightNow) { applyNight(); }
        else           { applyDay(); }
    }
};

#endif // _NIGHT_SWITCH_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
