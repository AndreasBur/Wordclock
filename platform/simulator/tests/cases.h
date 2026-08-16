/* Every case the runner calls, declared in one place so main.cpp needs no knowledge of
   which file a case lives in - and so a case that is written and never called shows up as
   a declaration nobody uses rather than as silence.

   Grouped by subject, the same way platform/esp32/test/cases/ is. The order they run in
   is main.cpp's business and is not arbitrary; it says why there. */
#ifndef _CASES_H_
#define _CASES_H_

/* display_test.cpp */
void testDisplayManagerLatch();
void testFadeDimsAndComesBack();
void testDisplayOffAndOnAgain();
void testPixelColorChannels();
void testDisplayCharacterLookup();

/* overlay_test.cpp */
void testShowNowProcedures();
void testTemperatureOverlayWithoutSensor();
void testOverlayPeriodAndEndurance();

/* animation_test.cpp */
void testFlickerLeavesTheDisplayOn();
void testEveryAnimationEndsOnTheNewTime();

/* text_test.cpp */
void testCharacterToGlyphMapping();

/* clock_test.cpp */
void testRegionalWordings();
void testWordsChangeOnFiveMinuteStepsOnly();
void testInvalidTimeIsRejected();
void testClockModes();
void testClockWordsComparison();

/* scheduler_test.cpp */
void testSchedulerSpeedRoundTrip();
void testUptimeCountsWithoutWrapping();

/* persistence_test.cpp */
void testPersistence();
void testPersistenceSaveAndReset();

#endif // _CASES_H_
