/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       clock_test.cpp
 *      \brief      The four wordings, the five-minute raster, and what the clock refuses
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"

/* Whether a word is part of what the clock would light for a time, which is what the
   regional wordings differ in. */
bool wordsContain(const ClockWords& Words, DisplayWords::WordIdType Word)
{
    for(const DisplayWords::WordIdType Id : Words.getWordsList()) {
        if(Id == Word) { return true; }
    }
    return false;
}

/* The four regional wordings, at the four times where they part company. This is the table
   in docs/serial-commands.md, asserted rather than described: the modes differ in how the
   quarters are said and in whether twenty past is counted from the hour or from the half
   hour, and in nothing else.
   What makes it worth pinning is that all four share one pair of tables, indexed by mode -
   a row edited in the wrong one is a clock that is wrong in one region and right in the
   other three. */
void testRegionalWordings()
{
    Clock& clock = Clock::getInstance();

    /* Quarter past four: named after the hour it is past, or after the one it counts
       towards. */
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_NACH),
           "Wessi says a quarter *past* four");
    clock.setModeFast(Clock::MODE_OSSI);
    expect(!wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_NACH),
           "Ossi says viertel five, with no past in it");
    expect(wordsContain(wordsAt(4u, 15u), DisplayWords::WORD_VIERTEL),
           "and it is still a quarter");

    /* Quarter to five: the same difference, from the other side. */
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_VOR),
           "Wessi says a quarter *to* five");
    clock.setModeFast(Clock::MODE_SCHWABEN);
    expect(wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_DREIVIERTEL),
           "Schwaben says three quarters five");
    expect(!wordsContain(wordsAt(4u, 45u), DisplayWords::WORD_VOR),
           "which has no to in it");

    /* Twenty past four: counted from the hour, or as ten before half five. */
    clock.setModeFast(Clock::MODE_RHEIN_RUHR);
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_NACH),
           "Rhein-Ruhr counts twenty past the hour");
    expect(!wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_HALB),
           "and does not reach for the half hour");
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_HALB),
           "Wessi counts ten before half five");
    expect(wordsContain(wordsAt(4u, 20u), DisplayWords::WORD_VOR),
           "which is a before");

    /* Twenty to five, the mirror of it. */
    clock.setModeFast(Clock::MODE_RHEIN_RUHR);
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_VOR),
           "Rhein-Ruhr counts twenty to the hour");
    clock.setModeFast(Clock::MODE_WESSI);
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_HALB),
           "Wessi counts ten past half five");
    expect(wordsContain(wordsAt(4u, 40u), DisplayWords::WORD_NACH),
           "which is a past");

    /* And what none of them differ in: the full hour and the half hour always say "it is".
       Whether the times between them do is the one thing about the wording that is a
       setting, so both of its answers are asked here rather than whichever one the build
       happens to carry. */
    clock.setModeFast(Clock::MODE_WESSI);
    const bool ShowItIsPermanentlyBefore = clock.getShowItIsPermanently();

    clock.setShowItIsPermanently(false);
    expect(wordsAt(4u, 0u).getShowItIs(), "the full hour says it is");
    expect(wordsAt(4u, 30u).getShowItIs(), "and so does the half hour");
    expect(!wordsAt(4u, 5u).getShowItIs(), "five past does not");
    expect(!wordsAt(4u, 45u).getShowItIs(), "and neither does a quarter to");

    clock.setShowItIsPermanently(true);
    expect(wordsAt(4u, 5u).getShowItIs(), "with it is set permanently, five past says it too");
    expect(wordsAt(4u, 45u).getShowItIs(), "and so does a quarter to");
    expect(wordsAt(4u, 0u).getShowItIs(), "and the full hour still does");

    clock.setShowItIsPermanently(ShowItIsPermanentlyBefore);

    clock.setModeFast(Clock::MODE_WESSI);
}

/* The premise of driving the display off the word set instead of off the minute: a
   word change happens on every five-minute step and on none of the minutes between,
   over the whole day rather than at a few sample times. */
void testWordsChangeOnFiveMinuteStepsOnly()
{
    Clock::getInstance().setModeFast(Clock::MODE_WESSI);

    ClockWords previousWords = wordsAt(0u, 0u);

    for(byte hour = 0u; hour < 24u; hour++) {
        for(byte minute = 0u; minute < 60u; minute++) {
            if(hour == 0u && minute == 0u) { continue; }

            const ClockWords words = wordsAt(hour, minute);
            const bool isStep = (minute % 5u) == 0u;

            if(isStep) {
                expect(words != previousWords, "every five-minute step must change the words");
            } else {
                expect(words == previousWords, "minutes inside a step must leave the words alone");
            }
            previousWords = words;
        }
    }
}

void testInvalidTimeIsRejected()
{
    Clock& clock = Clock::getInstance();
    ClockWords invalidWords;

    expect(clock.getClockWords(24u, 0u, invalidWords) == E_NOT_OK,
           "hour 24 must be rejected");
    expect(clock.getClockWords(10u, 60u, invalidWords) == E_NOT_OK,
           "minute 60 must be rejected");
}

void testClockModes()
{
    Clock& clock = Clock::getInstance();
    clock.setModeFast(Clock::MODE_WESSI);
    const ClockWords wessiWords = wordsAt(10u, 15u);

    clock.setModeFast(Clock::MODE_OSSI);
    expect(wessiWords != wordsAt(10u, 15u),
           "changing the clock mode must change the words where modes differ");

    clock.setModeFast(Clock::MODE_WESSI);
    expect(wessiWords == wordsAt(10u, 15u),
           "the same mode and time must produce the same words again");
}

/* One wording further, which is all a knob can ask for. The round has to close: a walk of
   as many steps as there are wordings must arrive back where it started, or a control with
   no list in front of it can reach a mode and never leave it. */
void testClockModeStepsThroughEveryWording()
{
    Clock& clock = Clock::getInstance();
    clock.setModeFast(Clock::MODE_WESSI);

    bool seen[Clock::MODE_NUMBER_OF_MODES]{};
    for(byte Step = 0u; Step < Clock::MODE_NUMBER_OF_MODES; Step++) {
        seen[clock.getMode()] = true;
        clock.nextMode();
    }

    bool allSeen = true;
    for(const bool Seen : seen) { if(!Seen) { allSeen = false; } }
    expect(allSeen, "stepping must reach every wording");
    expect(clock.getMode() == Clock::MODE_WESSI, "and must wrap back to the one it started on");

    /* The step itself, said once outright: the mode after the last is the first again. */
    clock.setModeFast(Clock::MODE_SCHWABEN);
    clock.nextMode();
    expect(clock.getMode() == Clock::MODE_WESSI, "the wording after the last must be the first");

    clock.setModeFast(Clock::MODE_WESSI);
}

/* The comparison the display update hangs on: it has to notice a difference in each
   of the three fields, or a word change goes unnoticed and the display stays behind. */
void testClockWordsComparison()
{
    const ClockWords words = wordsAt(10u, 15u);

    expect(words == words, "clock words must equal themselves");

    ClockWords differentItIs = words;
    differentItIs.setShowItIs(!words.getShowItIs());
    expect(differentItIs != words, "a different \"it is\" must compare unequal");

    ClockWords differentHour = words;
    ClockWords::HourWordsType hourWords = words.getHourWords();
    hourWords[0] = (hourWords[0] == DisplayWords::WORD_NONE) ? DisplayWords::WORD_ES
                                                            : DisplayWords::WORD_NONE;
    differentHour.setHourWords(hourWords);
    expect(differentHour != words, "a different hour word must compare unequal");

    ClockWords differentMinute = words;
    ClockWords::MinutesWordsType minuteWords = words.getMinuteWords();
    minuteWords[0] = (minuteWords[0] == DisplayWords::WORD_NONE) ? DisplayWords::WORD_ES
                                                                : DisplayWords::WORD_NONE;
    differentMinute.setMinuteWords(minuteWords);
    expect(differentMinute != words, "a different minute word must compare unequal");
}
