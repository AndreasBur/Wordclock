// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Walks the display's colour round the wheel while the words stand still.
 *
 * Every one of the fifteen animations is a transition: something happens because the words changed,
 * and it is over. This is the other kind, and the clock had none - the display stands still for
 * five minutes at a time, and this is what happens during those five minutes.
 *
 * It is not an animation id, and that is a decision rather than an omission. An id would put it in
 * the list a word change picks from, where it has nothing to do: it does not begin, end, or have
 * anything to say about a transition. The favourites mask is exactly full at sixteen ids as well,
 * so it could not have gone there without widening that first.
 *
 * What it does not do is write the colour setting. The cycle is a level of its own that
 * DisplayColor applies on the way to the strip, so the colour somebody chose is still the colour
 * the command answers and Persistence stores - the same arrangement the night brightness has, and
 * for the same reason: a setting a feature overwrote is a setting nobody can get back. Switching
 * the cycle off therefore needs no memory of what was there before; the level goes away and what
 * was underneath it was never touched.
 *
 * The hue is deliberately not stored. Persistence writes when the stored blob differs from the live
 * settings, so a hue in that blob would be a flash write per step - a few hundred thousand of them
 * a day, on a part rated for a hundred thousand. What survives a restart is whether the cycle runs
 * and how fast; where in the round it was does not matter to anybody.
 */

#ifndef WORDCLOCK_SRC_COLORCYCLE_COLORCYCLE_H_
#define WORDCLOCK_SRC_COLORCYCLE_COLORCYCLE_H_

#include "Arduino.h"
#include "Pixel.h"
#include "StandardTypes.h"

/*!
 * \brief Scheduler ticks per step of hue, so 100 * 10 ms is a step a second and 256 of those are a
 *        round in a little over four minutes.
 *
 * Slow is the point: this runs on a clock somebody lives with, and a wheel spun fast enough to
 * notice is a wheel nobody can read the time off. It is also the initial value of a setting
 * rather than a fixed cycle, so a clock that wants faster says so with command 15.
 */
#define COLOR_CYCLE_TASK_CYCLE_INIT_VALUE 100u

/*!
 * \brief The colour cycle: walks the display's colour round the hue wheel while the words stand
 *        still.
 *
 * It sets the colour as a level of its own in Display, on top of the colour setting, which it
 * never writes. Each step is one of 256 hues, at full saturation and full value.
 */
class ColorCycle {
 private:
  // The wheel walked in one byte, so a step is a step of this and the round closes by
  // overflowing rather than by a comparison.
  using HueType = byte;

  static constexpr byte kTaskCycleInitValue{COLOR_CYCLE_TASK_CYCLE_INIT_VALUE};
  static constexpr bool kIsActiveInitValue{false};
  static constexpr HueType kHueInitValue{0u};

  /*
   * The six segments of the wheel, and the channel that is at full through each of them.
   * Named because ToColour() is otherwise six lines of arithmetic that only says what it
   * is if the reader already knows the wheel.
   */
  static constexpr byte kNumberOfSegments{6u};
  static constexpr byte kFullChannel{255u};

  bool is_active_{kIsActiveInitValue};
  byte task_cycle_{kTaskCycleInitValue};
  HueType hue_{kHueInitValue};

  ColorCycle() {}
  ~ColorCycle() = default;

  static Pixel ToColour(HueType hue);
  static bool IsDisplayShowingAnything();

 public:
  /*! \brief Returns the one colour cycle. */
  static ColorCycle& GetInstance() {
    static ColorCycle singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Returns the scheduler ticks per step of hue.
   *
   * Zero is a cycle standing still on the hue it reached, which the scheduler answers with a
   * task that never comes due - so it needs no rule of its own here.
   *
   * \return The ticks per step, or 0 for a cycle standing still.
   */
  byte GetTaskCycle() const { return task_cycle_; }

  /*! \brief Returns whether the cycle runs. */
  bool GetIsActive() const { return is_active_; }

  /*! \brief Returns the hue the cycle has reached, one of 256 steps round the wheel. */
  byte GetHue() const { return hue_; }

  /*!
   * \brief Sets the scheduler ticks per step of hue.
   *
   * \param[in] task_cycle The ticks per step; zero stands the cycle still on the hue it reached.
   */
  void SetTaskCycle(byte task_cycle) { task_cycle_ = task_cycle; }

  /*!
   * \brief Starts or stops the cycle.
   *
   * Starting sets the display to the colour of the hue reached at once. Stopping puts the
   * chosen colour back at once rather than at the next tick: the tick may be seconds away, and
   * a setting that takes seconds to take effect reads as one that did not work.
   *
   * \param[in] is_active True to start the cycle, false to stop it.
   */
  void SetIsActive(bool is_active);

  /*!
   * \brief Puts the cycle back to how a new clock has it: stopped, on the first hue, at
   *        COLOR_CYCLE_TASK_CYCLE_INIT_VALUE ticks per step.
   *
   * Stopping puts the chosen colour back at once, as SetIsActive() does.
   */
  void ResetToDefaults();

  /*!
   * \brief Steps the hue one further round the wheel and sets the display to its colour.
   *
   * Does nothing while the cycle is stopped or the display is disabled. After the last of the
   * 256 hues comes the first again.
   */
  void Task();
};

#endif  // WORDCLOCK_SRC_COLORCYCLE_COLORCYCLE_H_
