// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The fade animation: the display dims to dark, the time changes, and it fades back up.
 */

#ifndef _ANIMATION_FADE_H_
#define _ANIMATION_FADE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Animation.h"

/* AnimationFade configuration parameter */

/* AnimationFade parameter */

class AnimationFade : public Animation
{
  public:

  private:
    /*
     * How far the fade has got, in what Display calls a fade level: 255 is the display as
     * configured, 0 is dark. Counted here rather than read back from the display, which
     * scales the level by the setting, the gamma correction and the automatic.
     */
    static constexpr byte kLevelFull{255u};
    byte level_{kLevelFull};
    byte hour_{0u};
    byte minute_{0u};

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();
    void SetStateToSetTime();

  public:
    constexpr AnimationFade() { }
    ~AnimationFade() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
