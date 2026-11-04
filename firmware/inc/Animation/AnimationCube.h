// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The cube animation: a rectangle shrinks to the centre and grows back over the new time.
 */

#ifndef _ANIMATION_CUBE_H_
#define _ANIMATION_CUBE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Animation.h"
#include "Clock.h"

/* AnimationCube configuration parameter */

/* AnimationCube parameter */
#define ANIMATION_CUBE_COLUMN_START_MAX_VALUE         4u
#define ANIMATION_CUBE_COLUMN_END_MIN_VALUE           6u

#define ANIMATION_CUBE_ROW_START_MAX_VALUE            4u
#define ANIMATION_CUBE_ROW_END_MIN_VALUE              5u

class AnimationCube : public Animation
{
  public:
    struct BorderType {
        byte column_start;
        byte column_end;
        byte row_start;
        byte row_end;
    };

    enum class BorderStateType {
        kMin,
        kMax,
        kBetween
    };

  private:
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    BorderType border_{ANIMATION_CUBE_COLUMN_START_MAX_VALUE, ANIMATION_CUBE_COLUMN_END_MIN_VALUE,
      ANIMATION_CUBE_ROW_START_MAX_VALUE, ANIMATION_CUBE_ROW_END_MIN_VALUE};
    BorderStateType border_state_{BorderStateType::kMin};

    // functions
    void Reset();
    void SetStateToSetTime();
    void SetBorderPixels() { WriteBorderPixels(true); }
    void ClearBorderPixels() { WriteBorderPixels(false); }
    void ClearBorderPixelsWithoutClockPixels();
    void WriteBorderPixels(bool) const;
    StdReturnType IncreaseBorder();
    StdReturnType DecreaseBorder();
    void ClearTimeTask();
    void SetTimeTask();
    void SetMinBorder();
    void SetMaxBorder();

  public:
    constexpr AnimationCube() { }
    ~AnimationCube() { }

    // get methods

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();

};

#endif
