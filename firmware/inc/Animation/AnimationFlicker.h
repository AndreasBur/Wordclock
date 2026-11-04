// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The flicker animation: the display flickers a few times, then the new time is there.
 */

#ifndef _ANIMATION_FLICKER_H_
#define _ANIMATION_FLICKER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Animation.h"

/* AnimationFlicker configuration parameter */
#define ANIMATION_FLICKER_COUNTER_INIT_VALUE          4u

/* AnimationFlicker parameter */

class AnimationFlicker : public Animation
{
  public:

  private:
    byte minute_{0u};
    byte hour_{0u};
    bool is_clock_set_{false};
    byte flicker_counter_{ANIMATION_FLICKER_COUNTER_INIT_VALUE};

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

  public:
    constexpr AnimationFlicker() { }
    ~AnimationFlicker() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();

};

#endif
