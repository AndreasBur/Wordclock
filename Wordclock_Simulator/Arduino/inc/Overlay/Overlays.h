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
/**     \file       Overlays.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _OVERLAYS_H_
#define _OVERLAYS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "OverlayDate.h"
#include "OverlayText.h"
#include "OverlayTemperature.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Overlays configuration parameter */
#define OVERLAYS_SUPPORT_DATE             STD_ON
#define OVERLAYS_SUPPORT_TEMPERATURE      STD_ON
#define OVERLAYS_SUPPORT_TEXT             STD_ON

/* Overlays parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y S
******************************************************************************************************************************************************/
class Overlays
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum OverlayIdType {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        OVERLAY_ID_DATE,
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        OVERLAY_ID_TEMPERATURE,
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        OVERLAY_ID_TEXT
#endif
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    OverlayDate Date;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    OverlayTemperature Temperature;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    OverlayText Text;
#endif

    // functions
    constexpr Overlays() { }
    ~Overlays() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Overlays& getInstance() {
        static Overlays singletonInstance;
        return singletonInstance;
    }

	// get methods
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    byte getDatePeriodInMinutes() const { return Date.getPeriodInMinutes(); }
    byte getDateEnduranceInSeconds() const { return Date.getEnduranceInSeconds(); }
    byte getDateMonth() const { return Date.getMonth(); }
    byte getDateDay() const { return Date.getDay(); }
    byte getDateValidInDays() const { return Date.getValidInDays(); }
    bool getDateIsActive() const { return Date.getIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    byte getTemperaturePeriodInMinutes() const { return Temperature.getPeriodInMinutes(); }
    byte getTemperatureEnduranceInSeconds() const { return Temperature.getEnduranceInSeconds(); }
    byte getTemperatureMonth() const { return Temperature.getMonth(); }
    byte getTemperatureDay() const { return Temperature.getDay(); }
    byte getTemperatureValidInDays() const { return Temperature.getValidInDays(); }
    bool getTemperatureIsActive() const { return Temperature.getIsActive(); }
#endif
    byte getTextPeriodInMinutes() const { return Text.getPeriodInMinutes(); }
    byte getTextEnduranceInSeconds() const { return Text.getEnduranceInSeconds(); }
    byte getTextMonth() const { return Text.getMonth(); }
    byte getTextDay() const { return Text.getDay(); }
    byte getTextValidInDays() const { return Text.getValidInDays(); }
    bool getTextIsActive() const { return Text.getIsActive(); }

	// set methods
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    void setDatePeriodInMinutes(byte PeriodInMinutes) { Date.setPeriodInMinutes(PeriodInMinutes); }
    void setDateEnduranceInSeconds(byte EnduranceInSeconds) { Date.setEnduranceInSeconds(EnduranceInSeconds); }
    void setDateMonth(byte Month) { Date.setMonth(Month); }
    void setDateDay(byte Day) { Date.setDay(Day); }
    void setDateValidInDays(byte ValidInDays) { Date.setValidInDays(ValidInDays); }
    void setDateIsActive(bool IsActive) { Date.setIsActive(IsActive); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    void setTemperaturePeriodInMinutes(byte PeriodInMinutes) { Temperature.setPeriodInMinutes(PeriodInMinutes); }
    void setTemperatureEnduranceInSeconds(byte EnduranceInSeconds) { Temperature.setEnduranceInSeconds(EnduranceInSeconds); }
    void setTemperatureMonth(byte Month) { Temperature.setMonth(Month); }
    void setTemperatureDay(byte Day) { Temperature.setDay(Day); }
    void setTemperatureValidInDays(byte ValidInDays) { Temperature.setValidInDays(ValidInDays); }
    void setTemperatureIsActive(bool IsActive) { Temperature.setIsActive(IsActive); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    void setTextPeriodInMinutes(byte PeriodInMinutes) { Text.setPeriodInMinutes(PeriodInMinutes); }
    void setTextEnduranceInSeconds(byte EnduranceInSeconds) { Text.setEnduranceInSeconds(EnduranceInSeconds); }
    void setTextMonth(byte Month) { Text.setMonth(Month); }
    void setTextDay(byte Day) { Text.setDay(Day); }
    void setTextValidInDays(byte ValidInDays) { Text.setValidInDays(ValidInDays); }
    void setTextIsActive(bool IsActive) { Text.setIsActive(IsActive); }
#endif

	// methods

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
