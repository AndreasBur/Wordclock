/*
 * Every case the runner calls, declared in one place so main.cpp needs no knowledge of
 * which file a case lives in - and so a case that is written and never called shows up as
 * a declaration nobody uses rather than as silence.
 *
 * Grouped by subject, the same way platform/esp32/test/cases/ is. The order they run in
 * is main.cpp's business and is not arbitrary; it says why there.
 */
#ifndef WORDCLOCK_PLATFORM_SIMULATOR_TESTS_CASES_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_TESTS_CASES_H_

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
void TestWordsAreClearedOneOrAll();
void TestPixelRowsAndColumnsAreWritten();
void TestPixelByIndexTogglesAndReads();
void TestInitTurnsEveryPixelOff();

/* overlay_test.cpp */
void TestShowNowProcedures();
void TestTemperatureOverlayWithoutSensor();
void TestOverlayPeriodAndEndurance();
void TestOverlaySwitchedOffWhileItShows();
void TestStartingOverlayEndsTheAnimation();
void TestValidInDaysCountsFromTheDate();

/* animation_test.cpp */
void TestSnakeLifetime();
void TestFlickerLeavesTheDisplayOn();
void TestEveryAnimationEndsOnTheNewTime();
void TestAnimationStepsThroughEveryOne();
void TestFixedModeRunsEveryTimeAsTheFirst();
void TestEveryAnimationReportsItsStart();
void TestFlickerLeavesADarkDisplayDark();
void TestSelectingModesAnimateWithNothingSelected();
void TestLeavingAFadeUndimsTheDisplay();
void TestAWordChangeRunsAFadeOrFlickerOn();
void TestExplodeWordsLightsItsFirstWordOnADarkDisplay();

/* transformation_test.cpp */
void TestShiftsMoveEveryPixelByOne();
void TestRollAnimationRollsEveryWay();

/* text_test.cpp */
void TestCharacterToGlyphMapping();
void TestUndrawableCharacterShiftsAsASpace();

/* accessor_test.cpp */
void TestReadingPastTheEndIsEmpty();
void TestWritingPastTheEndIsRefused();
void TestGlyphAccessorsCheckBothWays();
void TestWordAccessorsCheckBothWays();
void TestGlyphWidthChecksBothWays();
void TestGridRowsAndColumnsReadTheLitPixels();

/* clock_test.cpp */
void TestRegionalWordings();
void TestWordsChangeOnFiveMinuteStepsOnly();
void TestInvalidTimeIsRejected();
void TestClockModes();
void TestClockModeStepsThroughEveryWording();
void TestClockWordsComparison();

/* date_test.cpp */
void TestWeekdayOfEveryDate();

/* serial_shim_test.cpp */
void TestSerialShimSpeaksLatin1();

/* scheduler_test.cpp */
void TestSchedulerSpeedRoundTrip();
void TestUptimeCountsWithoutWrapping();
void TestSchedulerRunsATaskFirstThenEveryCycle();
void TestSchedulerTakesANewCycleOnTheNextRun();
void TestSchedulerCycleZeroSwitchesATaskOff();
void TestSchedulerReadsTheSensorBeforeTheBrightness();
void TestSchedulerLetsTheNightGoBeforeTheColourCycle();
void TestSchedulerSavesWhatATickChanged();

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

/* command_test.cpp */
void TestTimeCommandSetsTheClock();
void TestDateCommandSetsTheCalendar();
void TestClockModeCommandSetsTheWording();
void TestDisplayColorCommandRecoloursTheWords();
void TestColorCycleCommandStartsAndStops();
void TestNightSwitchCommandSetsTheWindow();
void TestOverlayCommandSetsTheTemperatureOverlay();
void TestProceduresChangeTheDisplay();
void TestProcedureSelectionsWrapAround();
void TestUnknownProcedureRunsNothing();
void TestDisplayBrightnessCommandSetsTheBrightness();
void TestDisplayPixelCommandWritesOnePixel();
void TestAnimationCommandSelectsAndTunes();
void TestOverlayCommandsReachTheirOwnOverlay();
void TestProceduresReachTheirTargets();

/* persistence_test.cpp */
void TestPersistence();
void TestPersistenceSaveAndReset();

/* websocket_test.cpp */
void TestWebSocketHashAndEncoding();
void TestWebSocketHandshake();
void TestWebSocketEncodesBothLengths();
void TestWebSocketDecodesWhatABrowserSends();

/* stringtools_test.cpp */
void TestItoaWritesEveryInt32();
void TestStringReverseTakesAnyLength();
void TestStringCopyIntoNoRoom();
void TestSignedConversionKeepsToTheType();
void TestUnsignedConversionRefusesANegativeNumber();
void TestFloatConversionKeepsToTheType();

/* webhost_test.cpp */
void TestWebHostOutlastsAnIdleConnection();

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_TESTS_CASES_H_
