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
  getState()
******************************************************************************************************************************************************/
/*! \brief          the state of the overlays as a whole
 *  \details        Showing beats idle beats disabled, so that the one overlay holding the
 *                  display decides, and an armed one is not hidden behind the others that
 *                  are switched off. Disabled is what is left over when none of them has
 *                  anything to do, which is why it needs no test of its own.
 *
 *  \return         the state
******************************************************************************************************************************************************/
Overlays::StateType Overlays::getState() const {
    if(isShow()) { return OverlayType::STATE_SHOW; }
    if(isIdle()) { return OverlayType::STATE_IDLE; }
    return OverlayType::STATE_DISABLED;
} /* getState */

/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
void Overlays::task() {
    ClockDate date = RealTimeClock::getInstance().getDate();
    ClockTime time = RealTimeClock::getInstance().getTime();

    if(LastSecond != time.getSecond()) {
        LastSecond = time.getSecond();
        if(isShow()) { taskShow(date, time); }
        else { taskIdle(date, time); }
    }
} /* task */


/******************************************************************************************************************************************************
  abort()
******************************************************************************************************************************************************/
/*! \brief          ends the overlay that is showing
 *  \details        Through the same transition the expired show timer takes, so the
 *                  overlay stops its text and goes back to idle rather than being cut
 *                  off mid-word. What brings the clock back afterwards is DisplayManager,
 *                  which redraws once it sees that no overlay is showing any more.
 *
 *                  The timer is cleared here rather than left to run out: it is what
 *                  taskIdle() reads to decide whether an overlay may start, so a
 *                  leftover value would block the next one for as long as the aborted
 *                  overlay had left.
 *
 *  \return         E_OK if an overlay was showing
******************************************************************************************************************************************************/
StdReturnType Overlays::abort() {
    if(!isShow()) { return E_NOT_OK; }

    ClockDate date = RealTimeClock::getInstance().getDate();
    ClockTime time = RealTimeClock::getInstance().getTime();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(Date.getState() == OverlayDate::STATE_SHOW) { Date.stopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(Temperature.getState() == OverlayTemperature::STATE_SHOW) { Temperature.stopShow(date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(Text.getState() == OverlayText::STATE_SHOW) { Text.stopShow(date, time); }
#endif

    ShowTimerInSeconds = 0u;
    return E_OK;
} /* abort */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  taskIdle()
******************************************************************************************************************************************************/
void Overlays::taskIdle(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if((Date.getIsActive()) && (ShowTimerInSeconds == 0u)) {
        ShowTimerInSeconds = Date.task(ShowTimerInSeconds, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if((Temperature.getIsActive()) && (ShowTimerInSeconds == 0u)) {
        ShowTimerInSeconds =  Temperature.task(ShowTimerInSeconds, date, time);
    }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if((Text.getIsActive()) && (ShowTimerInSeconds == 0u)) {
        ShowTimerInSeconds =  Text.task(ShowTimerInSeconds, date, time);
    }
#endif
} /* taskIdle */

/******************************************************************************************************************************************************
  taskShow()
******************************************************************************************************************************************************/
void Overlays::taskShow(ClockDate date, ClockTime time) {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    if(Date.getState() == OverlayDate::STATE_SHOW) { ShowTimerInSeconds = Date.task(ShowTimerInSeconds, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    if(Temperature.getState() == OverlayTemperature::STATE_SHOW) { ShowTimerInSeconds =  Temperature.task(ShowTimerInSeconds, date, time); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    if(Text.getState() == OverlayText::STATE_SHOW) { ShowTimerInSeconds =  Text.task(ShowTimerInSeconds, date, time); }
#endif
} /* taskShow */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
