// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The drop animation: the letters of the new time fall down their column into place.
 */

#ifndef _ANIMATION_DROP_H_
#define _ANIMATION_DROP_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationDrop configuration parameter */

/* AnimationDrop parameter */

class AnimationDrop : public Animation
{
  public:

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte row_{0u};
    byte column_{0u};
    byte current_word_index_{0u};

    // functions
    void Reset();
    StdReturnType SetNextActivePixelIndex();
    void ClearTimeTask();
    void SetTimeTask();
    StdReturnType SetNextWordIndex();
    void SetStateToSetTime();
    StdReturnType SetNextRow(byte);
    StdReturnType SetNextColumn(byte);

  public:
    constexpr AnimationDrop() : words_() { }
    ~AnimationDrop() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
