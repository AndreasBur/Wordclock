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
/**     \file       Display_Word.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_WORD_H_
#define _DISPLAY_WORD_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/

 
/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
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
} DisplayWordWordsType;


typedef struct {
	byte Row;
	byte Column;
	byte Length;
} DisplayWordIlluminationType;


/******************************************************************************************************************************************************
 *  CLASS  DisplayWord
******************************************************************************************************************************************************/
class DisplayWord
{
  private:
  Display* pDisplay;
  static const DisplayWordIlluminationType WordIlluminationTable[];
  //boolean WordStates[DISPLAY_WORD_NUMBER_OF_WORDS];

  public:
	DisplayWord() {};
	~DisplayWord() {};

	// get methods


	// set methods

	// methods
	void setWord(DisplayWordWordsType);
	void clearWord(DisplayWordWordsType);
	void clearAll();
	void init();
	void show();
	void test();

};
#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
