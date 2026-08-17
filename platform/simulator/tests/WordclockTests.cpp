/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       WordclockTests.cpp
 *      \brief      Runs the regression tests for the firmware core, in the order they need
 *
 *      \details    The cases themselves live in cases/, one file per subject, the way
 *                  platform/esp32/test/cases/ is laid out. One binary rather than one per
 *                  file, though: that side went to five executables and had to build an
 *                  object cache to stop compiling the core five times a run, and there is
 *                  no reason to buy the same problem here.
 *
 *                  Nothing here needs a display: the pixel buffer lives in Pixels and the
 *                  window that renders it is a separate PixelsFrame, which is never
 *                  constructed.
 *
 *                  The order below is not arbitrary. testDisplayManagerLatch() runs first,
 *                  because the latch it checks is only unlatched before the first task, and
 *                  the two persistence cases run last, because they leave settings behind
 *                  and write to the real store in the working directory.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <cstdlib>
#include <iostream>

#include "cases.h"
#include "check.h"

/******************************************************************************************************************************************************
 * M A I N
******************************************************************************************************************************************************/
int main()
{
    testDisplayManagerLatch();
    testShowNowProcedures();
    testTemperatureOverlayWithoutSensor();
    testEveryAnimationEndsOnTheNewTime();
    testFlickerLeavesTheDisplayOn();
    testFadeDimsAndComesBack();
    testCharacterToGlyphMapping();
    testRegionalWordings();
    testOverlayPeriodAndEndurance();
    testSchedulerSpeedRoundTrip();
    testUptimeCountsWithoutWrapping();
    testWordsChangeOnFiveMinuteStepsOnly();
    testInvalidTimeIsRejected();
    testClockModes();
    testClockModeStepsThroughEveryWording();
    testClockWordsComparison();
    testAnimationStepsThroughEveryOne();
    testDisplayOffAndOnAgain();
    testDisplayToggleFollowsTheState();
    testBrightnessTogglesInvertTheSetting();
    testColourChangesReachTheStrip();
    testPixelColorChannels();
    testDisplayCharacterLookup();
    testNightWindowIncludesMidnight();
    testNightSwitchDimsWithoutLosingTheSetting();
    testNightSwitchActsOnTheCrossingOnly();
    testSupplyCutWaitsForTheBlankedFrame();
    testSupplyReturnsBeforeTheDataLine();
    testSupplyOffIsNotDisplayDisable();
    testSupplyRequestsAreReversible();
    testPersistence();
    testPersistenceSaveAndReset();

    if(Failures == 0) {
        std::cout << "All Wordclock tests passed.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
