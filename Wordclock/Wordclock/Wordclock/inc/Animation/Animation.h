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


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Animation configuration parameter*/
#define ANIMATION_SUPPORT_FONT_4X6				STD_ON
#define ANIMATION_SUPPORT_FONT_5X8				STD_ON
#define ANIMATION_SUPPORT_FONT_6X8				STD_ON
#define ANIMATION_SUPPORT_FONT_6X10				STD_ON
#define ANIMATION_SUPPORT_FONT_8X8				STD_ON

/* Animation parameter */

/* Fonts */
/* Font 4x6 */
#define ANIMATION_FONT_4X6_WIDTH				4
#define ANIMATION_FONT_4X6_HEIGHT				6
/* Font 5x8 */
#define ANIMATION_FONT_5X8_WIDTH				5
#define ANIMATION_FONT_5X8_HEIGHT				8
/* Font 6x8 */
#define ANIMATION_FONT_6X8_WIDTH				6
#define ANIMATION_FONT_6X8_HEIGHT				8
/* Font 6x10 */
#define ANIMATION_FONT_6X10_WIDTH				6
#define ANIMATION_FONT_6X10_HEIGHT				10
/* Font 8x8 */
#define ANIMATION_FONT_8X8_WIDTH				8
#define ANIMATION_FONT_8X8_HEIGHT				8


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/* type which describes the internal state of the Animation */
typedef enum {
	ANIMATION_STATE_NONE,
	ANIMATION_STATE_UNINIT,
	ANIMATION_STATE_INIT,
	ANIMATION_STATE_READY
} AnimationStateType;


typedef enum {
#if(ANIMATION_SUPPORT_FONT_4X6 == STD_ON)
	ANIMATION_FONT_4X6,
#endif
#if(ANIMATION_SUPPORT_FONT_5X8 == STD_ON)
	ANIMATION_FONT_5X8,
#endif
#if(ANIMATION_SUPPORT_FONT_6X8 == STD_ON)
	ANIMATION_FONT_6X8,
#endif
#if(ANIMATION_SUPPORT_FONT_6X10 == STD_ON)
	ANIMATION_FONT_6X10,
#endif
#if(ANIMATION_SUPPORT_FONT_8X8 == STD_ON)
	ANIMATION_FONT_8X8,
#endif
} AnimationFontType;


typedef enum {
	ANIMATION_SNAKE,
	ANIMATION_EXPLODE,
	ANIMATION_IMPLODE,
	ANIMATION_CUBE
} AnimationType;


/******************************************************************************************************************************************************
 *  CLASS  Template
******************************************************************************************************************************************************/
class Animation
{
  private:
	Display* pDisplay;
	AnimationType CurrentAnimation;

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

  public:
	Animation(Display*);
	~Animation();

	// get methods


	// set methods
	stdReturnType setChar(byte, byte, unsigned char, AnimationFontType);

	// methods
	void init();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
