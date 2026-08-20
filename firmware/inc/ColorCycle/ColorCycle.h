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
/**     \file       ColorCycle.h
 *      \brief      Walks the display's colour round the wheel while the words stand still
 *
 *      \details    Every one of the fifteen animations is a transition: something happens
 *                  because the words changed, and it is over. This is the other kind, and
 *                  the clock had none - the display stands still for five minutes at a time,
 *                  and this is what happens during those five minutes.
 *
 *                  It is not an animation id, and that is a decision rather than an
 *                  omission. An id would put it in the list a word change picks from, where
 *                  it has nothing to do: it does not begin, end, or have anything to say
 *                  about a transition. The favourites mask is exactly full at sixteen ids as
 *                  well, so it could not have gone there without widening that first.
 *
 *                  What it does not do is write the colour setting. The cycle is a level of
 *                  its own that DisplayColor applies on the way to the strip, so the colour
 *                  somebody chose is still the colour the command answers and Persistence
 *                  stores - the same arrangement the night brightness has, and for the same
 *                  reason: a setting a feature overwrote is a setting nobody can get back.
 *                  Switching the cycle off therefore needs no memory of what was there
 *                  before; the level goes away and what was underneath it was never
 *                  touched.
 *
 *                  The hue is deliberately not stored. Persistence writes when the stored
 *                  blob differs from the live settings, so a hue in that blob would be a
 *                  flash write per step - a few hundred thousand of them a day, on a part
 *                  rated for a hundred thousand. What survives a restart is whether the
 *                  cycle runs and how fast; where in the round it was does not matter to
 *                  anybody.
 *
******************************************************************************************************************************************************/
#ifndef _COLOR_CYCLE_H_
#define _COLOR_CYCLE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ColorCycle configuration parameter */
/* Scheduler ticks per step of hue, so 100 * 10 ms is a step a second and 256 of those are
   a round in a little over four minutes. Slow is the point: this runs on a clock somebody
   lives with, and a wheel spun fast enough to notice is a wheel nobody can read the time
   off. It is also the initial value of a setting rather than a fixed cycle, so a clock that
   wants faster says so with command 15. */
#define COLOR_CYCLE_TASK_CYCLE_INIT_VALUE               100u

/******************************************************************************************************************************************************
 *  C L A S S   C O L O R   C Y C L E
******************************************************************************************************************************************************/
class ColorCycle
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* The wheel walked in one byte, so a step is a step of this and the round closes by
       overflowing rather than by a comparison. */
    using HueType = byte;

    static constexpr byte TaskCycleInitValue{COLOR_CYCLE_TASK_CYCLE_INIT_VALUE};
    static constexpr bool IsActiveInitValue{false};
    static constexpr HueType HueInitValue{0u};

    /* The six segments of the wheel, and the channel that is at full through each of them.
       Named because the conversion below is otherwise six lines of arithmetic that only
       says what it is if the reader already knows the wheel. */
    static constexpr byte NumberOfSegments{6u};
    static constexpr byte FullChannel{255u};

    bool IsActive{IsActiveInitValue};
    byte TaskCycle{TaskCycleInitValue};
    HueType Hue{HueInitValue};

    ColorCycle() { }
    ~ColorCycle() { }

    // functions
    static Display& getDisplay() { return Display::getInstance(); }

    /* One hue as a colour, at full saturation and full value - the outside edge of the
       wheel. Anything inside it is a colour the strip shows as a paler version of white,
       and a cycle whose hue cannot be seen is not what this is for.

       Integer throughout, six segments of 256/6 each: within a segment one channel rises
       while the third stays dark, and which channel does what is the segment number. The
       multiplication by six before the shift is what keeps that division exact without a
       remainder to carry, which a hue split into 43-wide segments would have had.

       Plain static rather than constexpr: Pixel declares a destructor, so it is not a
       literal type and cannot be what a constexpr function returns. */
    static Pixel toColour(HueType Hue) {
        const uint16_t Scaled = static_cast<uint16_t>(Hue) * NumberOfSegments;
        const byte Segment = static_cast<byte>(Scaled >> 8u);
        const byte Rising = static_cast<byte>(Scaled & 0xFFu);
        const byte Falling = static_cast<byte>(FullChannel - Rising);

        switch(Segment) {
            case 0u  : return Pixel(FullChannel, Rising,      0u);
            case 1u  : return Pixel(Falling,     FullChannel, 0u);
            case 2u  : return Pixel(0u,          FullChannel, Rising);
            case 3u  : return Pixel(0u,          Falling,     FullChannel);
            case 4u  : return Pixel(Rising,      0u,          FullChannel);
            default  : return Pixel(FullChannel, 0u,          Falling);
        }
    }

    /* A display that is off has nothing to cycle, and stepping it anyway would cost more
       than the arithmetic: every step marks the pixel buffer, and the supply switch waits
       for a buffer that has settled before it drops the rail. A cycle running through the
       night would hold that wait open and spend the current the switch exists to save. */
    static bool isDisplayShowingAnything() { return getDisplay().isEnabled(); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static ColorCycle& getInstance() {
        static ColorCycle SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    /* Zero is a cycle standing still on the hue it reached, which the scheduler answers
       with a task that never comes due - so it needs no rule of its own here. */
    byte getTaskCycle() const { return TaskCycle; }

    bool getIsActive() const { return IsActive; }
    byte getHue() const { return Hue; }

    // set methods
    void setTaskCycle(byte sTaskCycle) { TaskCycle = sTaskCycle; }

    /* Switching off puts the chosen colour back at once rather than at the next tick: the
       tick may be seconds away, and a setting that takes seconds to take effect reads as
       one that did not work. */
    void setIsActive(bool sIsActive) {
        IsActive = sIsActive;

        if(IsActive) { getDisplay().setColorCycle(toColour(Hue)); }
        else         { getDisplay().clearColorCycle(); }
    }

    // methods
    void resetToDefaults() {
        setTaskCycle(TaskCycleInitValue);
        Hue = HueInitValue;
        setIsActive(IsActiveInitValue);
    }

    void task() {
        if(!IsActive) { return; }
        if(!isDisplayShowingAnything()) { return; }

        Hue++;
        getDisplay().setColorCycle(toColour(Hue));
    }
};

#endif // _COLOR_CYCLE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
