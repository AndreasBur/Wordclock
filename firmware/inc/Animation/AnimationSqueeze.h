// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The squeeze animation: the old words are eaten away letter by letter, the new ones grow.
 */

#ifndef _ANIMATION_SQUEEZE_H_
#define _ANIMATION_SQUEEZE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationSqueeze configuration parameter */

/* AnimationSqueeze parameter */

class AnimationSqueeze : public Animation
{
  public:

  public:
    // Side the letters are eaten away from, selected for each word change. From the right is what
    // the reference project does, from the left is the other half of the coin.
    enum class DirectionType {
        kFromLeft,
        kFromRight,
        kNumberOfDirections
    };
    static constexpr byte kNumberOfDirections{static_cast<byte>(DirectionType::kNumberOfDirections)};

  private:
    DisplayWords words_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte max_word_length_{0u};
    byte current_length_{1u};
    DirectionType direction_{DirectionType::kFromLeft};

    // functions
    void Reset();
    void ClearTimeTask();
    void SetTimeTask();

    void ClearRow(byte);
    void SetMaxWordLength();

  public:
    constexpr AnimationSqueeze() : words_() { }
    ~AnimationSqueeze() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
