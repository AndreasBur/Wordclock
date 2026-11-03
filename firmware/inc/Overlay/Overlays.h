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
#include "RealTimeClock.h"
#include "Overlay.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Overlays configuration parameter */
#define OVERLAYS_SUPPORT_DATE               STD_ON
#define OVERLAYS_SUPPORT_TEMPERATURE        STD_ON
#define OVERLAYS_SUPPORT_TEXT               STD_ON

#define OVERLAYS_TASK_CYCLE                 1u

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
    using OverlayType = Overlay<Overlays>;
    using MonthType = OverlayType::MonthType;
    using DayType = OverlayType::DayType;
    using HourType = OverlayType::HourType;
    using MinuteType = OverlayType::MinuteType;
    using SecondType = OverlayType::SecondType;
    using StateType = OverlayType::StateType;
    using FontType = OverlayType::FontType;

#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    using LengthType = OverlayText::LengthType;
#endif


    enum class OverlayIdType {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        kDate,
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        kTemperature,
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        kText
#endif
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte kTaskCycle{OVERLAYS_TASK_CYCLE};
    SecondType show_timer_in_seconds_{0u};
    SecondType last_second_{0u};
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    OverlayDate date_;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    OverlayTemperature temperature_;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    OverlayText text_;
#endif

    // functions
    constexpr Overlays() { }
    ~Overlays() { }

    bool IsShow() const {
        bool show{false};
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        show = show || (date_.GetState() == OverlayDate::StateType::kShow);
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        show = show || (temperature_.GetState() == OverlayTemperature::StateType::kShow);
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        show = show || (text_.GetState() == OverlayText::StateType::kShow);
#endif
        return show;
    }

    bool IsIdle() const {
        bool idle{false};
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        idle = idle || (date_.GetState() == OverlayDate::StateType::kIdle);
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        idle = idle || (temperature_.GetState() == OverlayTemperature::StateType::kIdle);
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        idle = idle || (text_.GetState() == OverlayText::StateType::kIdle);
#endif
        return idle;
    }

    void TaskIdle(ClockDate, ClockTime);
    void TaskShow(ClockDate, ClockTime);

    /* Shared by the three "show now" entry points below, which differ in nothing but the
       overlay they name - a template rather than three copies of the two rules that
       decide whether an overlay may start at all.

       Switched off is a configuration and not a state to be talked out of, and only one
       overlay may hold the display, because there is one show timer for all of them. */
    template <typename OverlayClassType>
    StdReturnType ShowNow(OverlayClassType& overlay) {
        if(!overlay.GetIsActive()) { return kENotOk; }
        if(!overlay.CanShow()) { return kENotOk; }
        if(IsShow()) { return kENotOk; }

        show_timer_in_seconds_ = overlay.StartShow(RealTimeClock::GetInstance().GetDate(),
                                               RealTimeClock::GetInstance().GetTime());
        return kEOk;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Overlays& GetInstance() {
        static Overlays singleton_instance;
        return singleton_instance;
    }

	// get methods
    static byte GetTaskCycle() { return kTaskCycle; }
    StateType GetState() const;

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    MinuteType GetDatePeriodInMinutes() const { return date_.GetPeriodInMinutes(); }
    SecondType GetDateEnduranceInSeconds() const { return date_.GetEnduranceInSeconds(); }
    MonthType GetDateMonth() const { return date_.GetMonth(); }
    DayType GetDateDay() const { return date_.GetDay(); }
    DayType GetDateValidInDays() const { return date_.GetValidInDays(); }
    byte GetDateSpeed() const { return date_.GetSpeed(); }
    Text::FontType GetDateFont() const { return date_.GetFont(); }
    bool GetDateIsActive() const { return date_.GetIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    MinuteType GetTemperaturePeriodInMinutes() const { return temperature_.GetPeriodInMinutes(); }
    SecondType GetTemperatureEnduranceInSeconds() const { return temperature_.GetEnduranceInSeconds(); }
    MonthType GetTemperatureMonth() const { return temperature_.GetMonth(); }
    DayType GetTemperatureDay() const { return temperature_.GetDay(); }
    DayType GetTemperatureValidInDays() const { return temperature_.GetValidInDays(); }
    byte GetTemperatureSpeed() const { return temperature_.GetSpeed(); }
    Text::FontType GetTemperatureFont() const { return temperature_.GetFont(); }
    /* What the overlay would put on the display, which is the only way to read the
       temperature back out of the firmware until there is a status command. */
    const char* GetTemperatureString() const { return temperature_.GetTemperatureString(); }
    bool GetTemperatureIsActive() const { return temperature_.GetIsActive(); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    MinuteType GetTextPeriodInMinutes() const { return text_.GetPeriodInMinutes(); }
    SecondType GetTextEnduranceInSeconds() const { return text_.GetEnduranceInSeconds(); }
    MonthType GetTextMonth() const { return text_.GetMonth(); }
    DayType GetTextDay() const { return text_.GetDay(); }
    DayType GetTextValidInDays() const { return text_.GetValidInDays(); }
    byte GetTextSpeed() const { return text_.GetSpeed(); }
    Text::FontType GetTextFont() const { return text_.GetFont(); }
    bool GetTextIsActive() const { return text_.GetIsActive(); }
    const char* GetTextText() const { return text_.GetText(); }
#endif

	// set methods
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    StdReturnType SetDatePeriodInMinutes(MinuteType period_in_minutes) { return date_.SetPeriodInMinutes(period_in_minutes); }
    StdReturnType SetDateEnduranceInSeconds(SecondType endurance_in_seconds) { return date_.SetEnduranceInSeconds(endurance_in_seconds); }
    void SetDateMonth(MonthType month) { date_.SetMonth(month); }
    void SetDateDay(DayType day) { date_.SetDay(day); }
    void SetDateValidInDays(DayType valid_in_days) { date_.SetValidInDays(valid_in_days); }
    void SetDateSpeed(byte speed) { date_.SetSpeed(speed); }
    StdReturnType SetDateFont(FontType font) { return date_.SetFont(font); }
    void SetDateIsActive(bool is_active) { date_.SetIsActive(is_active); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    StdReturnType SetTemperaturePeriodInMinutes(MinuteType period_in_minutes) { return temperature_.SetPeriodInMinutes(period_in_minutes); }
    StdReturnType SetTemperatureEnduranceInSeconds(SecondType endurance_in_seconds) { return temperature_.SetEnduranceInSeconds(endurance_in_seconds); }
    void SetTemperatureMonth(MonthType month) { temperature_.SetMonth(month); }
    void SetTemperatureDay(DayType day) { temperature_.SetDay(day); }
    void SetTemperatureValidInDays(DayType valid_in_days) { temperature_.SetValidInDays(valid_in_days); }
    void SetTemperatureSpeed(byte speed) { temperature_.SetSpeed(speed); }
    StdReturnType SetTemperatureFont(FontType font) { return temperature_.SetFont(font); }
    void SetTemperatureIsActive(bool is_active) { temperature_.SetIsActive(is_active); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    StdReturnType SetTextPeriodInMinutes(MinuteType period_in_minutes) { return text_.SetPeriodInMinutes(period_in_minutes); }
    StdReturnType SetTextEnduranceInSeconds(SecondType endurance_in_seconds) { return text_.SetEnduranceInSeconds(endurance_in_seconds); }
    void SetTextMonth(MonthType month) { text_.SetMonth(month); }
    void SetTextDay(DayType day) { text_.SetDay(day); }
    void SetTextValidInDays(DayType valid_in_days) { text_.SetValidInDays(valid_in_days); }
    void SetTextIsActive(bool is_active) { text_.SetIsActive(is_active); }
    void SetTextSpeed(byte speed) { text_.SetSpeed(speed); }
    StdReturnType SetTextFont(FontType font) { return text_.SetFont(font); }
    void SetTextText(const char* text, LengthType length) { text_.SetText(text, length); }
#endif

	// methods
    /* Every overlay back to what it starts with, the same way Display, Clock, Animations
       and Illuminance answer Persistence::Reset(). Here since the overlays are stored -
       before that a reset had nothing of theirs to undo. */
    void ResetToDefaults() {
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        date_.ResetToDefaults();
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        temperature_.ResetToDefaults();
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        text_.ResetToDefaults();
#endif
    }

	void Task();

    /* Show an overlay now instead of at its next period, and end the one that is
       showing. Without these an overlay can only be watched by waiting for its raster,
       which is also what makes one hard to develop against. */
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    StdReturnType ShowDateNow() { return ShowNow(date_); }
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    StdReturnType ShowTemperatureNow() { return ShowNow(temperature_); }
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    StdReturnType ShowTextNow() { return ShowNow(text_); }
#endif
    StdReturnType Abort();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
