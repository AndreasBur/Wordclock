/*
 * Runs the regression tests for the firmware core, in the order they need.
 *
 * The cases themselves live in cases/, one file per subject, the way platform/esp32/test/cases/ is
 * laid out. One binary rather than one per file, though: that side went to five executables and had
 * to build an object cache to stop compiling the core five times a run, and there is no reason to
 * buy the same problem here.
 *
 * Nothing here needs a display: the pixel buffer lives in Pixels and the window that renders it is
 * a separate PixelsFrame, which is never constructed.
 *
 * The order below is not arbitrary. TestDisplayManagerLatch() runs first, because the latch it
 * checks is only unlatched before the first task, and the two persistence cases run last, because
 * they leave settings behind and write to the real store in the working directory.
 */

#include <cstdlib>
#include <iostream>

#include "cases.h"
#include "check.h"

int main()
{
    TestSingleBits();
    TestBitGroups();
    TestHeldBits();
    TestBitsChangedInPlace();
    TestColourDimmingAcrossByteRange();
    TestGammaCorrectionCurve();
    TestSnakeLifetime();
    TestDisplayManagerLatch();
    TestShowNowProcedures();
    TestTemperatureOverlayWithoutSensor();
    TestEveryAnimationEndsOnTheNewTime();
    TestFlickerLeavesTheDisplayOn();
    TestFadeDimsAndComesBack();
    TestCharacterToGlyphMapping();
    TestUndrawableCharacterShiftsAsASpace();
    TestReadingPastTheEndIsEmpty();
    TestWritingPastTheEndIsRefused();
    TestGlyphAccessorsCheckBothWays();
    TestRegionalWordings();
    TestOverlayPeriodAndEndurance();
    TestOverlaySwitchedOffWhileItShows();
    TestStartingOverlayEndsTheAnimation();
    TestValidInDaysCountsFromTheDate();
    TestSchedulerSpeedRoundTrip();
    TestUptimeCountsWithoutWrapping();
    TestWordsChangeOnFiveMinuteStepsOnly();
    TestInvalidTimeIsRejected();
    TestClockModes();
    TestClockModeStepsThroughEveryWording();
    TestClockWordsComparison();
    TestWeekdayOfEveryDate();
    TestItoaWritesEveryInt32();
    TestStringReverseTakesAnyLength();
    TestStringCopyIntoNoRoom();
    TestSignedConversionKeepsToTheType();
    TestUnsignedConversionRefusesANegativeNumber();
    TestFloatConversionKeepsToTheType();
    TestSerialShimSpeaksLatin1();
    TestAnimationStepsThroughEveryOne();
    TestFixedModeRunsEveryTimeAsTheFirst();
    TestEveryAnimationReportsItsStart();
    TestFlickerLeavesADarkDisplayDark();
    TestSelectingModesAnimateWithNothingSelected();
    TestLeavingAFadeUndimsTheDisplay();
    TestAWordChangeRunsAFadeOrFlickerOn();
    TestExplodeWordsLightsItsFirstWordOnADarkDisplay();
    TestDisplayOffAndOnAgain();
    TestDisplayToggleFollowsTheState();
    TestBrightnessTogglesInvertTheSetting();
    TestColourChangesReachTheStrip();
    TestPixelColorChannels();
    TestDisplayCharacterLookup();
    TestNightWindowIncludesMidnight();
    TestNightSwitchDimsWithoutLosingTheSetting();
    TestNightSwitchActsOnTheCrossingOnly();
    TestNightWithNoBrightnessCutsTheSupply();
    TestDisplaySwitchedOnAtNightGetsItsSupplyBack();
    TestColourCycleLeavesTheChosenColourAlone();
    TestColourCycleWalksTheWholeWheel();
    TestColourCycleStandsStillOnADarkDisplay();
    TestColourCycleBudgetsTheColourItShows();
    TestSupplyCutWaitsForTheBlankedFrame();
    TestSupplyReturnsBeforeTheDataLine();
    TestSupplyOffIsNotDisplayDisable();
    TestSupplyRequestsAreReversible();
    TestAutomaticDefaultsSuitARoom();
    TestAutomaticSpreadsBetweenBothBounds();
    TestCurrentLimitFitsTheBudgetAtEveryLitCount();
    TestCurrentLimitFollowsTheColour();
    TestCurrentLimitLeavesTheRestoredFaceAlone();
    TestCurrentLimitDoesNotSwitchTheDisplayOn();
    TestPersistence();
    TestPersistenceSaveAndReset();
    TestWebSocketHashAndEncoding();
    TestWebSocketHandshake();
    TestWebSocketEncodesBothLengths();
    TestWebSocketDecodesWhatABrowserSends();
    /* Last, because it hands the output lines to the host for the rest of the run. */
    TestWebHostOutlastsAnIdleConnection();

    if(failures == 0) {
        std::cout << "All Wordclock tests passed.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
