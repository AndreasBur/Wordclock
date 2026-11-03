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
/**     \file       DisplayCharacters.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_CHARACTERS_H_
#define _DISPLAY_CHARACTERS_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* DisplayCharacters configuration parameter */


/* DisplayCharacters parameter */
#define DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS                11u
#define DISPLAY_CHARACTERS_NUMBER_OF_ROWS                   10u
#define DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS             (DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS * DISPLAY_CHARACTERS_NUMBER_OF_ROWS)

/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D I S P L A Y C H A R A C T E R S
******************************************************************************************************************************************************/
class DisplayCharacters
{
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
  public:
    enum CharacterIdType {
        kCharacterE1,
        kCharacterS1,
        kCharacterK1,
        kCharacterI1,
        kCharacterS2,
        kCharacterT1,
        kCharacterL1,
        kCharacterF1,
        kCharacterUe1,
        kCharacterN1,
        kCharacterF2,
        kCharacterZ1,
        kCharacterE2,
        kCharacterH1,
        kCharacterN2,
        kCharacterZ2,
        kCharacterW1,
        kCharacterA1,
        kCharacterN3,
        kCharacterZ3,
        kCharacterI2,
        kCharacterG1,
        kCharacterD1,
        kCharacterR1,
        kCharacterE3,
        kCharacterI3,
        kCharacterV1,
        kCharacterI4,
        kCharacterE4,
        kCharacterR2,
        kCharacterT2,
        kCharacterE5,
        kCharacterL2,
        kCharacterT3,
        kCharacterG2,
        kCharacterN4,
        kCharacterA2,
        kCharacterC1,
        kCharacterH2,
        kCharacterV2,
        kCharacterO1,
        kCharacterR3,
        kCharacterJ1,
        kCharacterM1,
        kCharacterH3,
        kCharacterA3,
        kCharacterL3,
        kCharacterB1,
        kCharacterQ1,
        kCharacterZ4,
        kCharacterW2,
        kCharacterOe1,
        kCharacterL4,
        kCharacterF3,
        kCharacterP1,
        kCharacterZ5,
        kCharacterW3,
        kCharacterE6,
        kCharacterI5,
        kCharacterN5,
        kCharacterS3,
        kCharacterI6,
        kCharacterE7,
        kCharacterB2,
        kCharacterE8,
        kCharacterN6,
        kCharacterK2,
        kCharacterD2,
        kCharacterR4,
        kCharacterE9,
        kCharacterI7,
        kCharacterR5,
        kCharacterH4,
        kCharacterF4,
        kCharacterUe2,
        kCharacterN7,
        kCharacterF5,
        kCharacterE10,
        kCharacterL5,
        kCharacterF6,
        kCharacterN8,
        kCharacterE11,
        kCharacterU3,
        kCharacterN9,
        kCharacterV3,
        kCharacterI8,
        kCharacterE12,
        kCharacterR6,
        kCharacterW4,
        kCharacterA4,
        kCharacterC2,
        kCharacterH5,
        kCharacterT5,
        kCharacterZ6,
        kCharacterE13,
        kCharacterH6,
        kCharacterN10,
        kCharacterR7,
        kCharacterS4,
        kCharacterB3,
        kCharacterS5,
        kCharacterE14,
        kCharacterC3,
        kCharacterH7,
        kCharacterS6,
        kCharacterF7,
        kCharacterM2,
        kCharacterU4,
        kCharacterH8,
        kCharacterR8,
        kCharacterNumberOfCharacters
    };

    using DisplayCharactersTableElementType = char;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const char kDisplayCharactersTable[][DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS + 1u];

    // functions
    /* Row first, because that is how the table is written - one string per row. It used
       to be indexed the other way round, which every caller then compensated for by
       swapping its arguments; the two that did not compensate read past the end of the
       table for the last column. Nothing called any of them, so nothing showed it. */
    static DisplayCharactersTableElementType GetDisplayCharactersTableElement(byte column, byte row) {
        DisplayCharactersTableElementType display_characters_table_element;
        memcpy_P(&display_characters_table_element, &kDisplayCharactersTable[row][column], sizeof(DisplayCharactersTableElementType));
        return display_characters_table_element;
    }
    static DisplayCharactersTableElementType GetDisplayCharactersTableElement(byte index) {
        byte column, row;
        IndexToColumnAndRow(index, column, row);
        return GetDisplayCharactersTableElement(column, row);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DisplayCharacters() { }
    ~DisplayCharacters() { }

    /* Two forms of every reader, and the argument list says which: the one taking a
       reference answers whether the character was there, the one taking none answers the
       character. Both check, and the unchecked read is the private table element above -
       a plate has no character outside its grid, so STD_NULL_CHARACTER is the honest
       answer where a read past the table used to be. */
    // get methods
    static char GetCharacter(byte column, byte row) {
        if(!IsColumnAndRowValid(column, row)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(column, row);
    }
    static char GetCharacter(byte index) {
        if(!IsIndexValid(index)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(index);
    }
    static char GetCharacter(CharacterIdType character_id) {
        if(!IsCharacterIdValid(character_id)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(character_id);
    }
    static StdReturnType GetCharacter(byte, byte, char&);
    static StdReturnType GetCharacter(byte, char&);
    static StdReturnType GetCharacter(CharacterIdType, char&);

    // set methods

    // methods
    static bool IsCharacterIdValid(CharacterIdType character_id) { return character_id < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; };
    static bool IsIndexValid(byte index) { return index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; };
    static constexpr bool IsColumnAndRowValid(byte column, byte row) {
        return (column < DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS) && (row < DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    }
    /* Column before Row, matching both the name and Display's function of the same name.
       They were the other way round here, so a caller that read the two classes side by
       side got a silent swap. */
    static void IndexToColumnAndRow(byte index, byte& column, byte& row) { column = index % DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS; row = index / DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS; }
    static byte ColumnAndRowToIndex(byte column, byte row) { return (row * DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS) + column; }

};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
