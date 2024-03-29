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
#define TEXT_ASCII_TABLE_OFFSET               -32
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
    enum StateType {
        STATE_IDLE,
        //STATE_PAUSE,
        STATE_TEXT_SHIFT,
        STATE_CHAR_SHIFT
    };

    enum ShiftStateType {
        SHIFT_STATE_IDLE,
        SHIFT_STATE_BUSY
    };

    enum FontType {
        FONT_5X8,
        FONT_7X9,
        FONT_7X10,
        FONT_9X10,
        FONT_10X10,
        FONT_NUMBER_OF_FONTS
    };

    struct ShiftType {
        const char* Text;
        FontType Font;
        char Char;
        byte CharWidth;
        byte Counter : 7;
        ShiftStateType State : 1;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    Transformation wcTransformation;
    ShiftType Shift;
    byte TaskCycle{TEXT_TASK_CYCLE_INIT_VALUE};
    StateType State{STATE_IDLE};

#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    FontSprite5x8 Font5x8;
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    FontCourierNew7x9 Font7x9;
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    FontCourierNew7x10 Font7x10;
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    FontLucidaSans9x10 Font9x10;
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    FontTahoma10x10 Font10x10;
#endif

    // functions
    constexpr Text() : wcTransformation(), Shift{nullptr, FONT_10X10, STD_NULL_CHARACTER, 0, 0, SHIFT_STATE_IDLE} { }
    ~Text() { }


    template <typename RowType, byte RowsSize>
    StdReturnType setCharFontHorizontal(byte, byte, const FontCharHorizontal<RowType, RowsSize>&, byte);
    template <typename RowType>
    StdReturnType setCharRow(RowType, byte, byte, byte);

    template <typename RowType, byte RowsSize>
    void setCharFontHorizontalFast(byte, byte, const FontCharHorizontal<RowType, RowsSize>&, byte);
    template <typename RowType>
    void setCharRowFast(RowType, byte, byte, byte);

    template <typename ColumnType, byte ColumnsSize>
    StdReturnType setCharFontVertical(byte, byte, const FontCharVertical<ColumnType, ColumnsSize>&, byte);
    template <typename ColumnType>
    StdReturnType setCharColumn(ColumnType, byte, byte, byte);

    template <typename ColumnType, byte ColumnsSize>
    void setCharFontVerticalFast(byte, byte, const FontCharVertical<ColumnType, ColumnsSize>&, byte);
    template <typename ColumnType>
    void setCharColumnFast(ColumnType, byte, byte, byte);

    StdReturnType convertCharToFontIndex(char, byte&) const;
    byte convertCharToFontIndexFast(char) const;
    void stringShiftTask();
    void charShiftTask();
    byte getColumnCenter(FontType Font) const { return (DISPLAY_NUMBER_OF_COLUMNS / 2u) - (getFontWidth(Font) / 2u); }
    byte getRowCenter(FontType Font) const { return (DISPLAY_NUMBER_OF_ROWS / 2u) - (getFontHeight(Font) / 2u); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Text& getInstance() {
        static Text singletonInstance;
        return singletonInstance;
    }

    // get methods
    StateType getState() const { return State; }
    byte getTaskCycle() const { return TaskCycle; }
    FontType getFontShift() const { return Shift.Font; }

    // set methods
    void setTaskCycle(byte Cycle) { TaskCycle = Cycle; }
    void setFontShift(FontType Font) { if(isFontValid(Font)) { Shift.Font = Font; } }

    // methods
    static bool isFontValid(FontType sFont) { return sFont < FONT_NUMBER_OF_FONTS; }
    StdReturnType show() const { return Display::getInstance().show(); }
    void task(bool=false);
    void stop();
    StdReturnType setChar(byte, byte, char, FontType);
    void setCharFast(byte, byte, char, FontType);
    void setCharWithShift(char, FontType);
    void setText(const char*, FontType);
    void setTextWithShift(const char*, FontType);
    void setTextWithShift(const char* Text) { setTextWithShift(Text, Shift.Font); }
    byte getFontHeight(FontType) const;
    byte getFontWidth(FontType) const;
    byte getFontCharWidth(FontType, char) const;
    Orientation getFontOrientation(FontType) const;
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
