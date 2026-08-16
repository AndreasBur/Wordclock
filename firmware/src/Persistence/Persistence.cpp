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
    byte PeriodInMinutes;
    byte EnduranceInSeconds;
    byte Month;
    byte Day;
    byte ValidInDays;
    byte Speed;
    byte Font;
    byte IsActive;
};

/* All three slots exist whether or not the build compiles the overlays in. Overlays'
   own OverlayIdType is conditional and would have been the obvious index, which is
   exactly why it is not used here: a layout that changed with OVERLAYS_SUPPORT_* would
   let two builds write mutually unreadable blobs under the same version number, and the
   version is the only thing that says which layout a blob has. */
constexpr byte NumberOfOverlaySlots{3u};
constexpr byte OverlaySlotDate{0u};
constexpr byte OverlaySlotTemperature{1u};
constexpr byte OverlaySlotText{2u};

/* Every two-byte field comes first, so the struct needs no padding between its members
   and its layout follows from the declaration rather than from the compiler's alignment
   rules. What padding remains at the end is covered by zero-initialising every instance,
   which the checksum and the comparison both depend on. */
struct SettingsType {
    uint16_t Magic;
    uint16_t AnimationFavourites;
    uint16_t IlluminanceCalibrationMax;
    uint16_t IlluminanceCalibrationMin;

    byte Version;
    byte ColourRed;
    byte ColourGreen;
    byte ColourBlue;
    byte Brightness;
    /* byte rather than bool: what a bool occupies in a stored blob is not worth relying
       on, and the conversion is one comparison. */
    byte BrightnessUseAutomatic;
    byte BrightnessUseGammaCorrection;
    byte ClockMode;
    byte ClockShowItIsPermanently;
    byte AnimationsMode;
    byte AnimationId;
    byte AnimationTaskCycles[Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS];
    OverlaySettingsType Overlays[NumberOfOverlaySlots];
    /* The text overlay's own, and the only variable-length thing here - stored at its full
       buffer size rather than length-prefixed, so the struct stays one fixed block that
       memcmp and the checksum walk without knowing anything about its contents. */
    char OverlayText[OVERLAY_TEXT_TEXT_SIZE];

    byte NightSwitchIsActive;
    byte NightSwitchStartHour;
    byte NightSwitchStartMinute;
    byte NightSwitchEndHour;
    byte NightSwitchEndMinute;
    byte NightSwitchBrightness;

    byte Checksum;
};

/* 'W' and 'C'. Together with the version it tells a blob of this format from whatever
   else a store might hand back. */
constexpr uint16_t SettingsMagic{0x5743u};
/* Raised whenever the struct above changes, which is what makes a blob written by another
   build recognisable rather than read as this one. A raised version throws the stored
   settings away on the next start - there is no converting an old blob into a new one
   here, and a clock that comes up on its defaults says so more clearly than one that comes
   up with a field read out of the wrong offset.
     1  the first format
     2  the "it is" rule, which used to be a compile-time switch
     3  the three overlays and the text overlay's text
     4  the night switch */
constexpr byte SettingsVersion{4u};

static_assert(sizeof(SettingsType) <= Storage::Capacity,
              "Persistence: the settings no longer fit the store, please raise STORAGE_CAPACITY on every platform");

/* What was last written, so task() can tell a changed setting from an unchanged one
   without anything having to report the change. Kept here rather than as a member so the
   format stays out of the header. */
SettingsType LastSaved{};

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* Sum of every byte but the checksum's own. It is there to catch a truncated or foreign
   blob, not to detect tampering, and a sum does that for one byte of overhead.

   Every other byte is walked, rather than stopping at the checksum's offset: whether the
   struct ends in padding depends on how many animations there are, and a byte the sum did
   not cover would be a byte a damaged blob could hide in. Zero-initialising every instance
   is what makes the padding a defined value to sum over. */
byte calcChecksum(const SettingsType& Settings)
{
    const byte* Bytes = reinterpret_cast<const byte*>(&Settings);
    byte Checksum{0u};

    for(size_t Index = 0u; Index < sizeof(SettingsType); Index++) {
        if(Index == offsetof(SettingsType, Checksum)) { continue; }

        Checksum = static_cast<byte>(Checksum + Bytes[Index]);
    }
    return Checksum;
}

bool isValid(const SettingsType& Settings)
{
    if(Settings.Magic != SettingsMagic) { return false; }
    if(Settings.Version != SettingsVersion) { return false; }

    return Settings.Checksum == calcChecksum(Settings);
}

/* One block per overlay rather than a loop: Overlays names its three flat, and the private
   members a loop would need are not reachable from here. The blocks are guarded the same
   way the accessors are, so a slot whose overlay is switched off stays at the zero the
   struct was initialised with and reads back as a switched-off overlay. */
void gatherOverlays(SettingsType& Settings)
{
    const Overlays& overlays = Overlays::getInstance();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    OverlaySettingsType& Date = Settings.Overlays[OverlaySlotDate];
    Date.PeriodInMinutes = overlays.getDatePeriodInMinutes();
    Date.EnduranceInSeconds = overlays.getDateEnduranceInSeconds();
    Date.Month = overlays.getDateMonth();
    Date.Day = overlays.getDateDay();
    Date.ValidInDays = overlays.getDateValidInDays();
    Date.Speed = overlays.getDateSpeed();
    Date.Font = static_cast<byte>(overlays.getDateFont());
    Date.IsActive = overlays.getDateIsActive() ? 1u : 0u;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    OverlaySettingsType& Temperature = Settings.Overlays[OverlaySlotTemperature];
    Temperature.PeriodInMinutes = overlays.getTemperaturePeriodInMinutes();
    Temperature.EnduranceInSeconds = overlays.getTemperatureEnduranceInSeconds();
    Temperature.Month = overlays.getTemperatureMonth();
    Temperature.Day = overlays.getTemperatureDay();
    Temperature.ValidInDays = overlays.getTemperatureValidInDays();
    Temperature.Speed = overlays.getTemperatureSpeed();
    Temperature.Font = static_cast<byte>(overlays.getTemperatureFont());
    Temperature.IsActive = overlays.getTemperatureIsActive() ? 1u : 0u;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    OverlaySettingsType& Text = Settings.Overlays[OverlaySlotText];
    Text.PeriodInMinutes = overlays.getTextPeriodInMinutes();
    Text.EnduranceInSeconds = overlays.getTextEnduranceInSeconds();
    Text.Month = overlays.getTextMonth();
    Text.Day = overlays.getTextDay();
    Text.ValidInDays = overlays.getTextValidInDays();
    Text.Speed = overlays.getTextSpeed();
    Text.Font = static_cast<byte>(overlays.getTextFont());
    Text.IsActive = overlays.getTextIsActive() ? 1u : 0u;

    StringTools::stringCopy(Settings.OverlayText, overlays.getTextText(), OVERLAY_TEXT_TEXT_SIZE);
#endif
}

void applyOverlays(const SettingsType& Settings)
{
    Overlays& overlays = Overlays::getInstance();

#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    const OverlaySettingsType& Date = Settings.Overlays[OverlaySlotDate];
    overlays.setDateMonth(Date.Month);
    overlays.setDateDay(Date.Day);
    overlays.setDateValidInDays(Date.ValidInDays);
    overlays.setDateSpeed(Date.Speed);
    /* The three that validate what they are given are left to refuse a value the store
       should never have held - the setting then stays at its default, which is the same
       answer a blob from another build gets. */
    (void)overlays.setDatePeriodInMinutes(Date.PeriodInMinutes);
    (void)overlays.setDateEnduranceInSeconds(Date.EnduranceInSeconds);
    (void)overlays.setDateFont(static_cast<Text::FontType>(Date.Font));
    overlays.setDateIsActive(Date.IsActive != 0u);
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    const OverlaySettingsType& Temperature = Settings.Overlays[OverlaySlotTemperature];
    overlays.setTemperatureMonth(Temperature.Month);
    overlays.setTemperatureDay(Temperature.Day);
    overlays.setTemperatureValidInDays(Temperature.ValidInDays);
    overlays.setTemperatureSpeed(Temperature.Speed);
    (void)overlays.setTemperaturePeriodInMinutes(Temperature.PeriodInMinutes);
    (void)overlays.setTemperatureEnduranceInSeconds(Temperature.EnduranceInSeconds);
    (void)overlays.setTemperatureFont(static_cast<Text::FontType>(Temperature.Font));
    overlays.setTemperatureIsActive(Temperature.IsActive != 0u);
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    const OverlaySettingsType& Text = Settings.Overlays[OverlaySlotText];
    overlays.setTextMonth(Text.Month);
    overlays.setTextDay(Text.Day);
    overlays.setTextValidInDays(Text.ValidInDays);
    overlays.setTextSpeed(Text.Speed);
    (void)overlays.setTextPeriodInMinutes(Text.PeriodInMinutes);
    (void)overlays.setTextEnduranceInSeconds(Text.EnduranceInSeconds);
    (void)overlays.setTextFont(static_cast<Text::FontType>(Text.Font));

    /* The text before the switch, so an overlay that comes back active comes back with
       what it is meant to show rather than with the previous run's text for one period. */
    overlays.setTextText(Settings.OverlayText, OVERLAY_TEXT_TEXT_SIZE);
    overlays.setTextIsActive(Text.IsActive != 0u);
#endif
}

/* Read from the modules that own the settings, never from a copy kept here - a second
   copy is what would need keeping in step. */
SettingsType gather()
{
    SettingsType Settings{};

    Settings.Magic = SettingsMagic;
    Settings.Version = SettingsVersion;

    const Display& display = Display::getInstance();
    Settings.ColourRed = display.getColorRed();
    Settings.ColourGreen = display.getColorGreen();
    Settings.ColourBlue = display.getColorBlue();
    Settings.Brightness = display.getBrightness();
    Settings.BrightnessUseAutomatic = display.getBrightnessUseAutomatic() ? 1u : 0u;
    Settings.BrightnessUseGammaCorrection = display.getBrightnessUseGammaCorrection() ? 1u : 0u;

    Settings.ClockMode = static_cast<byte>(Clock::getInstance().getMode());
    Settings.ClockShowItIsPermanently = Clock::getInstance().getShowItIsPermanently() ? 1u : 0u;

    const Animations& animations = Animations::getInstance();
    Settings.AnimationsMode = static_cast<byte>(animations.getMode());
    /* The selected animation, not the running one: a selecting mode changes what runs on
       every word change, and storing that would mean a write every few minutes. */
    Settings.AnimationId = static_cast<byte>(animations.getAnimation());

    for(byte Index = 0u; Index < Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS; Index++) {
        const Animations::AnimationIdType AnimationId = static_cast<Animations::AnimationIdType>(Index);

        Settings.AnimationTaskCycles[Index] = animations.getTaskCycle(AnimationId);
        if(animations.isFavourite(AnimationId)) {
            Settings.AnimationFavourites = static_cast<uint16_t>(Settings.AnimationFavourites | (1u << Index));
        }
    }

    const Illuminance& illuminance = Illuminance::getInstance();
    Settings.IlluminanceCalibrationMax = illuminance.getCalibrationValuesMaxValue();
    Settings.IlluminanceCalibrationMin = illuminance.getCalibrationValuesMinValue();

    const NightSwitch& nightSwitch = NightSwitch::getInstance();
    Settings.NightSwitchIsActive = nightSwitch.getIsActive() ? 1u : 0u;
    Settings.NightSwitchStartHour = nightSwitch.getStartHour();
    Settings.NightSwitchStartMinute = nightSwitch.getStartMinute();
    Settings.NightSwitchEndHour = nightSwitch.getEndHour();
    Settings.NightSwitchEndMinute = nightSwitch.getEndMinute();
    Settings.NightSwitchBrightness = nightSwitch.getNightBrightness();

    gatherOverlays(Settings);

    Settings.Checksum = calcChecksum(Settings);
    return Settings;
}

void apply(const SettingsType& Settings)
{
    Display& display = Display::getInstance();
    display.setColor(Settings.ColourRed, Settings.ColourGreen, Settings.ColourBlue);
    display.setBrightnessUseAutomatic(Settings.BrightnessUseAutomatic != 0u);
    display.setBrightnessUseGammaCorrection(Settings.BrightnessUseGammaCorrection != 0u);
    /* After the two switches, because it is this call that recalculates what reaches the
       LEDs, and it has to do so with both of them already in place. */
    display.setBrightness(Settings.Brightness);

    Clock::getInstance().setMode(static_cast<Clock::ModeType>(Settings.ClockMode));
    Clock::getInstance().setShowItIsPermanently(Settings.ClockShowItIsPermanently != 0u);

    Animations& animations = Animations::getInstance();

    for(byte Index = 0u; Index < Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS; Index++) {
        const Animations::AnimationIdType AnimationId = static_cast<Animations::AnimationIdType>(Index);
        const bool IsFavourite = (Settings.AnimationFavourites & static_cast<uint16_t>(1u << Index)) != 0u;

        animations.setFavourite(AnimationId, IsFavourite);
        animations.setTaskCycle(AnimationId, Settings.AnimationTaskCycles[Index]);
    }

    /* The animation before the mode: setting the mode to MODE_FIXED starts the selected
       animation, so it has to find the restored one rather than the default. */
    animations.setAnimation(static_cast<Animations::AnimationIdType>(Settings.AnimationId));
    animations.setMode(static_cast<Animations::ModeType>(Settings.AnimationsMode));

    Illuminance& illuminance = Illuminance::getInstance();
    illuminance.setCalibrationValuesMaxValue(Settings.IlluminanceCalibrationMax);
    illuminance.setCalibrationValuesMinValue(Settings.IlluminanceCalibrationMin);

    NightSwitch& nightSwitch = NightSwitch::getInstance();
    nightSwitch.setStartHour(Settings.NightSwitchStartHour);
    nightSwitch.setStartMinute(Settings.NightSwitchStartMinute);
    nightSwitch.setEndHour(Settings.NightSwitchEndHour);
    nightSwitch.setEndMinute(Settings.NightSwitchEndMinute);
    nightSwitch.setNightBrightness(Settings.NightSwitchBrightness);
    /* Last of the five, because switching it on is what makes the next tick an edge - and
       an edge acting on a half-restored window would use yesterday's times. */
    nightSwitch.setIsActive(Settings.NightSwitchIsActive != 0u);

    applyOverlays(Settings);
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  load()
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
 *  \return         E_OK if a stored configuration was applied
******************************************************************************************************************************************************/
StdReturnType Persistence::load()
{
    SettingsType Settings{};
    StdReturnType ReturnValue{E_NOT_OK};

    if(Storage::getInstance().read(reinterpret_cast<byte*>(&Settings), sizeof(Settings)) == E_OK) {
        if(isValid(Settings)) {
            apply(Settings);
            ReturnValue = E_OK;
        }
    }

    if(ReturnValue == E_NOT_OK) { Storage::getInstance().clear(); }

    LastSaved = gather();
    return ReturnValue;
} /* load */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
/*! \brief          Writes the configuration if it changed since the last write
 *  \details        A failed write leaves LastSaved alone, so the next task tries again
 *                  rather than treating the change as stored.
******************************************************************************************************************************************************/
void Persistence::task()
{
    const SettingsType Current = gather();

    /* The padding is what makes this defined, and both sides are zero-initialised for
       exactly that reason - see the struct's own comment. Comparing member by member is
       what the check asks for, and is the copy that would fall behind the next setting
       somebody adds. */
    /* All three names, because which of them a finding is reported under depends on the
       clang-tidy version: 21 leads with the bugprone one and lists the CERT rules beside
       it, 18 reports only the CERT rules - and a suppression that names the wrong one
       silences nothing. */
    // NOLINTNEXTLINE(bugprone-suspicious-memory-comparison,cert-exp42-c,cert-flp37-c)
    if(memcmp(&Current, &LastSaved, sizeof(Current)) == 0) { return; }

    if(Storage::getInstance().write(reinterpret_cast<const byte*>(&Current), sizeof(Current)) == E_OK) {
        LastSaved = Current;
    }
} /* task */

/******************************************************************************************************************************************************
  save()
******************************************************************************************************************************************************/
/*! \brief          Writes the configuration now
 *  \details        Unconditionally, unlike task(): a caller that asks for this is about to
 *                  cut the power, and "nothing changed since the last write" is a state it
 *                  cannot check for itself.
 *
 *  \return         E_OK if the store took the configuration
******************************************************************************************************************************************************/
StdReturnType Persistence::save()
{
    const SettingsType Current = gather();

    if(Storage::getInstance().write(reinterpret_cast<const byte*>(&Current), sizeof(Current)) == E_NOT_OK) { return E_NOT_OK; }

    LastSaved = Current;
    return E_OK;
} /* save */


/******************************************************************************************************************************************************
  reset()
******************************************************************************************************************************************************/
/*! \brief          Puts the configuration back to what a clock starts with
 *  \details        Each module answers for its own defaults, the same way gather() reads
 *                  from each of them rather than from a copy kept here - so a setting
 *                  added to one of them is reset by the module that owns it and not
 *                  forgotten in a list over here.
 *
 *                  What is remembered as "last written" afterwards is what the modules now
 *                  hold, so the next task does not write the defaults straight back into
 *                  the store that was just emptied.
 *
 *  \return         E_OK if the store was emptied
******************************************************************************************************************************************************/
StdReturnType Persistence::reset()
{
    Display::getInstance().resetToDefaults();
    Clock::getInstance().resetToDefaults();
    Animations::getInstance().resetToDefaults();
    Illuminance::getInstance().resetToDefaults();
    Overlays::getInstance().resetToDefaults();
    NightSwitch::getInstance().resetToDefaults();

    const StdReturnType ReturnValue = Storage::getInstance().clear();

    LastSaved = gather();
    return ReturnValue;
} /* reset */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
