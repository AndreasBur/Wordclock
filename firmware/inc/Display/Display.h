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
#include "DisplayColor.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
#define DISPLAY_DATA_PIN                            10u
#define DISPLAY_LED_STRIPE_SERPENTINE               STD_OFF
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
# define DISPLAY_USE_PIXELS_DIMMING                 STD_OFF
#else
# define DISPLAY_USE_PIXELS_DIMMING                 STD_ON
#endif

//# if (PIXELS_IS_SINGLETON == STD_ON)
//#  define Pixels                              Pixels::getInstance()
//# endif

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON) && (PIXELS_SUPPORT_DIMMING == STD_OFF)
#  error "Display: Please activate Pixels dimming support"
# endif

/* Display parameter */
#define DISPLAY_NUMBER_OF_ROWS                      DISPLAY_CHARACTERS_NUMBER_OF_ROWS
#define DISPLAY_NUMBER_OF_COLUMNS                   DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
#define DISPLAY_NUMBER_OF_LEDS                      PIXELS_NUMBER_OF_LEDS
#define DISPLAY_NUMBER_OF_PIXELS                    DISPLAY_NUMBER_OF_LEDS

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
    /* Whether the display was last asked to be on, true because a strip comes up lit. Kept
       here rather than read back from the strip's master brightness, which is what enable()
       and disable() write: under DISPLAY_USE_PIXELS_DIMMING that same register carries the
       brightness setting, so a zero in it would answer "dark" where the question is
       "switched off". */
    bool Enabled{true};
    /* brightness the pixels on the display were last written with. Zero rather than the
       initial brightness, so the first task establishes a defined state; the display is
       still empty then, which makes that first pass free. */
    byte AppliedBrightness{0u};
# if (PIXELS_IS_SINGLETON == STD_OFF)
    Pixels PixelStripe;
# else
    Pixels& PixelStripe;
# endif
    DisplayColor Color;
    DisplayWords Words;
    DisplayBrightness Brightness; 

    // functions
    Display(Pixel);
    Display(byte, byte, byte);
    ~Display();

    byte transformToSerpentine(byte, byte) const;
    byte transformToSerpentine(byte) const;

    void applyColor();

    Pixel getColorDimmed(byte);

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

    Pixel getColor() const { return Color.getColor(); }
    ColorType getColorRed() const { return Color.getColorRed(); }
    ColorType getColorGreen() const { return Color.getColorGreen(); }
    ColorType getColorBlue() const { return Color.getColorBlue(); }

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
    byte getBrightness() const { return PixelStripe.getBrightness(); }
# else
    byte getBrightness() const { return Brightness.getBrightness(); }
# endif

    void setBrightnessUseAutomatic(bool BrightnessUseAutomatic) {
        Brightness.setUseAutomatic(BrightnessUseAutomatic);
        applyBrightness();
    }
    void setBrightnessUseGammaCorrection(bool BrightnessUseGammaCorrection) {
        Brightness.setUseGammaCorrection(BrightnessUseGammaCorrection);
        applyBrightness();
    }
    /* Every one of them applies what it set, for the reason applyColor() gives. */
    void setColor(Pixel sColor) { Color.setColor(sColor); applyColor(); }
    void setColor(ColorType Red, ColorType Green, ColorType Blue) { Color.setColorRed(Red); Color.setColorGreen(Green); Color.setColorBlue(Blue); applyColor(); }
    void setColorRed(ColorType Red) { Color.setColorRed(Red); applyColor(); }
    void setColorGreen(ColorType Green) { Color.setColorGreen(Green); applyColor(); }
    void setColorBlue(ColorType Blue) { Color.setColorBlue(Blue); applyColor(); }

    /* Takes the brightness the user asked for. What reaches the LEDs is
       calcBrightness(), which folds in gamma correction and the light sensor. */
    void setBrightness(byte);

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
    StdReturnType setPixel(byte, byte, byte);
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
    void setPixelFast(byte, byte, byte);
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
    void enable() { Enabled = true; PixelStripe.enablePixels(); }
    void disable() { Enabled = false; PixelStripe.disablePixels(); }
    bool isEnabled() const { return Enabled; }
    /* The other way round from whatever it is now, for a button that has nothing to read
       the state off. The three toggles below are the same idea for the two settings that
       are a yes or a no. */
    void toggle() { if(isEnabled()) { disable(); } else { enable(); } }

    /* The strip as a wire rather than as a picture, for the supply switch: whether a frame
       is still waiting to go out, whether one is still going out, and the gate that stops
       any from going out at all. Forwarded here rather than reached through Pixels directly
       so that the module driving the switch needs one collaborator instead of two - and so
       the rule stays where the rest of the strip's handling is.

       isDirty() and isFrameOnTheWire() are two different waits and both are needed: the
       first says the blanked frame has not been handed over yet, the second that it has and
       is still on the wire. Cutting the supply at either point leaves the strip holding
       half a frame with nothing to latch it. */
    bool isDirty() const { return PixelStripe.isDirty(); }
    bool isFrameOnTheWire() const { return PixelStripe.isFrameOnTheWire(); }
    bool isOutputSuspended() const { return PixelStripe.isOutputSuspended(); }
    void suspendOutput() { PixelStripe.suspendOutput(); }
    void resumeOutput() { PixelStripe.resumeOutput(); }
    void enableBrightnessAutomatic() { Brightness.enableAutomatic(); }
    void disableBrightnessAutomatic() { Brightness.disableAutomatic(); }
    void enableBrightnessGammaCorrection() { Brightness.enableGammaCorrection(); }
    void disableBrightnessGammaCorrection() { Brightness.disableGammaCorrection(); }
    /* Through the setters rather than through the enable/disable pair above, which is the
       half of it that leaves applyBrightness() to the next task: a knob turned once should
       show what it did, not a tick later. */
    void toggleBrightnessAutomatic() { setBrightnessUseAutomatic(!getBrightnessUseAutomatic()); }
    void toggleBrightnessGammaCorrection() { setBrightnessUseGammaCorrection(!getBrightnessUseGammaCorrection()); }
    void test() { PixelStripe.setPixels(Color.getColorDimmed()); }
    void clear() { PixelStripe.clearPixels(); }
    bool isCleared() { for(byte Index = 0; Index < DISPLAY_NUMBER_OF_PIXELS; Index++) { if(getPixelFast(Index)) return false; } return true; }

    void incrementColorRed() { Color.incrementColorRed(); applyColor(); }
    void incrementColorGreen() {  Color.incrementColorGreen(); applyColor(); }
    void incrementColorBlue() { Color.incrementColorBlue(); applyColor(); }
    void incrementBrightness() { Brightness.incrementBrightness(); applyBrightness(); }

    void decrementColorRed() { Color.decrementColorRed(); applyColor(); }
    void decrementColorGreen() { Color.decrementColorGreen(); applyColor(); }
    void decrementColorBlue() { Color.decrementColorBlue(); applyColor(); }
    void decrementBrightness() { Brightness.decrementBrightness(); applyBrightness(); }

    /* White again, and through DisplayColor's own reset so that what this puts back is the
       one named value the settings reset and the member initialiser already use. */
    void resetColor() { Color.resetToDefaults(); applyColor(); }

    void applyBrightness();

    /* What AnimationFade dims the display with while it runs. It goes through the display
       rather than through the strip because applyBrightness() runs on every task and
       would otherwise write the configured brightness back over it - and it is a level of
       its own rather than the brightness setting, so that what a fade dims is never what
       Persistence stores. */
    void setBrightnessFadeLevel(byte Level) { Brightness.setFadeLevel(Level); applyBrightness(); }
    void clearBrightnessFade() { Brightness.clearFade(); applyBrightness(); }

    /* The night switch's dimming, which scales what the setting arrived at instead of
       replacing it - so the brightness the owner chose is what morning returns to. */
    byte getBrightnessNightLevel() const { return Brightness.getNightLevel(); }
    void setBrightnessNightLevel(byte Level) { Brightness.setNightLevel(Level); applyBrightness(); }
    void clearBrightnessNight() { Brightness.clearNight(); applyBrightness(); }

    /* Back to what a clock that was never configured shows. Each part answers for its own
       defaults, so nothing here has to know what they are; the brightness is applied
       afterwards, because that is what recalculates what reaches the LEDs. */
    void resetToDefaults() {
        Color.resetToDefaults();
        Brightness.resetToDefaults();
        applyBrightness();
        /* After the brightness, not instead of it: applyBrightness() does nothing where the
           brightness was already at its default, and the colour would stay behind. */
        applyColor();
    }

    static void indexToColumnAndRow(IndexType Index, byte& Column, byte& Row) { Row = Index / DISPLAY_NUMBER_OF_COLUMNS; Column = Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToColumn(IndexType Index) const { return Index % DISPLAY_NUMBER_OF_COLUMNS; }
    byte indexToRow(IndexType Index) const { return Index / DISPLAY_NUMBER_OF_COLUMNS; }
    byte columnAndRowToIndex(byte Column, byte Row) const { return (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column; }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
