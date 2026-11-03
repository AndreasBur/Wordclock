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
/**     \file       Overlays.cpp
 *      \brief
 *
 *      \details
 *
 *
******************************************************************************************************************************************************/
#define _OVERLAYS_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Overlays.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  GetState()
******************************************************************************************************************************************************/
/*! \brief          the state of the overlays as a whole
 *  \details        Showing beats idle beats disabled, so that the one overlay holding the
 *                  display decides, and an armed one is not hidden behind the others that
 *                  are switched off. Disabled is what is left over when none of them has
 *                  anything to do, which is why it needs no test of its own.
 *
 *  \return         the state
******************************************************************************************************************************************************/
Overlays::StateType Overlays::GetState() const {
    if(IsShow()) { return OverlayType::StateType::kShow; }
    if(IsIdle()) { return OverlayType::StateType::kIdle; }
    return OverlayType::StateType::kDisabled;
} /* GetState */

/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
void Overlays::Task() {
    ClockDate date = RealTimeClock::GetInstance().GetDate();
    ClockTime time = RealTimeClock::GetInstance().GetTime();

    if(last_second_ != time.GetSecond()) {
        last_second_ = time.GetSecond();
        if(IsShow()) { TaskShow(date, time); }
        else { TaskIdle(date, time); }
    }
} /* Task */


/******************************************************************************************************************************************************
  Abort()
******************************************************************************************************************************************************/
/*! \brief          ends the overlay that is showing
 *  \details        Through the same transition the expired show timer takes, so the
 *                  overlay stops its text and goes back to idle rather than being cut
 *                  off mid-word. What brings the clock back afterwards is DisplayManager,
 *                  which redraws once it sees that no overlay is showing any more.
 *
 *                  The timer is cleared here rather than left to run out: it is what
 *                  TaskIdle() reads to decide whether an overlay may start, so a
 *                  leftover value would block the next one for as long as the aborted
 *                  overlay had left.
 *
 *  \return         kEOk if an overlay was showing
******************************************************************************************************************************************************/
StdReturnType Overlays::Abort() {
    if(!IsShow()) { return kENotOk; }

    ClockDate date = RealTimeClock::GetInstance().GetDate();
    ClockTime time = RealTimeClock::GetInstance().GetTime();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(date_.GetState() == OverlayDate::StateType::kShow) { date_.StopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(temperature_.GetState() == OverlayTemperature::StateType::kShow) { temperature_.StopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(text_.GetState() == OverlayText::StateType::kShow) { text_.StopShow(date, time); }
#endif

    show_timer_in_seconds_ = 0u;
    return kEOk;
} /* Abort */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  TaskIdle()
******************************************************************************************************************************************************/
void Overlays::TaskIdle(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if((date_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ = date_.Task(show_timer_in_seconds_, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if((temperature_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ =  temperature_.Task(show_timer_in_seconds_, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if((text_.GetIsActive()) && (show_timer_in_seconds_ == 0u)) {
        show_timer_in_seconds_ =  text_.Task(show_timer_in_seconds_, date, time);
    }
#endif
} /* TaskIdle */

/******************************************************************************************************************************************************
  TaskShow()
******************************************************************************************************************************************************/
void Overlays::TaskShow(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(date_.GetState() == OverlayDate::StateType::kShow) { show_timer_in_seconds_ = date_.Task(show_timer_in_seconds_, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(temperature_.GetState() == OverlayTemperature::StateType::kShow) { show_timer_in_seconds_ =  temperature_.Task(show_timer_in_seconds_, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(text_.GetState() == OverlayText::StateType::kShow) { show_timer_in_seconds_ =  text_.Task(show_timer_in_seconds_, date, time); }
#endif
} /* TaskShow */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
