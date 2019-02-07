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
#include "GammaCorrection.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN                        10
#define DISPLAY_LED_STRIPE_SERPENTINE           STD_OFF
#define DISPLAY_USE_WS2812_DIMMING              STD_ON
//#define Pixels                                  WS2812::getInstance()

#if (DISPLAY_USE_WS2812_DIMMING == STD_ON) && (WS2812_SUPPORT_DIMMING == STD_OFF)
    #error "Display: Please activate WS2812 dimming support"
#endif

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS                  DISPLAY_CHARACTERS_NUMBER_OF_ROWS
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
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y
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

    using PixelType = boolean;
    using CharacterIdType = DisplayCharacters::CharacterIdType;

#if (DISPLAY_NUMBER_OF_ROWS > 16)
    #error "Display: too many rows, please extend PixelRowType"
#endif

    using PixelRowType = uint16_t;

#if (DISPLAY_NUMBER_OF_COLUMNS > 16)
    #error "Display: too many columns, please extend PixelColumnType"
#endif

    using PixelColumnType = uint16_t;

    /* mapping to underlying hardware */
    using PixelColorType = WS2812::PixelType;
    using Stripe = WS2812;
    using WordIdType = DisplayWords::WordIdType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    StateType State;
    Stripe Pixels;
    PixelColorType Color;
    DisplayWords Words;

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    GammaCorrection GCorrection;
    byte Brightness;
    PixelColorType ColorDimmed;
#endif

    // functions
    byte transformToSerpentine(byte, byte) const;
    byte transformToSerpentine(byte) const;

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    byte dimmColor(byte Color) const { return (Color * Brightness) >> 8; }
#endif

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

#if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    byte getBrightness() const { return Pixels.getBrightness(); }
#else
    byte getBrightness() const { return Brightness; }
#endif

    // set methods
    void setColor(PixelColorType sColor) { Color = sColor; }
    void setColor(byte Red, byte Green, byte Blue) { Color.Red = Red; Color.Green = Green; Color.Blue = Blue; }

#if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    void setBrightness(byte Brightness) { Pixels.setBrightness(Brightness, true); }
#else
    void setBrightness(byte);
#endif

    // char methods
    stdReturnType setCharacter(CharacterIdType CharacterId) { return setPixel(CharacterId); }
    stdReturnType clearCharacter(CharacterIdType CharacterId) { return clearPixel(CharacterId); }
    stdReturnType getCharacter(CharacterIdType CharacterId, boolean* Value) const { return getPixel(CharacterId, Value); }

    // char methods fast
    void setCharacterFast(CharacterIdType CharacterId) { setPixelFast(CharacterId); }
    void clearCharacterFast(CharacterIdType CharacterId) { clearPixelFast(CharacterId); }
    boolean getCharacterFast(CharacterIdType CharacterId) const { return getPixelFast(CharacterId); }

    // word methods
    stdReturnType setWord(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    stdReturnType clearWord(WordIdType);
    stdReturnType clearWords();

    // word methods fast
    void setWordFast(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    void clearWordFast(WordIdType);
    void clearWordsFast();

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
    void clear() { Pixels.clearPixels(); }
    void indexToColumnAndRow(byte Index, byte& Column, byte& Row) const { Row = Index / DISPLAY_NUMBER_OF_COLUMNS; Column = Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToColumn(byte Index) const { return Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToRow(byte Index) const { return Index / DISPLAY_NUMBER_OF_COLUMNS; }
    byte columnAndRowToIndex(byte Column, byte Row) const { return (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column; }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
