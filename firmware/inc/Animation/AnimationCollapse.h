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
/**     \file       AnimationCollapse.h
 *      \brief      Letters slide together and the new ones slide apart again
 *
 *      \details    The letters of the previous time slide to one edge of their row,
 *                  closing every gap between the words on the way, until each row is
 *                  one solid block. The letters of the new time start as such a block
 *                  and slide apart into their words. The edge is drawn per minute.
 *
 *                  Unlike AnimationSqueeze, which lets the letters disappear one by
 *                  one, nothing vanishes here: the letters move.
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_COLLAPSE_H_
#define _ANIMATION_COLLAPSE_H_

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
/* AnimationCollapse configuration parameter */


/* AnimationCollapse parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   C O L L A P S E
******************************************************************************************************************************************************/
class AnimationCollapse : public Animation
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* Edge the letters collapse against, drawn per minute */
    enum DirectionType {
        DIRECTION_TO_LEFT,
        DIRECTION_TO_RIGHT,
        DIRECTION_NUMBER_OF_DIRECTIONS
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    ClockWords::WordsListType ClockWordsTable{{DisplayWords::WORD_NONE}};
    byte Step{0u};
    DirectionType Direction{DIRECTION_TO_LEFT};
    /* Steps the expansion needs, which is the longest way a single letter has to
       travel. Taking the display width instead would let the letters arrive long
       before the animation ends, because the gaps between words are only one or two
       columns wide. */
    byte ExpandSteps{1u};

    // functions
    void reset();
    void clearTimeTask();
    void setTimeTask();
    bool collapseRow(byte);
    byte calcColumn(byte, byte) const;
    byte calcBlockColumn(byte, byte) const;
    byte countLettersOfRow(byte) const;
    byte calcExpandSteps() const;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr AnimationCollapse() { }
    ~AnimationCollapse() { }

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
