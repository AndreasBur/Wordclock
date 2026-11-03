/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       animation_test.cpp
 *      \brief      That every animation ends on the time it was started for
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Animations.h"
#include "AnimationSnake.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"

/* The one promise every animation makes, and the one that is easy to break: whatever it
   does in between, the display it leaves behind is the new time and nothing else.
   Animation::FinishWithClockWords() exists because that was got wrong once - an animation
   that places letters itself can leave a pixel of the previous time lit, and on a word
   clock that is a word nobody notices is wrong.
   The second promise is that they end at all. Both are checked for every animation there
   is, so an added one is covered by being added to the enumeration. */
/* The flicker makes its effect by switching the display off and on, and it used to end on
   an off: the toggle happens before the decision to stop, so the last one always darkened.
   Asked at the output rather than at the buffer, which is where it hid - the words are in
   the buffer either way, and every existing check looked there. */
void TestFlickerLeavesTheDisplayOn()
{
    Display& display = Display::GetInstance();
    Animations& animations = Animations::GetInstance();

    display.Enable();
    animations.SetMode(Animations::kModeFixed);
    animations.SetAnimation(Animations::kAnimationIdFlicker);

    SetTime(10u, 5u, 0u);
    DisplayManager::GetInstance().Task();
    /* Well past the flicker's own count, so the animation has certainly finished. */
    for(byte tick = 0u; tick < 64u; tick++) { animations.Task(true); }

    Expect(IsAnyOutputPixelLit(), "a finished flicker must leave the display switched on");

    animations.SetAnimation(Animations::kAnimationIdNone);
}

void TestEveryAnimationEndsOnTheNewTime()
{
    /* A generous bound rather than a tight one: what it is here for is a runaway, not a
       measurement. The longest of them walks all 110 pixels several times over. */
    constexpr int kTickLimit{5000};

    Clock::GetInstance().SetMode(Clock::kModeWessi);
    Animations& animations = Animations::GetInstance();
    animations.SetMode(Animations::kModeFixed);

    /* Two times whose word sets differ in every part: hour, minute and the "it is". */
    const PixelBufferType target = DrawClockFace(10u, 35u);

    for(byte id = 1u; id < Animations::kAnimationIdNumberOfAnimations; id++) {
        const Animations::AnimationIdType animation_id = static_cast<Animations::AnimationIdType>(id);
        char description[64];

        /* The display holds the previous time when a word change starts, which is what
           the animations transition away from. */
        DrawClockFace(10u, 4u);
        animations.SetAnimation(animation_id);
        animations.SetTime(10u, 35u);

        int ticks = 0;
        while((animations.GetState() == Animations::kStatePending) && (ticks < kTickLimit)) {
            animations.Task(true);
            ticks++;
        }

        {
            const int length = snprintf(description, sizeof(description), "animation %u must come to an end", id);
            Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
                   "test description must fit its buffer");
        }
        Expect(ticks < kTickLimit, description);

        {
            const int length = snprintf(description, sizeof(description), "animation %u must leave the new time behind", id);
            Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
                   "test description must fit its buffer");
        }
        Expect(ArePixelsEqual(ReadPixels(), target), description);
    }

    animations.SetAnimation(Animations::kAnimationIdNone);
}

/* Stepping the selection, which is the only way a control with no list reaches an
   animation. Three things have to hold, and the third is why this does not reuse the walk
   kModeSequence already had: that one visits the favourites only, and a knob has to reach
   an animation that is not one. */
void TestAnimationStepsThroughEveryOne()
{
    Animations& animations = Animations::GetInstance();
    animations.SetMode(Animations::kModeFixed);
    animations.SetAnimation(Animations::kAnimationIdNone);

    /* The round closes, and "no animation" is part of it - a knob is the only way back to
       it for somebody without a phone. */
    bool seen[Animations::kAnimationIdNumberOfAnimations]{};
    for(byte step = 0u; step < Animations::kAnimationIdNumberOfAnimations; step++) {
        seen[animations.GetAnimation()] = true;
        animations.NextAnimation();
    }

    bool all_seen = true;
    for(const bool was_seen : seen) { if(!was_seen) { all_seen = false; } }
    Expect(all_seen, "stepping must reach every animation, and \"none\" among them");
    Expect(animations.GetAnimation() == Animations::kAnimationIdNone,
           "and must wrap back to the one it started on");

    /* One back is one forward undone, at the end where the wrapping happens. */
    animations.PreviousAnimation();
    Expect(animations.GetAnimation() == Animations::kAnimationIdNumberOfAnimations - 1u,
           "one back from \"none\" must be the last animation");
    animations.NextAnimation();
    Expect(animations.GetAnimation() == Animations::kAnimationIdNone,
           "and one forward again must undo it");

    /* An animation that no mode would pick has to be reachable by hand. */
    const Animations::AnimationIdType not_a_favourite = Animations::kAnimationIdCursor;
    Expect(animations.SetFavourite(not_a_favourite, false) == kEOk,
           "dropping one favourite of many must be allowed");

    animations.SetAnimation(Animations::kAnimationIdNone);
    animations.NextAnimation();
    Expect(animations.GetAnimation() == not_a_favourite,
           "stepping must reach an animation that is not a favourite");

    Expect(animations.SetFavourite(not_a_favourite, true) == kEOk, "and it must go back to being one");
    animations.SetAnimation(Animations::kAnimationIdNone);
}

void TestSnakeLifetime()
{
    /* Automatic storage exercises the destructor that the long-lived animation pool
       does not call during a normal firmware run. */
    AnimationSnake snake;
    snake.Init();
    Expect(snake.GetState() == Animation::kStateIdle, "a fresh snake animation must be idle");
}
