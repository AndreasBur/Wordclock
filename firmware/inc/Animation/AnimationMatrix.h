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
/**     \file       AnimationMatrix.h
 *      \brief      Falling code rain, as seen in the Matrix movie
 *
 *      \details    One drop falls per column. All drops move at the same speed, the
 *                  rain pattern comes from their different start delays only. Letters
 *                  of the new time light up at full intensity once the drop has
 *                  passed them far enough, so the time appears out of the rain.
 *
 *                  Everything is drawn in the display color and differs in intensity
 *                  only, so the display keeps one uniform color. Set the display
 *                  color to green for the look of the movie.
 *
 *                  Ported from the wordclock24h project by Frank Meyer
 *                  (display_animation_generic_matrix in src/display/display.c), which
 *                  uses a yellow head and green rain regardless of the display color.
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_MATRIX_H_
#define _ANIMATION_MATRIX_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Clock.h"
#include "Animation.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* AnimationMatrix configuration parameter */


/* AnimationMatrix parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   M A T R I X
******************************************************************************************************************************************************/
class AnimationMatrix : public Animation
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* number of frames a column stays dark before its drop enters the display */
    static constexpr byte MaxStartDelay{DISPLAY_NUMBER_OF_ROWS};
    /* the drop has to run past the last row until its trail has left the display */
    static constexpr byte NumberOfFrames{4u * DISPLAY_NUMBER_OF_ROWS};
    /* fade step from which letters of the new time are shown, see drawColumn() */
    static constexpr byte RevealFadeStep{3u};
    /* last fade step of the trail, above it a pixel goes dark. The original has no
       such limit, its trail always reaches up to the first row. With the limit the
       trail follows the drop out of the display instead of leaving a dim haze
       behind, so the animation ends on a black display showing only the new time. */
    static constexpr byte TrailFadeSteps{6u};
    /* odd factor of the hash in startFrame(), see there */
    static constexpr byte StartFrameHashFactor{181u};
    /* per pixel brightness of the brightest trail pixel, divided by the fade step for
       the ones behind it. The drop head and the letters of the new time do not use it,
       they are set without a brightness and are therefore at full display brightness. */
    static constexpr byte TrailBrightness{255u};

    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte FrameCounter{0u};
    /* varies the start frames from minute to minute, see startFrame() */
    byte Seed{0u};

    // functions
    void reset();
    void setTimeTask();
    void drawColumn(byte, byte);
    byte startFrame(byte) const;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationMatrix() { }
    ~AnimationMatrix() { }

    // get methods


    // set methods


    // methods
    void init();
    StdReturnType setTime(byte, byte);
    void task();
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
