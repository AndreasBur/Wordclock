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
/**     \file       Text.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _TEXT_H_
#define _TEXT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "Transformation.h"
#include "FontCourierNew7x10.h"
#include "FontCourierNew7x9.h"
#include "FontLucidaSans9x10.h"
#include "FontSprite5x8.h"
#include "FontTahoma10x10.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Text configuration parameter */
#define TEXT_TASK_CYCLE_INIT_VALUE            1u
#define TEXT_SUPPORT_FONT_5X8                 STD_ON
#define TEXT_SUPPORT_FONT_7X9                 STD_ON
#define TEXT_SUPPORT_FONT_7X10                STD_ON
#define TEXT_SUPPORT_FONT_9X10                STD_ON
#define TEXT_SUPPORT_FONT_10X10               STD_ON

/* Text parameter */
#define TEXT_ASCII_TABLE_OFFSET               (-32)
#define TEXT_ASCII_CHAR_MIN                    32
#define TEXT_ASCII_CHAR_MAX                   127


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   A N I M A T I O N   F O N T
******************************************************************************************************************************************************/
class Text
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum class StateType {
        kIdle,
        kTextShift,
        kCharShift
    };

    /* uint8_t and not the default int: it is stored in a one-bit field below, where a signed
       type holds 0 and -1, and kBusy would read back as -1. */
    enum class ShiftStateType : uint8_t {
        kIdle,
        kBusy
    };

    enum class FontType {
        kFont5x8,
        kFont7x9,
        kFont7x10,
        kFont9x10,
        kFont10x10,
        kNumberOfFonts
    };
    static constexpr byte kNumberOfFonts{static_cast<byte>(FontType::kNumberOfFonts)};

    struct ShiftType {
        const char* text;
        FontType font;
        char character;
        byte char_width;
        byte counter : 7;
        ShiftStateType state : 1;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* The first glyph of every table is the space, which is what a character with no glyph
       of its own is drawn as. */
    static constexpr byte kSpaceFontIndex{0u};
    Transformation wc_transformation_;
    ShiftType shift_;
    byte task_cycle_{TEXT_TASK_CYCLE_INIT_VALUE};
    StateType state_{StateType::kIdle};

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    FontSprite5x8 font5x8_;
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    FontCourierNew7x9 font7x9_;
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    FontCourierNew7x10 font7x10_;
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    FontLucidaSans9x10 font9x10_;
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    FontTahoma10x10 font10x10_;
#endif

    // functions
    constexpr Text() : wc_transformation_(), shift_{nullptr, FontType::kFont10x10, STD_NULL_CHARACTER, 0, 0, ShiftStateType::kIdle} { }
    ~Text() { }


    /* Each of these four had a twin that differed in nothing a reader sees - it dropped the
       answer instead of passing it on - and calling the wrong one of the pair from the wrong
       place cost bounds checks and looked right. That happened once, in SetChar()'s 9x10
       branch. There is one of each now, so the mistake has nowhere left to happen, and the
       [[nodiscard]] that used to police it is gone with the twins. */
    template <typename RowType, byte RowsSize>
    StdReturnType SetCharFontHorizontal(byte, byte, const FontCharHorizontal<RowType, RowsSize>&, byte);
    template <typename RowType>
    static StdReturnType SetCharRow(RowType, byte, byte, byte);

    template <typename ColumnType, byte ColumnsSize>
    StdReturnType SetCharFontVertical(byte, byte, const FontCharVertical<ColumnType, ColumnsSize>&, byte);
    template <typename ColumnType>
    static StdReturnType SetCharColumn(ColumnType, byte, byte, byte);

    static StdReturnType ConvertCharToFontIndex(char, byte&);
    static byte ConvertCharToFontIndex(char);
    void StringShiftTask();
    void CharShiftTask();
    byte GetColumnCenter(FontType font) const { return (DISPLAY_NUMBER_OF_COLUMNS / 2u) - (GetFontWidth(font) / 2u); }
    byte GetRowCenter(FontType font) const { return (DISPLAY_NUMBER_OF_ROWS / 2u) - (GetFontHeight(font) / 2u); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Text& GetInstance() {
        static Text singleton_instance;
        return singleton_instance;
    }

    // get methods
    StateType GetState() const { return state_; }
    byte GetTaskCycle() const { return task_cycle_; }
    FontType GetFontShift() const { return shift_.font; }

    // set methods
    void SetTaskCycle(byte cycle) { task_cycle_ = cycle; }
    void SetFontShift(FontType font) { if(IsFontValid(font)) { shift_.font = font; } }

    // methods
    static bool IsFontValid(FontType font) { return font < FontType::kNumberOfFonts; }
    static StdReturnType Show() { return Display::GetInstance().Show(); }
    void Task(bool=false);
    void Stop();
    StdReturnType SetChar(byte, byte, char, FontType);
    void SetCharWithShift(char, FontType);
    void SetTextWithShift(const char*, FontType);
    void SetTextWithShift(const char* text) { SetTextWithShift(text, shift_.font); }
    byte GetFontHeight(FontType) const;
    byte GetFontWidth(FontType) const;
    byte GetFontCharWidth(FontType, char) const;
    Orientation GetFontOrientation(FontType) const;
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
