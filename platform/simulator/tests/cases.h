/*
 * Every case the runner calls, declared in one place so main.cpp needs no knowledge of
 * which file a case lives in - and so a case that is written and never called shows up as
 * a declaration nobody uses rather than as silence.
 *
 * Grouped by subject, the same way platform/esp32/test/cases/ is. The order they run in
 * is main.cpp's business and is not arbitrary; it says why there.
 */
#ifndef _CASES_H_
#define _CASES_H_


/* display_test.cpp */
void TestSingleBits();
void TestBitGroups();
void TestHeldBits();
void TestBitsChangedInPlace();

void TestColourDimmingAcrossByteRange();
void TestGammaCorrectionCurve();
void TestDisplayManagerLatch();
void TestFadeDimsAndComesBack();
void TestDisplayOffAndOnAgain();
void TestDisplayToggleFollowsTheState();
void TestBrightnessTogglesInvertTheSetting();
void TestColourChangesReachTheStrip();
void TestPixelColorChannels();
void TestDisplayCharacterLookup();

/* overlay_test.cpp */
void TestShowNowProcedures();
void TestTemperatureOverlayWithoutSensor();
void TestOverlayPeriodAndEndurance();

/* animation_test.cpp */
void TestSnakeLifetime();
void TestFlickerLeavesTheDisplayOn();
void TestEveryAnimationEndsOnTheNewTime();
void TestAnimationStepsThroughEveryOne();

/* text_test.cpp */
void TestCharacterToGlyphMapping();
void TestUndrawableCharacterShiftsAsASpace();

/* accessor_test.cpp */
void TestReadingPastTheEndIsEmpty();
void TestWritingPastTheEndIsRefused();
void TestGlyphAccessorsCheckBothWays();

/* clock_test.cpp */
void TestRegionalWordings();
void TestWordsChangeOnFiveMinuteStepsOnly();
void TestInvalidTimeIsRejected();
void TestClockModes();
void TestClockModeStepsThroughEveryWording();
void TestClockWordsComparison();

/* scheduler_test.cpp */
void TestSchedulerSpeedRoundTrip();
void TestUptimeCountsWithoutWrapping();

/* nightswitch_test.cpp */
void TestNightWindowIncludesMidnight();
void TestNightSwitchDimsWithoutLosingTheSetting();
void TestNightSwitchActsOnTheCrossingOnly();
void TestNightWithNoBrightnessCutsTheSupply();
void TestDisplaySwitchedOnAtNightGetsItsSupplyBack();

/* colorcycle_test.cpp */
void TestColourCycleLeavesTheChosenColourAlone();
void TestColourCycleWalksTheWholeWheel();
void TestColourCycleStandsStillOnADarkDisplay();
void TestColourCycleBudgetsTheColourItShows();

/* power_test.cpp */
void TestSupplyCutWaitsForTheBlankedFrame();
void TestSupplyReturnsBeforeTheDataLine();
void TestSupplyOffIsNotDisplayDisable();
void TestSupplyRequestsAreReversible();

/* illuminance_test.cpp */
void TestAutomaticDefaultsSuitARoom();
void TestAutomaticSpreadsBetweenBothBounds();

/* current_limit_test.cpp */
void TestCurrentLimitFitsTheBudgetAtEveryLitCount();
void TestCurrentLimitFollowsTheColour();
void TestCurrentLimitLeavesTheRestoredFaceAlone();
void TestCurrentLimitDoesNotSwitchTheDisplayOn();

/* persistence_test.cpp */
void TestPersistence();
void TestPersistenceSaveAndReset();

/* websocket_test.cpp */
void TestWebSocketHashAndEncoding();
void TestWebSocketHandshake();
void TestWebSocketEncodesBothLengths();
void TestWebSocketDecodesWhatABrowserSends();

/* webhost_test.cpp */
void TestWebHostOutlastsAnIdleConnection();

#endif // _CASES_H_
