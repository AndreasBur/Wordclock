/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       animation_test.cpp
 *      \brief      That every animation ends on the time it was started for
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"

/* The one promise every animation makes, and the one that is easy to break: whatever it
   does in between, the display it leaves behind is the new time and nothing else.
   Animation::finishWithClockWords() exists because that was got wrong once - an animation
   that places letters itself can leave a pixel of the previous time lit, and on a word
   clock that is a word nobody notices is wrong.
   The second promise is that they end at all. Both are checked for every animation there
   is, so an added one is covered by being added to the enumeration. */
/* The flicker makes its effect by switching the display off and on, and it used to end on
   an off: the toggle happens before the decision to stop, so the last one always darkened.
   Asked at the output rather than at the buffer, which is where it hid - the words are in
   the buffer either way, and every existing check looked there. */
void testFlickerLeavesTheDisplayOn()
{
    Display& display = Display::getInstance();
    Animations& animations = Animations::getInstance();

    display.enable();
    animations.setMode(Animations::MODE_FIXED);
    animations.setAnimation(Animations::ANIMATION_ID_FLICKER);

    setTime(10u, 5u, 0u);
    DisplayManager::getInstance().task();
    /* Well past the flicker's own count, so the animation has certainly finished. */
    for(byte Tick = 0u; Tick < 64u; Tick++) { animations.task(true); }

    expect(isAnyOutputPixelLit(), "a finished flicker must leave the display switched on");

    animations.setAnimation(Animations::ANIMATION_ID_NONE);
}

void testEveryAnimationEndsOnTheNewTime()
{
    /* A generous bound rather than a tight one: what it is here for is a runaway, not a
       measurement. The longest of them walks all 110 pixels several times over. */
    constexpr int TickLimit{5000};

    Clock::getInstance().setMode(Clock::MODE_WESSI);
    Animations& animations = Animations::getInstance();
    animations.setMode(Animations::MODE_FIXED);

    /* Two times whose word sets differ in every part: hour, minute and the "it is". */
    const PixelBufferType Target = drawClockFace(10u, 35u);

    for(byte Id = 1u; Id < Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS; Id++) {
        const Animations::AnimationIdType AnimationId = static_cast<Animations::AnimationIdType>(Id);
        char Description[64];

        /* The display holds the previous time when a word change starts, which is what
           the animations transition away from. */
        drawClockFace(10u, 4u);
        animations.setAnimation(AnimationId);
        animations.setTime(10u, 35u);

        int Ticks = 0;
        while((animations.getState() == Animations::STATE_PENDING) && (Ticks < TickLimit)) {
            animations.task(true);
            Ticks++;
        }

        snprintf(Description, sizeof(Description), "animation %u must come to an end", Id);
        expect(Ticks < TickLimit, Description);

        snprintf(Description, sizeof(Description), "animation %u must leave the new time behind", Id);
        expect(arePixelsEqual(readPixels(), Target), Description);
    }

    animations.setAnimation(Animations::ANIMATION_ID_NONE);
}

/* Stepping the selection, which is the only way a control with no list reaches an
   animation. Three things have to hold, and the third is why this does not reuse the walk
   MODE_SEQUENCE already had: that one visits the favourites only, and a knob has to reach
   an animation that is not one. */
void testAnimationStepsThroughEveryOne()
{
    Animations& animations = Animations::getInstance();
    animations.setMode(Animations::MODE_FIXED);
    animations.setAnimation(Animations::ANIMATION_ID_NONE);

    /* The round closes, and "no animation" is part of it - a knob is the only way back to
       it for somebody without a phone. */
    bool seen[Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS]{};
    for(byte Step = 0u; Step < Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS; Step++) {
        seen[animations.getAnimation()] = true;
        animations.nextAnimation();
    }

    bool allSeen = true;
    for(const bool Seen : seen) { if(!Seen) { allSeen = false; } }
    expect(allSeen, "stepping must reach every animation, and \"none\" among them");
    expect(animations.getAnimation() == Animations::ANIMATION_ID_NONE,
           "and must wrap back to the one it started on");

    /* One back is one forward undone, at the end where the wrapping happens. */
    animations.previousAnimation();
    expect(animations.getAnimation() == Animations::ANIMATION_ID_NUMBER_OF_ANIMATIONS - 1u,
           "one back from \"none\" must be the last animation");
    animations.nextAnimation();
    expect(animations.getAnimation() == Animations::ANIMATION_ID_NONE,
           "and one forward again must undo it");

    /* An animation that no mode would pick has to be reachable by hand. */
    const Animations::AnimationIdType NotAFavourite = Animations::ANIMATION_ID_CURSOR;
    expect(animations.setFavourite(NotAFavourite, false) == E_OK,
           "dropping one favourite of many must be allowed");

    animations.setAnimation(Animations::ANIMATION_ID_NONE);
    animations.nextAnimation();
    expect(animations.getAnimation() == NotAFavourite,
           "stepping must reach an animation that is not a favourite");

    expect(animations.setFavourite(NotAFavourite, true) == E_OK, "and it must go back to being one");
    animations.setAnimation(Animations::ANIMATION_ID_NONE);
}
