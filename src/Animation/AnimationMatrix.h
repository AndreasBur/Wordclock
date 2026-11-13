// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The matrix animation: falling code rain that leaves the new time behind.
 *
 * One drop falls per column. All drops move at the same speed, the rain pattern comes from their
 * different start delays only. Letters of the new time light up at full intensity once the drop has
 * passed them far enough, so the time appears out of the rain.
 *
 * Everything is drawn in the display color and differs in intensity only, so the display keeps one
 * uniform color. Set the display color to green for the look of the movie.
 *
 * Ported from the wordclock24h project by Frank Meyer (display_animation_generic_matrix in
 * src/display/display.c), which uses a yellow head and green rain regardless of the display color.
 */

#ifndef WORDCLOCK_SRC_ANIMATION_ANIMATIONMATRIX_H_
#define WORDCLOCK_SRC_ANIMATION_ANIMATIONMATRIX_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ClockWords.h"
#include "Animation.h"
#include "Display.h"

/*!
 * \brief The matrix animation: falling code rain in the display color; the new time stays behind as
 *        the drops pass over its letters.
 *
 * One drop falls per column, one row per step, with a trail above its head that fades with the
 * distance. Each drop starts at a step between the first and the
 * DISPLAY_NUMBER_OF_ROWS-th, which SetTime() derives from the time. A column keeps the previous
 * time until its drop starts, and the rows below a head keep it until the head passes them. A
 * letter of the new time that lies deep enough in the trail is lit at full brightness and stays
 * lit; the rest of the column goes dark behind the trail. It runs in one part, while GetState()
 * returns kSetTime.
 *
 * SetTime() starts it from whatever the display shows, and each Task() takes one step of it. The
 * last step clears the display, lights the words of the new time whole and makes the animation
 * idle.
 */
class AnimationMatrix : public Animation
{
  public:

  private:
    /* number of frames a column stays dark before its drop enters the display */
    static constexpr byte kMaxStartDelay{DISPLAY_NUMBER_OF_ROWS};
    /* the drop has to run past the last row until its trail has left the display */
    static constexpr byte kNumberOfFrames{4u * DISPLAY_NUMBER_OF_ROWS};
    /* fade step from which letters of the new time are shown, see DrawColumn() */
    static constexpr byte kRevealFadeStep{3u};
    /*
     * last fade step of the trail, above it a pixel goes dark. The original has no
     * such limit, its trail always reaches up to the first row. With the limit the
     * trail follows the drop out of the display instead of leaving a dim haze
     * behind, so the animation ends on a black display showing only the new time.
     */
    static constexpr byte kTrailFadeSteps{6u};
    /* odd factor of the hash in StartFrame(), see there */
    static constexpr byte kStartFrameHashFactor{181u};
    /*
     * per pixel brightness of the brightest trail pixel, divided by the fade step for
     * the ones behind it. The drop head and the letters of the new time do not use it,
     * they are set without a brightness and are therefore at full display brightness.
     */
    static constexpr byte kTrailBrightness{255u};

    ClockWords::WordsListType clock_words_table_{{DisplayWords::WordIdType::kNone}};
    byte frame_counter_{0u};
    /* varies the start frames from minute to minute, see StartFrame() */
    byte seed_{0u};

    void Reset();
    void SetTimeTask();
    void DrawColumn(byte column, byte head);
    byte StartFrame(byte column) const;

  public:
    /*! \brief Makes the animation, with GetState() kNone until Init(). */
    constexpr AnimationMatrix() { }

    /*! \brief Destroys the animation. */
    ~AnimationMatrix() = default;

    /*!
     * \brief Makes the animation idle and resets its progress.
     *
     * A running animation ends where it stands, and the display keeps what its last step drew.
     */
    void Init();

    /*!
     * \brief Starts the animation towards the time `hour`:`minute`.
     *
     * It draws nothing itself. It derives the steps the drops start at from `hour` and `minute`,
     * so that the rain varies from one word change to the next and the same time always rains
     * the same.
     *
     * \param[in] hour The hour, 0 to 23.
     * \param[in] minute The minute, 0 to 59.
     * \return kEOk, or kENotOk if `hour` is above 23 or `minute` above 59, and the animation is
     *         then left as it was, or if it is not idle; a running one then runs on, but ends on
     *         the words of `hour`:`minute`.
     */
    StdReturnType SetTime(byte hour, byte minute);

    /*!
     * \brief Takes one step of the animation; does nothing while it is idle.
     *
     * A step draws one frame of the rain: in every column whose drop has started, it lights the
     * head at full brightness, the trail above it dimmer with the distance and the letters of the
     * new time deep in the trail at full brightness, and turns the pixels above the trail off. It
     * takes 4 * DISPLAY_NUMBER_OF_ROWS frames, and the step after them ends it.
     *
     * It writes the pixels of Display and leaves showing them to Show().
     */
    void Task();
};

#endif  // WORDCLOCK_SRC_ANIMATION_ANIMATIONMATRIX_H_
