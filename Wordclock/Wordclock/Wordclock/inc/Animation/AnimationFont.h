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
/**     \file       AnimationFont.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _ANIMATION_FONT_H_
#define _ANIMATION_FONT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* AnimationFont configuration parameter */
#define ANIMATION_SUPPORT_FONT_4X6              STD_ON
#define ANIMATION_SUPPORT_FONT_5X8              STD_ON
#define ANIMATION_SUPPORT_FONT_6X8              STD_ON
#define ANIMATION_SUPPORT_FONT_6X10             STD_ON
#define ANIMATION_SUPPORT_FONT_8X8              STD_ON

/* AnimationFont parameter */
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
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class AnimationFont
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
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
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
      Display* pDisplay;
  
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
    AnimationFont(Display*);
    ~AnimationFont();

	// get methods


	// set methods

	// methods
    void init();
    stdReturnType setChar(byte, byte, unsigned char, FontType);
    void show() { pDisplay->show(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
