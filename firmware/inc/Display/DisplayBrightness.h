// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The brightness of the display, from the light sensor or set by hand.
 */

#ifndef _DISPLAY_BRIGHTNESS_H_
#define _DISPLAY_BRIGHTNESS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Illuminance.h"
#include "GammaCorrection.h"

/*!
 * \brief The brightness of the display: the brightness set, and what of it is to reach the LEDs.
 *
 * The brightness set runs from 0 to 255, and a new clock has 255. CalcBrightness() answers what
 * is to reach the LEDs: the brightness set, through the gamma correction where it is on, then
 * through the automatic where it is on, then scaled by the night level and the fade level. The
 * class only computes; Display writes the result to the strip.
 *
 * The automatic scales the brightness by where the light sensor's last reading lies between the
 * lower and the upper bound of its calibration: by 0 at or below the lower bound, by 1 at or above
 * the upper bound, and in proportion in between. Bounds that are not in order count as a reading
 * at the upper bound. The result never drops below 8, and a brightness already below 8 passes
 * unchanged, so the automatic never brightens beyond the brightness set.
 */
class DisplayBrightness
{
  public:
     /*! \brief An illuminance in lux, as Illuminance reads it. */
     using IlluminanceType = Illuminance::IlluminanceType;

  private:
    // What a clock starts with, and what the reset puts back - named once so the two
    // cannot drift apart.
    static constexpr byte kBrightnessInitValue{255u};
    static constexpr bool kUseAutomaticInitValue{false};
    static constexpr bool kUseGammaCorrectionInitValue{false};
    // What a fade scales the result by. Full means there is no fade, which is the state
    // everything but a running AnimationFade is in.
    static constexpr byte kFadeLevelFull{255u};

    byte brightness_{kBrightnessInitValue};
    byte fade_level_{kFadeLevelFull};
    /*
     * The night switch's own scaling, kept apart from fade_level_ although it does the same
     * arithmetic. A fade belongs to a running animation, which clears it when it ends -
     * and an animation ending in the small hours would take the night dimming with it.
     */
    byte night_level_{kFadeLevelFull};
    bool use_automatic_{kUseAutomaticInitValue};
    bool use_gamma_correction_{kUseGammaCorrectionInitValue};
    GammaCorrection g_correction_;

    static constexpr float kAutomaticCorrectionFactor{1.0f};
    static constexpr byte kBrightnessMaxValue{255};
    static constexpr byte kBrightnessAutomaticMinValue{8u};

    static byte CalcBrightnessAutomatic(byte brightness);

    /*
     * Where the reading sits between the bounds: 0 at the dark one, 1 at the bright one.
     * Bounds that are not in order are what a calibration in the wrong order stores, and
     * nothing rejects it - so they are answered with 1 here, which is the display the owner
     * set and the same thing the automatic did before it had a lower bound at all.
     */
    static constexpr float ToIlluminanceFactor(IlluminanceType measured, IlluminanceType dark, IlluminanceType bright) {
        if(bright <= dark) { return 1.0f; }
        if(measured <= dark) { return 0.0f; }
        if(measured >= bright) { return 1.0f; }

        return static_cast<float>(measured - dark) / static_cast<float>(bright - dark);
    }
    // Never darker than kBrightnessAutomaticMinValue: a display the automatic took all the
    // way down reads as a clock that has failed, not as a dark room.
    static constexpr byte ToAutomaticLevel(byte brightness, float illuminance_factor) {
        const byte level = static_cast<byte>(brightness * illuminance_factor * kAutomaticCorrectionFactor);

        return (level < kBrightnessAutomaticMinValue) ? kBrightnessAutomaticMinValue : level;
    }
    // The same shape as Pixel's own dimming: a level of 255 leaves the value alone, and any
    // lower level scales it by (level + 1) / 256, rounding down - so a small value can reach 0.
    static constexpr byte ScaleByLevel(byte value, byte level) {
        return static_cast<byte>((static_cast<uint16_t>(value) * (level + 1u)) >> 8u);
    }

    byte CalcBrightnessCorrected() const { return g_correction_.CalcCorrectedValue(brightness_); }
    byte CalcBrightnessAutomatic() const { return CalcBrightnessAutomatic(brightness_); }
    byte CalcBrightnessAutomaticCorrected() const { return CalcBrightnessAutomatic(g_correction_.CalcCorrectedValue(brightness_)); }

  public:
    /*!
     * \brief Makes the brightness of a new clock: 255, with the automatic and the gamma correction
     *        off and neither a night nor a fade level.
     */
    constexpr DisplayBrightness() { }

    /*! \brief Destroys the brightness. */
    ~DisplayBrightness() = default;

    /*! \brief Returns the brightness set, 0 to 255, before anything is applied to it. */
    byte GetBrightness() const { return brightness_; }

    /*!
     * \brief Returns the brightness set after the gamma correction and the automatic, where each is
     *        on.
     *
     * The gamma correction comes first, and the automatic scales what it gives. With the automatic
     * on, this reads the light sensor's last reading and its calibration from Illuminance. The
     * night and the fade level are not applied here; CalcBrightness() applies them.
     *
     * \return The brightness, 0 to 255.
     */
    byte CalcConfiguredBrightness() const;

    /*! \brief Returns whether the automatic follows the light sensor. */
    bool GetUseAutomatic() const { return use_automatic_; }

    /*! \brief Returns whether the gamma correction is on. */
    bool GetUseGammaCorrection() const { return use_gamma_correction_; }

    /*!
     * \brief Sets the brightness.
     *
     * \param[in] brightness The brightness, 0 to 255.
     */
    void SetBrightness(byte brightness) { brightness_ = brightness; }

    /*!
     * \brief Switches the automatic on or off.
     *
     * \param[in] use_automatic True to switch it on, false to switch it off.
     */
    void SetUseAutomatic(bool use_automatic) { use_automatic_ = use_automatic; }

    /*!
     * \brief Switches the gamma correction on or off.
     *
     * \param[in] use_gamma_correction True to switch it on, false to switch it off.
     */
    void SetUseGammaCorrection(bool use_gamma_correction) { use_gamma_correction_ = use_gamma_correction; }

    /*!
     * \brief Puts the brightness back to how a new clock has it: 255, with the automatic and the
     *        gamma correction off and no night level.
     *
     * The fade level is left as it is; it belongs to a running fade, which clears it when it ends.
     */
    void ResetToDefaults();

    /*! \brief Switches the automatic on. */
    void EnableAutomatic() { use_automatic_ = true; }

    /*! \brief Switches the automatic off. */
    void DisableAutomatic() { use_automatic_ = false; }

    /*! \brief Switches the gamma correction on. */
    void EnableGammaCorrection() { use_gamma_correction_ = true; }

    /*! \brief Switches the gamma correction off. */
    void DisableGammaCorrection() { use_gamma_correction_ = false; }

    /*! \brief Raises the brightness by one, unless it is 255 already. */
    void IncrementBrightness() { if(brightness_ < kBrightnessMaxValue) { brightness_++; }}

    /*! \brief Lowers the brightness by one, unless it is 0 already. */
    void DecrementBrightness() { if(brightness_ > 0u) { brightness_--; }}

    /*!
     * \brief Returns the brightness that is to reach the LEDs.
     *
     * It is CalcConfiguredBrightness() scaled by the night level and then by the fade level, each
     * where it is below 255. A level scales by (level + 1) / 256, rounding down, so a level of 0
     * gives 0.
     *
     * A fade scales whatever the setting, the gamma correction and the automatic arrived at,
     * rather than replacing it: what fades out is the display as it was configured, and the
     * automatic keeps following the room while it does.
     *
     * \return The brightness, 0 to 255.
     */
    byte CalcBrightness() const;

    /*! \brief Returns how dark a fade has made the display, 255 being not at all. */
    byte GetFadeLevel() const { return fade_level_; }

    /*!
     * \brief Sets how dark a fade makes the display.
     *
     * The level is kept here rather than in the animation because Display::ApplyBrightness() runs
     * on every task and would otherwise undo what the animation wrote on the previous one.
     *
     * \param[in] level 255 for no fade, anything lower for darker, down to 0 for dark.
     */
    void SetFadeLevel(byte level) { fade_level_ = level; }

    /*! \brief Ends the fade: puts the fade level back to 255. */
    void ClearFade() { fade_level_ = kFadeLevelFull; }

    /*! \brief Returns how dark the night switch has made the display, 255 being not at all. */
    byte GetNightLevel() const { return night_level_; }

    /*!
     * \brief Sets how dark the night switch makes the display.
     *
     * The level scales rather than replaces, for the same reason a fade does: what is dimmed is
     * the display as it was configured, so the brightness the owner set is still what daylight
     * returns to and is still what Persistence stores.
     *
     * \param[in] level 255 for no dimming, anything lower for darker, down to 0 for dark.
     */
    void SetNightLevel(byte level) { night_level_ = level; }

    /*! \brief Ends the night's dimming: puts the night level back to 255. */
    void ClearNight() { night_level_ = kFadeLevelFull; }

};

#endif
