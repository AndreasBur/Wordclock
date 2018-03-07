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
/**     \file       Animation.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "AnimationClock.h"
#include "AnimationFont.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Animation configuration parameter*/


/* Animation parameter */


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  Animation
******************************************************************************************************************************************************/
class Animation
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
    //using FontType = AnimationFont::FontType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    AnimationClock wcAnimationClock;
    AnimationFont wcAnimationFont;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    //Animation() {}
    Animation(Display*, Clock*);
    ~Animation();

    // get methods

    // set methods


    // methods
    void init();
    void task() { wcAnimationFont.task(); }
    //void show() { pDisplay->show(); }
    stdReturnType setChar(byte Column, byte Row, char Char, AnimationFont::FontType Font) { return wcAnimationFont.setChar(Column, Row, Char, Font); }
    void setCharWithShift(char Char, AnimationFont::FontType Font) { wcAnimationFont.setCharWithShift(Char, Font); }
    void setText(char* Text, AnimationFont::FontType Font) { wcAnimationFont.setText(Text, Font); }
    void setTextWithShift(char* Text, AnimationFont::FontType Font) { wcAnimationFont.setTextWithShift(Text, Font); }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
