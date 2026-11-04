// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The wipe animation: a diagonal line sweeps across the display and uncovers the new time.
 */

#ifndef _ANIMATION_WIPE_H_
#define _ANIMATION_WIPE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationWipe configuration parameter */

/* AnimationWipe parameter */

class AnimationWipe : public Animation
{
  public:
    enum class SetPixelStateType {
        kRight,
        kDown
    };

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte index_{0u};
    SetPixelStateType set_pixel_state_{SetPixelStateType::kDown};

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();
    StdReturnType SetNextIndex();
    static void SetPixelDown(byte, byte);
    static void SetPixelRight(byte, byte);

  public:
    constexpr AnimationWipe() { }
    ~AnimationWipe() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
