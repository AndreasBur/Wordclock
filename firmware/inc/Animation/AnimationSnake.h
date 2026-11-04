// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The snake animation: a short snake travels along the strip and leaves the new time behind.
 */

#ifndef _ANIMATION_SNAKE_H_
#define _ANIMATION_SNAKE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/* AnimationSnake configuration parameter */
#define ANIMATION_SNAKE_LENGTH              8u

/* AnimationSnake parameter */

class AnimationSnake : public Animation
{
  public:
    enum class SnakeStateType {
        kToRight,
        kToLeft
    };

  private:
    SnakeStateType snake_state_{SnakeStateType::kToRight};
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte snake_begin_index_{0u};
    byte snake_end_index_{0u};

    // functions
    void Reset();
    static byte TransformToSerpentine(byte, byte);
    static byte TransformToSerpentine(byte);

  public:
    constexpr AnimationSnake() { }
    ~AnimationSnake() = default;

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
