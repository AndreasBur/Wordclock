// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The text output: draws characters and scrolls text over the display in several fonts.
 */

#ifndef WORDCLOCK_SRC_TEXT_TEXT_H_
#define WORDCLOCK_SRC_TEXT_TEXT_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Transformation.h"
#include "FontCourierNew7x10.h"
#include "FontCourierNew7x9.h"
#include "FontLucidaSans9x10.h"
#include "FontSprite5x8.h"
#include "FontTahoma10x10.h"

/*! \brief The scheduler ticks between two calls of Text::Task() to start with: 1, every tick. */
#define TEXT_TASK_CYCLE_INIT_VALUE 1u
#define TEXT_SUPPORT_FONT_5X8 STD_ON   /*!< STD_ON builds in FontSprite5x8. */
#define TEXT_SUPPORT_FONT_7X9 STD_ON   /*!< STD_ON builds in FontCourierNew7x9. */
#define TEXT_SUPPORT_FONT_7X10 STD_ON  /*!< STD_ON builds in FontCourierNew7x10. */
#define TEXT_SUPPORT_FONT_9X10 STD_ON  /*!< STD_ON builds in FontLucidaSans9x10. */
#define TEXT_SUPPORT_FONT_10X10 STD_ON /*!< STD_ON builds in FontTahoma10x10. */

/*! \brief The offset that turns an ASCII code into the index of its glyph in a font table. */
#define TEXT_ASCII_TABLE_OFFSET (-32)
#define TEXT_ASCII_CHAR_MIN 32  /*!< The first ASCII code with a glyph. */
#define TEXT_ASCII_CHAR_MAX 127 /*!< The last ASCII code with a glyph. */

/*!
 * \brief The text output: draws a character in a font at a place of the display, or shifts a
 *        character or a text in from the right, one column per Task().
 *
 * It writes the pixels of Display and leaves showing them to Show(), or to Task() when it is
 * asked to. Each font has a glyph for the ASCII codes 32 to 127, for the umlauts of Latin-1
 * 0xC4, 0xD6, 0xDC, 0xE4, 0xF6 and 0xFC, and for the degree sign, Latin-1 0xB0.
 *
 * The TEXT_SUPPORT_FONT macros say which fonts are built in. A font of FontType that is not built
 * in draws nothing: SetChar() returns kENotOk for it, and the font getters return 0 or
 * kOrientationNone.
 */
class Text {
 public:
  /*! \brief What the text output is doing. */
  enum class StateType {
    kIdle,      /*!< Nothing: Task() does nothing. */
    kTextShift, /*!< Shifting a text in, started by SetTextWithShift(). */
    kCharShift  /*!< Shifting one character in, started by SetCharWithShift(). */
  };

  /*! \brief Whether a shift has a character coming in. */
  enum class ShiftStateType : uint8_t {
    kIdle, /*!< No character is coming in; the next step of a text loads one. */
    kBusy  /*!< A character is coming in. */
  };

  /*! \brief The fonts the text output draws in. */
  enum class FontType {
    kFont5x8,      /*!< FontSprite5x8, 5 columns by 8 rows. */
    kFont7x9,      /*!< FontCourierNew7x9, 7 columns by 9 rows. */
    kFont7x10,     /*!< FontCourierNew7x10, 7 columns by 10 rows. */
    kFont9x10,     /*!< FontLucidaSans9x10, 9 columns by 10 rows. */
    kFont10x10,    /*!< FontTahoma10x10, 10 columns by 10 rows. */
    kNumberOfFonts /*!< The number of fonts, not a font. */
  };

  /*! \brief The number of fonts of FontType, as a byte. */
  static constexpr byte kNumberOfFonts{static_cast<byte>(FontType::kNumberOfFonts)};

  /*! \brief How far a shift has come: the text, the character coming in and its columns. */
  struct ShiftType {
    char const* text; /*!< The rest of the text, from the next character to load. */
    FontType font;    /*!< The font the shift draws in. */
    char character;   /*!< The character coming in. */
    byte char_width;  /*!< The width of its glyph in columns. */
    /*
     * Whole bytes, where the two used to share one as seven bits and one: GCC's -Wconversion
     * reports every assignment to a bit field narrower than the value, a cast included, and
     * what the field saved was one byte of the one Text object.
     */
    byte counter;         /*!< The columns of the glyph still to come in. */
    ShiftStateType state; /*!< Whether a character is coming in. */
  };

 private:
  // The first glyph of every table is the space, which is what a character with no glyph
  // of its own is drawn as.
  static constexpr byte kSpaceFontIndex{0u};
  Transformation wc_transformation_;
  ShiftType shift_;
  byte task_cycle_{TEXT_TASK_CYCLE_INIT_VALUE};
  StateType state_{StateType::kIdle};

#if (TEXT_SUPPORT_FONT_5X8 == STD_ON)
  FontSprite5x8 font5x8_;
#endif
#if (TEXT_SUPPORT_FONT_7X9 == STD_ON)
  FontCourierNew7x9 font7x9_;
#endif
#if (TEXT_SUPPORT_FONT_7X10 == STD_ON)
  FontCourierNew7x10 font7x10_;
#endif
#if (TEXT_SUPPORT_FONT_9X10 == STD_ON)
  FontLucidaSans9x10 font9x10_;
#endif
#if (TEXT_SUPPORT_FONT_10X10 == STD_ON)
  FontTahoma10x10 font10x10_;
#endif

  constexpr Text()
      : wc_transformation_(),
        shift_{nullptr, FontType::kFont10x10, STD_NULL_CHARACTER, 0, 0, ShiftStateType::kIdle} {}
  ~Text() = default;

  /*
   * Each of these four had a twin that differed in nothing a reader sees - it dropped the
   * answer instead of passing it on - and calling the wrong one of the pair from the wrong
   * place cost bounds checks and looked right. That happened once, in SetChar()'s 9x10
   * branch. There is one of each now, so the mistake has nowhere left to happen, and the
   * [[nodiscard]] that used to police it is gone with the twins.
   */
  template <typename RowType, byte RowsSize>
  StdReturnType SetCharFontHorizontal(byte column, byte row,
                                      FontCharHorizontal<RowType, RowsSize> const& font_char,
                                      byte font_height);
  template <typename RowType>
  static StdReturnType SetCharRow(RowType char_row, byte column, byte row_abs, byte font_width);

  template <typename ColumnType, byte ColumnsSize>
  StdReturnType SetCharFontVertical(byte column, byte row,
                                    FontCharVertical<ColumnType, ColumnsSize> const& font_char,
                                    byte font_height);
  template <typename ColumnType>
  static StdReturnType SetCharColumn(ColumnType char_column, byte column_abs, byte row,
                                     byte font_height);

  static StdReturnType ConvertCharToFontIndex(char character, byte& index);
  static byte ConvertCharToFontIndex(char character);
  void StringShiftTask();
  void CharShiftTask();
  byte GetColumnCenter(FontType font) const {
    return (DISPLAY_NUMBER_OF_COLUMNS / 2u) - (GetFontWidth(font) / 2u);
  }
  byte GetRowCenter(FontType font) const {
    return (DISPLAY_NUMBER_OF_ROWS / 2u) - (GetFontHeight(font) / 2u);
  }

 public:
  /*! \brief Returns the one text output. */
  static Text& GetInstance() {
    static Text singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns what the text output is doing. */
  StateType GetState() const { return state_; }

  /*! \brief Returns the scheduler ticks between two calls of Task(). */
  byte GetTaskCycle() const { return task_cycle_; }

  /*!
   * \brief Returns the font of the shift, the one SetTextWithShift() without a font draws in.
   *
   * It is the font the last of SetFontShift(), SetCharWithShift() and SetTextWithShift() was
   * given, and FontType::kFont10x10 before any of them.
   */
  FontType GetFontShift() const { return shift_.font; }

  /*!
   * \brief Sets the scheduler ticks between two calls of Task().
   *
   * \param[in] cycle The ticks; at 0 the scheduler does not call Task().
   */
  void SetTaskCycle(byte cycle) { task_cycle_ = cycle; }

  /*!
   * \brief Sets the font of the shift, the one SetTextWithShift() without a font draws in.
   *
   * A shift that runs draws its next columns in it.
   *
   * \param[in] font The font; one that IsFontValid() refuses leaves the font as it was.
   */
  void SetFontShift(FontType font) {
    if (IsFontValid(font)) {
      shift_.font = font;
    }
  }

  /*!
   * \brief Returns whether `font` is one of FontType's fonts.
   *
   * It does not say whether the font is built in; see the TEXT_SUPPORT_FONT macros.
   *
   * \param[in] font The font.
   * \return True if `font` lies below FontType::kNumberOfFonts.
   */
  static bool IsFontValid(FontType font) { return font < FontType::kNumberOfFonts; }

  /*!
   * \brief Has the display show what is drawn, through Display::Show().
   *
   * \return What Display::Show() returns.
   */
  static StdReturnType Show() { return Display::GetInstance().Show(); }

  /*!
   * \brief Takes one step of the shift that runs; does nothing while the text output is idle.
   *
   * A step moves the whole display one column to the left, through
   * Transformation::ShiftLeft(), so that what it showed leaves at the left and the column at the
   * right is turned off. It then draws the character coming in one column further in from the
   * right edge. The character stands in the rows that centre its font on the display: its top
   * row is DISPLAY_NUMBER_OF_ROWS / 2 - the font's height / 2, each halved rounding down.
   *
   * A character whose glyph is n columns wide takes n steps to stand whole at the right edge
   * and one more that leaves a dark column after it. A shift of one character is idle after
   * that. A text goes on with its next character in the step after, and after its last
   * character it takes one step more, so two dark columns follow it, before it is idle. A
   * character without a glyph is shifted in as a space.
   *
   * \param[in] show True to call Display::Show() after a step; the scheduler passes true.
   */
  void Task(bool show = false);

  /*! \brief Ends the shift that runs, leaving the display as it stands. */
  void Stop();

  /*!
   * \brief Draws `character` in `font` with the glyph's top-left pixel at `column` and `row`.
   *
   * It writes every pixel of the glyph's box, the glyph's width by the font's height, the dark
   * ones as well as the lit ones, and leaves the pixels around the box as they were. The part of
   * the box right of the last column or below the last row is left out. It only writes the
   * pixels; Show() has them shown.
   *
   * \param[in] column The column of the glyph's left edge, counted from 0 at the left.
   * \param[in] row The row of the glyph's top edge, counted from 0 at the top.
   * \param[in] character The character: an ASCII code from 32 to 127, an umlaut or the degree sign
   *                      of Latin-1.
   * \param[in] font The font.
   * \return kEOk, or kENotOk if `character` has no glyph or `font` is not built in; the display is
   *         then left as it was. kENotOk also if the display refuses a pixel; the glyph is then
   *         drawn in part.
   */
  StdReturnType SetChar(byte column, byte row, char character, FontType font);

  /*!
   * \brief Starts shifting `character` in from the right, in `font`.
   *
   * Task() takes the steps; GetState() returns kCharShift until the character stands whole at
   * the right edge with a dark column after it. It ends a shift that runs, where it stands.
   *
   * `font` becomes the font of the shift as it is given, unlike in SetFontShift(). A font that
   * is not built in has no glyph width, so the shift then only moves the display one column.
   *
   * \param[in] character The character to shift in.
   * \param[in] font The font to draw it in.
   */
  void SetCharWithShift(char character, FontType font);

  /*!
   * \brief Starts shifting `text` in from the right, character by character, in `font`.
   *
   * Task() takes the steps; GetState() returns kTextShift until the last character has come in
   * and two dark columns after it. It ends a shift that runs, where it stands.
   *
   * `font` becomes the font of the shift as it is given, unlike in SetFontShift().
   *
   * \param[in] text The text, ended by a null character.
   * \param[in] font The font to draw it in.
   * \warning The text output keeps the pointer, not a copy: `text` must stay as it is until
   *          GetState() returns kIdle or Stop() ends the shift.
   */
  void SetTextWithShift(char const* text, FontType font);

  /*!
   * \brief Starts shifting `text` in from the right, in the font GetFontShift() returns.
   *
   * Otherwise as SetTextWithShift(const char*, FontType).
   *
   * \param[in] text The text, ended by a null character.
   * \warning The text output keeps the pointer, not a copy: `text` must stay as it is until
   *          GetState() returns kIdle or Stop() ends the shift.
   */
  void SetTextWithShift(char const* text) { SetTextWithShift(text, shift_.font); }

  /*!
   * \brief Returns the height of `font` in rows.
   *
   * \param[in] font The font.
   * \return The height, such as 8 for kFont5x8, or 0 if `font` is not built in.
   */
  byte GetFontHeight(FontType font) const;

  /*!
   * \brief Returns the width of `font` in columns.
   *
   * \param[in] font The font.
   * \return The width, such as 5 for kFont5x8, or 0 if `font` is not built in.
   */
  byte GetFontWidth(FontType font) const;

  /*!
   * \brief Returns the width of the glyph of `character` in `font`, in columns.
   *
   * \param[in] font The font.
   * \param[in] character The character.
   * \return The width; the space's for a character without a glyph, and 0 if `font` is not built
   *         in.
   */
  byte GetFontCharWidth(FontType font, char character) const;

  /*!
   * \brief Returns how the table of `font` is packed.
   *
   * \param[in] font The font.
   * \return kOrientationHorizontal or kOrientationVertical, or kOrientationNone if `font` is not
   *         built in.
   */
  Orientation GetFontOrientation(FontType font) const;
};

#endif  // WORDCLOCK_SRC_TEXT_TEXT_H_
