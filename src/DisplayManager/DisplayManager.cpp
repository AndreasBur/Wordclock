// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Decides what the display shows.
 */

#include "DisplayManager.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "Overlays.h"
#include "RealTimeClock.h"

bool DisplayManager::IsOverlayShowing() {
  return Overlays::GetInstance().GetState() == Overlays::OverlayType::StateType::kShow;
}

void DisplayManager::ShowClock(HourType hour, MinuteType minute) {
  Display::GetInstance().Clear();
  Clock::GetInstance().SetTime(hour, minute);
  Display::GetInstance().Show();
}

void DisplayManager::TaskClockWordsChange(HourType hour, MinuteType minute) {
  Animations& animations = Animations::GetInstance();
  // The mode first: a selecting mode animates whatever animation is selected; a selected
  // 0 means "no animation" only in -M0.
  bool const is_animated{animations.GetMode() != Animations::ModeType::kFixed ||
                         animations.GetAnimation() != Animations::AnimationIdType::kNone};
  if (!is_animated) {
    ShowClock(hour, minute);
  } else {
    animations.SetTime(hour, minute);
  }
}

bool DisplayManager::UpdateClockWords(HourType hour, MinuteType minute) {
  ClockWords clock_words;
  if (Clock::GetInstance().GetClockWords(hour, minute, clock_words) == kENotOk) {
    return false;
  }

  if (clock_words_initialized_ && clock_words == last_clock_words_) {
    return false;
  }

  last_clock_words_ = clock_words;
  clock_words_initialized_ = true;
  return true;
}

void DisplayManager::Task() {
  Display::GetInstance().ApplyBrightness();

  ClockTime time = RealTimeClock::GetInstance().GetTime();

  // An overlay owns the display while it shows: Text draws its letters there and the
  // clock must not write over them.
  if (IsOverlayShowing()) {
    overlay_was_showing_ = true;
    return;
  }

  /*
   * The overlay has ended. Nothing else brings the clock back — Overlay's transition
   * to idle only stops the Text — so without this the grid would keep the last
   * overlay frame until the next word change, which can be nearly five minutes away.
   * Updating last_clock_words_ keeps that change from drawing a second time.
   */
  if (overlay_was_showing_) {
    overlay_was_showing_ = false;
    UpdateClockWords(time.GetHour(), time.GetMinute());
    ShowClock(time.GetHour(), time.GetMinute());
    return;
  }

  if (UpdateClockWords(time.GetHour(), time.GetMinute())) {
    TaskClockWordsChange(time.GetHour(), time.GetMinute());
  }
}

StdReturnType DisplayManager::RefreshClock() {
  if (IsOverlayShowing()) {
    return kENotOk;
  }

  ClockTime const time = RealTimeClock::GetInstance().GetTime();
  UpdateClockWords(time.GetHour(), time.GetMinute());
  ShowClock(time.GetHour(), time.GetMinute());
  return kEOk;
}

StdReturnType DisplayManager::StartAnimation() {
  if (IsOverlayShowing()) {
    return kENotOk;
  }

  ClockTime const time = RealTimeClock::GetInstance().GetTime();
  return Animations::GetInstance().SetTime(time.GetHour(), time.GetMinute());
}

StdReturnType DisplayManager::AbortAnimation() {
  Animations::GetInstance().Abort();
  return RefreshClock();
}
