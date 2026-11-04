// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The teletype animation: the words of the new time are written letter by letter.
 */

#ifndef _ANIMATION_TELETYPE_H_
#define _ANIMATION_TELETYPE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationTeletype configuration parameter */

/* AnimationTeletype parameter */

class AnimationTeletype : public Animation
{
  public:

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte current_word_index_{0u};
    byte current_word_length_{0u};
    byte current_char_index_{0u};

    // functions
    void Reset();
    StdReturnType SetNextWordIndex();

  public:
    constexpr AnimationTeletype() : words_() { }
    ~AnimationTeletype() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
