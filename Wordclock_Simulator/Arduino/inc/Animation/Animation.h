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
#include "Display.h"
#include "AnimationTeletype.h"
#include "AnimationCursor.h"
#include "AnimationDrop.h"
#include "AnimationWipe.h"
#include "AnimationSnake.h"
#include "AnimationShift.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Animation configuration parameter*/
#define ANIMATION_SUPPORT_FONT_4X6              STD_ON
#define ANIMATION_SUPPORT_FONT_5X8              STD_ON
#define ANIMATION_SUPPORT_FONT_6X8              STD_ON
#define ANIMATION_SUPPORT_FONT_6X10             STD_ON
#define ANIMATION_SUPPORT_FONT_8X8              STD_ON

/* Animation parameter */

/* Fonts */
/* Font 4x6 */
#define ANIMATION_FONT_4X6_WIDTH                4
#define ANIMATION_FONT_4X6_HEIGHT               6
/* Font 5x8 */
#define ANIMATION_FONT_5X8_WIDTH                5
#define ANIMATION_FONT_5X8_HEIGHT               8
/* Font 6x8 */
#define ANIMATION_FONT_6X8_WIDTH                6
#define ANIMATION_FONT_6X8_HEIGHT               8
/* Font 6x10 */
#define ANIMATION_FONT_6X10_WIDTH               6
#define ANIMATION_FONT_6X10_HEIGHT              10
/* Font 8x8 */
#define ANIMATION_FONT_8X8_WIDTH                8
#define ANIMATION_FONT_8X8_HEIGHT               8


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
  public:
    /* type which describes the internal state of the Animation */
    enum StateType {
        STATE_NONE,
        STATE_UNINIT,
        STATE_INIT,
        STATE_READY
    };

    enum FontType {
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
        FONT_4X6,
#endif
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
        FONT_5X8,
#endif
#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
        FONT_6X8,
#endif
#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
        FONT_6X10,
#endif
#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
        FONT_8X8,
#endif
    };

    enum AnimationType {
        ANIMATION_CURSOR,
        ANIMATION_TELETYPE,
        ANIMATION_DROP,
        ANIMATION_SHIFT,
        ANIMATION_FADE, // langsam verdunkeln
        ANIMATION_SNAKE,
        ANIMATION_WIPE,
        ANIMATION_EXPLODE,
        ANIMATION_IMPLODE,
        ANIMATION_NONE
    };

    union AnimationsType {
        AnimationCursor Cursor;
        AnimationTeletype Teletype;
        AnimationDrop Drop;
        AnimationWipe Wipe;
        AnimationShift Shift;
        AnimationSnake Snake;

        AnimationsType() {}
        ~AnimationsType() {}
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    Display* pDisplay;
    Clock* pClock;
    AnimationType CurrentAnimation;
    AnimationsType Animations;

#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
    static const unsigned char Font_4x6[][ANIMATION_FONT_4X6_WIDTH];
#endif
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
    static const unsigned char Font_5x8[][ANIMATION_FONT_5X8_WIDTH];
#endif
#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
    static const unsigned char Font_6x8[][ANIMATION_FONT_6X8_WIDTH];
#endif
#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
    static const unsigned char Font_6x10[][ANIMATION_FONT_6X10_HEIGHT];
#endif
#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
    static const unsigned char Font_8x8[][ANIMATION_FONT_8X8_WIDTH];
#endif

    // functions
    stdReturnType convertCharToFontIndex(unsigned char, byte*);
    stdReturnType setCharFontHorizontal(byte, byte, unsigned char, const unsigned char*, byte, byte);
    stdReturnType setCharFontVertical(byte, byte, unsigned char, const unsigned char*, byte, byte);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    //Animation() {}
    Animation(Display*, Clock*);
    ~Animation();

    // get methods
    AnimationType getAnimation() const { return CurrentAnimation; }

    // set methods
    void setAnimation(AnimationType);
    stdReturnType setChar(byte, byte, unsigned char, FontType);
    stdReturnType setClock(byte, byte);
    // methods
    void init();
    void task();
    void show() { pDisplay->show(); }
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
