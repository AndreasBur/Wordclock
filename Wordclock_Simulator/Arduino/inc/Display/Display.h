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
#include "Pixels.h"
#include "DisplayCharacters.h"
#include "DisplayWords.h"
#include "DisplayBrightness.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN                        10u
#define DISPLAY_LED_STRIPE_SERPENTINE           STD_OFF
#define DISPLAY_USE_PIXELS_DIMMING              STD_OFF

//# if (PIXELS_IS_SINGLETON == STD_ON)
//#  define Pixels                              Pixels::getInstance()
//# endif

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON) && (PIXELS_SUPPORT_DIMMING == STD_OFF)
#  error "Display: Please activate Pixels dimming support"
# endif

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS                  DISPLAY_CHARACTERS_NUMBER_OF_ROWS
#define DISPLAY_NUMBER_OF_COLUMNS               DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
#define DISPLAY_NUMBER_OF_LEDS                  PIXELS_NUMBER_OF_LEDS
#define DISPLAY_NUMBER_OF_PIXELS                DISPLAY_NUMBER_OF_LEDS

# if ((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
#  error "Display: LED number missmatch"
# endif

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

    using PixelValueType = bool;
    using CharacterIdType = DisplayCharacters::CharacterIdType;

# if (DISPLAY_NUMBER_OF_ROWS > 16u)
#  error "Display: too many rows, please extend PixelRowType"
# endif

    using PixelRowType = uint16_t;

# if (DISPLAY_NUMBER_OF_COLUMNS > 16u)
#  error "Display: too many columns, please extend PixelColumnType"
# endif

    using PixelColumnType = uint16_t;

    /* mapping to underlying hardware */
    using IlluminanceType = Illuminance::IlluminanceType;
    using IndexType = Pixels::IndexType;
    using ColorType = Pixel::ColorType;
    using WordIdType = DisplayWords::WordIdType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte WordLengthUnlimited{0u};
    StateType State{STATE_NONE};
# if (PIXELS_IS_SINGLETON == STD_OFF)
    Pixels PixelStripe;
# else
    Pixels& PixelStripe;
# endif
    Pixel Color{255u, 255u, 255u};
    DisplayWords Words;
    DisplayBrightness Brightness; 

# if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    Pixel ColorDimmed{255u, 255u, 255u};
# endif

    // functions
    Display(Pixel);
    Display(byte, byte, byte);
    ~Display();

    byte transformToSerpentine(byte, byte) const;
    byte transformToSerpentine(byte) const;

# if (DISPLAY_USE_PIXELS_DIMMING == STD_OFF)
    byte dimmColor(byte Color, byte BrightnessCorrected) const {
        byte dimmedColor = (Color * BrightnessCorrected) >> 8u;
        if(dimmedColor == 0u) { return 1u; }
        else { return dimmedColor; }
    }
# endif

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Display& getInstance() {
        static Display SingletonInstance(255u, 255u, 255u);
        return SingletonInstance;
    }

    // get methods
    StateType getState() const { return State; }
    bool getBrightnessUseAutomatic() const { return Brightness.getUseAutomatic(); }
    bool getBrightnessUseGammaCorrection() const { return Brightness.getUseGammaCorrection(); }

    Pixel getColor() const { return Color; }
    ColorType getColorRed() const { return Color.getRed(); }
    ColorType getColorGreen() const { return Color.getGreen(); }
    ColorType getColorBlue() const { return Color.getBlue(); }

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    byte getBrightness() const { return PixelStripe.getBrightness(); }
# else
    byte getBrightness() const { return Brightness.getBrightness(); }
# endif

    // set methods
    void setBrightnessUseAutomatic(bool BrightnessUseAutomatic) { Brightness.setUseAutomatic(BrightnessUseAutomatic); }
    void setBrightnessUseGammaCorrection(bool BrightnessUseGammaCorrection) { Brightness.setUseGammaCorrection(BrightnessUseGammaCorrection); }
    void setColor(Pixel sColor) { Color = sColor; }
    void setColor(ColorType Red, byte Green, ColorType Blue) { Color.setRed(Red); Color.setGreen(Green); Color.setBlue(Blue); }
    void setColorRed(ColorType Red) { Color.setRed(Red); }
    void setColorGreen(ColorType Green) { Color.setGreen(Green); }
    void setColorBlue(ColorType Blue) { Color.setBlue(Blue);}

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    void setBrightness(byte sBrightness) {
        Brightness.setBrightness(sBrightness);
        PixelStripe.setBrightness(Brightness.calcBrightness());
    }
# else
    void setBrightness(byte);
# endif

    // char methods
    StdReturnType setCharacter(CharacterIdType CharacterId) { return setPixel(CharacterId); }
    StdReturnType clearCharacter(CharacterIdType CharacterId) { return clearPixel(CharacterId); }
    StdReturnType getCharacter(CharacterIdType CharacterId, PixelValueType& Value) const { return getPixel(CharacterId, Value); }

    // char methods fast
    void setCharacterFast(CharacterIdType CharacterId) { setPixelFast(CharacterId); }
    void clearCharacterFast(CharacterIdType CharacterId) { clearPixelFast(CharacterId); }
    bool getCharacterFast(CharacterIdType CharacterId) const { return getPixelFast(CharacterId); }

    // word methods
    StdReturnType setWord(WordIdType, byte MaxLength = WordLengthUnlimited);
    StdReturnType clearWord(WordIdType);
    StdReturnType clearWords();

    // word methods fast
    void setWordFast(WordIdType, byte MaxLength = WordLengthUnlimited);
    void clearWordFast(WordIdType);
    void clearWordsFast();

    // pixel methods
    StdReturnType writePixel(byte Column, byte Row, PixelValueType Value) { if(Value) return setPixel(Column, Row); else return clearPixel(Column, Row); }
    StdReturnType writePixel(IndexType Index, PixelValueType Value) { if(Value) return setPixel(Index); else return clearPixel(Index); }
    StdReturnType setPixel(byte, byte);
    StdReturnType setPixel(IndexType);
    StdReturnType clearPixel(byte, byte);
    StdReturnType clearPixel(IndexType);
    StdReturnType togglePixel(byte, byte);
    StdReturnType togglePixel(IndexType);
    StdReturnType getPixel(byte, byte, PixelValueType&) const;
    StdReturnType getPixel(IndexType, PixelValueType&) const;
    StdReturnType getPixelRow(byte, PixelRowType&) const;
    StdReturnType getPixelColumn(byte, PixelColumnType&) const;
    StdReturnType setPixelRow(byte, PixelRowType);
    StdReturnType setPixelColumn(byte, PixelColumnType);

    // pixel methods fast
    bool isIndexValid(IndexType Index) { return PixelStripe.isIndexValid(Index); }
    void writePixelFast(byte Column, byte Row, PixelValueType Value) { if(Value) setPixelFast(Column, Row); else clearPixelFast(Column, Row); }
    void writePixelFast(IndexType Index, PixelValueType Value) { if(Value) setPixelFast(Index); else clearPixelFast(Index); }
    void setPixelFast(byte, byte);
    void setPixelFast(byte);
    void clearPixelFast(byte, byte);
    void clearPixelFast(byte);
    void togglePixelFast(byte, byte);
    void togglePixelFast(byte);
    PixelValueType getPixelFast(byte, byte) const;
    PixelValueType getPixelFast(byte) const;
    PixelRowType getPixelRowFast(byte) const;
    PixelColumnType getPixelColumnFast(byte) const;
    void setPixelRowFast(byte, PixelRowType);
    void setPixelColumnFast(byte, PixelColumnType);

    // methods
    void init();
    StdReturnType show() { return PixelStripe.show(); }
    void enable() { PixelStripe.enablePixels(); }
    void disable() { PixelStripe.disablePixels(); }
    void enableBrightnessAutomatic() { Brightness.enableAutomatic(); }
    void disableBrightnessAutomatic() { Brightness.disableAutomatic(); }
    void enbleBrightnessGammaCorrection() { Brightness.enableGammaCorrection(); }
    void disableBrightnessGammaCorrection() { Brightness.disableGammaCorrection(); }
    void test() { PixelStripe.setPixels(ColorDimmed); }
    void clear() { PixelStripe.clearPixels(); }
    bool isCleared() { for(byte Index = 0; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) { if(getPixelFast(Index)) return false; } return true; }

    void incrementColorRed() { Color.incrementColorRed(); }
    void incrementColorGreen() {  Color.incrementColorGreen(); }
    void incrementColorBlue() { Color.incrementColorBlue(); }
    void incrementBrightness() { Brightness.incrementBrightness(); }

    void decrementColorRed() { Color.decrementColorRed(); }
    void decrementColorGreen() { Color.decrementColorGreen(); }
    void decrementColorBlue() { Color.decrementColorBlue(); }
    void decrementBrightness() { Brightness.decrementBrightness(); }

    static void indexToColumnAndRow(IndexType Index, byte& Column, byte& Row) { Row = Index / DISPLAY_NUMBER_OF_COLUMNS; Column = Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToColumn(IndexType Index) const { return Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToRow(IndexType Index) const { return Index / DISPLAY_NUMBER_OF_COLUMNS; }
    byte columnAndRowToIndex(byte Column, byte Row) const { return (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column; }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
