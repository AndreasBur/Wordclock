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
bool WordsContain(const ClockWords& words, DisplayWords::WordIdType word)
{
    for(const DisplayWords::WordIdType id : words.GetWordsList()) {
        if(id == word) { return true; }
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
void TestRegionalWordings()
{
    Clock& clock = Clock::GetInstance();

    /* Quarter past four: named after the hour it is past, or after the one it counts
       towards. */
    clock.SetMode(Clock::kModeWessi);
    Expect(WordsContain(WordsAt(4u, 15u), DisplayWords::kWordNach),
           "Wessi says a quarter *past* four");
    clock.SetMode(Clock::kModeOssi);
    Expect(!WordsContain(WordsAt(4u, 15u), DisplayWords::kWordNach),
           "Ossi says viertel five, with no past in it");
    Expect(WordsContain(WordsAt(4u, 15u), DisplayWords::kWordViertel),
           "and it is still a quarter");

    /* Quarter to five: the same difference, from the other side. */
    clock.SetMode(Clock::kModeWessi);
    Expect(WordsContain(WordsAt(4u, 45u), DisplayWords::kWordVor),
           "Wessi says a quarter *to* five");
    clock.SetMode(Clock::kModeSchwaben);
    Expect(WordsContain(WordsAt(4u, 45u), DisplayWords::kWordDreiviertel),
           "Schwaben says three quarters five");
    Expect(!WordsContain(WordsAt(4u, 45u), DisplayWords::kWordVor),
           "which has no to in it");

    /* Twenty past four: counted from the hour, or as ten before half five. */
    clock.SetMode(Clock::kModeRheinRuhr);
    Expect(WordsContain(WordsAt(4u, 20u), DisplayWords::kWordNach),
           "Rhein-Ruhr counts twenty past the hour");
    Expect(!WordsContain(WordsAt(4u, 20u), DisplayWords::kWordHalb),
           "and does not reach for the half hour");
    clock.SetMode(Clock::kModeWessi);
    Expect(WordsContain(WordsAt(4u, 20u), DisplayWords::kWordHalb),
           "Wessi counts ten before half five");
    Expect(WordsContain(WordsAt(4u, 20u), DisplayWords::kWordVor),
           "which is a before");

    /* Twenty to five, the mirror of it. */
    clock.SetMode(Clock::kModeRheinRuhr);
    Expect(WordsContain(WordsAt(4u, 40u), DisplayWords::kWordVor),
           "Rhein-Ruhr counts twenty to the hour");
    clock.SetMode(Clock::kModeWessi);
    Expect(WordsContain(WordsAt(4u, 40u), DisplayWords::kWordHalb),
           "Wessi counts ten past half five");
    Expect(WordsContain(WordsAt(4u, 40u), DisplayWords::kWordNach),
           "which is a past");

    /* And what none of them differ in: the full hour and the half hour always say "it is".
       Whether the times between them do is the one thing about the wording that is a
       setting, so both of its answers are asked here rather than whichever one the build
       happens to carry. */
    clock.SetMode(Clock::kModeWessi);
    const bool show_it_is_permanently_before = clock.GetShowItIsPermanently();

    clock.SetShowItIsPermanently(false);
    Expect(WordsAt(4u, 0u).GetShowItIs(), "the full hour says it is");
    Expect(WordsAt(4u, 30u).GetShowItIs(), "and so does the half hour");
    Expect(!WordsAt(4u, 5u).GetShowItIs(), "five past does not");
    Expect(!WordsAt(4u, 45u).GetShowItIs(), "and neither does a quarter to");

    clock.SetShowItIsPermanently(true);
    Expect(WordsAt(4u, 5u).GetShowItIs(), "with it is set permanently, five past says it too");
    Expect(WordsAt(4u, 45u).GetShowItIs(), "and so does a quarter to");
    Expect(WordsAt(4u, 0u).GetShowItIs(), "and the full hour still does");

    clock.SetShowItIsPermanently(show_it_is_permanently_before);

    clock.SetMode(Clock::kModeWessi);
}

/* The premise of driving the display off the word set instead of off the minute: a
   word change happens on every five-minute step and on none of the minutes between,
   over the whole day rather than at a few sample times. */
void TestWordsChangeOnFiveMinuteStepsOnly()
{
    Clock::GetInstance().SetMode(Clock::kModeWessi);

    ClockWords previous_words = WordsAt(0u, 0u);

    for(byte hour = 0u; hour < 24u; hour++) {
        for(byte minute = 0u; minute < 60u; minute++) {
            if(hour == 0u && minute == 0u) { continue; }

            const ClockWords words = WordsAt(hour, minute);
            const bool is_step = (minute % 5u) == 0u;

            if(is_step) {
                Expect(words != previous_words, "every five-minute step must change the words");
            } else {
                Expect(words == previous_words, "minutes inside a step must leave the words alone");
            }
            previous_words = words;
        }
    }
}

void TestInvalidTimeIsRejected()
{
    Clock& clock = Clock::GetInstance();
    ClockWords invalid_words;

    Expect(clock.GetClockWords(24u, 0u, invalid_words) == kENotOk,
           "hour 24 must be rejected");
    Expect(clock.GetClockWords(10u, 60u, invalid_words) == kENotOk,
           "minute 60 must be rejected");
}

void TestClockModes()
{
    Clock& clock = Clock::GetInstance();
    clock.SetMode(Clock::kModeWessi);
    const ClockWords wessi_words = WordsAt(10u, 15u);

    clock.SetMode(Clock::kModeOssi);
    Expect(wessi_words != WordsAt(10u, 15u),
           "changing the clock mode must change the words where modes differ");

    clock.SetMode(Clock::kModeWessi);
    Expect(wessi_words == WordsAt(10u, 15u),
           "the same mode and time must produce the same words again");
}

/* One wording further, which is all a knob can ask for. The round has to close: a walk of
   as many steps as there are wordings must arrive back where it started, or a control with
   no list in front of it can reach a mode and never leave it. */
void TestClockModeStepsThroughEveryWording()
{
    Clock& clock = Clock::GetInstance();
    clock.SetMode(Clock::kModeWessi);

    bool seen[Clock::kModeNumberOfModes]{};
    for(byte step = 0u; step < Clock::kModeNumberOfModes; step++) {
        seen[clock.GetMode()] = true;
        clock.NextMode();
    }

    bool all_seen = true;
    for(const bool was_seen : seen) { if(!was_seen) { all_seen = false; } }
    Expect(all_seen, "stepping must reach every wording");
    Expect(clock.GetMode() == Clock::kModeWessi, "and must wrap back to the one it started on");

    /* The step itself, said once outright: the mode after the last is the first again. */
    clock.SetMode(Clock::kModeSchwaben);
    clock.NextMode();
    Expect(clock.GetMode() == Clock::kModeWessi, "the wording after the last must be the first");

    clock.SetMode(Clock::kModeWessi);
}

/* The comparison the display update hangs on: it has to notice a difference in each
   of the three fields, or a word change goes unnoticed and the display stays behind. */
void TestClockWordsComparison()
{
    const ClockWords words = WordsAt(10u, 15u);

    Expect(words == words, "clock words must equal themselves");

    ClockWords different_it_is = words;
    different_it_is.SetShowItIs(!words.GetShowItIs());
    Expect(different_it_is != words, "a different \"it is\" must compare unequal");

    ClockWords different_hour = words;
    ClockWords::HourWordsType hour_words = words.GetHourWords();
    hour_words[0] = (hour_words[0] == DisplayWords::kWordNone) ? DisplayWords::kWordEs
                                                            : DisplayWords::kWordNone;
    different_hour.SetHourWords(hour_words);
    Expect(different_hour != words, "a different hour word must compare unequal");

    ClockWords different_minute = words;
    ClockWords::MinutesWordsType minute_words = words.GetMinuteWords();
    minute_words[0] = (minute_words[0] == DisplayWords::kWordNone) ? DisplayWords::kWordEs
                                                                : DisplayWords::kWordNone;
    different_minute.SetMinuteWords(minute_words);
    Expect(different_minute != words, "a different minute word must compare unequal");
}
