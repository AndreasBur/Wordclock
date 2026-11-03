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
#include "DisplayCurrentLimit.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Display configuration parameter */
/* The one display parameter that is a property of the board rather than of the display:
   which pin is free is decided by the part, not by the strip. So a board that cannot use
   this one overrides it from its own build instead of editing here - the way
   WORDCLOCK_VERSION is overridden - because every target shares this header, and editing
   the line would move the pin for all four of them at once. Which pin each backend needs is
   in its own README. */
#ifndef DISPLAY_DATA_PIN
# define DISPLAY_DATA_PIN                           10u
#endif
#define DISPLAY_LED_STRIPE_SERPENTINE               STD_OFF
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
# define DISPLAY_USE_PIXELS_DIMMING                 STD_OFF
#else
# define DISPLAY_USE_PIXELS_DIMMING                 STD_ON
#endif


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
        kStateNone,
        kStateUninit,
        kStateInit,
        kStateReady
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
    static constexpr byte kWordLengthUnlimited{0u};
    StateType state_{kStateNone};
    /* Whether the display was last asked to be on, true because a strip comes up lit. Kept
       here rather than read back from the strip's master brightness, which is what Enable()
       and Disable() write: under DISPLAY_USE_PIXELS_DIMMING that same register carries the
       brightness setting, so a zero in it would answer "dark" where the question is
       "switched off". */
    bool enabled_{true};
    /* brightness the pixels on the display were last written with. Zero rather than the
       initial brightness, so the first task establishes a defined state; the display is
       still empty then, which makes that first pass free. */
    byte applied_brightness_{0u};
    /* master the strip was last handed. Full, because that is where Pixels starts - and
       because writing it marks the frame dirty, so this exists to write it only when the cap
       has actually moved rather than on every task run. */
    byte applied_current_limit_{DisplayCurrentLimit::kNoLimit};
# if (PIXELS_IS_SINGLETON == STD_OFF)
    Pixels pixel_stripe_;
# else
    Pixels& pixel_stripe_;
# endif
    DisplayColor color_;
    DisplayWords words_;
    DisplayBrightness brightness_; 

    // functions
    Display(Pixel);
    Display(byte, byte, byte);
    ~Display();

    static byte TransformToSerpentine(byte, byte);
    static byte TransformToSerpentine(byte);
    /* The one place a column and a row become an index on the strip, so the wiring is
       written once instead of as a #if in every entry point that takes a column. */
    static byte ToIndex(byte column, byte row) {
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
        /* if led stripe is snake or serpentine the odd row: count from right to left */
        return TransformToSerpentine(column, row);
#else
        return ColumnAndRowToIndex(column, row);
#endif
    }

    void ApplyColor();
    /* A colour change moves the current as much as a brightness change does - white draws
       three times what amber at the same brightness does - so both have to go past the
       limit. This is what the colour setters call instead of ApplyColor() alone. */
    void ApplyColorAndCurrentLimit();

    void ApplyCurrentLimit();
    void ResumeAtCurrentLimit();

    byte GetNumberOfLitPixels() const;
    byte GetCurrentLimit() const;

    Pixel GetColorDimmed(byte) const;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Display& GetInstance() {
        static Display singleton_instance(255u, 255u, 255u);
        return singleton_instance;
    }

    // get methods
    StateType GetState() const { return state_; }
    bool GetBrightnessUseAutomatic() const { return brightness_.GetUseAutomatic(); }
    bool GetBrightnessUseGammaCorrection() const { return brightness_.GetUseGammaCorrection(); }

    Pixel GetColor() const { return color_.GetColor(); }
    ColorType GetColorRed() const { return color_.GetColorRed(); }
    ColorType GetColorGreen() const { return color_.GetColorGreen(); }
    ColorType GetColorBlue() const { return color_.GetColorBlue(); }

    /* The brightness that was asked for, not the one on the strip. The two used to differ by
       which dimming path was compiled, and now they differ for a second reason: the current
       limit can cap what reaches the LEDs. Both callers - Persistence and the brightness
       command's answer - want the setting. A saved configuration that came back one part
       dimmer for having been saved on a hot afternoon would be the alternative, and the
       command would report a number nobody typed. */
    byte GetBrightness() const { return brightness_.GetBrightness(); }

    void SetBrightnessUseAutomatic(bool brightness_use_automatic) {
        brightness_.SetUseAutomatic(brightness_use_automatic);
        ApplyBrightness();
    }
    void SetBrightnessUseGammaCorrection(bool brightness_use_gamma_correction) {
        brightness_.SetUseGammaCorrection(brightness_use_gamma_correction);
        ApplyBrightness();
    }
    /* Every one of them applies what it set, for the reason ApplyColor() gives. */
    void SetColor(Pixel color) { color_.SetColor(color); ApplyColorAndCurrentLimit(); }
    void SetColor(ColorType red, ColorType green, ColorType blue) { color_.SetColorRed(red); color_.SetColorGreen(green); color_.SetColorBlue(blue); ApplyColorAndCurrentLimit(); }
    void SetColorRed(ColorType red) { color_.SetColorRed(red); ApplyColorAndCurrentLimit(); }
    void SetColorGreen(ColorType green) { color_.SetColorGreen(green); ApplyColorAndCurrentLimit(); }
    void SetColorBlue(ColorType blue) { color_.SetColorBlue(blue); ApplyColorAndCurrentLimit(); }

    /* Takes the brightness the user asked for. What reaches the LEDs is
       CalcBrightness(), which folds in gamma correction and the light sensor. */
    void SetBrightness(byte);

    // char methods
    StdReturnType SetCharacter(CharacterIdType character_id) { return SetPixel(character_id); }
    StdReturnType ClearCharacter(CharacterIdType character_id) { return ClearPixel(character_id); }
    StdReturnType GetCharacter(CharacterIdType character_id, PixelValueType& value) const { return GetPixel(character_id, value); }
    PixelValueType GetCharacter(CharacterIdType character_id) const { return GetPixel(character_id); }

    // word methods
    StdReturnType SetWord(WordIdType, byte max_length = kWordLengthUnlimited);
    StdReturnType ClearWord(WordIdType);
    StdReturnType ClearWords();

    /* Two forms of a reader, and the argument list says which: the one taking a reference
       answers whether the index was there, the one taking none answers the pixel and leaves
       an unlit one where it was not. Writers have a single form, since a caller who ignores
       what it answers is written the same way as one that never asked. */
    // pixel methods
    StdReturnType WritePixel(byte column, byte row, PixelValueType value) { if(value) return SetPixel(column, row); else return ClearPixel(column, row); }
    StdReturnType WritePixel(IndexType index, PixelValueType value) { if(value) return SetPixel(index); else return ClearPixel(index); }
    StdReturnType SetPixel(byte, byte);
    StdReturnType SetPixel(IndexType);
    StdReturnType SetPixel(byte, byte, byte);
    StdReturnType ClearPixel(byte, byte);
    StdReturnType ClearPixel(IndexType);
    StdReturnType TogglePixel(byte, byte);
    StdReturnType TogglePixel(IndexType);
    StdReturnType GetPixel(byte, byte, PixelValueType&) const;
    StdReturnType GetPixel(IndexType, PixelValueType&) const;
    PixelValueType GetPixel(byte, byte) const;
    PixelValueType GetPixel(IndexType) const;
    StdReturnType GetPixelRow(byte, PixelRowType&) const;
    PixelRowType GetPixelRow(byte) const;
    StdReturnType GetPixelColumn(byte, PixelColumnType&) const;
    PixelColumnType GetPixelColumn(byte) const;
    StdReturnType SetPixelRow(byte, PixelRowType);
    StdReturnType SetPixelColumn(byte, PixelColumnType);

    bool IsIndexValid(IndexType index) const { return pixel_stripe_.IsIndexValid(index); }
    /* Asked by every entry point taking a column and a row, because the index they map to
       is a valid one for the next row: a column past the last one used to light the first
       letter of the row below rather than being refused. */
    static constexpr bool IsColumnAndRowValid(byte column, byte row) {
        return (column < DISPLAY_NUMBER_OF_COLUMNS) && (row < DISPLAY_NUMBER_OF_ROWS);
    }

    // methods
    void Init();
    StdReturnType Show() { return pixel_stripe_.Show(); }
    /* Not EnablePixels(): that puts the master back at full, which is the one value the
       supply may not allow. The way back on goes through the cap instead. */
    void Enable() { enabled_ = true; ResumeAtCurrentLimit(); }
    void Disable() { enabled_ = false; pixel_stripe_.DisablePixels(); }
    bool IsEnabled() const { return enabled_; }
    /* The other way round from whatever it is now, for a button that has nothing to read
       the state off. The three toggles below are the same idea for the two settings that
       are a yes or a no. */
    void Toggle() { if(IsEnabled()) { Disable(); } else { Enable(); } }

    /* The strip as a wire rather than as a picture, for the supply switch: whether a frame
       is still waiting to go out, whether one is still going out, and the gate that stops
       any from going out at all. Forwarded here rather than reached through Pixels directly
       so that the module driving the switch needs one collaborator instead of two - and so
       the rule stays where the rest of the strip's handling is.

       IsDirty() and IsFrameOnTheWire() are two different waits and both are needed: the
       first says the blanked frame has not been handed over yet, the second that it has and
       is still on the wire. Cutting the supply at either point leaves the strip holding
       half a frame with nothing to latch it. */
    bool IsDirty() const { return pixel_stripe_.IsDirty(); }
    bool IsFrameOnTheWire() const { return pixel_stripe_.IsFrameOnTheWire(); }
    bool IsOutputSuspended() const { return pixel_stripe_.IsOutputSuspended(); }
    void SuspendOutput() { pixel_stripe_.SuspendOutput(); }
    void ResumeOutput() { pixel_stripe_.ResumeOutput(); }
    void EnableBrightnessAutomatic() { brightness_.EnableAutomatic(); }
    void DisableBrightnessAutomatic() { brightness_.DisableAutomatic(); }
    void EnableBrightnessGammaCorrection() { brightness_.EnableGammaCorrection(); }
    void DisableBrightnessGammaCorrection() { brightness_.DisableGammaCorrection(); }
    /* Through the setters rather than through the enable/disable pair above, which is the
       half of it that leaves ApplyBrightness() to the next task: a knob turned once should
       show what it did, not a tick later. */
    void ToggleBrightnessAutomatic() { SetBrightnessUseAutomatic(!GetBrightnessUseAutomatic()); }
    void ToggleBrightnessGammaCorrection() { SetBrightnessUseGammaCorrection(!GetBrightnessUseGammaCorrection()); }
    /* The one call that reaches every LED at once, and the reason the README names it when it
       sizes a supply. So it asks for the cap itself rather than leaving a task to notice. */
    void Test() { pixel_stripe_.SetPixels(color_.GetColorDimmed()); ApplyCurrentLimit(); }
    void Clear() { pixel_stripe_.ClearPixels(); }
    bool IsCleared() const { for(byte index = 0; index < DISPLAY_NUMBER_OF_PIXELS; index++) { if(GetPixel(index)) return false; } return true; }

    void IncrementColorRed() { color_.IncrementColorRed(); ApplyColor(); }
    void IncrementColorGreen() {  color_.IncrementColorGreen(); ApplyColor(); }
    void IncrementColorBlue() { color_.IncrementColorBlue(); ApplyColor(); }
    void IncrementBrightness() { brightness_.IncrementBrightness(); ApplyBrightness(); }

    void DecrementColorRed() { color_.DecrementColorRed(); ApplyColor(); }
    void DecrementColorGreen() { color_.DecrementColorGreen(); ApplyColor(); }
    void DecrementColorBlue() { color_.DecrementColorBlue(); ApplyColor(); }
    void DecrementBrightness() { brightness_.DecrementBrightness(); ApplyBrightness(); }

    /* The colour cycle's level, which ColorCycle drives. Through the current limit like every
       other colour change and for the same reason: white draws three times what a saturated
       hue does, so a wheel stepping without recomputing the cap would be a wheel walking past
       the supply's budget on one segment and leaving it unused on the next. */
    void SetColorCycle(Pixel cycle_color) { color_.SetCycleColor(cycle_color); ApplyColorAndCurrentLimit(); }
    void ClearColorCycle() { color_.ClearCycle(); ApplyColorAndCurrentLimit(); }
    bool IsColorCycleActive() const { return color_.IsCycleActive(); }
    Pixel GetColorCycle() const { return color_.GetCycleColor(); }

    /* White again, and through DisplayColor's own reset so that what this puts back is the
       one named value the settings reset and the member initialiser already use. */
    void ResetColor() { color_.ResetToDefaults(); ApplyColor(); }

    void ApplyBrightness();

    /* What AnimationFade dims the display with while it runs. It goes through the display
       rather than through the strip because ApplyBrightness() runs on every task and
       would otherwise write the configured brightness back over it - and it is a level of
       its own rather than the brightness setting, so that what a fade dims is never what
       Persistence stores. */
    void SetBrightnessFadeLevel(byte level) { brightness_.SetFadeLevel(level); ApplyBrightness(); }
    void ClearBrightnessFade() { brightness_.ClearFade(); ApplyBrightness(); }

    /* The night switch's dimming, which scales what the setting arrived at instead of
       replacing it - so the brightness the owner chose is what morning returns to. */
    byte GetBrightnessNightLevel() const { return brightness_.GetNightLevel(); }
    void SetBrightnessNightLevel(byte level) { brightness_.SetNightLevel(level); ApplyBrightness(); }
    void ClearBrightnessNight() { brightness_.ClearNight(); ApplyBrightness(); }

    /* Back to what a clock that was never configured shows. Each part answers for its own
       defaults, so nothing here has to know what they are; the brightness is applied
       afterwards, because that is what recalculates what reaches the LEDs. */
    void ResetToDefaults() {
        color_.ResetToDefaults();
        brightness_.ResetToDefaults();
        ApplyBrightness();
        /* After the brightness, not instead of it: ApplyBrightness() does nothing where the
           brightness was already at its default, and the colour would stay behind. */
        ApplyColor();
    }

    static void IndexToColumnAndRow(IndexType index, byte& column, byte& row) { row = index / DISPLAY_NUMBER_OF_COLUMNS; column = index % DISPLAY_NUMBER_OF_COLUMNS; }
    static byte IndexToColumn(IndexType index) { return index % DISPLAY_NUMBER_OF_COLUMNS; }
    static byte IndexToRow(IndexType index) { return index / DISPLAY_NUMBER_OF_COLUMNS; }
    static byte ColumnAndRowToIndex(byte column, byte row) { return (row * DISPLAY_NUMBER_OF_COLUMNS) + column; }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
