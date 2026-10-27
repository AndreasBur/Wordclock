/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       GammaCorrection.h
 *      \brief      Maps a linear brightness setting onto an exponential curve, so that equal
 *                  steps of the setting look like equal steps of light
 *
 *      \details    The eye answers light roughly logarithmically (Weber-Fechner): going from
 *                  10 to 20 looks like as big a step as going from 100 to 200. A brightness
 *                  that reaches the LEDs linearly therefore seems to rise quickly at the
 *                  bottom and then hardly change across the upper half of the setting. An
 *                  exponential curve in front of the LEDs undoes that. The background,
 *                  including why exponential rather than a power curve is good enough for
 *                  LEDs, is the article
 *                  https://www.mikrocontroller.net/articles/LED-Fading
 *
 *                  Despite the class name this is that exponential curve, not a gamma (power)
 *                  curve.
 *
 *                  The implementation is the one from the article's discussion page, section
 *                  "Sparsamere Implementierung":
 *                  https://www.mikrocontroller.net/articles/Diskussion:LED-Fading
 *                  Written for ATtinys, so it needs neither a 256-entry table nor any
 *                  multiplication:
 *
 *                  - The curve doubles every 16 steps, so 128 steps cover 8 doublings - from
 *                    1 to 255, the whole range of a byte. The setting is halved to those 128
 *                    steps first.
 *                  - Only the top doubling is stored: 16 values of 2^(7 + n/16) for n = 1..16,
 *                    i.e. 133..255. Every lower doubling is the same 16 values shifted right,
 *                    one bit per doubling. A step therefore picks its value with its low four
 *                    bits and its shift with its upper three.
 *                  - That only closes if steps = values per doubling * doublings, and the
 *                    doublings are the bits of the result: 128 = 16 * 8. The static_assert
 *                    below holds it.
 *
 *                  Properties worth knowing before changing anything:
 *
 *                  - 0 maps to 0. The exponential itself never reaches zero - the lowest step
 *                    is 1 - and the source recommends exactly this answer for it: at a
 *                    setting of 0 the LEDs are to get 0, not the faintest light. It is also
 *                    what an uncorrected setting of 0 hands on; whether the display then
 *                    goes dark is decided after this, not here.
 *                  - The bottom is coarse. The settings 1..31 all give 1 and up to 63 give at
 *                    most 3: 8 bits have nothing between 0, 1 and 2 to offer, so a curve
 *                    that is exponential all the way down has to repeat itself there. The
 *                    source calls 128 steps on 8 bits "verschwenderisch" for that reason.
 *                  - The middle is dark: half the setting gives 16 of 255, about 6 %. That is
 *                    the base of the curve, 256 over the whole range, fixed by the shift
 *                    trick. The article suggests a base between 10 and 100 and says it takes
 *                    trying out; a flatter curve would need a different table.
 *                  - The stored values lie up to 1 below the formula above - 151 where it
 *                    gives 152.2. They are kept as published: one 256th of a doubling is
 *                    not something anybody sees.
 *
******************************************************************************************************************************************************/
#ifndef _GAMMA_CORRECTION_H_
#define _GAMMA_CORRECTION_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  C L A S S   G A M M A   C O R R E C T I O N
******************************************************************************************************************************************************/
class GammaCorrection
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using TopDoublingElementType = byte;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte ValuesPerDoubling{16u};
    static constexpr byte NumberOfDoublings{sizeof(byte) * CHAR_BIT};
    static constexpr byte NumberOfSteps{128u};

    static_assert(ValuesPerDoubling * NumberOfDoublings == NumberOfSteps,
                  "GammaCorrection: the steps must be exactly the stored doubling shifted through every bit of the result");

    /* 2^(7 + n/16) for n = 1..16, as published - see the file header for the 1 they lie
       below the formula. */
    static constexpr TopDoublingElementType TopDoublingTable[ValuesPerDoubling] PROGMEM {
        133u, 139u, 145u, 151u, 158u, 165u, 172u, 180u, 188u, 196u, 205u, 214u, 224u, 234u, 244u, 255u
    };

    // functions
    static TopDoublingElementType getTopDoublingElement(byte Index) {
        TopDoublingElementType Element;
        memcpy_P(&Element, &TopDoublingTable[Index], sizeof(TopDoublingElementType));
        return Element;
    }

    /* The top doubling needs no shift, each one below it one bit more. */
    static constexpr byte toShift(byte Step) {
        return static_cast<byte>((NumberOfDoublings - 1u) - (Step / ValuesPerDoubling));
    }

    /* About 1.0443^Step - 2^(1/16) per step - from 1 at step 0 to 255 at step 127. */
    static byte toExponential(byte Step) {
        return static_cast<byte>(getTopDoublingElement(Step % ValuesPerDoubling) >> toShift(Step));
    }

    static constexpr byte toStep(byte ValueLinear) { return static_cast<byte>(ValueLinear / 2u); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr GammaCorrection() { }

    // methods
    /* 0 to 0, everything else onto the curve: see the file header for why 0 is answered
       apart rather than by subtracting 1 from every result, which would take 255 down to
       254 as well. */
    static byte calcCorrectedValue(byte ValueLinear) {
        if(ValueLinear == 0u) { return 0u; }
        return toExponential(toStep(ValueLinear));
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
