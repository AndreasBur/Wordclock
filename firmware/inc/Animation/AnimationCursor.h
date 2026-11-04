// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The cursor animation: a single pixel walks across all LEDs and leaves the new time behind.
 */

#ifndef _ANIMATION_CURSOR_H_
#define _ANIMATION_CURSOR_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationCursor configuration parameter */

/* AnimationCursor parameter */

class AnimationCursor : public Animation
{
  public:

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte current_pixel_index_{0u};

    // functions
    void Reset();

  public:
    constexpr AnimationCursor() { }
    ~AnimationCursor() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
