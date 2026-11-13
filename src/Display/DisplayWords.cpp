// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The table of words and where each sits on the letter grid.
 */

#include "DisplayWords.h"

/*
 * One entry per WordIdType, in its order: the column of the word's first letter, its row and its
 * length, as DisplayWord takes them. It has to stay in step with WordIdType and with the letters
 * of DisplayCharacters by hand, as nothing checks either.
 */
const DisplayWord DisplayWords::kDisplayWordsTable[] PROGMEM
{
    DisplayWord(0,0,0),                  //  0 = DISPLAY_WORD_NONE           = ""
    DisplayWord(0,0,2),                  //  1 = DisplayWords::WordIdType::kEs       = "ES"
    DisplayWord(3,0,3),                  //  2 = DISPLAY_WORD_IST            = "IST"
    DisplayWord(7,0,4),                  //  3 = DISPLAY_WORD_FUENF          = "FÜNF"
    DisplayWord(0,1,4),                  //  4 = DISPLAY_WORD_ZEHN           = "ZEHN"
    DisplayWord(4,1,7),                  //  5 = DISPLAY_WORD_ZWANZIG        = "ZWANZIG"
    DisplayWord(0,2,4),                  //  6 = DISPLAY_WORD_DREI           = "DREI"
    DisplayWord(4,2,4),                  //  7 = DISPLAY_WORD_VIER           = "VIER"
    DisplayWord(4,2,7),                  //  8 = DISPLAY_WORD_VIERTEL        = "VIERTEL"
    DisplayWord(0,2,11),                 //  9 = DISPLAY_WORD_DREIVIERTEL    = "DREIVIERTEL"
    DisplayWord(2,3,4),                  // 10 = DISPLAY_WORD_NACH           = "NACH"
    DisplayWord(6,3,3),                  // 11 = DISPLAY_WORD_VOR            = "VOR"
    DisplayWord(0,4,4),                  // 12 = DISPLAY_WORD_HALB           = "HALB"
    DisplayWord(5,4,5),                  // 13 = DISPLAY_WORD_HOUR_ZWOELF    = "ZWÖLF"
    DisplayWord(0,5,4),                  // 14 = DISPLAY_WORD_HOUR_ZWEI      = "ZWEI"
    DisplayWord(2,5,3),                  // 15 = DISPLAY_WORD_HOUR_EIN       = "EIN"
    DisplayWord(2,5,4),                  // 16 = DISPLAY_WORD_HOUR_EINS      = "EINS"
    DisplayWord(5,5,6),                  // 17 = DISPLAY_WORD_HOUR_SIEBEN    = "SIEBEN"
    DisplayWord(1,6,4),                  // 18 = DISPLAY_WORD_HOUR_DREI      = "DREI"
    DisplayWord(7,6,4),                  // 19 = DISPLAY_WORD_HOUR_FUENF     = "FÜNF"
    DisplayWord(0,7,3),                  // 20 = DISPLAY_WORD_HOUR_ELF       = "ELF"
    DisplayWord(3,7,4),                  // 21 = DISPLAY_WORD_HOUR_NEUN      = "NEUN"
    DisplayWord(7,7,4),                  // 22 = DISPLAY_WORD_HOUR_VIER      = "VIER"
    DisplayWord(1,8,4),                  // 23 = DISPLAY_WORD_HOUR_ACHT      = "ACHT"
    DisplayWord(5,8,4),                  // 24 = DISPLAY_WORD_HOUR_ZEHN      = "ZEHN"
    DisplayWord(1,9,5),                  // 25 = DISPLAY_WORD_HOUR_SECHS     = "SECHS"
    DisplayWord(8,9,3),                  // 26 = DISPLAY_WORD_UHR            = "UHR"
};

StdReturnType DisplayWords::GetDisplayWord(WordIdType word_id, DisplayWord& word)
{
    StdReturnType return_value{kENotOk};

    if(IsWordIdValid(word_id)) {
        word = GetDisplayWordsTableElement(ToUnderlying(word_id));
        return_value = kEOk;
    } else {
        return_value = kENotOk;
    }
    return return_value;
}

StdReturnType DisplayWords::GetDisplayWordLength(WordIdType word_id, byte& length)
{
    StdReturnType return_value{kENotOk};

    if(IsWordIdValid(word_id)) {
        length = GetDisplayWordsTableElement(ToUnderlying(word_id)).GetLength();
        return_value = kEOk;
    } else {
        return_value = kENotOk;
    }
    return return_value;
}

StdReturnType DisplayWords::GetDisplayWordColumn(WordIdType word_id, byte& column)
{
    StdReturnType return_value{kENotOk};

    if(IsWordIdValid(word_id)) {
        column = GetDisplayWordsTableElement(ToUnderlying(word_id)).GetColumn();
        return_value = kEOk;
    } else {
        return_value = kENotOk;
    }
    return return_value;
}

StdReturnType DisplayWords::GetDisplayWordRow(WordIdType word_id, byte& row)
{
    StdReturnType return_value{kENotOk};

    if(IsWordIdValid(word_id)) {
        row = GetDisplayWordsTableElement(ToUnderlying(word_id)).GetRow();
        return_value = kEOk;
    } else {
        return_value = kENotOk;
    }
    return return_value;
}
