// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The display: the pixels of the letter grid, with brightness, colour and current limit.
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Pixels.h"
#include "DisplayCharacters.h"
#include "DisplayWords.h"
#include "DisplayBrightness.h"
#include "DisplayColor.h"
#include "DisplayCurrentLimit.h"

#ifndef DISPLAY_DATA_PIN
/*!
 * \brief The pin of the strip's data line, which Display::Init() hands to Pixels::Init().
 *
 * The one display parameter that is a property of the board rather than of the display: which pin
 * is free is decided by the part, not by the strip. So a board that cannot use this one overrides
 * it from its own build instead of editing here - the way WORDCLOCK_VERSION is overridden -
 * because every target shares this header, and editing the line would move the pin for all four
 * of them at once. Which pin each backend needs is in its own README.
 *
 * The AVR backend does not use it: its data line is driven by the look-up table's own output,
 * which WS2812.h selects.
 */
# define DISPLAY_DATA_PIN                           10u
#endif
/*!
 * \brief STD_ON for a strip wired as a serpentine, which runs through its odd rows from right to
 *        left; STD_OFF for one that runs through every row from left to right.
 */
#define DISPLAY_LED_STRIPE_SERPENTINE               STD_OFF
/*!
 * \brief STD_ON where the strip's master brightness dims the display, STD_OFF where DisplayColor
 *        dims the colour the pixels are written in.
 *
 * It follows DISPLAY_COLOR_SUPPORT_DIMMING rather than being set here: it is STD_OFF while
 * DisplayColor supports dimming, as it does by default. STD_ON needs PIXELS_SUPPORT_DIMMING.
 *
 * \warning Under STD_ON the master is meant to carry the brightness and the current limit at once,
 *          but only ApplyBrightness() writes the lower of the two, and only when the brightness
 *          moved. Enable(), and a move of the limit on its own - in ApplyBrightness(), a colour
 *          setter or Test() - write the limit alone, which drops the brightness until it next
 *          moves. A brightness that moves while the display is disabled writes the master and
 *          lights the display again.
 */
#if (DISPLAY_COLOR_SUPPORT_DIMMING == STD_ON)
# define DISPLAY_USE_PIXELS_DIMMING                 STD_OFF
#else
# define DISPLAY_USE_PIXELS_DIMMING                 STD_ON
#endif

# if (DISPLAY_USE_PIXELS_DIMMING == STD_ON) && (PIXELS_SUPPORT_DIMMING == STD_OFF)
#  error "Display: Please activate Pixels dimming support"
# endif

/*! \brief The number of rows of the letter grid, as DisplayCharacters has it. */
#define DISPLAY_NUMBER_OF_ROWS                      DISPLAY_CHARACTERS_NUMBER_OF_ROWS
/*! \brief The number of columns of the letter grid, as DisplayCharacters has it. */
#define DISPLAY_NUMBER_OF_COLUMNS                   DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
/*! \brief The number of LEDs of the strip, as Pixels has it, which must be rows times columns. */
#define DISPLAY_NUMBER_OF_LEDS                      PIXELS_NUMBER_OF_LEDS
/*! \brief The number of pixels of the display, one per LED. */
#define DISPLAY_NUMBER_OF_PIXELS                    DISPLAY_NUMBER_OF_LEDS

# if ((DISPLAY_NUMBER_OF_ROWS * DISPLAY_NUMBER_OF_COLUMNS) != DISPLAY_NUMBER_OF_LEDS)
#  error "Display: LED number missmatch"
# endif

/*!
 * \brief The display: the pixels of the letter grid, with brightness, colour and current limit.
 *
 * It maps the letters of the grid to the LEDs of the strip, Pixels. A pixel is addressed by its
 * column, counted from 0 at the left, and its row, counted from 0 at the top, or by its index,
 * row * DISPLAY_NUMBER_OF_COLUMNS + column. The functions that read or write a pixel refuse one off
 * the grid. A pixel is lit when any of its colours is above 0.
 *
 * The writers write the strip's buffer and send nothing: the application sends the buffer on its
 * next tick once it is marked as changed, which Show() does.
 *
 * Two forms of a reader, and the argument list says which: the one taking a reference answers
 * whether the index was there, the one taking none answers the pixel and leaves an unlit one where
 * it was not. Writers have a single form, since a caller who ignores what it answers is written
 * the same way as one that never asked.
 *
 * Where DisplayColor does the dimming, as by default, a lit pixel is written in the colour dimmed
 * to the brightness, and the strip's master brightness carries the current limit: the largest
 * brightness DisplayCurrentLimit allows for the number of lit pixels and the colour shown. Under
 * DISPLAY_USE_PIXELS_DIMMING a lit pixel is written in the plain colour, and the master carries
 * the brightness as well. ApplyBrightness() keeps both up to date.
 */
class Display
{
  public:
    /*! \brief The state of the display. */
    enum class StateType {
        kNone, /*!< Before the constructor; GetState() never returns it. */
        kUninit, /*!< Made, but Init() not called yet. */
        kInit, /*!< Init() was called. */
        kReady /*!< Not used: no code sets it. */
    };

    /*! \brief The value of a pixel: true for lit, false for off. */
    using PixelValueType = bool;

    /*! \brief The id of a letter of the grid, which the display takes as the letter's index. */
    using CharacterIdType = DisplayCharacters::CharacterIdType;

# if (DISPLAY_NUMBER_OF_ROWS > 16u)
#  error "Display: too many rows, please extend PixelRowType"
# endif

    /*! \brief The pixels of a row as bits: bit n for column n, set for lit. */
    using PixelRowType = uint16_t;

# if (DISPLAY_NUMBER_OF_COLUMNS > 16u)
#  error "Display: too many columns, please extend PixelColumnType"
# endif

    /*! \brief The pixels of a column as bits: bit n for row n, set for lit. */
    using PixelColumnType = uint16_t;

    /*! \copydoc Illuminance::IlluminanceType */
    using IlluminanceType = Illuminance::IlluminanceType;

    /*! \brief The index of a pixel, row * DISPLAY_NUMBER_OF_COLUMNS + column. */
    using IndexType = Pixels::IndexType;

    /*! \copydoc Pixel::ColorType */
    using ColorType = Pixel::ColorType;

    /*! \brief The id of a word of the clock, as DisplayWords counts it. */
    using WordIdType = DisplayWords::WordIdType;

  private:
    static constexpr byte kWordLengthUnlimited{0u};
    StateType state_{StateType::kNone};
    /*
     * Whether the display was last asked to be on, true because a strip comes up lit. Kept
     * here rather than read back from the strip's master brightness, which is what Enable()
     * and Disable() write: under DISPLAY_USE_PIXELS_DIMMING that same register carries the
     * brightness setting, so a zero in it would answer "dark" where the question is
     * "switched off".
     */
    bool enabled_{true};
    /*
     * brightness the pixels on the display were last written with. Zero rather than the
     * initial brightness, so the first task establishes a defined state; the display is
     * still empty then, which makes that first pass free.
     */
    byte applied_brightness_{0u};
    /*
     * master the strip was last handed. Full, because that is where Pixels starts - and
     * because writing it marks the frame dirty, so this exists to write it only when the cap
     * has actually moved rather than on every task run.
     */
    byte applied_current_limit_{DisplayCurrentLimit::kNoLimit};
# if (PIXELS_IS_SINGLETON == STD_OFF)
    Pixels pixel_stripe_;
# else
    Pixels& pixel_stripe_;
# endif
    DisplayColor color_;
    DisplayWords words_;
    DisplayBrightness brightness_; 

    Display(Pixel color);
    Display(byte red, byte green, byte blue);
    ~Display() = default;

    static byte TransformToSerpentine(byte column, byte row);
    static byte TransformToSerpentine(byte index);
    // The one place a column and a row become an index on the strip, so the wiring is
    // written once instead of as a #if in every entry point that takes a column.
    static byte ToIndex(byte column, byte row) {
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
        /* if led stripe is snake or serpentine the odd row: count from right to left */
        return TransformToSerpentine(column, row);
#else
        return ColumnAndRowToIndex(column, row);
#endif
    }

    void ApplyColor();
    /*
     * A colour change moves the current as much as a brightness change does - white draws
     * three times what amber at the same brightness does - so both have to go past the
     * limit. This is what the colour setters call instead of ApplyColor() alone.
     */
    void ApplyColorAndCurrentLimit();

    void ApplyCurrentLimit();
    void ResumeAtCurrentLimit();

    byte GetNumberOfLitPixels() const;
    byte GetCurrentLimit() const;

    Pixel GetColorDimmed(byte brightness) const;

    /*
     * The colour a lit pixel is written in, decided here once rather than at every writer -
     * the writers that each decided it for themselves were how the strip-dimming path came
     * to hand SetPixel() a DisplayColor and stop compiling. Where the colour does the
     * dimming that is its dimmed copy; where the strip does it, the colour as it is to be
     * shown, which the strip's master then scales on the way out.
     */
    Pixel GetColorToWrite() const {
#if (DISPLAY_USE_PIXELS_DIMMING == STD_ON)
        return color_.GetColorToShow();
#else
        return color_.GetColorDimmed();
#endif
    }

  public:
    /*! \brief Returns the one display. */
    static Display& GetInstance() {
        static Display singleton_instance(255u, 255u, 255u);
        return singleton_instance;
    }

    /*! \brief Returns the state of the display: kUninit until Init(), kInit after it. */
    StateType GetState() const { return state_; }

    /*! \brief Returns whether the brightness follows the light sensor. */
    bool GetBrightnessUseAutomatic() const { return brightness_.GetUseAutomatic(); }

    /*! \brief Returns whether the brightness is gamma corrected. */
    bool GetBrightnessUseGammaCorrection() const { return brightness_.GetUseGammaCorrection(); }

    /*! \brief Returns the colour set, which a running colour cycle does not change. */
    Pixel GetColor() const { return color_.GetColor(); }

    /*! \brief Returns the red value of the colour set. */
    ColorType GetColorRed() const { return color_.GetColorRed(); }

    /*! \brief Returns the green value of the colour set. */
    ColorType GetColorGreen() const { return color_.GetColorGreen(); }

    /*! \brief Returns the blue value of the colour set. */
    ColorType GetColorBlue() const { return color_.GetColorBlue(); }

    /*!
     * \brief Returns the brightness that was asked for, not the one on the strip.
     *
     * The two used to differ by which dimming path was compiled, and now they differ for a second
     * reason: the current limit can cap what reaches the LEDs. Both callers - Persistence and the
     * brightness command's answer - want the setting. A saved configuration that came back one
     * part dimmer for having been saved on a hot afternoon would be the alternative, and the
     * command would report a number nobody typed.
     *
     * \return The brightness setting, from 0 to 255.
     */
    byte GetBrightness() const { return brightness_.GetBrightness(); }

    /*!
     * \brief Sets whether the brightness follows the light sensor, and applies it at once.
     *
     * \param[in] brightness_use_automatic True to follow the light sensor.
     */
    void SetBrightnessUseAutomatic(bool brightness_use_automatic) {
        brightness_.SetUseAutomatic(brightness_use_automatic);
        ApplyBrightness();
    }

    /*!
     * \brief Sets whether the brightness is gamma corrected, and applies it at once.
     *
     * \param[in] brightness_use_gamma_correction True to correct it.
     */
    void SetBrightnessUseGammaCorrection(bool brightness_use_gamma_correction) {
        brightness_.SetUseGammaCorrection(brightness_use_gamma_correction);
        ApplyBrightness();
    }

    /*!
     * \brief Sets the colour of the display, and applies it at once.
     *
     * The lit pixels are rewritten in it, and the current limit is recomputed for it: a colour
     * change moves the current as much as a brightness change does - white draws three times what
     * amber at the same brightness does. While a colour cycle runs, the pixels keep its colour.
     *
     * \param[in] color The colour.
     */
    void SetColor(Pixel color) { color_.SetColor(color); ApplyColorAndCurrentLimit(); }

    /*!
     * \brief Sets the colour of the display from its red, green and blue values, and applies it at
     *        once, as SetColor(Pixel) does.
     *
     * \param[in] red The red value.
     * \param[in] green The green value.
     * \param[in] blue The blue value.
     */
    void SetColor(ColorType red, ColorType green, ColorType blue) { color_.SetColorRed(red); color_.SetColorGreen(green); color_.SetColorBlue(blue); ApplyColorAndCurrentLimit(); }

    /*!
     * \brief Sets the red value of the colour, and applies it at once, as SetColor(Pixel) does.
     *
     * \param[in] red The red value.
     */
    void SetColorRed(ColorType red) { color_.SetColorRed(red); ApplyColorAndCurrentLimit(); }

    /*!
     * \brief Sets the green value of the colour, and applies it at once, as SetColor(Pixel) does.
     *
     * \param[in] green The green value.
     */
    void SetColorGreen(ColorType green) { color_.SetColorGreen(green); ApplyColorAndCurrentLimit(); }

    /*!
     * \brief Sets the blue value of the colour, and applies it at once, as SetColor(Pixel) does.
     *
     * \param[in] blue The blue value.
     */
    void SetColorBlue(ColorType blue) { color_.SetColorBlue(blue); ApplyColorAndCurrentLimit(); }

    /*!
     * \brief Sets the brightness asked for, and applies it at once.
     *
     * It takes the brightness the user asked for. What reaches the LEDs is
     * DisplayBrightness::CalcBrightness(), which folds in gamma correction and the light sensor,
     * and then the night level and the fade level; the current limit can hold the strip below it.
     *
     * \param[in] brightness The brightness, from 0 to 255.
     * \note Where DisplayColor does the dimming, as by default, no brightness turns a lit pixel
     *       off: DisplayColor dims a colour that is above 0 to 1 at the least.
     */
    void SetBrightness(byte brightness);

    /*!
     * \brief Lights the letter `character_id` in the colour of the display.
     *
     * \param[in] character_id The letter.
     * \return kEOk, or kENotOk if `character_id` is not a letter of the grid, such as
     *         `kNumberOfCharacters`; the pixels are then left as they were.
     */
    StdReturnType SetCharacter(CharacterIdType character_id) { return SetPixel(ToUnderlying(character_id)); }

    /*!
     * \brief Turns the letter `character_id` off.
     *
     * \param[in] character_id The letter.
     * \return kEOk, or kENotOk if `character_id` is not a letter of the grid; the pixels are then
     *         left as they were.
     */
    StdReturnType ClearCharacter(CharacterIdType character_id) { return ClearPixel(ToUnderlying(character_id)); }

    /*!
     * \brief Returns in `value` whether the letter `character_id` is lit.
     *
     * \param[in] character_id The letter.
     * \param[out] value True if the letter is lit.
     * \return kEOk, or kENotOk if `character_id` is not a letter of the grid; `value` is then left
     *         as it was.
     */
    StdReturnType GetCharacter(CharacterIdType character_id, PixelValueType& value) const { return GetPixel(ToUnderlying(character_id), value); }

    /*!
     * \brief Returns whether the letter `character_id` is lit.
     *
     * \param[in] character_id The letter.
     * \return True if the letter is lit; false if `character_id` is not a letter of the grid.
     */
    PixelValueType GetCharacter(CharacterIdType character_id) const { return GetPixel(ToUnderlying(character_id)); }

    /*!
     * \brief Lights the first `max_length` letters of the word `word_id` in the colour of the
     *        display.
     *
     * The other pixels are left as they were.
     *
     * \param[in] word_id The word; `kNone` lights nothing.
     * \param[in] max_length The letters to light; 0, the default, or a length the word does not
     *                       exceed, lights the word whole.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`, and then
     *         nothing is lit, or if a letter lies off the grid; the other letters are lit all the
     *         same.
     */
    StdReturnType SetWord(WordIdType word_id, byte max_length = kWordLengthUnlimited);

    /*!
     * \brief Turns the letters of the word `word_id` off.
     *
     * \param[in] word_id The word; `kNone` turns nothing off.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`, and then
     *         nothing is turned off, or if a letter lies off the grid; the other letters are turned
     *         off all the same.
     */
    StdReturnType ClearWord(WordIdType word_id);

    /*!
     * \brief Turns the letters of every word off.
     *
     * The letters that belong to no word are left as they were; Clear() turns off every pixel.
     *
     * \return kEOk, or kENotOk if a letter of a word lies off the grid; the other words are turned
     *         off all the same.
     */
    StdReturnType ClearWords();

    /*!
     * \brief Lights the pixel at `column` and `row` if `value` is true, and turns it off otherwise.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \param[in] value True to light the pixel, false to turn it off.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left as
     *         they were.
     */
    StdReturnType WritePixel(byte column, byte row, PixelValueType value) { if(value) return SetPixel(column, row); else return ClearPixel(column, row); }

    /*!
     * \brief Lights the pixel at `index` if `value` is true, and turns it off otherwise.
     *
     * \param[in] index The index of the pixel.
     * \param[in] value True to light the pixel, false to turn it off.
     * \return kEOk, or kENotOk if `index` lies off the grid; the pixels are then left as they were.
     */
    StdReturnType WritePixel(IndexType index, PixelValueType value) { if(value) return SetPixel(index); else return ClearPixel(index); }

    /*!
     * \brief Lights the pixel at `column` and `row` in the colour of the display.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left as
     *         they were.
     */
    StdReturnType SetPixel(byte column, byte row);

    /*!
     * \brief Lights the pixel at `index` in the colour of the display.
     *
     * \param[in] index The index of the pixel.
     * \return kEOk, or kENotOk if `index` lies off the grid; the pixels are then left as they were.
     */
    StdReturnType SetPixel(IndexType index);

    /*!
     * \brief Lights the pixel at `column` and `row` in the colour of the display, dimmed by
     *        `brightness` on top of the brightness of the display.
     *
     * A change of the colour, or of the brightness where DisplayColor does the dimming, rewrites
     * every lit pixel in the colour of the display; this pixel then loses its own brightness.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \param[in] brightness How far to dim, as Pixel::DimmPixel() takes it: 255 keeps the colour, 0
     *                       turns the pixel off.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left as
     *         they were.
     */
    StdReturnType SetPixel(byte column, byte row, byte brightness);

    /*!
     * \brief Turns the pixel at `column` and `row` off.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left as
     *         they were.
     */
    StdReturnType ClearPixel(byte column, byte row);

    /*!
     * \brief Turns the pixel at `index` off.
     *
     * \param[in] index The index of the pixel.
     * \return kEOk, or kENotOk if `index` lies off the grid; the pixels are then left as they were.
     */
    StdReturnType ClearPixel(IndexType index);

    /*!
     * \brief Turns the pixel at `column` and `row` off if it is lit, and lights it in the colour of
     *        the display otherwise.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; the pixels are then left as
     *         they were.
     */
    StdReturnType TogglePixel(byte column, byte row);

    /*!
     * \brief Turns the pixel at `index` off if it is lit, and lights it in the colour of the
     *        display otherwise.
     *
     * \param[in] index The index of the pixel.
     * \return kEOk, or kENotOk if `index` lies off the grid; the pixels are then left as they were.
     */
    StdReturnType TogglePixel(IndexType index);

    /*!
     * \brief Returns in `value` whether the pixel at `column` and `row` is lit.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \param[out] value True if the pixel is lit.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; `value` is then left as it
     *         was.
     */
    StdReturnType GetPixel(byte column, byte row, PixelValueType& value) const;

    /*!
     * \brief Returns in `value` whether the pixel at `index` is lit.
     *
     * \param[in] index The index of the pixel.
     * \param[out] value True if the pixel is lit.
     * \return kEOk, or kENotOk if `index` lies off the grid; `value` is then left as it was.
     */
    StdReturnType GetPixel(IndexType index, PixelValueType& value) const;

    /*!
     * \brief Returns whether the pixel at `column` and `row` is lit.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return True if the pixel is lit; false if `column` or `row` lies off the grid.
     */
    PixelValueType GetPixel(byte column, byte row) const;

    /*!
     * \brief Returns whether the pixel at `index` is lit.
     *
     * \param[in] index The index of the pixel.
     * \return True if the pixel is lit; false if `index` lies off the grid.
     */
    PixelValueType GetPixel(IndexType index) const;

    /*!
     * \brief Returns in `pixel_row` which pixels of the row `row` are lit.
     *
     * \param[in] row The row of the grid, counted from 0.
     * \param[in,out] pixel_row The pixels, bit n for column n. The bits above the last column are
     *                          left as they were.
     * \return kEOk, or kENotOk if `row` lies off the grid; `pixel_row` is then left as it was.
     */
    StdReturnType GetPixelRow(byte row, PixelRowType& pixel_row) const;

    /*!
     * \brief Returns which pixels of the row `row` are lit.
     *
     * \param[in] row The row of the grid, counted from 0.
     * \return The pixels, bit n for column n; 0 if `row` lies off the grid.
     */
    PixelRowType GetPixelRow(byte row) const;

    /*!
     * \brief Returns in `pixel_column` which pixels of the column `column` are lit.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in,out] pixel_column The pixels, bit n for row n. The bits above the last row are left
     *                             as they were.
     * \return kEOk, or kENotOk if `column` lies off the grid; `pixel_column` is then left as it
     *         was.
     */
    StdReturnType GetPixelColumn(byte column, PixelColumnType& pixel_column) const;

    /*!
     * \brief Returns which pixels of the column `column` are lit.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \return The pixels, bit n for row n; 0 if `column` lies off the grid.
     */
    PixelColumnType GetPixelColumn(byte column) const;

    /*!
     * \brief Lights the pixels of the row `row` whose bits are set, and turns the others off.
     *
     * \param[in] row The row of the grid, counted from 0.
     * \param[in] pixel_row The pixels, bit n for column n. The bits above the last column are not
     *                      read.
     * \return kEOk, or kENotOk if `row` lies off the grid; the pixels are then left as they were.
     */
    StdReturnType SetPixelRow(byte row, PixelRowType pixel_row);

    /*!
     * \brief Lights the pixels of the column `column` whose bits are set, and turns the others off.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] pixel_column The pixels, bit n for row n. The bits above the last row are not
     *                         read.
     * \return kEOk, or kENotOk if `column` lies off the grid; the pixels are then left as they
     *         were.
     */
    StdReturnType SetPixelColumn(byte column, PixelColumnType pixel_column);

    /*!
     * \brief Returns whether `index` is the index of a pixel.
     *
     * \param[in] index The index.
     * \return True if `index` lies below the number of pixels of the strip.
     */
    bool IsIndexValid(IndexType index) const { return pixel_stripe_.IsIndexValid(index); }

    /*!
     * \brief Returns whether `column` and `row` lie on the grid.
     *
     * Asked by every entry point taking a column and a row, because the index they map to is a
     * valid one for the next row: a column past the last one used to light the first letter of
     * the row below rather than being refused.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] row The row, counted from 0.
     * \return True if `column` lies below DISPLAY_NUMBER_OF_COLUMNS and `row` below
     *         DISPLAY_NUMBER_OF_ROWS.
     */
    static constexpr bool IsColumnAndRowValid(byte column, byte row) {
        return (column < DISPLAY_NUMBER_OF_COLUMNS) && (row < DISPLAY_NUMBER_OF_ROWS);
    }

    /*!
     * \brief Initialises the display: turns every pixel off and sets the strip up on
     *        DISPLAY_DATA_PIN.
     *
     * GetState() returns kInit afterwards.
     */
    void Init();

    /*!
     * \brief Marks the strip's buffer to be sent.
     *
     * It sends nothing itself. The application sends a marked buffer on its next tick, once,
     * however many calls came before; while the output is suspended, the buffer stays marked.
     *
     * \return kEOk on every backend.
     */
    StdReturnType Show() { return pixel_stripe_.Show(); }

    /*!
     * \brief Enables the display: sets the strip's master brightness back to the current limit.
     *
     * Not Pixels::EnablePixels(): that puts the master back at full, which is the one value the
     * supply may not allow. The way back on goes through the cap instead.
     */
    void Enable() { enabled_ = true; ResumeAtCurrentLimit(); }

    /*!
     * \brief Disables the display: sets the strip's master brightness to 0.
     *
     * The pixels keep their colours, and writes still go into them; Enable() shows them again.
     * The current limit leaves the master alone while the display is disabled.
     *
     * \note Under DISPLAY_USE_PIXELS_DIMMING a change of the brightness lights the display again;
     *       see the warning there.
     */
    void Disable() { enabled_ = false; pixel_stripe_.DisablePixels(); }

    /*! \brief Returns whether the display was last enabled rather than disabled; true at first. */
    bool IsEnabled() const { return enabled_; }

    /*!
     * \brief Disables the display if it is enabled, and enables it otherwise.
     *
     * The other way round from whatever it is now, for a button that has nothing to read the
     * state off. ToggleBrightnessAutomatic() and ToggleBrightnessGammaCorrection() are the same
     * idea for the two settings that are a yes or a no.
     */
    void Toggle() { if(IsEnabled()) { Disable(); } else { Enable(); } }

    /*!
     * \brief Returns whether the strip's buffer is marked to be sent.
     *
     * The strip as a wire rather than as a picture, for the supply switch: whether a frame is
     * still waiting to go out, whether one is still going out, and the gate that stops any from
     * going out at all. Forwarded here rather than reached through Pixels directly so that the
     * module driving the switch needs one collaborator instead of two - and so the rule stays
     * where the rest of the strip's handling is.
     *
     * IsDirty() and IsFrameOnTheWire() are two different waits and both are needed: the first
     * says the blanked frame has not been handed over yet, the second that it has and is still on
     * the wire. Cutting the supply at either point leaves the strip holding half a frame with
     * nothing to latch it.
     *
     * \return True while the buffer is marked: from Show(), or a write that marks it, until the
     *         application's tick has sent it.
     */
    bool IsDirty() const { return pixel_stripe_.IsDirty(); }

    /*!
     * \brief Returns whether a frame is still going out on the strip's data line; always false in
     *        the simulator.
     */
    bool IsFrameOnTheWire() const { return pixel_stripe_.IsFrameOnTheWire(); }

    /*! \brief Returns whether the output to the strip is suspended. */
    bool IsOutputSuspended() const { return pixel_stripe_.IsOutputSuspended(); }

    /*! \brief Stops every frame from going out to the strip, until ResumeOutput(). */
    void SuspendOutput() { pixel_stripe_.SuspendOutput(); }

    /*! \brief Lets frames go out to the strip again, and marks the buffer to be sent. */
    void ResumeOutput() { pixel_stripe_.ResumeOutput(); }

    /*! \brief Has the brightness follow the light sensor, from the next ApplyBrightness(). */
    void EnableBrightnessAutomatic() { brightness_.EnableAutomatic(); }

    /*! \brief Stops the brightness following the light sensor, from the next ApplyBrightness(). */
    void DisableBrightnessAutomatic() { brightness_.DisableAutomatic(); }

    /*! \brief Switches the gamma correction on, from the next ApplyBrightness(). */
    void EnableBrightnessGammaCorrection() { brightness_.EnableGammaCorrection(); }

    /*! \brief Switches the gamma correction off, from the next ApplyBrightness(). */
    void DisableBrightnessGammaCorrection() { brightness_.DisableGammaCorrection(); }

    /*!
     * \brief Switches the following of the light sensor the other way round, and applies it at
     *        once.
     *
     * Through the setters rather than through the enable/disable pair above, which is the half of
     * it that leaves ApplyBrightness() to the next task: a knob turned once should show what it
     * did, not a tick later.
     */
    void ToggleBrightnessAutomatic() { SetBrightnessUseAutomatic(!GetBrightnessUseAutomatic()); }

    /*!
     * \brief Switches the gamma correction the other way round, and applies it at once, as
     *        ToggleBrightnessAutomatic() does.
     */
    void ToggleBrightnessGammaCorrection() { SetBrightnessUseGammaCorrection(!GetBrightnessUseGammaCorrection()); }

    /*!
     * \brief Lights every pixel in the colour of the display, and applies the current limit.
     *
     * The one call that reaches every LED at once, and the reason the README names it when it
     * sizes a supply. So it asks for the cap itself rather than leaving a task to notice.
     */
    void Test() { pixel_stripe_.SetPixels(GetColorToWrite()); ApplyCurrentLimit(); }

    /*! \brief Turns every pixel off. */
    void Clear() { pixel_stripe_.ClearPixels(); }

    /*! \brief Returns whether every pixel is off. */
    bool IsCleared() const { for(byte index = 0; index < DISPLAY_NUMBER_OF_PIXELS; index++) { if(GetPixel(index)) return false; } return true; }

    /*!
     * \brief Raises the red value of the colour by one, unless it is 255 already, and rewrites the
     *        lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorRed().
     */
    void IncrementColorRed() { color_.IncrementColorRed(); ApplyColor(); }

    /*!
     * \brief Raises the green value of the colour by one, unless it is 255 already, and rewrites
     *        the lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorGreen().
     */
    void IncrementColorGreen() {  color_.IncrementColorGreen(); ApplyColor(); }

    /*!
     * \brief Raises the blue value of the colour by one, unless it is 255 already, and rewrites the
     *        lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorBlue().
     */
    void IncrementColorBlue() { color_.IncrementColorBlue(); ApplyColor(); }

    /*! \brief Raises the brightness asked for by one, unless it is 255 already, and applies it. */
    void IncrementBrightness() { brightness_.IncrementBrightness(); ApplyBrightness(); }

    /*!
     * \brief Lowers the red value of the colour by one, unless it is 0 already, and rewrites the
     *        lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorRed().
     */
    void DecrementColorRed() { color_.DecrementColorRed(); ApplyColor(); }

    /*!
     * \brief Lowers the green value of the colour by one, unless it is 0 already, and rewrites the
     *        lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorGreen().
     */
    void DecrementColorGreen() { color_.DecrementColorGreen(); ApplyColor(); }

    /*!
     * \brief Lowers the blue value of the colour by one, unless it is 0 already, and rewrites the
     *        lit pixels in it.
     *
     * The current limit follows at the next ApplyBrightness(), unlike after SetColorBlue().
     */
    void DecrementColorBlue() { color_.DecrementColorBlue(); ApplyColor(); }

    /*! \brief Lowers the brightness asked for by one, unless it is 0 already, and applies it. */
    void DecrementBrightness() { brightness_.DecrementBrightness(); ApplyBrightness(); }

    /*!
     * \brief Shows `cycle_color` in place of the colour set, and applies it at once.
     *
     * The colour cycle's level, which ColorCycle drives. Through the current limit like every
     * other colour change and for the same reason: white draws three times what a saturated hue
     * does, so a wheel stepping without recomputing the cap would be a wheel walking past the
     * supply's budget on one segment and leaving it unused on the next.
     *
     * GetColor() still returns the colour set, and ClearColorCycle() shows it again.
     *
     * \param[in] cycle_color The colour to show.
     */
    void SetColorCycle(Pixel cycle_color) { color_.SetCycleColor(cycle_color); ApplyColorAndCurrentLimit(); }

    /*! \brief Ends the colour cycle: shows the colour set again, and applies it at once. */
    void ClearColorCycle() { color_.ClearCycle(); ApplyColorAndCurrentLimit(); }

    /*! \brief Returns whether a colour cycle is shown in place of the colour set. */
    bool IsColorCycleActive() const { return color_.IsCycleActive(); }

    /*! \brief Returns the colour of the colour cycle, the one SetColorCycle() was last given. */
    Pixel GetColorCycle() const { return color_.GetCycleColor(); }

    /*!
     * \brief Sets the colour back to white, and rewrites the lit pixels in it.
     *
     * White again, and through DisplayColor's own reset so that what this puts back is the one
     * named value the settings reset and the member initialiser already use. A colour cycle keeps
     * running, and the current limit follows at the next ApplyBrightness().
     */
    void ResetColor() { color_.ResetToDefaults(); ApplyColor(); }

    /*!
     * \brief Brings the strip up to date with the brightness and the current limit.
     *
     * DisplayManager calls it on every task. It first recomputes the current limit for the pixels
     * lit now and the colour shown, and writes it to the strip's master brightness if it moved
     * and the display is enabled. Then, if the brightness DisplayBrightness::CalcBrightness()
     * arrives at moved since the last call, it rewrites every lit pixel in the colour dimmed to
     * it, or under DISPLAY_USE_PIXELS_DIMMING writes the lower of it and the limit to the master.
     */
    void ApplyBrightness();

    /*!
     * \brief Dims the display by the fade level `level`, and applies it at once.
     *
     * What AnimationFade dims the display with while it runs. It goes through the display rather
     * than through the strip because ApplyBrightness() runs on every task and would otherwise
     * write the configured brightness back over it - and it is a level of its own rather than the
     * brightness setting, so that what a fade dims is never what Persistence stores.
     *
     * \param[in] level 255 for no dimming; below that, the brightness is scaled by (`level` + 1) /
     *                  256, rounding down.
     */
    void SetBrightnessFadeLevel(byte level) { brightness_.SetFadeLevel(level); ApplyBrightness(); }

    /*! \brief Ends the fade's dimming, and applies it at once. */
    void ClearBrightnessFade() { brightness_.ClearFade(); ApplyBrightness(); }

    /*! \brief Returns the night level, 255 while the night does not dim the display. */
    byte GetBrightnessNightLevel() const { return brightness_.GetNightLevel(); }

    /*!
     * \brief Dims the display by the night level `level`, and applies it at once.
     *
     * The night switch's dimming, which scales what the setting arrived at instead of replacing
     * it - so the brightness the owner chose is what morning returns to.
     *
     * \param[in] level 255 for no dimming; below that, the brightness is scaled by (`level` + 1) /
     *                  256, rounding down.
     */
    void SetBrightnessNightLevel(byte level) { brightness_.SetNightLevel(level); ApplyBrightness(); }

    /*! \brief Ends the night's dimming, and applies it at once. */
    void ClearBrightnessNight() { brightness_.ClearNight(); ApplyBrightness(); }

    /*!
     * \brief Puts the colour and the brightness settings back to how a new clock has them, and
     *        applies them at once.
     *
     * Back to what a clock that was never configured shows. Each part answers for its own
     * defaults, so nothing here has to know what they are; the brightness is applied afterwards,
     * because that is what recalculates what reaches the LEDs.
     *
     * The night's dimming ends with them; a fade and a colour cycle are left as they are.
     */
    void ResetToDefaults() {
        color_.ResetToDefaults();
        brightness_.ResetToDefaults();
        ApplyBrightness();
        // After the brightness, not instead of it: ApplyBrightness() does nothing where the
        // brightness was already at its default, and the colour would stay behind.
        ApplyColor();
    }

    /*!
     * \brief Returns in `column` and `row` where the pixel `index` stands.
     *
     * It does not check `index`: an index past the last pixel gives a row past the last row.
     *
     * \param[in] index The index of the pixel.
     * \param[out] column The column, counted from 0.
     * \param[out] row The row, counted from 0.
     */
    static void IndexToColumnAndRow(IndexType index, byte& column, byte& row) { row = index / DISPLAY_NUMBER_OF_COLUMNS; column = index % DISPLAY_NUMBER_OF_COLUMNS; }

    /*!
     * \brief Returns the column of the pixel `index`.
     *
     * \param[in] index The index of the pixel.
     * \return The column, counted from 0.
     */
    static byte IndexToColumn(IndexType index) { return index % DISPLAY_NUMBER_OF_COLUMNS; }

    /*!
     * \brief Returns the row of the pixel `index`.
     *
     * \param[in] index The index of the pixel.
     * \return The row, counted from 0. It is not checked: an index past the last pixel gives a row
     *         past the last row.
     */
    static byte IndexToRow(IndexType index) { return index / DISPLAY_NUMBER_OF_COLUMNS; }

    /*!
     * \brief Returns the index of the pixel at `column` and `row`.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] row The row, counted from 0.
     * \return row * DISPLAY_NUMBER_OF_COLUMNS + column. It is not checked: a column past the last
     *         one gives the index of a pixel in a later row.
     */
    static byte ColumnAndRowToIndex(byte column, byte row) { return (row * DISPLAY_NUMBER_OF_COLUMNS) + column; }

};

#endif
