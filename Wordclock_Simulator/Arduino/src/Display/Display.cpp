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
/**     \file       Display.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _DISPLAY_SOURCE_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "Display.h"
#if defined(SIMULATOR) && (defined(__APPLE__ ) || defined(__linux__))
#include "WordclockIcon.xpm"
#endif

/******************************************************************************************************************************************************
 *  LOCAL CONSTANT MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  LOCAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  LOCAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  CONSTRUCTOR OF Display
******************************************************************************************************************************************************/
/*! \brief          Display Constructor
 *  \details        Instantiation of the Display library
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::Display(PixelColorType sColor) : Pixels(DISPLAY_DATA_PIN)
{
    Color = sColor;
    State = STATE_UNINIT;

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
    Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  CONSTRUCTOR OF Display
******************************************************************************************************************************************************/
/*! \brief          Display Constructor
 *  \details        Instantiation of the Display library
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::Display(byte Red, byte Green, byte Blue) : Pixels(DISPLAY_DATA_PIN)
{
    Color.Red = Red;
    Color.Green = Green;
    Color.Blue = Blue;
    State = STATE_UNINIT;

#ifdef SIMULATOR
    Pixels.SetIcon(wxICON(WordclockIcon));
    Pixels.Show();
#endif
} /* Display */


/******************************************************************************************************************************************************
  DESTRUCTOR OF Display
******************************************************************************************************************************************************/
Display::~Display()
{

} /* ~Template */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::init()
{
    clear();
    State = STATE_INIT;
} /* init */


/******************************************************************************************************************************************************
  setWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setWord(WordIdType WordId, byte MaxLength)
{
    stdReturnType ReturnValue = E_NOT_OK;
    byte Length;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        ReturnValue = E_OK;
        DisplayWords::Word Word = Words.getDisplayWordFast(WordId);

        if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) Length = Word.Length;
        else Length = MaxLength;

        for(byte Index = 0; Index < Length; Index++) {
            if(setPixel(Word.Column + Index,  Word.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setWord */


/******************************************************************************************************************************************************
  setWordFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::setWordFast(WordIdType WordId, byte MaxLength)
{
    byte Length;

    DisplayWords::Word Word = Words.getDisplayWordFast(WordId);

    if(MaxLength == DISPLAY_WORD_LENGTH_UNLIMITED) Length = Word.Length;
    else Length = MaxLength;

    for(byte Index = 0; Index < Length; Index++) { setPixelFast(Word.Column + Index,  Word.Row); }
} /* setWordFast */


/******************************************************************************************************************************************************
  clearWord()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearWord(WordIdType WordId)
{
    stdReturnType ReturnValue = E_NOT_OK;

    if(WordId < DisplayWords::WORD_NUMBER_OF_WORDS) {
        ReturnValue = E_OK;
        DisplayWords::Word Word = Words.getDisplayWordFast(WordId);

        for(byte Index = 0; Index < Word.Length; Index++) {
            if(clearPixel(Word.Column + Index,  Word.Row) == E_NOT_OK) ReturnValue = E_NOT_OK;
        }
    } else {
        ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* clearWord */


/******************************************************************************************************************************************************
  clearWordFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearWordFast(WordIdType WordId)
{
    DisplayWords::Word Word = Words.getDisplayWordFast(WordId);

    for(byte Index = 0; Index < Word.Length; Index++) { clearPixelFast(Word.Column + Index,  Word.Row); }
} /* clearWordFast */


/******************************************************************************************************************************************************
  clearAllWords()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearAllWords()
{
    stdReturnType ReturnValue = E_OK;

    for(byte i = DisplayWords::WORD_ES; i < DisplayWords::WORD_NUMBER_OF_WORDS; i++) if(clearWord((WordIdType) i) == E_NOT_OK) ReturnValue = E_NOT_OK;
    return ReturnValue;
} /* clearAllWords */


/******************************************************************************************************************************************************
  clearAllWordsFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearAllWordsFast()
{
    for(byte i = DisplayWords::WORD_ES; i < DisplayWords::WORD_NUMBER_OF_WORDS; i++) clearWordFast((WordIdType) i);
} /* clearAllWordsFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixel(byte Index, boolean* Value) const
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    return getPixel(Column, Row, Value);
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean Display::getPixelFast(byte Index) const
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    return getPixelFast(Column, Row);
} /* getPixelFast */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixel(byte Column, byte Row, boolean* Value)  const
{
    stdReturnType ReturnValue{E_NOT_OK};
    PixelColorType Pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    ReturnValue = Pixels.getPixel(transformToSerpentine(Column,  Row), &Pixel);
#else
    ReturnValue = Pixels.getPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, &Pixel);
#endif
    if(ReturnValue == E_OK) {
        /* Pixel is only off when all colors are zero */
        if(Pixel.Red == 0 && Pixel.Green == 0 && Pixel.Blue == 0) *Value = false;
        else *Value = true;
    }
    return ReturnValue;
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
boolean Display::getPixelFast(byte Column, byte Row)  const
{
    PixelColorType Pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    Pixel = Pixels.getPixelFast(transformToSerpentine(Column,  Row));
#else
    Pixel = Pixels.getPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
    if(Pixel.Red == 0 && Pixel.Green == 0 && Pixel.Blue == 0) return false;
    else return true;
} /* getPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixel(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
    return Pixels.setPixel(transformToSerpentine(Column,  Row), Color);
#else
    return Pixels.setPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
#endif
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine the odd row: count from right to left */
    Pixels.setPixelFast(transformToSerpentine(Column,  Row), Color);
#else
    Pixels.setPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column, Color);
#endif
} /* setPixelFast */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixel(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    return setPixel(Column,  Row);
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelFast(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    setPixelFast(Column,  Row);
} /* setPixelFast */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearPixel(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    return Pixels.clearPixel(transformToSerpentine(Column,  Row));
#else
    return Pixels.clearPixel((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    Pixels.clearPixelFast(transformToSerpentine(Column,  Row));
#else
    Pixels.clearPixelFast((Row * DISPLAY_NUMBER_OF_COLUMNS) + Column);
#endif
} /* clearPixelFast */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::clearPixel(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    return clearPixel(Column,  Row);
} /* clearPixel */


/******************************************************************************************************************************************************
  clearPixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::clearPixelFast(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    clearPixelFast(Column,  Row);
} /* clearPixelFast */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::togglePixel(byte Column, byte Row)
{
    boolean Pixel;

#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    byte Index = transformToSerpentine(Column,  Row);
    getPixel(Index, &Pixel);
    if(Pixel) { return Pixels.clearPixel(Index); }
    else { return Pixels.setPixel(Index, Color); }
#else
    byte Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    getPixel(Index, &Pixel);
    if(Pixel) { return Pixels.clearPixel(Index); }
    else { return Pixels.setPixel(Index, Color); }
#endif
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixelFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::togglePixelFast(byte Column, byte Row)
{
#if (DISPLAY_LED_STRIPE_SERPENTINE == STD_ON)
    /* if led stripe is snake or serpentine then odd row: count from right to left */
    byte Index = transformToSerpentine(Column,  Row);
    if(getPixelFast(Index)) Pixels.clearPixelFast(Index);
    else Pixels.setPixelFast(Index, Color);
#else
    byte Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    if(getPixelFast(Index)) Pixels.clearPixelFast(Index);
    else Pixels.setPixelFast(Index, Color);
#endif
} /* togglePixelFast */


/******************************************************************************************************************************************************
  togglePixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::togglePixel(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    return togglePixel(Column,  Row);
} /* togglePixel */


/******************************************************************************************************************************************************
  togglePixelFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void Display::togglePixelFast(byte Index)
{
    byte Row, Column;
    indexToColumnAndRow(Index, Row, Column);
    togglePixelFast(Column,  Row);
} /* togglePixelFast */


/******************************************************************************************************************************************************
  getPixelRow()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixelRow(byte Row, PixelRowType* PixelRow) const
{
    stdReturnType ReturnValue = E_OK;
    PixelType Pixel;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(getPixel(Column, Row, &Pixel) == E_OK) {
            WRITE_BIT(*PixelRow, Column, Pixel);
        } else {
            ReturnValue = E_NOT_OK;
        }
    }
    return ReturnValue;
} /* getPixelRow */


/******************************************************************************************************************************************************
  getPixelRowFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::PixelRowType Display::getPixelRowFast(byte Row)  const
{
    PixelRowType PixelRow = 0;
    
    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        WRITE_BIT(PixelRow, Column, getPixelFast(Column, Row));
    }
    return PixelRow;
} /* getPixelRowFast */


/******************************************************************************************************************************************************
  getPixelColumn()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::getPixelColumn(byte Column, PixelRowType* PixelColumn)  const
{
    stdReturnType ReturnValue = E_OK;
    PixelType Pixel;

    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(getPixel(Column, Row, &Pixel) == E_OK) {
            WRITE_BIT(*PixelColumn, Row, Pixel);
        } else {
            ReturnValue = E_NOT_OK;
        }
    }
    return ReturnValue;
} /* getPixelColumn */


/******************************************************************************************************************************************************
  getPixelColumnFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
Display::PixelColumnType Display::getPixelColumnFast(byte Column)  const
{
    Display::PixelColumnType PixelColumn = 0;
    
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        WRITE_BIT(PixelColumn, Row, getPixelFast(Column, Row));
    }
    return PixelColumn;
} /* getPixelColumnFast */


/******************************************************************************************************************************************************
  setPixelRow()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixelRow(byte Row, PixelRowType PixelRow)
{
    stdReturnType ReturnValue = E_OK;

    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        if(writePixel(Column, Row, READ_BIT(PixelRow, Column)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setPixelRow */


/******************************************************************************************************************************************************
  setPixelRowFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelRowFast(byte Row, PixelRowType PixelRow)
{
    for(byte Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        writePixelFast(Column, Row, READ_BIT(PixelRow, Column));
    }
} /* setPixelRowFast */


/******************************************************************************************************************************************************
  setPixelColumn()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
stdReturnType Display::setPixelColumn(byte Column, PixelRowType PixelColumn)
{
    stdReturnType ReturnValue = E_OK;
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        if(writePixel(Column, Row, READ_BIT(PixelColumn, Row)) == E_NOT_OK) ReturnValue = E_NOT_OK;
    }
    return ReturnValue;
} /* setPixelColumn */


/******************************************************************************************************************************************************
  setPixelColumnFast()
******************************************************************************************************************************************************/
/*! \brief
 *  \details
 *
 *  \return         -
******************************************************************************************************************************************************/
void Display::setPixelColumnFast(byte Column, PixelRowType PixelColumn)
{
    for(byte Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        writePixelFast(Column, Row, READ_BIT(PixelColumn, Row));
    }
} /* setPixelColumnFast */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  transformToSerpentine()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
byte Display::transformToSerpentine(byte Column, byte Row) const
{
    byte Index;

    if(IS_BIT_CLEARED(Row, 0)) Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + Column;
    else Index = (Row * DISPLAY_NUMBER_OF_COLUMNS) + (DISPLAY_NUMBER_OF_COLUMNS - Column - 1);
    
    return Index;
} /* transformToSerpentine */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
