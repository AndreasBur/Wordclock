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
    enum class CharacterIdType {
        kE1,
        kS1,
        kK1,
        kI1,
        kS2,
        kT1,
        kL1,
        kF1,
        kUe1,
        kN1,
        kF2,
        kZ1,
        kE2,
        kH1,
        kN2,
        kZ2,
        kW1,
        kA1,
        kN3,
        kZ3,
        kI2,
        kG1,
        kD1,
        kR1,
        kE3,
        kI3,
        kV1,
        kI4,
        kE4,
        kR2,
        kT2,
        kE5,
        kL2,
        kT3,
        kG2,
        kN4,
        kA2,
        kC1,
        kH2,
        kV2,
        kO1,
        kR3,
        kJ1,
        kM1,
        kH3,
        kA3,
        kL3,
        kB1,
        kQ1,
        kZ4,
        kW2,
        kOe1,
        kL4,
        kF3,
        kP1,
        kZ5,
        kW3,
        kE6,
        kI5,
        kN5,
        kS3,
        kI6,
        kE7,
        kB2,
        kE8,
        kN6,
        kK2,
        kD2,
        kR4,
        kE9,
        kI7,
        kR5,
        kH4,
        kF4,
        kUe2,
        kN7,
        kF5,
        kE10,
        kL5,
        kF6,
        kN8,
        kE11,
        kU3,
        kN9,
        kV3,
        kI8,
        kE12,
        kR6,
        kW4,
        kA4,
        kC2,
        kH5,
        kT5,
        kZ6,
        kE13,
        kH6,
        kN10,
        kR7,
        kS4,
        kB3,
        kS5,
        kE14,
        kC3,
        kH7,
        kS6,
        kF7,
        kM2,
        kU4,
        kH8,
        kR8,
        kNumberOfCharacters
    };
    static constexpr byte kNumberOfCharacters{static_cast<byte>(CharacterIdType::kNumberOfCharacters)};
    static_assert(kNumberOfCharacters == DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS,
                  "DisplayCharacters: one character id per letter of the plate");

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
        return GetDisplayCharactersTableElement(ToUnderlying(character_id));
    }
    static StdReturnType GetCharacter(byte, byte, char&);
    static StdReturnType GetCharacter(byte, char&);
    static StdReturnType GetCharacter(CharacterIdType, char&);

    // set methods

    // methods
    static bool IsCharacterIdValid(CharacterIdType character_id) { return character_id < CharacterIdType::kNumberOfCharacters; };
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
