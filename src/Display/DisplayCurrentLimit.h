// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the supply behind the strip can deliver, as a ceiling on brightness.
 *
 * The firmware can ask for more current than the supply has. This turns the budget into the largest
 * brightness that still fits it.
 */

#ifndef WORDCLOCK_SRC_DISPLAY_DISPLAYCURRENTLIMIT_H_
#define WORDCLOCK_SRC_DISPLAY_DISPLAYCURRENTLIMIT_H_

#include "Arduino.h"
#include "StandardTypes.h"

/*!
 * \brief What the supply behind the strip can deliver, in milliamps, minus the margin it should
 *        keep.
 *
 * A property of the build rather than of the running clock: the number belongs to the power
 * supply somebody screwed into the case, and a limit a command could raise would not be one. So
 * it is overridden from a board's own build, the way DISPLAY_DATA_PIN is. A static_assert below
 * the class holds it at 16000 at most.
 *
 * The default suits a 5 V / 3 A supply. It is not a guess at the display: 110 WS2812 reach 6.6 A
 * on white at full brightness, which every supply short of the 10 A that platform/esp32/README.md
 * asks for is well under.
 */
#ifndef DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS
#define DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS 2500u
#endif

/*!
 * \brief The current limit: the largest brightness at which the strip's estimated current still
 *        fits DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS.
 *
 * The estimate takes 20 mA per channel at full for a lit LED and 1 mA for every LED, lit or not,
 * and has every lit pixel carry the same colour. Currents are in microamps. Every function is
 * static and constexpr, and the class cannot be constructed.
 */
class DisplayCurrentLimit {
 public:
  /*! \brief A current in microamps. */
  using CurrentType = uint32_t;

  /*!
   * \brief The answer where the supply is not the binding constraint, 255, and the value the
   *        strip's master carries when nothing is being held back.
   */
  static constexpr byte kNoLimit{255u};

 private:
  static constexpr CurrentType kBudgetMicroAmps{
      static_cast<CurrentType>(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS) * 1000u};

  /*
   * The two numbers the whole estimate rests on, both from the WS2812's datasheet and both
   * confirmed by the figure the README sizes the supply from: 20 mA per channel at full
   * makes 60 mA for a white LED, and 110 of those are the 6.6 A quoted there. The idle
   * current is what a dark strip still draws - some 1 mA each, so a tenth of an amp for
   * this display before a single letter is lit.
   */
  static constexpr CurrentType kChannelCurrentAtFullMicroAmps{20000u};
  static constexpr CurrentType kIdleCurrentPerLedMicroAmps{1000u};

  static constexpr byte kChannelValueAtFull{255u};

  /*
   * Dimming multiplies by the brightness and shifts by eight, so the factor a brightness
   * stands for is over 256 rather than over 255. Reproduced here rather than assumed,
   * because an estimate that used 255 would run one part in 256 low.
   */
  static constexpr CurrentType kBrightnessScale{256u};

  // Bounded by 255 * 765 * 20000, which stays inside 32 bits with room to spare - so no
  // display this code could be built for overflows it.
  static constexpr CurrentType GetLitCurrentAtFullMicroAmps(byte lit_pixels, uint16_t channel_sum) {
    return static_cast<CurrentType>(lit_pixels) * channel_sum * kChannelCurrentAtFullMicroAmps /
           kChannelValueAtFull;
  }

  static constexpr CurrentType GetAvailableMicroAmps(byte led_count) {
    CurrentType const idle = GetIdleCurrentMicroAmps(led_count);

    return (kBudgetMicroAmps > idle) ? (kBudgetMicroAmps - idle) : 0u;
  }

 public:
  // Nothing to construct: the budget is a constant and the rest are the caller's numbers.
  DisplayCurrentLimit() = delete;

  /*!
   * \brief Returns what a dark but powered strip still draws.
   *
   * Public because it is the floor every other answer here sits on, and because a caller
   * comparing an estimate against a supply needs to know that a display showing nothing is not
   * free.
   *
   * \param[in] led_count The number of LEDs of the strip.
   * \return The current, 1 mA per LED, in microamps.
   */
  static constexpr CurrentType GetIdleCurrentMicroAmps(byte led_count) {
    return static_cast<CurrentType>(led_count) * kIdleCurrentPerLedMicroAmps;
  }

  /*!
   * \brief Returns the sum of the three channels of a colour, the form the other functions take a
   *        colour in.
   *
   * \param[in] red The red channel, 0 to 255.
   * \param[in] green The green channel, 0 to 255.
   * \param[in] blue The blue channel, 0 to 255.
   * \return The sum, 0 to 765.
   */
  static constexpr uint16_t ToChannelSum(byte red, byte green, byte blue) {
    return static_cast<uint16_t>(red + green + blue);
  }

  /*!
   * \brief Returns what the strip would draw at full brightness, idle current included.
   *
   * The answer ToBrightnessLimit() is derived from, and worth having on its own: it is the
   * number to compare against a supply's rating. Each lit pixel adds 20 mA per channel in
   * proportion to the channel's value over 255; the lit pixels' total is rounded down to a whole
   * microamp.
   *
   * \param[in] led_count The number of LEDs of the strip, lit or not.
   * \param[in] lit_pixels The number of lit pixels.
   * \param[in] channel_sum The colour of the lit pixels, as ToChannelSum() gives it.
   * \return The current, in microamps.
   */
  static constexpr CurrentType GetCurrentAtFullBrightnessMicroAmps(byte led_count, byte lit_pixels,
                                                                   uint16_t channel_sum) {
    return GetIdleCurrentMicroAmps(led_count) +
           GetLitCurrentAtFullMicroAmps(lit_pixels, channel_sum);
  }

  /*!
   * \brief Returns the largest brightness whose estimated current still fits the budget.
   *
   * Solved rather than searched: dimming is linear in the brightness, so the brightness the
   * budget allows follows from one division instead of from stepping down until it fits. That is
   * what makes this affordable on every task run, and on the AVR at all.
   *
   * Answers kNoLimit where nothing is lit or where the display cannot reach the budget anyway,
   * so a clock on a supply with headroom never pays for this beyond the division. The one
   * exception is a budget the idle current alone takes up, which answers 0 even then.
   *
   * \param[in] led_count The number of LEDs of the strip, lit or not.
   * \param[in] lit_pixels The number of lit pixels.
   * \param[in] channel_sum The colour of the lit pixels, as ToChannelSum() gives it.
   * \return 0 if the idle current of `led_count` LEDs takes up the whole budget; otherwise kNoLimit
   *         if the lit pixels at full brightness fit what is left of it, and else what is left
   *         times 256 over their current at full brightness, rounded down.
   */
  static constexpr byte ToBrightnessLimit(byte led_count, byte lit_pixels, uint16_t channel_sum) {
    CurrentType const available = GetAvailableMicroAmps(led_count);
    CurrentType const at_full = GetLitCurrentAtFullMicroAmps(lit_pixels, channel_sum);

    if (available == 0u) {
      return 0u;
    }
    if (at_full <= available) {
      return kNoLimit;
    }

    return static_cast<byte>(available * kBrightnessScale / at_full);
  }
};

/*
 * Available * kBrightnessScale is the one product that could leave 32 bits, and this is the
 * budget at which it would. Far above any word clock, so the assert documents the ceiling
 * rather than guarding a plausible mistake.
 */
static_assert(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS <= 16000u,
              "DisplayCurrentLimit: budget beyond 16 A would overflow the brightness solution");

#endif  // WORDCLOCK_SRC_DISPLAY_DISPLAYCURRENTLIMIT_H_
