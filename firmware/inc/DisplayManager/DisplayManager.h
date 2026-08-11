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
 *                  between the clock, the animation that carries a minute change and
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

#define DISPLAY_MANAGER_TASK_CYCLE                  50u

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
    static constexpr byte TaskCycle{DISPLAY_MANAGER_TASK_CYCLE};
    /* No minute, so the first task draws whatever time it finds instead of waiting for
       the next change. */
    static constexpr MinuteType MinuteNone{255u};

    MinuteType LastMinute{MinuteNone};
    /* Whether an overlay held the display on the previous task, which is what makes the
       end of one detectable. */
    bool OverlayWasShowing{false};

    // functions
    constexpr DisplayManager() { }
    ~DisplayManager() { }

    bool isOverlayShowing() const {
        return Overlays::getInstance().getState() == Overlays::OverlayType::STATE_SHOW;
    }

    void showClock(HourType Hour, MinuteType Minute) const {
        Display::getInstance().clear();
        Clock::getInstance().setTime(Hour, Minute);
        Display::getInstance().show();
    }

    void taskMinuteChange(HourType Hour, MinuteType Minute) const {
        Animations& animations = Animations::getInstance();
        if(animations.getAnimation() == Animations::ANIMATION_ID_NONE) {
            showClock(Hour, Minute);
        } else {
            animations.setTime(Hour, Minute);
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static DisplayManager& getInstance() {
        static DisplayManager SingletonInstance;
        return SingletonInstance;
    }

	// get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

	// set methods

	// methods
    void init();
    void task();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
