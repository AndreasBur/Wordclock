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
/**     \file       Persistence.cpp
 *      \brief      Keeps the configuration across a restart
 *
 *      \details    Holds the stored format: which settings it covers, how they are laid
 *                  out, and how a blob that does not belong to this build is recognised.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Persistence.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "Illuminance.h"
#include "ColorCycle.h"
#include "NightSwitch.h"
#include "Overlays.h"
#include "Storage.h"

#include <string.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
namespace {

/* What one overlay carries, which is the same set for all three - the base class owns it,
   and only the text overlay adds anything on top. Every member is a byte, so this nests
   into the struct below without disturbing its alignment. */
struct OverlaySettingsType {
    byte period_in_minutes;
    byte endurance_in_seconds;
    byte month;
    byte day;
    byte valid_in_days;
    byte speed;
    byte font;
    byte is_active;
};

/* All three slots exist whether or not the build compiles the overlays in. Overlays'
   own OverlayIdType is conditional and would have been the obvious index, which is
   exactly why it is not used here: a layout that changed with OVERLAYS_SUPPORT_* would
   let two builds write mutually unreadable blobs under the same version number, and the
   version is the only thing that says which layout a blob has. */
constexpr byte kNumberOfOverlaySlots{3u};
constexpr byte kOverlaySlotDate{0u};
constexpr byte kOverlaySlotTemperature{1u};
constexpr byte kOverlaySlotText{2u};

/* Every two-byte field comes first, so the struct needs no padding between its members
   and its layout follows from the declaration rather than from the compiler's alignment
   rules. What padding remains at the end is covered by zero-initialising every instance,
   which the checksum and the comparison both depend on. */
struct SettingsType {
    uint16_t magic;
    uint16_t animation_favourites;
    uint16_t illuminance_calibration_max;
    uint16_t illuminance_calibration_min;

    byte version;
    byte colour_red;
    byte colour_green;
    byte colour_blue;
    byte brightness;
    /* byte rather than bool: what a bool occupies in a stored blob is not worth relying
       on, and the conversion is one comparison. */
    byte brightness_use_automatic;
    byte brightness_use_gamma_correction;
    byte clock_mode;
    byte clock_show_it_is_permanently;
    byte animations_mode;
    byte animation_id;
    byte animation_task_cycles[Animations::kAnimationIdNumberOfAnimations];
    OverlaySettingsType overlays[kNumberOfOverlaySlots];
    /* The text overlay's own, and the only variable-length thing here - stored at its full
       buffer size rather than length-prefixed, so the struct stays one fixed block that
       memcmp and the checksum walk without knowing anything about its contents. */
    char overlay_text[OVERLAY_TEXT_TEXT_SIZE];

    byte night_switch_is_active;
    byte night_switch_start_hour;
    byte night_switch_start_minute;
    byte night_switch_end_hour;
    byte night_switch_end_minute;
    byte night_switch_brightness;

    /* The cycle's two settings, and deliberately not its hue: this blob is written whenever
       it differs from the live settings, so a stored hue would be one flash write per step
       of the wheel - a few hundred thousand a day on a part rated for a hundred thousand
       cycles. Where the wheel had got to matters to nobody after a restart. */
    byte color_cycle_is_active;
    byte color_cycle_task_cycle;

    byte checksum;
};

/* 'W' and 'C'. Together with the version it tells a blob of this format from whatever
   else a store might hand back. */
constexpr uint16_t kSettingsMagic{0x5743u};
/* Raised whenever the struct above changes, which is what makes a blob written by another
   build recognisable rather than read as this one. A raised version throws the stored
   settings away on the next start - there is no converting an old blob into a new one
   here, and a clock that comes up on its defaults says so more clearly than one that comes
   up with a field read out of the wrong offset.
     1  the first format
     2  the "it is" rule, which used to be a compile-time switch
     3  the three overlays and the text overlay's text
     4  the night switch
     5  the colour cycle */
constexpr byte kSettingsVersion{5u};

static_assert(sizeof(SettingsType) <= Storage::kCapacity,
              "Persistence: the settings no longer fit the store, please raise STORAGE_CAPACITY on every platform");

/* What was last written, so Task() can tell a changed setting from an unchanged one
   without anything having to report the change. Kept here rather than as a member so the
   format stays out of the header. */
SettingsType last_saved{};

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* Sum of every byte but the checksum's own. It is there to catch a truncated or foreign
   blob, not to detect tampering, and a sum does that for one byte of overhead.

   Every other byte is walked, rather than stopping at the checksum's offset: whether the
   struct ends in padding depends on how many animations there are, and a byte the sum did
   not cover would be a byte a damaged blob could hide in. Zero-initialising every instance
   is what makes the padding a defined value to sum over. */
byte CalcChecksum(const SettingsType& settings)
{
    const byte* bytes = reinterpret_cast<const byte*>(&settings);
    byte checksum{0u};

    for(size_t index = 0u; index < sizeof(SettingsType); index++) {
        if(index == offsetof(SettingsType, checksum)) { continue; }

        checksum = static_cast<byte>(checksum + bytes[index]);
    }
    return checksum;
}

bool IsValid(const SettingsType& settings)
{
    if(settings.magic != kSettingsMagic) { return false; }
    if(settings.version != kSettingsVersion) { return false; }

    return settings.checksum == CalcChecksum(settings);
}

/* One block per overlay rather than a loop: Overlays names its three flat, and the private
   members a loop would need are not reachable from here. The blocks are guarded the same
   way the accessors are, so a slot whose overlay is switched off stays at the zero the
   struct was initialised with and reads back as a switched-off overlay. */
void GatherOverlays(SettingsType& settings)
{
    const Overlays& overlays = Overlays::GetInstance();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    OverlaySettingsType& date = settings.overlays[kOverlaySlotDate];
    date.period_in_minutes = overlays.GetDatePeriodInMinutes();
    date.endurance_in_seconds = overlays.GetDateEnduranceInSeconds();
    date.month = overlays.GetDateMonth();
    date.day = overlays.GetDateDay();
    date.valid_in_days = overlays.GetDateValidInDays();
    date.speed = overlays.GetDateSpeed();
    date.font = static_cast<byte>(overlays.GetDateFont());
    date.is_active = overlays.GetDateIsActive() ? 1u : 0u;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    OverlaySettingsType& temperature = settings.overlays[kOverlaySlotTemperature];
    temperature.period_in_minutes = overlays.GetTemperaturePeriodInMinutes();
    temperature.endurance_in_seconds = overlays.GetTemperatureEnduranceInSeconds();
    temperature.month = overlays.GetTemperatureMonth();
    temperature.day = overlays.GetTemperatureDay();
    temperature.valid_in_days = overlays.GetTemperatureValidInDays();
    temperature.speed = overlays.GetTemperatureSpeed();
    temperature.font = static_cast<byte>(overlays.GetTemperatureFont());
    temperature.is_active = overlays.GetTemperatureIsActive() ? 1u : 0u;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    OverlaySettingsType& text = settings.overlays[kOverlaySlotText];
    text.period_in_minutes = overlays.GetTextPeriodInMinutes();
    text.endurance_in_seconds = overlays.GetTextEnduranceInSeconds();
    text.month = overlays.GetTextMonth();
    text.day = overlays.GetTextDay();
    text.valid_in_days = overlays.GetTextValidInDays();
    text.speed = overlays.GetTextSpeed();
    text.font = static_cast<byte>(overlays.GetTextFont());
    text.is_active = overlays.GetTextIsActive() ? 1u : 0u;

    StringTools::StringCopy(settings.overlay_text, overlays.GetTextText(), OVERLAY_TEXT_TEXT_SIZE);
#endif
}

void ApplyOverlays(const SettingsType& settings)
{
    Overlays& overlays = Overlays::GetInstance();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    const OverlaySettingsType& date = settings.overlays[kOverlaySlotDate];
    overlays.SetDateMonth(date.month);
    overlays.SetDateDay(date.day);
    overlays.SetDateValidInDays(date.valid_in_days);
    overlays.SetDateSpeed(date.speed);
    /* The three that validate what they are given are left to refuse a value the store
       should never have held - the setting then stays at its default, which is the same
       answer a blob from another build gets. */
    (void)overlays.SetDatePeriodInMinutes(date.period_in_minutes);
    (void)overlays.SetDateEnduranceInSeconds(date.endurance_in_seconds);
    (void)overlays.SetDateFont(static_cast<Text::FontType>(date.font));
    overlays.SetDateIsActive(date.is_active != 0u);
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    const OverlaySettingsType& temperature = settings.overlays[kOverlaySlotTemperature];
    overlays.SetTemperatureMonth(temperature.month);
    overlays.SetTemperatureDay(temperature.day);
    overlays.SetTemperatureValidInDays(temperature.valid_in_days);
    overlays.SetTemperatureSpeed(temperature.speed);
    (void)overlays.SetTemperaturePeriodInMinutes(temperature.period_in_minutes);
    (void)overlays.SetTemperatureEnduranceInSeconds(temperature.endurance_in_seconds);
    (void)overlays.SetTemperatureFont(static_cast<Text::FontType>(temperature.font));
    overlays.SetTemperatureIsActive(temperature.is_active != 0u);
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    const OverlaySettingsType& text = settings.overlays[kOverlaySlotText];
    overlays.SetTextMonth(text.month);
    overlays.SetTextDay(text.day);
    overlays.SetTextValidInDays(text.valid_in_days);
    overlays.SetTextSpeed(text.speed);
    (void)overlays.SetTextPeriodInMinutes(text.period_in_minutes);
    (void)overlays.SetTextEnduranceInSeconds(text.endurance_in_seconds);
    (void)overlays.SetTextFont(static_cast<Text::FontType>(text.font));

    /* The text before the switch, so an overlay that comes back active comes back with
       what it is meant to show rather than with the previous run's text for one period. */
    overlays.SetTextText(settings.overlay_text, OVERLAY_TEXT_TEXT_SIZE);
    overlays.SetTextIsActive(text.is_active != 0u);
#endif
}

/* Read from the modules that own the settings, never from a copy kept here - a second
   copy is what would need keeping in step. */
SettingsType Gather()
{
    SettingsType settings{};

    settings.magic = kSettingsMagic;
    settings.version = kSettingsVersion;

    const Display& display = Display::GetInstance();
    settings.colour_red = display.GetColorRed();
    settings.colour_green = display.GetColorGreen();
    settings.colour_blue = display.GetColorBlue();
    settings.brightness = display.GetBrightness();
    settings.brightness_use_automatic = display.GetBrightnessUseAutomatic() ? 1u : 0u;
    settings.brightness_use_gamma_correction = display.GetBrightnessUseGammaCorrection() ? 1u : 0u;

    settings.clock_mode = static_cast<byte>(Clock::GetInstance().GetMode());
    settings.clock_show_it_is_permanently = Clock::GetInstance().GetShowItIsPermanently() ? 1u : 0u;

    const Animations& animations = Animations::GetInstance();
    settings.animations_mode = static_cast<byte>(animations.GetMode());
    /* The selected animation, not the running one: a selecting mode changes what runs on
       every word change, and storing that would mean a write every few minutes. */
    settings.animation_id = static_cast<byte>(animations.GetAnimation());

    for(byte index = 0u; index < Animations::kAnimationIdNumberOfAnimations; index++) {
        const Animations::AnimationIdType animation_id = static_cast<Animations::AnimationIdType>(index);

        settings.animation_task_cycles[index] = animations.GetTaskCycle(animation_id);
        if(animations.IsFavourite(animation_id)) {
            settings.animation_favourites = static_cast<uint16_t>(settings.animation_favourites | (1u << index));
        }
    }

    const Illuminance& illuminance = Illuminance::GetInstance();
    settings.illuminance_calibration_max = illuminance.GetCalibrationValuesMaxValue();
    settings.illuminance_calibration_min = illuminance.GetCalibrationValuesMinValue();

    const NightSwitch& night_switch = NightSwitch::GetInstance();
    settings.night_switch_is_active = night_switch.GetIsActive() ? 1u : 0u;
    settings.night_switch_start_hour = night_switch.GetStartHour();
    settings.night_switch_start_minute = night_switch.GetStartMinute();
    settings.night_switch_end_hour = night_switch.GetEndHour();
    settings.night_switch_end_minute = night_switch.GetEndMinute();
    settings.night_switch_brightness = night_switch.GetNightBrightness();

    const ColorCycle& color_cycle = ColorCycle::GetInstance();
    settings.color_cycle_is_active = color_cycle.GetIsActive() ? 1u : 0u;
    settings.color_cycle_task_cycle = color_cycle.GetTaskCycle();

    GatherOverlays(settings);

    settings.checksum = CalcChecksum(settings);
    return settings;
}

void Apply(const SettingsType& settings)
{
    Display& display = Display::GetInstance();
    display.SetColor(settings.colour_red, settings.colour_green, settings.colour_blue);
    display.SetBrightnessUseAutomatic(settings.brightness_use_automatic != 0u);
    display.SetBrightnessUseGammaCorrection(settings.brightness_use_gamma_correction != 0u);
    /* After the two switches, because it is this call that recalculates what reaches the
       LEDs, and it has to do so with both of them already in place. */
    display.SetBrightness(settings.brightness);

    Clock::GetInstance().SetMode(static_cast<Clock::ModeType>(settings.clock_mode));
    Clock::GetInstance().SetShowItIsPermanently(settings.clock_show_it_is_permanently != 0u);

    Animations& animations = Animations::GetInstance();

    for(byte index = 0u; index < Animations::kAnimationIdNumberOfAnimations; index++) {
        const Animations::AnimationIdType animation_id = static_cast<Animations::AnimationIdType>(index);
        const bool is_favourite = (settings.animation_favourites & static_cast<uint16_t>(1u << index)) != 0u;

        animations.SetFavourite(animation_id, is_favourite);
        animations.SetTaskCycle(animation_id, settings.animation_task_cycles[index]);
    }

    /* The animation before the mode: setting the mode to kModeFixed starts the selected
       animation, so it has to find the restored one rather than the default. */
    animations.SetAnimation(static_cast<Animations::AnimationIdType>(settings.animation_id));
    animations.SetMode(static_cast<Animations::ModeType>(settings.animations_mode));

    Illuminance& illuminance = Illuminance::GetInstance();
    illuminance.SetCalibrationValuesMaxValue(settings.illuminance_calibration_max);
    illuminance.SetCalibrationValuesMinValue(settings.illuminance_calibration_min);

    NightSwitch& night_switch = NightSwitch::GetInstance();
    night_switch.SetStartHour(settings.night_switch_start_hour);
    night_switch.SetStartMinute(settings.night_switch_start_minute);
    night_switch.SetEndHour(settings.night_switch_end_hour);
    night_switch.SetEndMinute(settings.night_switch_end_minute);
    night_switch.SetNightBrightness(settings.night_switch_brightness);
    /* Last of the five, because switching it on is what makes the next tick an edge - and
       an edge acting on a half-restored window would use yesterday's times. */
    night_switch.SetIsActive(settings.night_switch_is_active != 0u);

    ColorCycle& color_cycle = ColorCycle::GetInstance();
    color_cycle.SetTaskCycle(settings.color_cycle_task_cycle);
    /* After the speed, for the night switch's reason read across: switching it on puts a
       colour on the strip there and then, and doing that before the speed has landed would
       start the wheel at whatever rate the defaults left behind. */
    color_cycle.SetIsActive(settings.color_cycle_is_active != 0u);

    ApplyOverlays(settings);
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Load()
******************************************************************************************************************************************************/
/*! \brief          Restores the stored configuration, if there is a usable one
 *  \details        A blob that cannot be used is thrown away rather than left in place:
 *                  it would otherwise be read and rejected on every start, and the store
 *                  has nothing to offer that a fresh write does not. Nothing stored at all
 *                  takes the same path, where clearing is a no-op.
 *
 *                  What is remembered as "last written" afterwards is what the modules
 *                  actually hold, not what the blob said. A value one of them rejected
 *                  therefore shows up as a difference on the next task and gets corrected
 *                  in the store, instead of being read and rejected forever.
 *
 *  \return         kEOk if a stored configuration was applied
******************************************************************************************************************************************************/
StdReturnType Persistence::Load()
{
    SettingsType settings{};
    StdReturnType return_value{kENotOk};

    if(Storage::GetInstance().Read(reinterpret_cast<byte*>(&settings), sizeof(settings)) == kEOk) {
        if(IsValid(settings)) {
            Apply(settings);
            return_value = kEOk;
        }
    }

    if(return_value == kENotOk) { Storage::GetInstance().Clear(); }

    last_saved = Gather();
    return return_value;
} /* Load */


/******************************************************************************************************************************************************
  Task()
******************************************************************************************************************************************************/
/*! \brief          Writes the configuration if it changed since the last write
 *  \details        A failed write leaves last_saved alone, so the next task tries again
 *                  rather than treating the change as stored.
******************************************************************************************************************************************************/
void Persistence::Task()
{
    const SettingsType current = Gather();

    /* The padding is what makes this defined, and both sides are zero-initialised for
       exactly that reason - see the struct's own comment. Comparing member by member is
       what the check asks for, and is the copy that would fall behind the next setting
       somebody adds. */
    /* All three names, because which of them a finding is reported under depends on the
       clang-tidy version: 21 leads with the bugprone one and lists the CERT rules beside
       it, 18 reports only the CERT rules - and a suppression that names the wrong one
       silences nothing. */
    // NOLINTNEXTLINE(bugprone-suspicious-memory-comparison,cert-exp42-c,cert-flp37-c)
    if(memcmp(&current, &last_saved, sizeof(current)) == 0) { return; }

    if(Storage::GetInstance().Write(reinterpret_cast<const byte*>(&current), sizeof(current)) == kEOk) {
        last_saved = current;
    }
} /* Task */

/******************************************************************************************************************************************************
  Save()
******************************************************************************************************************************************************/
/*! \brief          Writes the configuration now
 *  \details        Unconditionally, unlike Task(): a caller that asks for this is about to
 *                  cut the power, and "nothing changed since the last write" is a state it
 *                  cannot check for itself.
 *
 *  \return         kEOk if the store took the configuration
******************************************************************************************************************************************************/
StdReturnType Persistence::Save()
{
    const SettingsType current = Gather();

    if(Storage::GetInstance().Write(reinterpret_cast<const byte*>(&current), sizeof(current)) == kENotOk) { return kENotOk; }

    last_saved = current;
    return kEOk;
} /* Save */


/******************************************************************************************************************************************************
  Reset()
******************************************************************************************************************************************************/
/*! \brief          Puts the configuration back to what a clock starts with
 *  \details        Each module answers for its own defaults, the same way Gather() reads
 *                  from each of them rather than from a copy kept here - so a setting
 *                  added to one of them is reset by the module that owns it and not
 *                  forgotten in a list over here.
 *
 *                  What is remembered as "last written" afterwards is what the modules now
 *                  hold, so the next task does not write the defaults straight back into
 *                  the store that was just emptied.
 *
 *  \return         kEOk if the store was emptied
******************************************************************************************************************************************************/
StdReturnType Persistence::Reset()
{
    Display::GetInstance().ResetToDefaults();
    Clock::GetInstance().ResetToDefaults();
    Animations::GetInstance().ResetToDefaults();
    Illuminance::GetInstance().ResetToDefaults();
    Overlays::GetInstance().ResetToDefaults();
    NightSwitch::GetInstance().ResetToDefaults();
    ColorCycle::GetInstance().ResetToDefaults();

    const StdReturnType return_value = Storage::GetInstance().Clear();

    last_saved = Gather();
    return return_value;
} /* Reset */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
