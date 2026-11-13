// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The letters on the front panel, one string per row.
 *
 * One byte per letter, so the umlauts are escapes rather than literals: \\xDC is U with an umlaut
 * and \\xD6 is O with one, in Latin-1. Written as escapes so the table cannot be changed by
 * whatever encoding an editor saves the file in, and read as Latin-1 by whoever displays it.
 *
 * This is the one table of the front panel's letters. It has to stay in step with the
 * CharacterIdType enumeration in the header, which names the same 110 positions, and with the word
 * positions in DisplayWords.
 */

#define _DISPLAY_CHARACTERS_SOURCE_

#include "DisplayCharacters.h"

// The table the file block describes: one string per row, row 0 first, each with the null that
// ends it - hence the + 1.
const DisplayCharacters::DisplayCharactersTableElementType DisplayCharacters::kDisplayCharactersTable[][DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS + 1] PROGMEM
{
    "ESKISTLF\xDCNF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZW\xD6LFP",
    "ZWEINSIEBEN",
    "KDREIRHF\xDCNF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
};

StdReturnType DisplayCharacters::GetCharacter(byte column, byte row, char& character)
{
    if(IsColumnAndRowValid(column, row)) {
        character =  GetDisplayCharactersTableElement(column, row);
        return kEOk;
    } else {
        return kENotOk;
    }
}

StdReturnType DisplayCharacters::GetCharacter(byte index, char& character)
{
    if(IsIndexValid(index)) {
        character =  GetDisplayCharactersTableElement(index);
        return kEOk;
    } else {
        return kENotOk;
    }
}

StdReturnType DisplayCharacters::GetCharacter(CharacterIdType character_id, char& character)
{
    if(IsCharacterIdValid(character_id)) {
        character =  GetDisplayCharactersTableElement(ToUnderlying(character_id));
        return kEOk;
    } else {
        return kENotOk;
    }
}
