// Copyright 2021 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the animations facade does itself.
 */

#include "Animations.h"

#include "Clock.h"
#include "Display.h"

Animations::StateType Animations::GetState() const {
  if (GetStateOfCurrentAnimation() == Animation::StateType::kClearTime ||
      GetStateOfCurrentAnimation() == Animation::StateType::kSetTime) {
    return StateType::kPending;
  }

  return StateType::kIdle;
}

void Animations::Task(bool show) {
  if (GetStateOfCurrentAnimation() == Animation::StateType::kClearTime ||
      GetStateOfCurrentAnimation() == Animation::StateType::kSetTime) {
    TaskOfCurrentAnimation();
    if (show) {
      ShowOfCurrentAnimation();
    }
  }
}

StdReturnType Animations::Show() const {
  if (GetStateOfCurrentAnimation() == Animation::StateType::kClearTime ||
      GetStateOfCurrentAnimation() == Animation::StateType::kSetTime) {
    return ShowOfCurrentAnimation();
  }

  return kENotOk;
}

StdReturnType Animations::SetTaskCycle(AnimationIdType animation_id, byte cycle) {
  if (IsAnimationValid(animation_id)) {
    task_cycles_[ToUnderlying(animation_id)] = cycle;
    return kEOk;
  } else {
    return kENotOk;
  }
}

StdReturnType Animations::SetAnimation(AnimationIdType animation_id) {
  if (!IsAnimationValid(animation_id)) {
    return kENotOk;
  }

  animation_id_ = animation_id;
  // the selected animation becomes the current one right away, even in a mode that selects
  // on its own; that mode picks again on the next word change
  SwitchCurrentAnimation(animation_id_);
  return kEOk;
}

StdReturnType Animations::SetMode(ModeType mode) {
  if (!IsModeValid(mode)) {
    return kENotOk;
  }

  mode_ = mode;

  // Back to the selected animation. The selecting modes leave the current animation
  // alone and take over on the next word change.
  if (mode_ == ModeType::kFixed) {
    SwitchCurrentAnimation(animation_id_);
  }
  return kEOk;
}

void Animations::InitCurrentAnimation() {
  switch (current_animation_id_) {
#if (ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    case AnimationIdType::kCursor:
      return animations_raw_.cursor.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    case AnimationIdType::kTeletype:
      return animations_raw_.teletype.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_DROP == STD_ON)
    case AnimationIdType::kDrop:
      return animations_raw_.drop.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    case AnimationIdType::kShift:
      return animations_raw_.shift.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FADE == STD_ON)
    case AnimationIdType::kFade:
      return animations_raw_.fade.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    case AnimationIdType::kSnake:
      return animations_raw_.snake.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_WIPE == STD_ON)
    case AnimationIdType::kWipe:
      return animations_raw_.wipe.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_CUBE == STD_ON)
    case AnimationIdType::kCube:
      return animations_raw_.cube.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    case AnimationIdType::kFlicker:
      return animations_raw_.flicker.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    case AnimationIdType::kSqueeze:
      return animations_raw_.squeeze.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    case AnimationIdType::kExplodePixels:
      return animations_raw_.explode_pixels.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    case AnimationIdType::kExplodeWords:
      return animations_raw_.explode_words.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    case AnimationIdType::kMatrix:
      return animations_raw_.matrix.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
    case AnimationIdType::kRoll:
      return animations_raw_.roll.Init();
      break;
#endif
#if (ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    case AnimationIdType::kCollapse:
      return animations_raw_.collapse.Init();
      break;
#endif
    default:
      current_animation_id_ = AnimationIdType::kNone;
      break;
  }
}

StdReturnType Animations::SetTime(byte hour, byte minute) {
  /*
   * Only while nothing is running, so a word change during a still running animation starts
   * nothing here; the running animation gets the time and ends on it. A new run starts from
   * a freshly initialised animation in every mode: an animation keeps its counters where its
   * last run left them, and Shift, Wipe, Squeeze and Explode words took half their steps on
   * every run after the first while mode kFixed skipped this.
   */
  if (GetStateOfCurrentAnimation() == Animation::StateType::kIdle) {
    if (mode_ == ModeType::kFixed) {
      InitCurrentAnimation();
    } else {
      SelectAnimationOfMode();
    }
  }

  switch (current_animation_id_) {
    case AnimationIdType::kNone:
      return Clock::GetInstance().SetTime(hour, minute);
      break;
#if (ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    case AnimationIdType::kCursor:
      return animations_raw_.cursor.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    case AnimationIdType::kTeletype:
      return animations_raw_.teletype.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_DROP == STD_ON)
    case AnimationIdType::kDrop:
      return animations_raw_.drop.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    case AnimationIdType::kShift:
      return animations_raw_.shift.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_FADE == STD_ON)
    case AnimationIdType::kFade:
      return animations_raw_.fade.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    case AnimationIdType::kSnake:
      return animations_raw_.snake.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_WIPE == STD_ON)
    case AnimationIdType::kWipe:
      return animations_raw_.wipe.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_CUBE == STD_ON)
    case AnimationIdType::kCube:
      return animations_raw_.cube.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    case AnimationIdType::kFlicker:
      return animations_raw_.flicker.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    case AnimationIdType::kSqueeze:
      return animations_raw_.squeeze.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    case AnimationIdType::kExplodePixels:
      return animations_raw_.explode_pixels.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    case AnimationIdType::kExplodeWords:
      return animations_raw_.explode_words.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    case AnimationIdType::kMatrix:
      return animations_raw_.matrix.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
    case AnimationIdType::kRoll:
      return animations_raw_.roll.SetTime(hour, minute);
      break;
#endif
#if (ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    case AnimationIdType::kCollapse:
      return animations_raw_.collapse.SetTime(hour, minute);
      break;
#endif
    default:
      return kENotOk;
      break;
  }
}

Animation::StateType Animations::GetStateOfCurrentAnimation() const {
  switch (current_animation_id_) {
    case AnimationIdType::kNone:
      return Animation::StateType::kIdle;
      break;
#if (ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    case AnimationIdType::kCursor:
      return animations_raw_.cursor.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    case AnimationIdType::kTeletype:
      return animations_raw_.teletype.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_DROP == STD_ON)
    case AnimationIdType::kDrop:
      return animations_raw_.drop.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    case AnimationIdType::kShift:
      return animations_raw_.shift.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FADE == STD_ON)
    case AnimationIdType::kFade:
      return animations_raw_.fade.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    case AnimationIdType::kSnake:
      return animations_raw_.snake.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_WIPE == STD_ON)
    case AnimationIdType::kWipe:
      return animations_raw_.wipe.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_CUBE == STD_ON)
    case AnimationIdType::kCube:
      return animations_raw_.cube.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    case AnimationIdType::kFlicker:
      return animations_raw_.flicker.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    case AnimationIdType::kSqueeze:
      return animations_raw_.squeeze.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    case AnimationIdType::kExplodePixels:
      return animations_raw_.explode_pixels.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    case AnimationIdType::kExplodeWords:
      return animations_raw_.explode_words.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    case AnimationIdType::kMatrix:
      return animations_raw_.matrix.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
    case AnimationIdType::kRoll:
      return animations_raw_.roll.GetState();
      break;
#endif
#if (ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    case AnimationIdType::kCollapse:
      return animations_raw_.collapse.GetState();
      break;
#endif
    default:
      return Animation::StateType::kNone;
      break;
  }
}

void Animations::TaskOfCurrentAnimation() {
  switch (current_animation_id_) {
    case AnimationIdType::kNone:
      break;
#if (ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    case AnimationIdType::kCursor:
      animations_raw_.cursor.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    case AnimationIdType::kTeletype:
      animations_raw_.teletype.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_DROP == STD_ON)
    case AnimationIdType::kDrop:
      animations_raw_.drop.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    case AnimationIdType::kShift:
      animations_raw_.shift.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FADE == STD_ON)
    case AnimationIdType::kFade:
      animations_raw_.fade.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    case AnimationIdType::kSnake:
      animations_raw_.snake.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_WIPE == STD_ON)
    case AnimationIdType::kWipe:
      animations_raw_.wipe.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_CUBE == STD_ON)
    case AnimationIdType::kCube:
      animations_raw_.cube.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    case AnimationIdType::kFlicker:
      animations_raw_.flicker.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    case AnimationIdType::kSqueeze:
      animations_raw_.squeeze.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    case AnimationIdType::kExplodePixels:
      animations_raw_.explode_pixels.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    case AnimationIdType::kExplodeWords:
      animations_raw_.explode_words.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    case AnimationIdType::kMatrix:
      animations_raw_.matrix.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
    case AnimationIdType::kRoll:
      animations_raw_.roll.Task();
      break;
#endif
#if (ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    case AnimationIdType::kCollapse:
      animations_raw_.collapse.Task();
      break;
#endif
    default:
      break;
  }
}

StdReturnType Animations::ShowOfCurrentAnimation() const {
  switch (current_animation_id_) {
    case AnimationIdType::kNone:
      return Display::GetInstance().Show();
      break;
#if (ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    case AnimationIdType::kCursor:
      return animations_raw_.cursor.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    case AnimationIdType::kTeletype:
      return animations_raw_.teletype.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_DROP == STD_ON)
    case AnimationIdType::kDrop:
      return animations_raw_.drop.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    case AnimationIdType::kShift:
      return animations_raw_.shift.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FADE == STD_ON)
    case AnimationIdType::kFade:
      return animations_raw_.fade.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    case AnimationIdType::kSnake:
      return animations_raw_.snake.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_WIPE == STD_ON)
    case AnimationIdType::kWipe:
      return animations_raw_.wipe.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_CUBE == STD_ON)
    case AnimationIdType::kCube:
      return animations_raw_.cube.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    case AnimationIdType::kFlicker:
      return animations_raw_.flicker.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    case AnimationIdType::kSqueeze:
      return animations_raw_.squeeze.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    case AnimationIdType::kExplodePixels:
      return animations_raw_.explode_pixels.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    case AnimationIdType::kExplodeWords:
      return animations_raw_.explode_words.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    case AnimationIdType::kMatrix:
      return animations_raw_.matrix.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
    case AnimationIdType::kRoll:
      return animations_raw_.roll.Show();
      break;
#endif
#if (ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    case AnimationIdType::kCollapse:
      return animations_raw_.collapse.Show();
      break;
#endif
    default:
      return kENotOk;
      break;
  }
}

void Animations::SelectAnimationOfMode() {
  switch (mode_) {
    case ModeType::kRandom:
      SwitchCurrentAnimation(CalcRandomAnimation());
      break;
    case ModeType::kSequence:
      SwitchCurrentAnimation(CalcNextAnimation());
      break;
    default:
      break;
  }
}

/*
 * The current animation is initialised before it is left rather than overwritten: Fade has
 * set the display's fade level and Flicker may have switched the display off, and only their
 * own Init() puts that back - an animation left half way through SetAnimation() kept
 * the display dimmed. The new one shares its memory with the previous one, so it is
 * initialised before it is used.
 */
void Animations::SwitchCurrentAnimation(AnimationIdType animation_id) {
  InitCurrentAnimation();
  current_animation_id_ = animation_id;
  InitCurrentAnimation();
}

Animations::AnimationIdType Animations::CalcRandomAnimation() {
  /*
   * Advancing state instead of deriving it from the time: the same animation would
   * otherwise always run at the same minute of every day. Both halves of the state
   * go into the hash, otherwise the order of animations would repeat every 256
   * minutes instead of every 65536.
   */
  random_state_++;

  byte hash = static_cast<byte>((random_state_ ^ (random_state_ >> 8u)) * kRandomHashFactor);
  hash ^= static_cast<byte>(hash >> 4u);
  hash = static_cast<byte>(hash * kRandomHashFactor);

  /*
   * Drawn among the favourites only, so the draw counts them and then walks to the
   * one it landed on. Scaled by multiplication instead of a modulo, which would
   * favour the first animations because 256 is not a multiple of their number.
   */
  byte offset = static_cast<byte>((static_cast<uint16_t>(hash) * NumberOfFavourites()) >> 8u);

  for (byte animation_id = kFirstAnimation; animation_id < kNumberOfAnimations; animation_id++) {
    if (IsFavourite(static_cast<AnimationIdType>(animation_id))) {
      if (offset == 0u) {
        return static_cast<AnimationIdType>(animation_id);
      }
      offset--;
    }
  }

  /* unreachable while at least one favourite is set, which SetFavourite() enforces */
  return animation_id_;
}

Animations::AnimationIdType Animations::CalcNextAnimation() const {
  byte next = ToUnderlying(current_animation_id_);

  // Walks on until it finds a favourite. It cannot loop forever, because at least
  // one favourite is always set and there are only that many animations to try.
  for (byte tries = 0u; tries < kNumberOfSelectableAnimations; tries++) {
    /* none as the running animation means nothing ran yet, so start at the first */
    if (next == ToUnderlying(AnimationIdType::kNone) || next + 1u >= kNumberOfAnimations) {
      next = kFirstAnimation;
    } else {
      next++;
    }

    if (IsFavourite(static_cast<AnimationIdType>(next))) {
      return static_cast<AnimationIdType>(next);
    }
  }

  return animation_id_;
}

byte Animations::NumberOfFavourites() const {
  byte count{0u};

  for (byte animation_id = kFirstAnimation; animation_id < kNumberOfAnimations; animation_id++) {
    if (IsFavourite(static_cast<AnimationIdType>(animation_id))) {
      count++;
    }
  }

  return count;
}
