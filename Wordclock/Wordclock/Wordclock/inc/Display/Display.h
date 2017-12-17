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
#include "DisplayCharacters.h"
#include "DisplayWords.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN                        10
#define DISPLAY_LED_STRIPE_SERPENTINE           STD_OFF

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS                  10
#define DISPLAY_NUMBER_OF_COLUMNS               DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
#define DISPLAY_NUMBER_OF_LEDS                  WS2812_NUMBER_OF_LEDS
#define DISPLAY_NUMBER_OF_PIXELS                DISPLAY_NUMBER_OF_LEDS

#if ((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
    #error "Display: LED number missmatch"
#endif

#define DISPLAY_WORD_LENGTH_UNLIMITED           0


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  Display
******************************************************************************************************************************************************/
class Display
{
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
  public:
    /* type which describes the internal state of the Display */
    enum StateType {
        STATE_NONE,
        STATE_UNINIT,
        STATE_INIT,
        STATE_READY
    };

    /* type which describes the  */
    struct WordIlluminationType {
        byte Row;
        byte Column;
        byte Length;
    };

    using PixelType = boolean;

#if (DISPLAY_NUMBER_OF_ROWS > 16)
    #error "Display: too many Rows, please extend PixelRowType"
#endif

    using PixelRowType = uint16_t;

#if (DISPLAY_NUMBER_OF_COLUMNS > 16)
    #error "Display: too many Columns, please extend PixelColumnType"
#endif

    using PixelColumnType = uint16_t;

    /* mapping to underlying hardware */
    using PixelColorType = WS2812PixelType;
    using Stripe = WS2812;
    using WordIdType = DisplayWords::WordIdType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    StateType State;
    Stripe Pixels;
    PixelColorType Color;

    static const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1];
    static const WordIlluminationType WordIlluminationTable[];
    
    // functions
    byte transformToSerpentine(byte, byte) const;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Display(PixelColorType);
    Display(byte, byte, byte);
    ~Display();

    // get methods
    PixelColorType getColor() const { return Color; }
    StateType getState() const { return State; }

    // set methods
    void setColor(PixelColorType sColor) { Color = sColor; }

    // char methods
    stdReturnType setCharacter(DisplayCharacters::CharacterIdType CharacterId) { return setPixel(CharacterId); }
    stdReturnType clearCharacter(DisplayCharacters::CharacterIdType CharacterId) { return clearPixel(CharacterId); }
    stdReturnType getCharacter(DisplayCharacters::CharacterIdType CharacterId, boolean* Value) const { return getPixel(CharacterId, Value); }
    stdReturnType getCharacter(byte, byte, char*) const;
    stdReturnType getCharacter(byte, char*) const;

    // char methods fast
    void setCharacterFast(DisplayCharacters::CharacterIdType CharacterId) { setPixelFast(CharacterId); }
    void clearCharacterFast(DisplayCharacters::CharacterIdType CharacterId) { clearPixelFast(CharacterId); }
    boolean getCharacterFast(DisplayCharacters::CharacterIdType CharacterId) const { return getPixelFast(CharacterId); }
    char getCharacterFast(byte Column, byte Row) const { return pgm_read_byte(&DisplayCharacters[Row][Column]); }
    char getCharacterFast(byte Index) const { return pgm_read_byte(&DisplayCharacters[Index / DISPLAY_NUMBER_OF_COLUMNS][Index % DISPLAY_NUMBER_OF_COLUMNS]); }

    // word methods
    stdReturnType getWordIllumination(WordIdType, WordIlluminationType*) const;
    stdReturnType getWordLength(byte*) const;
    stdReturnType getWordColumn(byte*) const;
    stdReturnType getWordRow(byte*) const;
    stdReturnType setWord(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    stdReturnType clearWord(WordIdType);
    stdReturnType clearAllWords();

    // word methods fast
    WordIlluminationType getWordIlluminationFast(WordIdType Word) const { WordIlluminationType WordIllu; memcpy_P(&WordIllu, &WordIlluminationTable[Word], sizeof(WordIllu)); return WordIllu; }
    byte getWordLengthFast(WordIdType Word) const { WordIlluminationType WordIllu = getWordIlluminationFast(Word); return WordIllu.Length; }
    byte getWordRowFast(WordIdType Word) const { WordIlluminationType WordIllu = getWordIlluminationFast(Word); return WordIllu.Row; }
    byte getWordColumnFast(WordIdType Word) const { WordIlluminationType WordIllu = getWordIlluminationFast(Word); return WordIllu.Column; }
    void setWordFast(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    void clearWordFast(WordIdType);
    void clearAllWordsFast();
    
    // pixel methods
    stdReturnType writePixel(byte Column, byte Row, boolean Value) { if(Value) return setPixel(Column, Row); else return clearPixel(Column, Row); }
    stdReturnType writePixel(byte Index, boolean Value) { if(Value) return setPixel(Index); else return clearPixel(Index); }
    stdReturnType setPixel(byte, byte);
    stdReturnType setPixel(byte);
    stdReturnType clearPixel(byte, byte);
    stdReturnType clearPixel(byte);
    stdReturnType togglePixel(byte, byte);
    stdReturnType togglePixel(byte);
    stdReturnType getPixel(byte, byte, boolean*) const;
    stdReturnType getPixel(byte, boolean*) const;
    stdReturnType getPixelRow(byte, PixelRowType*) const;
    stdReturnType getPixelColumn(byte, PixelColumnType*) const;
    stdReturnType setPixelRow(byte, PixelRowType);
    stdReturnType setPixelColumn(byte, PixelColumnType);

    // pixel methods fast
    void writePixelFast(byte Column, byte Row, boolean Value) { if(Value) setPixelFast(Column, Row); else clearPixelFast(Column, Row); }
    void writePixelFast(byte Index, boolean Value) { if(Value) setPixelFast(Index); else clearPixelFast(Index); }
    void setPixelFast(byte, byte);
    void setPixelFast(byte);
    void clearPixelFast(byte, byte);
    void clearPixelFast(byte);
    void togglePixelFast(byte, byte);
    void togglePixelFast(byte);
    boolean getPixelFast(byte, byte) const;
    boolean getPixelFast(byte) const;
    PixelRowType getPixelRowFast(byte) const;
    PixelColumnType getPixelColumnFast(byte) const;
    void setPixelRowFast(byte, PixelRowType);
    void setPixelColumnFast(byte, PixelColumnType);

    // methods
    void init();
    void show() { Pixels.show(); }
    void test();
    void clear() { Pixels.clearAllPixels(); }

    void indexToColumnAndRow(byte Index, byte& Row, byte& Column) const {
        Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
        Column = Index % DISPLAY_NUMBER_OF_COLUMNS;
    }
    byte columnAndRowToIndex(byte Column, byte Row) const {
        return (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
