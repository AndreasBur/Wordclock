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
/**     \file       DisplayCurrentLimit.h
 *      \brief      What the supply behind the strip can deliver, as a ceiling on brightness
 *
 *      \details    The firmware can ask for more current than the supply has. This turns the
 *                  budget into the largest brightness that still fits it.
 *
******************************************************************************************************************************************************/
#ifndef _DISPLAY_CURRENT_LIMIT_H_
#define _DISPLAY_CURRENT_LIMIT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G   L   O   B   A   L       C   O   N   S   T   A   N   T       M   A   C   R   O   S
******************************************************************************************************************************************************/
/* DisplayCurrentLimit configuration parameter */
/* What the supply behind the strip can deliver, in milliamps, minus the margin it should keep.
   A property of the build rather than of the running clock: the number belongs to the power
   supply somebody screwed into the case, and a limit a command could raise would not be one.
   So it is overridden from a board's own build, the way DISPLAY_DATA_PIN is.

   The default suits a 5 V / 3 A supply. It is not a guess at the display: 110 WS2812 reach
   6.6 A on white at full brightness, which every supply short of the 10 A that
   platform/esp32/README.md asks for is well under. */
#ifndef DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS
# define DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS     2500u
#endif


/* DisplayCurrentLimit parameter */


/******************************************************************************************************************************************************
 *  C   L   A   S   S       D   I   S   P   L   A   Y       C   U   R   R   E   N   T       L   I   M   I   T
******************************************************************************************************************************************************/
class DisplayCurrentLimit
{
/******************************************************************************************************************************************************
 *  P   U   B   L   I   C       D   A   T   A       T   Y   P   E   S       A   N   D       S   T   R   U   C   T   U   R   E   S
******************************************************************************************************************************************************/
  public:
    using CurrentType = uint32_t;

    /* The answer where the supply is not the binding constraint, and the value the strip's
       master carries when nothing is being held back. */
    static constexpr byte NoLimit{255u};

/******************************************************************************************************************************************************
 *  P   R   I   V   A   T   E       D   A   T   A       A   N   D       F   U   N   C   T   I   O   N   S
******************************************************************************************************************************************************/
  private:
    static constexpr CurrentType BudgetMicroAmps{static_cast<CurrentType>(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS) * 1000u};

    /* The two numbers the whole estimate rests on, both from the WS2812's datasheet and both
       confirmed by the figure the README sizes the supply from: 20 mA per channel at full
       makes 60 mA for a white LED, and 110 of those are the 6.6 A quoted there. The idle
       current is what a dark strip still draws - some 1 mA each, so a tenth of an amp for
       this display before a single letter is lit. */
    static constexpr CurrentType ChannelCurrentAtFullMicroAmps{20000u};
    static constexpr CurrentType IdleCurrentPerLedMicroAmps{1000u};

    static constexpr byte ChannelValueAtFull{255u};

    /* Dimming multiplies by the brightness and shifts by eight, so the factor a brightness
       stands for is over 256 rather than over 255. Reproduced here rather than assumed,
       because an estimate that used 255 would run one part in 256 low. */
    static constexpr CurrentType BrightnessScale{256u};

    /* Bounded by 255 * 765 * 20000, which stays inside 32 bits with room to spare - so no
       display this code could be built for overflows it. */
    static constexpr CurrentType getLitCurrentAtFullMicroAmps(byte LitPixels, uint16_t ChannelSum) {
        return static_cast<CurrentType>(LitPixels) * ChannelSum * ChannelCurrentAtFullMicroAmps / ChannelValueAtFull;
    }

    static constexpr CurrentType getAvailableMicroAmps(byte LedCount) {
        const CurrentType Idle = getIdleCurrentMicroAmps(LedCount);

        return (BudgetMicroAmps > Idle) ? (BudgetMicroAmps - Idle) : 0u;
    }

/******************************************************************************************************************************************************
 *  P   U   B   L   I   C       F   U   N   C   T   I   O   N   S
******************************************************************************************************************************************************/
  public:
    /* Nothing to construct: the budget is a constant and the rest are the caller's numbers. */
    DisplayCurrentLimit() = delete;

    /* What a dark but powered strip still draws. Public because it is the floor every other
       answer here sits on, and because a caller comparing an estimate against a supply needs
       to know that a display showing nothing is not free. */
    static constexpr CurrentType getIdleCurrentMicroAmps(byte LedCount) {
        return static_cast<CurrentType>(LedCount) * IdleCurrentPerLedMicroAmps;
    }

    static constexpr uint16_t toChannelSum(byte Red, byte Green, byte Blue) {
        return static_cast<uint16_t>(Red) + Green + Blue;
    }

    /* What the strip would draw at full brightness, idle current included. The answer the
       limit below is derived from, and worth having on its own: it is the number to compare
       against a supply's rating. */
    static constexpr CurrentType getCurrentAtFullBrightnessMicroAmps(byte LedCount, byte LitPixels, uint16_t ChannelSum) {
        return getIdleCurrentMicroAmps(LedCount) + getLitCurrentAtFullMicroAmps(LitPixels, ChannelSum);
    }

    /* The largest brightness whose estimated current still fits the budget.

       Solved rather than searched: dimming is linear in the brightness, so the brightness the
       budget allows follows from one division instead of from stepping down until it fits.
       That is what makes this affordable on every task run, and on the AVR at all.

       Answers BrightnessAtFull where nothing is lit or where the display cannot reach the
       budget anyway, so a clock on a supply with headroom never pays for this beyond the
       division. */
    static constexpr byte toBrightnessLimit(byte LedCount, byte LitPixels, uint16_t ChannelSum) {
        const CurrentType Available = getAvailableMicroAmps(LedCount);
        const CurrentType AtFull = getLitCurrentAtFullMicroAmps(LitPixels, ChannelSum);

        if(Available == 0u) { return 0u; }
        if(AtFull <= Available) { return NoLimit; }

        return static_cast<byte>(Available * BrightnessScale / AtFull);
    }
};

/* Available * BrightnessScale is the one product that could leave 32 bits, and this is the
   budget at which it would. Far above any word clock, so the assert documents the ceiling
   rather than guarding a plausible mistake. */
static_assert(DISPLAY_CURRENT_LIMIT_BUDGET_MILLI_AMPS <= 16000u,
              "DisplayCurrentLimit: budget beyond 16 A would overflow the brightness solution");

#endif // _DISPLAY_CURRENT_LIMIT_H_
