// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Maps a linear brightness setting onto an exponential curve, so that equal steps of the
 *        setting look like equal steps of light.
 *
 * The eye answers light roughly logarithmically (Weber-Fechner): going from 10 to 20 looks like as
 * big a step as going from 100 to 200. A brightness that reaches the LEDs linearly therefore seems
 * to rise quickly at the bottom and then hardly change across the upper half of the setting. An
 * exponential curve in front of the LEDs undoes that. The background, including why exponential
 * rather than a power curve is good enough for LEDs, is the article
 * https://www.mikrocontroller.net/articles/LED-Fading.
 *
 * Despite the class name this is that exponential curve, not a gamma (power) curve.
 *
 * The implementation is the one from the article's discussion page, section "Sparsamere
 * Implementierung": https://www.mikrocontroller.net/articles/Diskussion:LED-Fading. Written for
 * ATtinys, so it needs neither a 256-entry table nor any multiplication:
 *
 * - The curve doubles every 16 steps, so 128 steps cover 8 doublings - from 1 to 255, the whole
 *   range of a byte. The setting is halved to those 128 steps first.
 * - Only the top doubling is stored: 16 values of 2^(7 + n/16) for n = 1..16, i.e. 133..255.
 *   Every lower doubling is the same 16 values shifted right, one bit per doubling. A step
 *   therefore picks its value with its low four bits and its shift with its upper three.
 * - That only closes if steps = values per doubling * doublings, and the doublings are the bits
 *   of the result: 128 = 16 * 8. The static_assert below holds it.
 *
 * Properties worth knowing before changing anything:
 *
 * - 0 maps to 0. The exponential itself never reaches zero - the lowest step is 1 - and the source
 *   recommends exactly this answer for it: at a setting of 0 the LEDs are to get 0, not the
 *   faintest light. It is also what an uncorrected setting of 0 hands on; whether the display
 *   then goes dark is decided after this, not here.
 * - The bottom is coarse. The settings 1..31 all give 1 and up to 63 give at most 3: 8 bits have
 *   nothing between 0, 1 and 2 to offer, so a curve that is exponential all the way down has to
 *   repeat itself there. The source calls 128 steps on 8 bits "verschwenderisch" for that reason.
 * - The middle is dark: half the setting gives 16 of 255, about 6 %. That is the base of the
 *   curve, 256 over the whole range, fixed by the shift trick. The article suggests a base
 *   between 10 and 100 and says it takes trying out; a flatter curve would need a different
 *   table.
 * - The stored values lie up to 1 below the formula above - 151 where it gives 152.2. They are
 *   kept as published: one 256th of a doubling is not something anybody sees.
 */

#ifndef _GAMMA_CORRECTION_H_
#define _GAMMA_CORRECTION_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief The brightness curve: maps a linear brightness setting onto an exponential curve, so that
 *        equal steps of the setting look like equal steps of light.
 *
 * Despite its name it is an exponential curve, not a gamma (power) curve; the file says why it
 * is and how it is computed.
 */
class GammaCorrection
{
  public:
    /*! \brief A value of the stored top doubling of the curve. */
    using TopDoublingElementType = byte;

  private:
    static constexpr byte kValuesPerDoubling{16u};
    static constexpr byte kNumberOfDoublings{sizeof(byte) * CHAR_BIT};
    static constexpr byte kNumberOfSteps{128u};

    static_assert(kValuesPerDoubling * kNumberOfDoublings == kNumberOfSteps,
                  "GammaCorrection: the steps must be exactly the stored doubling shifted through every bit of the result");

    // 2^(7 + n/16) for n = 1..16, as published - see the file header for the 1 they lie
    // below the formula.
    static constexpr TopDoublingElementType kTopDoublingTable[kValuesPerDoubling] PROGMEM {
        133u, 139u, 145u, 151u, 158u, 165u, 172u, 180u, 188u, 196u, 205u, 214u, 224u, 234u, 244u, 255u
    };

    static TopDoublingElementType GetTopDoublingElement(byte index) {
        TopDoublingElementType element;
        memcpy_P(&element, &kTopDoublingTable[index], sizeof(TopDoublingElementType));
        return element;
    }

    // The top doubling needs no shift, each one below it one bit more.
    static constexpr byte ToShift(byte step) {
        return static_cast<byte>((kNumberOfDoublings - 1u) - (step / kValuesPerDoubling));
    }

    // About 1.0443^Step - 2^(1/16) per step - from 1 at step 0 to 255 at step 127.
    static byte ToExponential(byte step) {
        return static_cast<byte>(GetTopDoublingElement(step % kValuesPerDoubling) >> ToShift(step));
    }

    static constexpr byte ToStep(byte value_linear) { return static_cast<byte>(value_linear / 2u); }

  public:
    /*! \brief Makes the curve, which holds no state of its own. */
    constexpr GammaCorrection() { }

    /*!
     * \brief Returns the brightness for the LEDs that the linear setting `value_linear` stands for.
     *
     * 0 gives 0, every other setting a point of the curve: the setting is halved to one of 128
     * steps, which give 1 to 255. So 1 to 31 all give 1, and 254 and 255 both give 255.
     *
     * \param[in] value_linear The brightness setting, 0 to 255.
     * \return The brightness for the LEDs: 0 for a setting of 0, 1 to 255 for every other.
     */
    static byte CalcCorrectedValue(byte value_linear) {
        /*
         * 0 to 0, everything else onto the curve: see the file header for why 0 is answered
         * apart rather than by subtracting 1 from every result, which would take 255 down to
         * 254 as well.
         */
        if(value_linear == 0u) { return 0u; }
        return ToExponential(ToStep(value_linear));
    }
};

#endif
