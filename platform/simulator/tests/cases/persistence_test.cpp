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

/* Reaches around Storage on purpose, which nothing else may: damaging a stored blob is
   the one thing the store's own interface will not do, and rejecting a damaged one is
   what has to be shown. */
bool damageStoredChecksum()
{
    std::array<byte, Storage::Capacity> blob{};
    size_t size{0u};

    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
    if(file == nullptr) { return false; }
    size = std::fread(blob.data(), 1u, blob.size(), file);
    std::fclose(file);

    if(size == 0u) { return false; }

    /* The last byte, which is behind every field the format names - so this flip leaves
       magic and version intact and is caught only if the checksum really covers the whole
       blob rather than the fields it happens to know about. */
    blob[size - 1u] = static_cast<byte>(blob[size - 1u] + 1u);

    file = std::fopen(STORAGE_FILE_NAME, "wb");
    if(file == nullptr) { return false; }
    const size_t written = std::fwrite(blob.data(), 1u, size, file);
    std::fclose(file);

    return written == size;
}

/* The whole persistence path, on the store the simulator really writes to. Runs last
   because it leaves the clock and the display on values of its own, and it clears the
   store afterwards so a second run starts where this one did. */
void testPersistence()
{
    Storage& storage = Storage::getInstance();
    Persistence& persistence = Persistence::getInstance();
    Display& display = Display::getInstance();
    Clock& clock = Clock::getInstance();

    expect(storage.clear() == E_OK, "clearing an empty store must succeed");
    expect(persistence.load() == E_NOT_OK, "an empty store must report that nothing was restored");

    display.setColor(10u, 20u, 30u);
    display.setBrightness(123u);
    display.setBrightnessUseGammaCorrection(true);
    clock.setModeFast(Clock::MODE_SCHWABEN);
    Animations::getInstance().setTaskCycleFast(Animations::ANIMATION_ID_NONE, 42u);
    Illuminance::getInstance().setCalibrationValuesMaxValue(4321u);

    persistence.task();

    /* Away from every stored value, so a restore that did nothing cannot pass. */
    display.setColor(1u, 2u, 3u);
    display.setBrightness(7u);
    display.setBrightnessUseGammaCorrection(false);
    clock.setModeFast(Clock::MODE_WESSI);
    Animations::getInstance().setTaskCycleFast(Animations::ANIMATION_ID_NONE, 1u);
    Illuminance::getInstance().setCalibrationValuesMaxValue(1u);

    expect(persistence.load() == E_OK, "a stored configuration must be restored");
    expect(display.getColorRed() == 10u && display.getColorGreen() == 20u && display.getColorBlue() == 30u,
           "the colour must come back");
    expect(display.getBrightness() == 123u, "the brightness must come back");
    expect(display.getBrightnessUseGammaCorrection(), "the gamma correction switch must come back");
    expect(clock.getMode() == Clock::MODE_SCHWABEN, "the clock mode must come back");
    expect(Animations::getInstance().getTaskCycle(Animations::ANIMATION_ID_NONE) == 42u,
           "an animation speed must come back");
    expect(Illuminance::getInstance().getCalibrationValuesMaxValue() == 4321u,
           "the sensor calibration must come back");

    /* Nothing changed since that restore, so nothing may be written - which is the whole
       reason there is no dirty flag to forget. */
    expect(storage.clear() == E_OK, "clearing a written store must succeed");
    persistence.task();
    expect(persistence.load() == E_NOT_OK, "an unchanged configuration must not have been written");

    /* And a blob whose checksum no longer fits must be refused rather than applied. */
    display.setColor(11u, 22u, 33u);
    persistence.task();
    expect(damageStoredChecksum(), "the test must be able to damage the stored blob");
    display.setColor(4u, 5u, 6u);
    expect(persistence.load() == E_NOT_OK, "a blob with a wrong checksum must be refused");
    expect(display.getColorRed() == 4u, "a refused blob must leave the settings alone");

    expect(storage.clear() == E_OK, "the test must leave the store as it found it");
}

/* The two procedures that reach the store directly. What they are for is what task()
   cannot do: write before the plug is pulled rather than within the next period, and get
   a clock back to what it left the factory as without erasing its flash over USB. */
void testPersistenceSaveAndReset()
{
    Storage& storage = Storage::getInstance();
    Persistence& persistence = Persistence::getInstance();
    Display& display = Display::getInstance();
    Clock& clock = Clock::getInstance();

    display.setColor(1u, 2u, 3u);
    clock.setModeFast(Clock::MODE_OSSI);
    clock.setShowItIsPermanently(false);
    expect(persistence.save() == E_OK, "saving must write the configuration");

    display.setColor(9u, 9u, 9u);
    clock.setShowItIsPermanently(true);
    expect(persistence.load() == E_OK, "what was saved must come back");
    expect(display.getColorRed() == 1u, "and it must be what was saved");
    expect(!clock.getShowItIsPermanently(), "including the it is rule");

    /* The overlays, which used to be the one configured thing a power cut took away. The
       text goes through the store as a whole buffer, so a shorter one written after a
       longer one must not leave the tail of the longer behind. */
    Overlays& overlays = Overlays::getInstance();

    overlays.setTextText("Herzlichen Glueckwunsch", 24u);
    (void)overlays.setTextPeriodInMinutes(17u);
    (void)overlays.setTextEnduranceInSeconds(9u);
    overlays.setTextMonth(7u);
    overlays.setTextDay(23u);
    overlays.setTextValidInDays(3u);
    overlays.setTextSpeed(4u);
    (void)overlays.setTextFont(Text::FONT_5X8);
    overlays.setTextIsActive(true);
    (void)overlays.setDatePeriodInMinutes(5u);
    overlays.setDateIsActive(true);

    expect(persistence.save() == E_OK, "saving must write the overlays too");

    overlays.setTextText("x", 2u);
    (void)overlays.setTextPeriodInMinutes(1u);
    overlays.setTextMonth(0u);
    overlays.setTextSpeed(1u);
    (void)overlays.setTextFont(Text::FONT_10X10);
    overlays.setTextIsActive(false);
    overlays.setDateIsActive(false);

    expect(persistence.load() == E_OK, "the stored overlays must come back");
    expect(strcmp(overlays.getTextText(), "Herzlichen Glueckwunsch") == 0,
           "the overlay text must come back whole");
    expect(overlays.getTextPeriodInMinutes() == 17u, "and its period");
    expect(overlays.getTextEnduranceInSeconds() == 9u, "and its endurance");
    expect(overlays.getTextMonth() == 7u && overlays.getTextDay() == 23u, "and its date");
    expect(overlays.getTextValidInDays() == 3u, "and how long that date stays valid");
    expect(overlays.getTextSpeed() == 4u, "and its speed");
    expect(overlays.getTextFont() == Text::FONT_5X8, "and its font");
    expect(overlays.getTextIsActive(), "and whether it is switched on");
    expect(overlays.getDatePeriodInMinutes() == 5u && overlays.getDateIsActive(),
           "a second overlay must not have been read out of the first one's slot");

    expect(persistence.reset() == E_OK, "resetting must empty the store");
    expect(overlays.getTextText()[0] == '\0', "the reset must empty the overlay text");
    expect(overlays.getTextPeriodInMinutes() == 1u && overlays.getTextSpeed() == 1u,
           "the reset must put the overlay settings back");
    expect(!overlays.getTextIsActive() && !overlays.getDateIsActive(),
           "the reset must switch every overlay off again");
    expect(display.getColorRed() == 255u && display.getColorGreen() == 255u && display.getColorBlue() == 255u,
           "the reset must put the colour back to white");
    expect(display.getBrightness() == 255u, "the reset must put the brightness back");
    /* Qualified because the macro expands to the bare enumerator, and asked through the
       macro rather than through the mode it happens to name today. */
    expect(clock.getMode() == Clock::CLOCK_INITIAL_MODE, "the reset must put the clock mode back");
    expect(Animations::getInstance().getAnimation() == Animations::ANIMATION_ID_NONE,
           "the reset must deselect the animation");

    /* The store stays empty rather than being refilled with the defaults on the next
       period - an empty store is what a clock that was never configured has. */
    persistence.task();
    expect(persistence.load() == E_NOT_OK, "the task after a reset must leave the store empty");

    expect(storage.clear() == E_OK, "the test must leave the store as it found it");
}
