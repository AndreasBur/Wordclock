// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

// That every animation ends on the time it was started for.

#include "AnimationSnake.h"
#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Pixels.h"
#include "RealTimeClock.h"
#include "test/cases.h"
#include "test/check.h"

/*
 * The one promise every animation makes, and the one that is easy to break: whatever it
 * does in between, the display it leaves behind is the new time and nothing else.
 * Animation::FinishWithClockWords() exists because that was got wrong once - an animation
 * that places letters itself can leave a pixel of the previous time lit, and on a word
 * clock that is a word nobody notices is wrong.
 * The second promise is that they end at all. Both are checked for every animation there
 * is, so an added one is covered by being added to the enumeration.
 */
/*
 * The flicker makes its effect by switching the display off and on, and it used to end on
 * an off: the toggle happens before the decision to stop, so the last one always darkened.
 * Asked at the output rather than at the buffer, which is where it hid - the words are in
 * the buffer either way, and every existing check looked there.
 */
void TestFlickerLeavesTheDisplayOn() {
  Display& display = Display::GetInstance();
  Animations& animations = Animations::GetInstance();

  display.Enable();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kFlicker);

  SetTime(10u, 5u, 0u);
  DisplayManager::GetInstance().Task();
  /* Well past the flicker's own count, so the animation has certainly finished. */
  for (byte tick = 0u; tick < 64u; tick++) {
    animations.Task(true);
  }

  Expect(IsAnyOutputPixelLit(), "a finished flicker must leave the display switched on");

  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

void TestEveryAnimationEndsOnTheNewTime() {
  // A generous bound rather than a tight one: what it is here for is a runaway, not a
  // measurement. The longest of them walks all 110 pixels several times over.
  constexpr int kTickLimit{5000};

  Clock::GetInstance().SetMode(Clock::ModeType::kWessi);
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);

  /* Two times whose word sets differ in every part: hour, minute and the "it is". */
  PixelBufferType const target = DrawClockFace(10u, 35u);

  for (byte id = 1u; id < Animations::kNumberOfAnimations; id++) {
    Animations::AnimationIdType const animation_id = static_cast<Animations::AnimationIdType>(id);
    char description[64];

    // The display holds the previous time when a word change starts, which is what
    // the animations transition away from.
    DrawClockFace(10u, 4u);
    animations.SetAnimation(animation_id);
    animations.SetTime(10u, 35u);

    int ticks = 0;
    while ((animations.GetState() == Animations::StateType::kPending) && (ticks < kTickLimit)) {
      animations.Task(true);
      ticks++;
    }

    {
      int const length =
          snprintf(description, sizeof(description), "animation %u must come to an end", id);
      Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
             "test description must fit its buffer");
    }
    Expect(ticks < kTickLimit, description);

    {
      int const length = snprintf(description, sizeof(description),
                                  "animation %u must leave the new time behind", id);
      Expect(length >= 0 && static_cast<size_t>(length) < sizeof(description),
             "test description must fit its buffer");
    }
    Expect(ArePixelsEqual(ReadPixels(), target), description);
  }

  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

/*
 * Stepping the selection, which is the only way a control with no list reaches an
 * animation. Three things have to hold, and the third is why this does not reuse the walk
 * ModeType::kSequence already had: that one visits the favourites only, and a knob has to reach
 * an animation that is not one.
 */
void TestAnimationStepsThroughEveryOne() {
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kNone);

  // The round closes, and "no animation" is part of it - a knob is the only way back to
  // it for somebody without a phone.
  bool seen[Animations::kNumberOfAnimations]{};
  for (byte step = 0u; step < Animations::kNumberOfAnimations; step++) {
    seen[ToUnderlying(animations.GetAnimation())] = true;
    animations.NextAnimation();
  }

  bool all_seen = true;
  for (bool const was_seen : seen) {
    if (!was_seen) {
      all_seen = false;
    }
  }
  Expect(all_seen, "stepping must reach every animation, and \"none\" among them");
  Expect(animations.GetAnimation() == Animations::AnimationIdType::kNone,
         "and must wrap back to the one it started on");

  /* One back is one forward undone, at the end where the wrapping happens. */
  animations.PreviousAnimation();
  Expect(ToUnderlying(animations.GetAnimation()) == Animations::kNumberOfAnimations - 1u,
         "one back from \"none\" must be the last animation");
  animations.NextAnimation();
  Expect(animations.GetAnimation() == Animations::AnimationIdType::kNone,
         "and one forward again must undo it");

  /* An animation that no mode would pick has to be reachable by hand. */
  Animations::AnimationIdType const not_a_favourite = FirstAnimation();
  Expect(animations.SetFavourite(not_a_favourite, false) == kEOk,
         "dropping one favourite of many must be allowed");

  animations.SetAnimation(Animations::AnimationIdType::kNone);
  animations.NextAnimation();
  Expect(animations.GetAnimation() == not_a_favourite,
         "stepping must reach an animation that is not a favourite");

  Expect(animations.SetFavourite(not_a_favourite, true) == kEOk,
         "and it must go back to being one");
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

void TestSnakeLifetime() {
  // Automatic storage exercises the destructor that the long-lived animation pool
  // does not call during a normal firmware run.
  AnimationSnake snake;
  snake.Init();
  Expect(snake.GetState() == Animation::StateType::kIdle, "a fresh snake animation must be idle");
}

namespace {

// Runs the current animation to its end, and says how many steps it took.
int RunAnimationToItsEnd() {
  Animations& animations = Animations::GetInstance();
  int ticks = 0;

  while ((animations.GetState() == Animations::StateType::kPending) && (ticks < 5000)) {
    animations.Task(true);
    ticks++;
  }
  return ticks;
}

}  // namespace

/*
 * A second run in mode kFixed, with no SetAnimation() between the two, against the same
 * transition run fresh. The facade used to initialise only when a selecting mode picked, so in
 * kFixed an animation kept the counters of its last run: Shift, Wipe, Squeeze and Explode words
 * took half their steps from the second word change on, which is every word change but the
 * first on a clock that has one animation selected. The test above selects before every run.
 */
void TestFixedModeRunsEveryTimeAsTheFirst() {
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);
  PixelBufferType const target = DrawClockFace(10u, 4u);

  for (byte id = 1u; id < Animations::kNumberOfAnimations; id++) {
    Animations::AnimationIdType const animation_id = static_cast<Animations::AnimationIdType>(id);

    DrawClockFace(10u, 4u);
    animations.SetAnimation(animation_id);
    animations.SetTime(10u, 35u);
    RunAnimationToItsEnd();
    animations.SetTime(10u, 4u);
    int const second = RunAnimationToItsEnd();
    bool const second_ends_on_the_time = ArePixelsEqual(ReadPixels(), target);

    DrawClockFace(10u, 35u);
    animations.SetAnimation(animation_id);
    animations.SetTime(10u, 4u);
    int const fresh = RunAnimationToItsEnd();

    Expect(second == fresh && second_ends_on_the_time,
           "a second run in mode kFixed must take the steps of a fresh one and end on the time");
  }
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

// Every animation reports a start from idle as one: Drop and Shift reported kENotOk, which RPC
// 23 hands on as a failure for an animation that runs.
void TestEveryAnimationReportsItsStart() {
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);

  for (byte id = 1u; id < Animations::kNumberOfAnimations; id++) {
    DrawClockFace(10u, 4u);
    animations.SetAnimation(static_cast<Animations::AnimationIdType>(id));
    Expect(animations.SetTime(10u, 35u) == kEOk, "an animation that starts must say so");
    RunAnimationToItsEnd();
  }
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

/*
 * A display switched off - by hand, or by the night switch - stays off through the flicker.
 * Its Init() and every other step switched the display on, and Init() runs whenever it is
 * selected or picked; at night the night switch then gave the supply back as well.
 */
void TestFlickerLeavesADarkDisplayDark() {
  Display& display = Display::GetInstance();
  Animations& animations = Animations::GetInstance();

  DrawClockFace(10u, 4u);
  display.Disable();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kFlicker);
  Expect(!display.IsEnabled(), "selecting the flicker must leave a dark display dark");

  animations.SetTime(10u, 35u);
  bool ever_enabled = false;
  while (animations.GetState() == Animations::StateType::kPending) {
    animations.Task(true);
    ever_enabled = ever_enabled || display.IsEnabled();
  }
  Expect(!ever_enabled && !display.IsEnabled(), "the flicker must not switch a dark display on");

  display.Enable();
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

/*
 * Modes 1 and 2 animate with animation 0 selected, which is how a clock comes: -A and -M are
 * independent. DisplayManager asked for the selection alone and drew the time without one.
 */
void TestSelectingModesAnimateWithNothingSelected() {
  Animations& animations = Animations::GetInstance();

  animations.SetAnimation(Animations::AnimationIdType::kNone);
  animations.SetMode(Animations::ModeType::kRandom);
  SetTime(7u, 50u, 0u);
  DisplayManager::GetInstance().Task();
  Expect(animations.GetState() == Animations::StateType::kPending,
         "a word change in mode 1 must start an animation with animation 0 selected");

  RunAnimationToItsEnd();
  animations.SetMode(Animations::ModeType::kFixed);
}

/*
 * Leaving a fade half way through puts the display's brightness back. The facade overwrote the
 * running fade with the new animation, and only the fade's own Init() clears the fade level.
 */
void TestLeavingAFadeUndimsTheDisplay() {
  Animations& animations = Animations::GetInstance();
  Pixels& pixels = Pixels::GetInstance();

  DrawClockFace(10u, 4u);
  byte lit = 0u;
  while ((lit < PIXELS_NUMBER_OF_PIXELS) && (pixels.GetPixel(lit).GetRed() == 0u)) {
    lit++;
  }
  Pixel::ColorType const before = pixels.GetPixel(lit).GetRed();

  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kFade);
  animations.SetTime(10u, 35u);
  for (byte tick = 0u; tick < 32u; tick++) {
    animations.Task(true);
  }
  Pixel::ColorType const faded = pixels.GetPixel(lit).GetRed();

  animations.SetAnimation(Animations::AnimationIdType::kNone);
  Expect(faded < before && pixels.GetPixel(lit).GetRed() == before,
         "leaving a running fade must put the brightness back");
}

/*
 * A word change during a fade or a flicker runs it on and ends it on the newer time, as every
 * other animation does. The two started over, a fade halfway down coming back up from full.
 */
void TestAWordChangeRunsAFadeOrFlickerOn() {
  Animations& animations = Animations::GetInstance();
  animations.SetMode(Animations::ModeType::kFixed);
  Animations::AnimationIdType const ids[]{Animations::AnimationIdType::kFade,
                                          Animations::AnimationIdType::kFlicker};

  for (Animations::AnimationIdType const id : ids) {
    PixelBufferType const target = DrawClockFace(10u, 40u);

    DrawClockFace(10u, 4u);
    animations.SetAnimation(id);
    animations.SetTime(10u, 35u);
    int const fresh = RunAnimationToItsEnd();

    DrawClockFace(10u, 4u);
    animations.SetAnimation(id);
    animations.SetTime(10u, 35u);
    for (byte tick = 0u; tick < 3u; tick++) {
      animations.Task(true);
    }
    bool const refused = animations.SetTime(10u, 40u) == kENotOk;
    int const interrupted = 3 + RunAnimationToItsEnd();

    Expect(refused && interrupted == fresh && ArePixelsEqual(ReadPixels(), target),
           "a word change must run a fade or flicker on and end it on the newer time");
  }
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}

// Explode words on a dark display lights its first word on its first step. It used to start
// on a word of no length, and nothing was lit until that had travelled for nothing.
void TestExplodeWordsLightsItsFirstWordOnADarkDisplay() {
  Animations& animations = Animations::GetInstance();
  PixelBufferType const target = DrawClockFace(10u, 35u);

  Display::GetInstance().Clear();
  animations.SetMode(Animations::ModeType::kFixed);
  animations.SetAnimation(Animations::AnimationIdType::kExplodeWords);
  animations.SetTime(10u, 35u);
  animations.Task(true);
  Expect(IsAnyPixelLit(ReadPixels()),
         "explode words must light its first word at once on a dark display");

  RunAnimationToItsEnd();
  Expect(ArePixelsEqual(ReadPixels(), target), "and end on the time");
  animations.SetAnimation(Animations::AnimationIdType::kNone);
}
