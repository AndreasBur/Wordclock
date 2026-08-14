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

    using TemperatureType = Temperature::TemperatureType;

    /* "-12.3C" and its terminator, which is the longest a reading the chip can produce
       makes: it measures from -40 to +85 degrees. */
    static constexpr byte TemperatureStringLength{6u + 1u};
    static constexpr char DegreeUnit{'C'};
    static constexpr char DecimalPoint{'.'};

    char TemperatureString[TemperatureStringLength]{0u};

    // functions
    /* Only ever entered with a reading in hand: isReady() is what the overlay is started
       through, and it is what says whether there is one. */
    void setStateToShow(ClockDate CurrentDate, ClockTime CurrentTime) {
        setTemperatureString();
        setText();
        UNUSED(CurrentDate);
        UNUSED(CurrentTime);
    }
    void setStateToIdle(ClockDate CurrentDate, ClockTime CurrentTime) {
        Text::getInstance().stop();
        UNUSED(CurrentDate);
        UNUSED(CurrentTime);
    }

    /* No reading, no overlay: a clock without the chip - and every clock until it has
       answered once - would otherwise hold the display for the whole endurance to show
       nothing, or show a zero that reads like a measurement. */
    bool isReady() const { return Temperature::getInstance().isTemperatureAvailable(); }

    void showTask() { if(Text::getInstance().getState() == Text::STATE_IDLE) { setText(); } }
    void setText() { Text::getInstance().setTextWithShift(TemperatureString, getFont()); }

    void setTemperatureString() {
        TemperatureType Tenths{0};
        if(Temperature::getInstance().getTemperature(Tenths) == E_NOT_OK) { return; }

        char* tmp = TemperatureString;
        tmp = appendSign(Tenths, tmp);
        tmp = appendNumber(wholeDegrees(Tenths), tmp);
        tmp = appendChar(DecimalPoint, tmp);
        tmp = appendNumber(tenthOfDegree(Tenths), tmp);
        tmp = appendChar(DegreeUnit, tmp);
        *tmp = STD_NULL_CHARACTER;
    }

    /* Sign, whole part and tenth are taken apart before they are written, so that the
       digits themselves are always the ones of a positive number - a remainder of a
       negative value is negative in C, and would print a second minus in the middle of
       the reading. */
    static char* appendSign(TemperatureType Tenths, char* String) {
        if(Tenths >= 0) { return String; }

        return appendChar('-', String);
    }
    static TemperatureType wholeDegrees(TemperatureType Tenths) { return absolute(Tenths) / Temperature::TenthsPerDegree; }
    static TemperatureType tenthOfDegree(TemperatureType Tenths) { return absolute(Tenths) % Temperature::TenthsPerDegree; }
    static constexpr TemperatureType absolute(TemperatureType Value) { return (Value < 0) ? static_cast<TemperatureType>(-Value) : Value; }

    static char* appendChar(char Char, char* String) { String[0u] = Char; return &String[1u]; }
    static char* appendNumber(TemperatureType Value, char* String) {
        itoa(Value, String, 10u);
        return &String[digitsOfNumber(Value)];
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr OverlayTemperature() { }
    ~OverlayTemperature() { }

	// get methods
    const char* getTemperatureString() const { return TemperatureString; }

	// set methods

	// methods
    SecondType task(SecondType ShowTimerInSeconds, ClockDate CurrentDate, ClockTime CurrentTime) {
        if(State == STATE_SHOW) { showTask(); }
        return Overlay::task(ShowTimerInSeconds, CurrentDate, CurrentTime);
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
