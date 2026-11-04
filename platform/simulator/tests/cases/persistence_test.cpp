/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       persistence_test.cpp
 *      \brief      What survives a restart, and what a reset puts back
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "Illuminance.h"
#include "Overlays.h"
#include "Persistence.h"
#include "Pixels.h"
#include "sim/Storage.h"

/*
 * Reaches around Storage on purpose, which nothing else may: damaging a stored blob is
 * the one thing the store's own interface will not do, and rejecting a damaged one is
 * what has to be shown.
 */
bool DamageStoredChecksum()
{
    std::array<byte, Storage::kCapacity> blob{};
    size_t size{0u};

    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
    if(file == nullptr) { return false; }
    size = std::fread(blob.data(), 1u, blob.size(), file);
    const bool read_failed = std::ferror(file) != 0;
    const int read_close_result = std::fclose(file);

    if(read_failed || read_close_result != 0 || size == 0u) { return false; }

    /*
     * The last byte, which is behind every field the format names - so this flip leaves
     * magic and version intact and is caught only if the checksum really covers the whole
     * blob rather than the fields it happens to know about.
     */
    blob[size - 1u] = static_cast<byte>(blob[size - 1u] + 1u);

    file = std::fopen(STORAGE_FILE_NAME, "wb");
    if(file == nullptr) { return false; }
    const size_t written = std::fwrite(blob.data(), 1u, size, file);
    const int write_close_result = std::fclose(file);

    return written == size && write_close_result == 0;
}

/*
 * The whole persistence path, on the store the simulator really writes to. Runs last
 * because it leaves the clock and the display on values of its own, and it clears the
 * store afterwards so a second run starts where this one did.
 */
void TestPersistence()
{
    Storage& storage = Storage::GetInstance();
    Persistence& persistence = Persistence::GetInstance();
    Display& display = Display::GetInstance();
    Clock& clock = Clock::GetInstance();

    Expect(storage.Clear() == kEOk, "clearing an empty store must succeed");
    Expect(persistence.Load() == kENotOk, "an empty store must report that nothing was restored");

    display.SetColor(10u, 20u, 30u);
    display.SetBrightness(123u);
    display.SetBrightnessUseGammaCorrection(true);
    clock.SetMode(Clock::ModeType::kSchwaben);
    Animations::GetInstance().SetTaskCycle(Animations::AnimationIdType::kNone, 42u);
    Illuminance::GetInstance().SetCalibrationValuesMaxValue(4321u);

    persistence.Task();

    /* Away from every stored value, so a restore that did nothing cannot pass. */
    display.SetColor(1u, 2u, 3u);
    display.SetBrightness(7u);
    display.SetBrightnessUseGammaCorrection(false);
    clock.SetMode(Clock::ModeType::kWessi);
    Animations::GetInstance().SetTaskCycle(Animations::AnimationIdType::kNone, 1u);
    Illuminance::GetInstance().SetCalibrationValuesMaxValue(1u);

    Expect(persistence.Load() == kEOk, "a stored configuration must be restored");
    Expect(display.GetColorRed() == 10u && display.GetColorGreen() == 20u && display.GetColorBlue() == 30u,
           "the colour must come back");
    Expect(display.GetBrightness() == 123u, "the brightness must come back");
    Expect(display.GetBrightnessUseGammaCorrection(), "the gamma correction switch must come back");
    Expect(clock.GetMode() == Clock::ModeType::kSchwaben, "the clock mode must come back");
    Expect(Animations::GetInstance().GetTaskCycle(Animations::AnimationIdType::kNone) == 42u,
           "an animation speed must come back");
    Expect(Illuminance::GetInstance().GetCalibrationValuesMaxValue() == 4321u,
           "the sensor calibration must come back");

    // Nothing changed since that restore, so nothing may be written - which is the whole
    // reason there is no dirty flag to forget.
    Expect(storage.Clear() == kEOk, "clearing a written store must succeed");
    persistence.Task();
    Expect(persistence.Load() == kENotOk, "an unchanged configuration must not have been written");

    /* And a blob whose checksum no longer fits must be refused rather than applied. */
    display.SetColor(11u, 22u, 33u);
    persistence.Task();
    Expect(DamageStoredChecksum(), "the test must be able to damage the stored blob");
    display.SetColor(4u, 5u, 6u);
    Expect(persistence.Load() == kENotOk, "a blob with a wrong checksum must be refused");
    Expect(display.GetColorRed() == 4u, "a refused blob must leave the settings alone");

    Expect(storage.Clear() == kEOk, "the test must leave the store as it found it");
}

/*
 * The two procedures that reach the store directly. What they are for is what Task()
 * cannot do: write before the plug is pulled rather than within the next period, and get
 * a clock back to what it left the factory as without erasing its flash over USB.
 */
void TestPersistenceSaveAndReset()
{
    Storage& storage = Storage::GetInstance();
    Persistence& persistence = Persistence::GetInstance();
    Display& display = Display::GetInstance();
    Clock& clock = Clock::GetInstance();

    display.SetColor(1u, 2u, 3u);
    clock.SetMode(Clock::ModeType::kOssi);
    clock.SetShowItIsPermanently(false);
    Expect(persistence.Save() == kEOk, "saving must write the configuration");

    display.SetColor(9u, 9u, 9u);
    clock.SetShowItIsPermanently(true);
    Expect(persistence.Load() == kEOk, "what was saved must come back");
    Expect(display.GetColorRed() == 1u, "and it must be what was saved");
    Expect(!clock.GetShowItIsPermanently(), "including the it is rule");

    /*
     * The overlays, which used to be the one configured thing a power cut took away. The
     * text goes through the store as a whole buffer, so a shorter one written after a
     * longer one must not leave the tail of the longer behind.
     */
    Overlays& overlays = Overlays::GetInstance();

    overlays.SetTextText("Herzlichen Glueckwunsch", 24u);
    (void)overlays.SetTextPeriodInMinutes(17u);
    (void)overlays.SetTextEnduranceInSeconds(9u);
    overlays.SetTextMonth(7u);
    overlays.SetTextDay(23u);
    overlays.SetTextValidInDays(3u);
    overlays.SetTextSpeed(4u);
    (void)overlays.SetTextFont(Text::FontType::kFont5x8);
    overlays.SetTextIsActive(true);
    (void)overlays.SetDatePeriodInMinutes(5u);
    overlays.SetDateIsActive(true);

    Expect(persistence.Save() == kEOk, "saving must write the overlays too");

    overlays.SetTextText("x", 2u);
    (void)overlays.SetTextPeriodInMinutes(1u);
    overlays.SetTextMonth(0u);
    overlays.SetTextSpeed(1u);
    (void)overlays.SetTextFont(Text::FontType::kFont10x10);
    overlays.SetTextIsActive(false);
    overlays.SetDateIsActive(false);

    Expect(persistence.Load() == kEOk, "the stored overlays must come back");
    Expect(strcmp(overlays.GetTextText(), "Herzlichen Glueckwunsch") == 0,
           "the overlay text must come back whole");
    Expect(overlays.GetTextPeriodInMinutes() == 17u, "and its period");
    Expect(overlays.GetTextEnduranceInSeconds() == 9u, "and its endurance");
    Expect(overlays.GetTextMonth() == 7u && overlays.GetTextDay() == 23u, "and its date");
    Expect(overlays.GetTextValidInDays() == 3u, "and how long that date stays valid");
    Expect(overlays.GetTextSpeed() == 4u, "and its speed");
    Expect(overlays.GetTextFont() == Text::FontType::kFont5x8, "and its font");
    Expect(overlays.GetTextIsActive(), "and whether it is switched on");
    Expect(overlays.GetDatePeriodInMinutes() == 5u && overlays.GetDateIsActive(),
           "a second overlay must not have been read out of the first one's slot");

    Expect(persistence.Reset() == kEOk, "resetting must empty the store");
    Expect(overlays.GetTextText()[0] == '\0', "the reset must empty the overlay text");
    Expect(overlays.GetTextPeriodInMinutes() == 1u && overlays.GetTextSpeed() == 1u,
           "the reset must put the overlay settings back");
    Expect(!overlays.GetTextIsActive() && !overlays.GetDateIsActive(),
           "the reset must switch every overlay off again");
    Expect(display.GetColorRed() == 255u && display.GetColorGreen() == 255u && display.GetColorBlue() == 255u,
           "the reset must put the colour back to white");
    Expect(display.GetBrightness() == 255u, "the reset must put the brightness back");
    // Qualified because the macro expands to the bare enumerator, and asked through the
    // macro rather than through the mode it happens to name today.
    Expect(clock.GetMode() == Clock::CLOCK_INITIAL_MODE, "the reset must put the clock mode back");
    Expect(Animations::GetInstance().GetAnimation() == Animations::AnimationIdType::kNone,
           "the reset must deselect the animation");

    // The store stays empty rather than being refilled with the defaults on the next
    // period - an empty store is what a clock that was never configured has.
    persistence.Task();
    Expect(persistence.Load() == kENotOk, "the task after a reset must leave the store empty");

    Expect(storage.Clear() == kEOk, "the test must leave the store as it found it");
}
