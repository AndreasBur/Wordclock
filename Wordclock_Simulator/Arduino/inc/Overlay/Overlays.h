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
#include "StringTools.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Overlays configuration parameter */
#define OVERLAYS_SUPPORT_DATE             STD_ON
#define OVERLAYS_SUPPORT_TEMPERATURE      STD_ON
#define OVERLAYS_SUPPORT_TEXT             STD_ON

/* Overlays parameter */

/******************************************************************************************************************************************************
 * D E P E N D E N D   I N C L U D E S
******************************************************************************************************************************************************/
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
# include "OverlayDate.h"
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
# include "OverlayTemperature.h"
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
# include "OverlayText.h"
#endif

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
    using MonthType = Overlay::MonthType;
    using MonthRawType = Overlay::MonthRawType;
    using DayType = Overlay::DayType;
    using HourType = Overlay::HourType;
    using MinuteType = Overlay::MinuteType;
    using SecondType = Overlay::SecondType;

#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    using LengthType = OverlayText::LengthType;
#endif


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
    MinuteType getDatePeriodInMinutes() const { return Date.getPeriodInMinutes(); }
    SecondType getDateEnduranceInSeconds() const { return Date.getEnduranceInSeconds(); }
    MonthRawType getDateMonth() const { return Date.getMonth(); }
    DayType getDateDay() const { return Date.getDay(); }
    DayType getDateValidInDays() const { return Date.getValidInDays(); }
    bool getDateIsActive() const { return Date.getIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    MinuteType getTemperaturePeriodInMinutes() const { return Temperature.getPeriodInMinutes(); }
    SecondType getTemperatureEnduranceInSeconds() const { return Temperature.getEnduranceInSeconds(); }
    MonthRawType getTemperatureMonth() const { return Temperature.getMonth(); }
    DayType getTemperatureDay() const { return Temperature.getDay(); }
    DayType getTemperatureValidInDays() const { return Temperature.getValidInDays(); }
    bool getTemperatureIsActive() const { return Temperature.getIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    MinuteType getTextPeriodInMinutes() const { return Text.getPeriodInMinutes(); }
    SecondType getTextEnduranceInSeconds() const { return Text.getEnduranceInSeconds(); }
    MonthRawType getTextMonth() const { return Text.getMonth(); }
    DayType getTextDay() const { return Text.getDay(); }
    DayType getTextValidInDays() const { return Text.getValidInDays(); }
    bool getTextIsActive() const { return Text.getIsActive(); }
    const char* getTextText() const { return Text.getText(); }
    byte getTextSpeed() const { return Text.getSpeed(); }
#endif

	// set methods
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    void setDatePeriodInMinutes(MinuteType PeriodInMinutes) { Date.setPeriodInMinutes(PeriodInMinutes); }
    void setDateEnduranceInSeconds(SecondType EnduranceInSeconds) { Date.setEnduranceInSeconds(EnduranceInSeconds); }
    void setDateMonth(MonthRawType Month) { Date.setMonth(Month); }
    void setDateDay(DayType Day) { Date.setDay(Day); }
    void setDateValidInDays(DayType ValidInDays) { Date.setValidInDays(ValidInDays); }
    void setDateIsActive(bool IsActive) { Date.setIsActive(IsActive); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    void setTemperaturePeriodInMinutes(MinuteType PeriodInMinutes) { Temperature.setPeriodInMinutes(PeriodInMinutes); }
    void setTemperatureEnduranceInSeconds(SecondType EnduranceInSeconds) { Temperature.setEnduranceInSeconds(EnduranceInSeconds); }
    void setTemperatureMonth(MonthRawType Month) { Temperature.setMonth(Month); }
    void setTemperatureDay(DayType Day) { Temperature.setDay(Day); }
    void setTemperatureValidInDays(DayType ValidInDays) { Temperature.setValidInDays(ValidInDays); }
    void setTemperatureIsActive(bool IsActive) { Temperature.setIsActive(IsActive); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    void setTextPeriodInMinutes(MinuteType PeriodInMinutes) { Text.setPeriodInMinutes(PeriodInMinutes); }
    void setTextEnduranceInSeconds(SecondType EnduranceInSeconds) { Text.setEnduranceInSeconds(EnduranceInSeconds); }
    void setTextMonth(MonthRawType Month) { Text.setMonth(Month); }
    void setTextDay(DayType Day) { Text.setDay(Day); }
    void setTextValidInDays(DayType ValidInDays) { Text.setValidInDays(ValidInDays); }
    void setTextIsActive(bool IsActive) { Text.setIsActive(IsActive); }
    void setTextText(const char* sText, LengthType Length) { Text.setText(sText, Length); }
    void setTextSpeed(byte Speed) { Text.setSpeed(Speed); }
#endif

	// methods
	void task() {

	}

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
