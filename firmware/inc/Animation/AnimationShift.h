// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The shift animation: the old time shifts out of the display, then the new one shifts in.
 */

#ifndef _ANIMATION_SHIFT_H_
#define _ANIMATION_SHIFT_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Transformation.h"
#include "Animation.h"

/* AnimationShift configuration parameter */
#define ANIMATION_SHIFT_HORIZONTAL              STD_ON
#define ANIMATION_SHIFT_VERTICAL                STD_OFF

#if (ANIMATION_SHIFT_HORIZONTAL == STD_OFF && ANIMATION_SHIFT_VERTICAL == STD_OFF)
# error "AnimationShift: Exactly one switch must be set to STD_ON"
#endif

#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON && ANIMATION_SHIFT_VERTICAL == STD_ON)
# error "AnimationShift: ANIMATION_SHIFT_HORIZONTAL or ANIMATION_SHIFT_VERTICAL has to be STD_ON, but not both"
#endif

/* AnimationShift parameter */

class AnimationShift : public Animation
{
  public:

  private:
    Transformation wc_transformation_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};

#if (ANIMATION_SHIFT_HORIZONTAL == STD_ON)
    byte current_column_{0u};
#endif

#if (ANIMATION_SHIFT_VERTICAL == STD_ON)
    byte current_row_{0u};
#endif

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

  public:
    constexpr AnimationShift() : wc_transformation_() { }
    ~AnimationShift() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
