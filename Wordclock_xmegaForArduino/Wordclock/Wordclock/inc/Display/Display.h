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
#include "BH1750.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN                                        10u
#define DISPLAY_LED_STRIPE_SERPENTINE                           STD_OFF
#define DISPLAY_USE_WS2812_DIMMING                              STD_OFF
#define DISPLAY_BRIGHTNESS_AUTOMATIC_CORRECTION_FACTOR          1.0f

#if (WS2812_IS_SINGLETON == STD_ON)
    #define Pixels                              WS2812::getInstance()
#endif

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

#define DISPLAY_WORD_LENGTH_UNLIMITED           0u


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

#if (DISPLAY_NUMBER_OF_ROWS > 16u)
    #error "Display: too many rows, please extend PixelRowType"
#endif

    using PixelRowType = uint16_t;

#if (DISPLAY_NUMBER_OF_COLUMNS > 16u)
    #error "Display: too many columns, please extend PixelColumnType"
#endif

    using PixelColumnType = uint16_t;

    /* mapping to underlying hardware */
    using IlluminanceType = BH1750::IlluminanceType;
    
    using PixelColorType = WS2812::PixelType;
    using ColorType = NeoPixel::ColorType;
    using Stripe = WS2812;
    using WordIdType = DisplayWords::WordIdType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    StateType State;
    boolean BrightnessAutomatic;
#if (WS2812_IS_SINGLETON == STD_OFF)
    Stripe Pixels;
#endif
    PixelColorType Color;
    DisplayWords Words;

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    GammaCorrection GCorrection;
    byte Brightness;
    PixelColorType ColorDimmed;
#endif
    
    // functions
    Display(PixelColorType);
    Display(byte, byte, byte);
    ~Display();
    
    byte calculateBrightnessAutomaticCorrected(byte) const;
    
    byte transformToSerpentine(byte, byte) const;
    byte transformToSerpentine(byte) const;

#if (DISPLAY_USE_WS2812_DIMMING == STD_OFF)
    byte dimmColor(byte Color, byte BrightnessCorrected) const {
        byte dimmedColor = (Color * BrightnessCorrected) >> 8u;
        if(dimmedColor == 0u) return 1u;
        else return dimmedColor;
    }
#endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Display& getInstance() {
        static Display SingletonInstance(0u, 0u, 0u);
        return SingletonInstance;
    }

    // get methods
    StateType getState() const { return State; }
    boolean getBrightnessAutomatic() const { return BrightnessAutomatic; }
    
    PixelColorType getColor() const { return Color; }
    ColorType getColorRed() const { return Color.Red; }
    ColorType getColorGreen() const { return Color.Green; }
    ColorType getColorBlue() const { return Color.Blue; }

#if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    byte getBrightness() const { return Pixels.getBrightness(); }
#else
    byte getBrightness() const { return Brightness; }
#endif

    // set methods
    void setBrightnessAutomatic(boolean sBrightnessAutomatic) { BrightnessAutomatic = sBrightnessAutomatic; }
    void setColor(PixelColorType sColor) { Color = sColor; }
    void setColor(ColorType Red, byte Green, ColorType Blue) { Color.Red = Red; Color.Green = Green; Color.Blue = Blue; }
    void setColorRed(ColorType Red) { Color.Red = Red; }
    void setColorGreen(ColorType Green) { Color.Green = Green; }
    void setColorBlue(ColorType Blue) { Color.Blue = Blue;}

#if (DISPLAY_USE_WS2812_DIMMING == STD_ON)
    void setBrightness(byte Brightness) { 
        if(BrightnessAutomatic == true) { Pixels.setBrightness(Brightness, true); }
        else { Pixels.setBrightness(calculateBrightnessAutomaticCorrected(Brightness)); } 
    }
#else
    void setBrightness(byte);
#endif

    // char methods
    StdReturnType setCharacter(CharacterIdType CharacterId) { return setPixel(CharacterId); }
    StdReturnType clearCharacter(CharacterIdType CharacterId) { return clearPixel(CharacterId); }
    StdReturnType getCharacter(CharacterIdType CharacterId, boolean& Value) const { return getPixel(CharacterId, Value); }

    // char methods fast
    void setCharacterFast(CharacterIdType CharacterId) { setPixelFast(CharacterId); }
    void clearCharacterFast(CharacterIdType CharacterId) { clearPixelFast(CharacterId); }
    boolean getCharacterFast(CharacterIdType CharacterId) const { return getPixelFast(CharacterId); }

    // word methods
    StdReturnType setWord(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    StdReturnType clearWord(WordIdType);
    StdReturnType clearWords();

    // word methods fast
    void setWordFast(WordIdType, byte MaxLength = DISPLAY_WORD_LENGTH_UNLIMITED);
    void clearWordFast(WordIdType);
    void clearWordsFast();
    
    // pixel methods
    StdReturnType writePixel(byte Column, byte Row, boolean Value) { if(Value) return setPixel(Column, Row); else return clearPixel(Column, Row); }
    StdReturnType writePixel(byte Index, boolean Value) { if(Value) return setPixel(Index); else return clearPixel(Index); }
    StdReturnType setPixel(byte, byte);
    StdReturnType setPixel(byte);
    StdReturnType clearPixel(byte, byte);
    StdReturnType clearPixel(byte);
    StdReturnType togglePixel(byte, byte);
    StdReturnType togglePixel(byte);
    StdReturnType getPixel(byte, byte, boolean&) const;
    StdReturnType getPixel(byte, boolean&) const;
    StdReturnType getPixelRow(byte, PixelRowType&) const;
    StdReturnType getPixelColumn(byte, PixelColumnType&) const;
    StdReturnType setPixelRow(byte, PixelRowType);
    StdReturnType setPixelColumn(byte, PixelColumnType);

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
    void enable() { Pixels.enablePixels(); }
    void disable() { Pixels.disablePixels(); }
    void enableBrightnessAutomatic() { BrightnessAutomatic = true; }
    void disableBrightnessAutomatic() { BrightnessAutomatic = false; }
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
