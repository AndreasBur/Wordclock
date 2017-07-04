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
/**     \file       Display.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "WS2812.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN						10
#define DISPLAY_LED_STRIPE_SERPENTINE			STD_ON

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS					10
#define DISPLAY_NUMBER_OF_COLUMNS				11
#define DISPLAY_NUMBER_OF_LEDS					WS2812_NUMBER_OF_LEDS

#if((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
	#error "Display: LED number missmatch"
#endif

//#define DisplayLedRgbType						WS2812PixelType


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
typedef enum {
	DISPLAY_CHARACTER_E_1,
	DISPLAY_CHARACTER_S_1,
	DISPLAY_CHARACTER_K_1,
	DISPLAY_CHARACTER_I_1,
	DISPLAY_CHARACTER_S_2,
	DISPLAY_CHARACTER_T_1,
	DISPLAY_CHARACTER_L_1,
	DISPLAY_CHARACTER_F_1,
	DISPLAY_CHARACTER_UE_1,
	DISPLAY_CHARACTER_N_1,
	DISPLAY_CHARACTER_F_2,
	DISPLAY_CHARACTER_Z_1,
	DISPLAY_CHARACTER_E_2,
	DISPLAY_CHARACTER_H_1,
	DISPLAY_CHARACTER_N_2,
	DISPLAY_CHARACTER_Z_2,
	DISPLAY_CHARACTER_W_1,
	DISPLAY_CHARACTER_A_1,
	DISPLAY_CHARACTER_N_3,
	DISPLAY_CHARACTER_Z_3,
	DISPLAY_CHARACTER_I_2,
	DISPLAY_CHARACTER_G_1,
	DISPLAY_CHARACTER_D_1,
	DISPLAY_CHARACTER_R_1,
	DISPLAY_CHARACTER_E_3,
	DISPLAY_CHARACTER_I_3,
	DISPLAY_CHARACTER_V_1,
	DISPLAY_CHARACTER_I_4,
	DISPLAY_CHARACTER_E_4,
	DISPLAY_CHARACTER_R_2,
	DISPLAY_CHARACTER_T_2,
	DISPLAY_CHARACTER_E_5,
	DISPLAY_CHARACTER_L_2,
	DISPLAY_CHARACTER_T_3,
	DISPLAY_CHARACTER_G_2,
	DISPLAY_CHARACTER_N_4,
	DISPLAY_CHARACTER_A_2,
	DISPLAY_CHARACTER_C_1,
	DISPLAY_CHARACTER_H_2,
	DISPLAY_CHARACTER_V_2,
	DISPLAY_CHARACTER_O_1,
	DISPLAY_CHARACTER_R_3,
	DISPLAY_CHARACTER_J_1,
	DISPLAY_CHARACTER_M_1,
	DISPLAY_CHARACTER_H_3,
	DISPLAY_CHARACTER_A_3,
	DISPLAY_CHARACTER_L_3,
	DISPLAY_CHARACTER_B_1,
	DISPLAY_CHARACTER_Q_1,
	DISPLAY_CHARACTER_Z_4,
	DISPLAY_CHARACTER_W_2,
	DISPLAY_CHARACTER_OE_1,
	DISPLAY_CHARACTER_L_4,
	DISPLAY_CHARACTER_F_3,
	DISPLAY_CHARACTER_P_1,
	DISPLAY_CHARACTER_Z_5,
	DISPLAY_CHARACTER_W_3,
	DISPLAY_CHARACTER_E_6,
	DISPLAY_CHARACTER_I_5,
	DISPLAY_CHARACTER_N_5,
	DISPLAY_CHARACTER_S_3,
	DISPLAY_CHARACTER_I_6,
	DISPLAY_CHARACTER_E_7,
	DISPLAY_CHARACTER_B_2,
	DISPLAY_CHARACTER_E_8,
	DISPLAY_CHARACTER_N_6,
	DISPLAY_CHARACTER_K_2,
	DISPLAY_CHARACTER_D_2,
	DISPLAY_CHARACTER_R_4,
	DISPLAY_CHARACTER_E_9,
	DISPLAY_CHARACTER_I_7,
	DISPLAY_CHARACTER_R_5,
	DISPLAY_CHARACTER_H_4,
	DISPLAY_CHARACTER_F_4,
	DISPLAY_CHARACTER_UE_2,
	DISPLAY_CHARACTER_N_7,
	DISPLAY_CHARACTER_F_5,
	DISPLAY_CHARACTER_E_10,
	DISPLAY_CHARACTER_L_5,
	DISPLAY_CHARACTER_F_6,
	DISPLAY_CHARACTER_N_8,
	DISPLAY_CHARACTER_E_11,
	DISPLAY_CHARACTER_U_3,
	DISPLAY_CHARACTER_N_9,
	DISPLAY_CHARACTER_V_3,
	DISPLAY_CHARACTER_I_8,
	DISPLAY_CHARACTER_E_12,
	DISPLAY_CHARACTER_R_6,
	DISPLAY_CHARACTER_W_4,
	DISPLAY_CHARACTER_A_4,
	DISPLAY_CHARACTER_C_2,
	DISPLAY_CHARACTER_H_5,
	DISPLAY_CHARACTER_T_5,
	DISPLAY_CHARACTER_Z_6,
	DISPLAY_CHARACTER_E_13,
	DISPLAY_CHARACTER_H_6,
	DISPLAY_CHARACTER_N_10,
	DISPLAY_CHARACTER_R_7,
	DISPLAY_CHARACTER_S_4,
	DISPLAY_CHARACTER_B_3,
	DISPLAY_CHARACTER_S_5,
	DISPLAY_CHARACTER_E_14,
	DISPLAY_CHARACTER_C_3,
	DISPLAY_CHARACTER_H_7,
	DISPLAY_CHARACTER_S_6,
	DISPLAY_CHARACTER_F_7,
	DISPLAY_CHARACTER_M_2,
	DISPLAY_CHARACTER_U_4,
	DISPLAY_CHARACTER_H_8,
	DISPLAY_CHARACTER_R_8,
	DISPLAY_CHARACTER_NUMBER_OF_CHARACTERS
} DisplayCharactersType;


typedef enum {
	DISPLAY_WORD_NONE,
	DISPLAY_WORD_ES,
	DISPLAY_WORD_IST,
	DISPLAY_WORD_FUENF_1,
	DISPLAY_WORD_ZEHN_1,
	DISPLAY_WORD_ZWANZIG,
	DISPLAY_WORD_DREI_1,
	DISPLAY_WORD_VIER,
	DISPLAY_WORD_VIERTEL,
	DISPLAY_WORD_DREIVIERTEL,
	DISPLAY_WORD_NACH,
	DISPLAY_WORD_VOR,
	DISPLAY_WORD_HALB,
	DISPLAY_WORD_ZWOELF,
	DISPLAY_WORD_ZWEI,
	DISPLAY_WORD_EIN,
	DISPLAY_WORD_EINS,
	DISPLAY_WORD_SIEBEN,
	DISPLAY_WORD_DREI_2,
	DISPLAY_WORD_FUENF_2,
	DISPLAY_WORD_ELF,
	DISPLAY_WORD_NEUN,
	DISPLAY_WORD_VIER_2,
	DISPLAY_WORD_ACHT,
	DISPLAY_WORD_ZEHN_2,
	DISPLAY_WORD_SECHS,
	DISPLAY_WORD_UHR,
	DISPLAY_WORD_NUMBER_OF_WORDS
} DisplayWordsType;


typedef struct {
	byte Row;
	byte Column;
	byte Length;
} DisplayWordIlluminationType;


/******************************************************************************************************************************************************
 *  CLASS  Display
******************************************************************************************************************************************************/
class Display
{
  private:
	WS2812 Pixels;
	WS2812PixelType DisplayColor;

	static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];
	static const DisplayWordIlluminationType WordIlluminationTable[];
	
	// functions
	byte transformToSerpentine(byte, byte);
	stdReturnType setLed(byte Row, byte Column);
	stdReturnType setLed(byte Index);

  public:
	Display();
	~Display();

	// get methods


	// set methods

	
	// methods

	// char methods
	void setCharacter(DisplayCharactersType Character);
	void clearCharacter(DisplayCharactersType Character);
	stdReturnType getCharacter(byte Row, byte Column, char* Character);
	stdReturnType getCharacter(byte Index, char* Character);

	// word methods
	void setWord(DisplayWordsType);
	void clearWord(DisplayWordsType);
	void clearAllWords();

	void init();
	void show() { Pixels.show(); }
	void test();
	
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
