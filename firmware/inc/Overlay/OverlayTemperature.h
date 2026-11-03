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
/**     \file       OverlayTemperature.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _OVERLAY_TEMPERATURE_H_
#define _OVERLAY_TEMPERATURE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Overlay.h"
#include "Temperature.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* OverlayTemperature configuration parameter */


/* OverlayTemperature parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y   T E M P E R A T U R E
******************************************************************************************************************************************************/
class OverlayTemperature : public Overlay<OverlayTemperature>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class Overlay;

    /* Written when the overlay starts and left alone while it shows: the text scrolls out
       of this buffer, and a reading that changed halfway through would swap a digit under
       the letters that are already on their way across. */
    char temperature_string_[Temperature::kStringToShowLength]{0u};

    // functions
    /* Only ever entered with a reading in hand: IsReady() is what the overlay is started
       through, and it is what says whether there is one. */
    void SetStateToShow(ClockDate current_date, ClockTime current_time) {
        SetTemperatureString();
        SetText();
        UNUSED(current_date);
        UNUSED(current_time);
    }
    static void SetStateToIdle(ClockDate current_date, ClockTime current_time) {
        Text::GetInstance().Stop();
        UNUSED(current_date);
        UNUSED(current_time);
    }

    /* No reading, no overlay: a clock without the chip - and every clock until it has
       answered once - would otherwise hold the display for the whole endurance to show
       nothing, or show a zero that reads like a measurement. */
    static bool IsReady() { return Temperature::GetInstance().IsTemperatureAvailable(); }

    /* Nothing of its own: the string it shows comes from the chip on every start, so
       there is no configuration here for a reset to undo. */
    static void ResetOwnToDefaults() { }

    void ShowTask() { if(Text::GetInstance().GetState() == Text::kStateIdle) { SetText(); } }
    void SetText() { Text::GetInstance().SetTextWithShift(temperature_string_, GetFont()); }

    void SetTemperatureString() { Temperature::GetInstance().GetTemperatureStringToShow(temperature_string_); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayTemperature() { }
    ~OverlayTemperature() { }

	// get methods
    const char* GetTemperatureString() const { return temperature_string_; }

	// set methods

	// methods
    SecondType Task(SecondType show_timer_in_seconds, ClockDate current_date, ClockTime current_time) {
        if(state_ == kStateShow) { ShowTask(); }
        return Overlay::Task(show_timer_in_seconds, current_date, current_time);
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
