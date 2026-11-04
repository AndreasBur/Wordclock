// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The roll animation: the old time rolls out while the new one rolls in behind it.
 *
 * Unlike AnimationShift, which first shifts the previous time out of an empty display and only then
 * shifts the new one in, both move together here. The display therefore never goes blank and the
 * animation takes one pass instead of two.
 *
 * The direction is selected for each word change out of right, left, down and up.
 *
 * Ported from the wordclock24h project by Frank Meyer (display_animation_roll in
 * src/display/display.c).
 */

#ifndef _ANIMATION_ROLL_H_
#define _ANIMATION_ROLL_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Transformation.h"
#include "Animation.h"

/* AnimationRoll configuration parameter */

/* AnimationRoll parameter */

class AnimationRoll : public Animation
{
  public:
    enum class DirectionType {
        kRight,
        kLeft,
        kDown,
        kUp,
        kNumberOfDirections
    };
    static constexpr byte kNumberOfDirections{static_cast<byte>(DirectionType::kNumberOfDirections)};

  private:
    Transformation wc_transformation_;
    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte step_{0u};
    DirectionType direction_{DirectionType::kRight};

    // functions
    void Reset();
    void SetTimeTask();
    void RollHorizontal();
    void RollVertical();
    byte NumberOfSteps() const;
    static DirectionType CalcDirection(byte, byte);

  public:
    constexpr AnimationRoll() : wc_transformation_() { }
    ~AnimationRoll() { }

    // get methods
    DirectionType GetDirection() const { return direction_; }

    // set methods

    // methods
    void Init();
    StdReturnType SetTime(byte, byte);
    void Task();
};

#endif
