// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The letters of the grid, by id, and the size of the grid.
 */

#ifndef _DISPLAY_CHARACTERS_H_
#define _DISPLAY_CHARACTERS_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The number of columns of the letter grid. */
#define DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS                11u
/*! \brief The number of rows of the letter grid. */
#define DISPLAY_CHARACTERS_NUMBER_OF_ROWS                   10u
/*! \brief The number of letters of the grid, its columns times its rows. */
#define DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS             (DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS * DISPLAY_CHARACTERS_NUMBER_OF_ROWS)

/*!
 * \brief The letters of the grid: which letter stands where, by column and row, by index or by id.
 *
 * Columns count from 0 at the first letter of a row, and rows from 0 at the row of ES IST. The
 * index of a letter is its row times DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS plus its column, and
 * the id of a letter is its index. A letter is one byte, and the umlauts are Latin-1: 0xDC is U
 * with an umlaut and 0xD6 is O with one.
 *
 * Every reader comes in two forms, and the argument list says which: the one taking a reference
 * answers whether the character was there, the one taking none answers the character. Both check.
 * A plate has no character outside its grid, so STD_NULL_CHARACTER is the honest answer where a
 * read past the table used to be.
 */
class DisplayCharacters
{
  public:
    /*! \brief The letters of the grid, by id: one per letter, in the order of their index. */
    enum class CharacterIdType : byte {
        kE1, /*!< The E of ES, column 0 of row 0. */
        kS1, /*!< The S of ES, column 1 of row 0. */
        kK1, /*!< The filler K, column 2 of row 0. */
        kI1, /*!< The I of IST, column 3 of row 0. */
        kS2, /*!< The S of IST, column 4 of row 0. */
        kT1, /*!< The T of IST, column 5 of row 0. */
        kL1, /*!< The filler L, column 6 of row 0. */
        kF1, /*!< The F of FÜNF, column 7 of row 0. */
        kUe1, /*!< The Ü of FÜNF, column 8 of row 0. */
        kN1, /*!< The N of FÜNF, column 9 of row 0. */
        kF2, /*!< The F of FÜNF, column 10 of row 0. */
        kZ1, /*!< The Z of ZEHN, column 0 of row 1. */
        kE2, /*!< The E of ZEHN, column 1 of row 1. */
        kH1, /*!< The H of ZEHN, column 2 of row 1. */
        kN2, /*!< The N of ZEHN, column 3 of row 1. */
        kZ2, /*!< The Z of ZWANZIG, column 4 of row 1. */
        kW1, /*!< The W of ZWANZIG, column 5 of row 1. */
        kA1, /*!< The A of ZWANZIG, column 6 of row 1. */
        kN3, /*!< The N of ZWANZIG, column 7 of row 1. */
        kZ3, /*!< The Z of ZWANZIG, column 8 of row 1. */
        kI2, /*!< The I of ZWANZIG, column 9 of row 1. */
        kG1, /*!< The G of ZWANZIG, column 10 of row 1. */
        kD1, /*!< The D of DREI and DREIVIERTEL, column 0 of row 2. */
        kR1, /*!< The R of DREI and DREIVIERTEL, column 1 of row 2. */
        kE3, /*!< The E of DREI and DREIVIERTEL, column 2 of row 2. */
        kI3, /*!< The I of DREI and DREIVIERTEL, column 3 of row 2. */
        kV1, /*!< The V of VIER, VIERTEL and DREIVIERTEL, column 4 of row 2. */
        kI4, /*!< The I of VIER, VIERTEL and DREIVIERTEL, column 5 of row 2. */
        kE4, /*!< The E of VIER, VIERTEL and DREIVIERTEL, column 6 of row 2. */
        kR2, /*!< The R of VIER, VIERTEL and DREIVIERTEL, column 7 of row 2. */
        kT2, /*!< The T of VIERTEL and DREIVIERTEL, column 8 of row 2. */
        kE5, /*!< The E of VIERTEL and DREIVIERTEL, column 9 of row 2. */
        kL2, /*!< The L of VIERTEL and DREIVIERTEL, column 10 of row 2. */
        kT3, /*!< The filler T, column 0 of row 3. */
        kG2, /*!< The filler G, column 1 of row 3. */
        kN4, /*!< The N of NACH, column 2 of row 3. */
        kA2, /*!< The A of NACH, column 3 of row 3. */
        kC1, /*!< The C of NACH, column 4 of row 3. */
        kH2, /*!< The H of NACH, column 5 of row 3. */
        kV2, /*!< The V of VOR, column 6 of row 3. */
        kO1, /*!< The O of VOR, column 7 of row 3. */
        kR3, /*!< The R of VOR, column 8 of row 3. */
        kJ1, /*!< The filler J, column 9 of row 3. */
        kM1, /*!< The filler M, column 10 of row 3. */
        kH3, /*!< The H of HALB, column 0 of row 4. */
        kA3, /*!< The A of HALB, column 1 of row 4. */
        kL3, /*!< The L of HALB, column 2 of row 4. */
        kB1, /*!< The B of HALB, column 3 of row 4. */
        kQ1, /*!< The filler Q, column 4 of row 4. */
        kZ4, /*!< The Z of ZWÖLF, column 5 of row 4. */
        kW2, /*!< The W of ZWÖLF, column 6 of row 4. */
        kOe1, /*!< The Ö of ZWÖLF, column 7 of row 4. */
        kL4, /*!< The L of ZWÖLF, column 8 of row 4. */
        kF3, /*!< The F of ZWÖLF, column 9 of row 4. */
        kP1, /*!< The filler P, column 10 of row 4. */
        kZ5, /*!< The Z of ZWEI, column 0 of row 5. */
        kW3, /*!< The W of ZWEI, column 1 of row 5. */
        kE6, /*!< The E of ZWEI, EIN and EINS, column 2 of row 5. */
        kI5, /*!< The I of ZWEI, EIN and EINS, column 3 of row 5. */
        kN5, /*!< The N of EIN and EINS, column 4 of row 5. */
        kS3, /*!< The S of EINS and SIEBEN, column 5 of row 5. */
        kI6, /*!< The I of SIEBEN, column 6 of row 5. */
        kE7, /*!< The E of SIEBEN, column 7 of row 5. */
        kB2, /*!< The B of SIEBEN, column 8 of row 5. */
        kE8, /*!< The E of SIEBEN, column 9 of row 5. */
        kN6, /*!< The N of SIEBEN, column 10 of row 5. */
        kK2, /*!< The filler K, column 0 of row 6. */
        kD2, /*!< The D of DREI, column 1 of row 6. */
        kR4, /*!< The R of DREI, column 2 of row 6. */
        kE9, /*!< The E of DREI, column 3 of row 6. */
        kI7, /*!< The I of DREI, column 4 of row 6. */
        kR5, /*!< The filler R, column 5 of row 6. */
        kH4, /*!< The filler H, column 6 of row 6. */
        kF4, /*!< The F of FÜNF, column 7 of row 6. */
        kUe2, /*!< The Ü of FÜNF, column 8 of row 6. */
        kN7, /*!< The N of FÜNF, column 9 of row 6. */
        kF5, /*!< The F of FÜNF, column 10 of row 6. */
        kE10, /*!< The E of ELF, column 0 of row 7. */
        kL5, /*!< The L of ELF, column 1 of row 7. */
        kF6, /*!< The F of ELF, column 2 of row 7. */
        kN8, /*!< The N of NEUN, column 3 of row 7. */
        kE11, /*!< The E of NEUN, column 4 of row 7. */
        kU3, /*!< The U of NEUN, column 5 of row 7. */
        kN9, /*!< The N of NEUN, column 6 of row 7. */
        kV3, /*!< The V of VIER, column 7 of row 7. */
        kI8, /*!< The I of VIER, column 8 of row 7. */
        kE12, /*!< The E of VIER, column 9 of row 7. */
        kR6, /*!< The R of VIER, column 10 of row 7. */
        kW4, /*!< The filler W, column 0 of row 8. */
        kA4, /*!< The A of ACHT, column 1 of row 8. */
        kC2, /*!< The C of ACHT, column 2 of row 8. */
        kH5, /*!< The H of ACHT, column 3 of row 8. */
        kT5, /*!< The T of ACHT, column 4 of row 8. */
        kZ6, /*!< The Z of ZEHN, column 5 of row 8. */
        kE13, /*!< The E of ZEHN, column 6 of row 8. */
        kH6, /*!< The H of ZEHN, column 7 of row 8. */
        kN10, /*!< The N of ZEHN, column 8 of row 8. */
        kR7, /*!< The filler R, column 9 of row 8. */
        kS4, /*!< The filler S, column 10 of row 8. */
        kB3, /*!< The filler B, column 0 of row 9. */
        kS5, /*!< The S of SECHS, column 1 of row 9. */
        kE14, /*!< The E of SECHS, column 2 of row 9. */
        kC3, /*!< The C of SECHS, column 3 of row 9. */
        kH7, /*!< The H of SECHS, column 4 of row 9. */
        kS6, /*!< The S of SECHS, column 5 of row 9. */
        kF7, /*!< The filler F, column 6 of row 9. */
        kM2, /*!< The filler M, column 7 of row 9. */
        kU4, /*!< The U of UHR, column 8 of row 9. */
        kH8, /*!< The H of UHR, column 9 of row 9. */
        kR8, /*!< The R of UHR, column 10 of row 9. */
        kNumberOfCharacters /*!< The number of letters; not a letter. */
    };

    /*! \brief The number of letters of the grid, DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS. */
    static constexpr byte kNumberOfCharacters{static_cast<byte>(CharacterIdType::kNumberOfCharacters)};
    static_assert(kNumberOfCharacters == DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS,
                  "DisplayCharacters: one character id per letter of the plate");

    /*! \brief A letter of the table: one byte, Latin-1 for the umlauts. */
    using DisplayCharactersTableElementType = char;

  private:
    static const char kDisplayCharactersTable[][DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS + 1u];

    /*
     * The unchecked reads: the public readers check their arguments and then call these.
     *
     * Row first, because that is how the table is written - one string per row. It used
     * to be indexed the other way round, which every caller then compensated for by
     * swapping its arguments; the two that did not compensate read past the end of the
     * table for the last column. Nothing called any of them, so nothing showed it.
     */
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

  public:
    /*! \brief Makes the letters, which hold no state. */
    constexpr DisplayCharacters() { }

    /*! \brief Destroys the letters. */
    ~DisplayCharacters() = default;

    /*!
     * \brief Returns the letter at `column` and `row`.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \return The letter, or STD_NULL_CHARACTER if `column` or `row` lies off the grid.
     */
    static char GetCharacter(byte column, byte row) {
        if(!IsColumnAndRowValid(column, row)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(column, row);
    }

    /*!
     * \brief Returns the letter at `index`.
     *
     * \param[in] index The index of the letter, counted from 0 row by row.
     * \return The letter, or STD_NULL_CHARACTER if `index` lies off the grid.
     */
    static char GetCharacter(byte index) {
        if(!IsIndexValid(index)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(index);
    }

    /*!
     * \brief Returns the letter `character_id` names.
     *
     * \param[in] character_id The id of the letter.
     * \return The letter, or STD_NULL_CHARACTER if `character_id` is not a letter, such as
     *         `kNumberOfCharacters`.
     */
    static char GetCharacter(CharacterIdType character_id) {
        if(!IsCharacterIdValid(character_id)) { return STD_NULL_CHARACTER; }
        return GetDisplayCharactersTableElement(ToUnderlying(character_id));
    }

    /*!
     * \brief Returns in `character` the letter at `column` and `row`.
     *
     * \param[in] column The column of the grid, counted from 0.
     * \param[in] row The row of the grid, counted from 0.
     * \param[out] character The letter.
     * \return kEOk, or kENotOk if `column` or `row` lies off the grid; `character` is then left as
     *         it was.
     */
    static StdReturnType GetCharacter(byte column, byte row, char& character);

    /*!
     * \brief Returns in `character` the letter at `index`.
     *
     * \param[in] index The index of the letter, counted from 0 row by row.
     * \param[out] character The letter.
     * \return kEOk, or kENotOk if `index` lies off the grid; `character` is then left as it was.
     */
    static StdReturnType GetCharacter(byte index, char& character);

    /*!
     * \brief Returns in `character` the letter `character_id` names.
     *
     * \param[in] character_id The id of the letter.
     * \param[out] character The letter.
     * \return kEOk, or kENotOk if `character_id` is not a letter, such as `kNumberOfCharacters`;
     *         `character` is then left as it was.
     */
    static StdReturnType GetCharacter(CharacterIdType character_id, char& character);

    /*!
     * \brief Returns whether `character_id` names a letter of the grid.
     *
     * \param[in] character_id The id.
     * \return True if it lies below `kNumberOfCharacters`.
     */
    static bool IsCharacterIdValid(CharacterIdType character_id) { return character_id < CharacterIdType::kNumberOfCharacters; };

    /*!
     * \brief Returns whether `index` lies on the grid.
     *
     * \param[in] index The index.
     * \return True if it lies below DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS.
     */
    static bool IsIndexValid(byte index) { return index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; };

    /*!
     * \brief Returns whether `column` and `row` lie on the grid.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] row The row, counted from 0.
     * \return True if `column` lies below DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS and `row` below
     *         DISPLAY_CHARACTERS_NUMBER_OF_ROWS.
     */
    static constexpr bool IsColumnAndRowValid(byte column, byte row) {
        return (column < DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS) && (row < DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    }

    /*!
     * \brief Returns in `column` and `row` where the letter at `index` stands.
     *
     * `index` is not checked: an index off the grid gives a row off the grid.
     *
     * \param[in] index The index of the letter, counted from 0 row by row.
     * \param[out] column The column, counted from 0.
     * \param[out] row The row, counted from 0.
     * \note Column before row, matching both the name and Display's function of the same name. They
     *       were the other way round here, so a caller that read the two classes side by side got a
     *       silent swap.
     */
    static void IndexToColumnAndRow(byte index, byte& column, byte& row) { column = index % DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS; row = index / DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS; }

    /*!
     * \brief Returns the index of the letter at `column` and `row`.
     *
     * Neither is checked: a column off the grid gives the index of a letter in a later row.
     *
     * \param[in] column The column, counted from 0.
     * \param[in] row The row, counted from 0.
     * \return `row` times DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS plus `column`.
     */
    static byte ColumnAndRowToIndex(byte column, byte row) { return static_cast<byte>((row * DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS) + column); }

};

#endif
