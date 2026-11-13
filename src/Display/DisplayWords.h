// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The words of the clock, by id, and where each sits on the letter grid.
 */

#ifndef WORDCLOCK_SRC_DISPLAY_DISPLAYWORDS_H_
#define WORDCLOCK_SRC_DISPLAY_DISPLAYWORDS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayWord.h"

/*!
 * \brief The words of the clock: where each word stands on the letter grid, by id.
 *
 * A word is the column and the row of its first letter, counted from 0 as in DisplayCharacters,
 * and its length; it runs along its row towards the higher columns.
 *
 * Every reader comes in two forms, and the argument list says which: the one taking a reference
 * answers whether the word exists, the one taking none answers the word. Both check. A word that
 * is not in the table has no place on the plate, which is what a length of zero says, and a word
 * of zero length lights nothing.
 */
class DisplayWords
{
  public:
    /*! \brief The words of the clock, by id. */
    enum class WordIdType : byte {
        kNone, /*!< No word: its length of 0 lights nothing. */
        kEs, /*!< The word ES. */
        kIst, /*!< The word IST. */
        kFuenf, /*!< The word FÜNF of the minutes. */
        kZehn, /*!< The word ZEHN of the minutes. */
        kZwanzig, /*!< The word ZWANZIG. */
        kDrei, /*!< The word DREI of the minutes, the DREI of DREIVIERTEL. */
        kVier, /*!< The word VIER of the minutes, the VIER of VIERTEL. */
        kViertel, /*!< The word VIERTEL. */
        kDreiviertel, /*!< The word DREIVIERTEL. */
        kNach, /*!< The word NACH. */
        kVor, /*!< The word VOR. */
        kHalb, /*!< The word HALB. */
        kHourZwoelf, /*!< The hour ZWÖLF. */
        kHourZwei, /*!< The hour ZWEI. */
        kHourEin, /*!< The hour EIN, as in EIN UHR. */
        kHourEins, /*!< The hour EINS, as in ZEHN NACH EINS. */
        kHourSieben, /*!< The hour SIEBEN. */
        kHourDrei, /*!< The hour DREI. */
        kHourFuenf, /*!< The hour FÜNF. */
        kHourElf, /*!< The hour ELF. */
        kHourNeun, /*!< The hour NEUN. */
        kHourVier, /*!< The hour VIER. */
        kHourAcht, /*!< The hour ACHT. */
        kHourZehn, /*!< The hour ZEHN. */
        kHourSechs, /*!< The hour SECHS. */
        kUhr, /*!< The word UHR of the full hour. */
        kNumberOfWords /*!< The number of words; not a word. */
    };

    /*! \brief The number of words, `kNone` included. */
    static constexpr byte kNumberOfWords{static_cast<byte>(WordIdType::kNumberOfWords)};

    /*! \brief An entry of the word table: a DisplayWord. */
    using DisplayWordsTableElementType = DisplayWord;

  private:
    static const DisplayWord kDisplayWordsTable[];

    // The unchecked read. The public readers check the id and then call this.
    static DisplayWordsTableElementType GetDisplayWordsTableElement(byte word_id) {
        DisplayWordsTableElementType display_words_table_element;
        memcpy_P(&display_words_table_element, &kDisplayWordsTable[word_id], sizeof(DisplayWordsTableElementType));
        return display_words_table_element;
    }

  public:
    /*! \brief Makes the words, which hold no state. */
    constexpr DisplayWords() { }

    /*! \brief Destroys the words. */
    ~DisplayWords() = default;

    /*!
     * \brief Returns in `word` where the word `word_id` stands.
     *
     * \param[in] word_id The id of the word.
     * \param[out] word The column and the row of its first letter, and its length.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`; `word` is then
     *         left as it was.
     */
    static StdReturnType GetDisplayWord(WordIdType word_id, DisplayWord& word);

    /*!
     * \brief Returns in `length` the length of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \param[out] length The number of its letters.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`; `length` is
     *         then left as it was.
     */
    static StdReturnType GetDisplayWordLength(WordIdType word_id, byte& length);

    /*!
     * \brief Returns in `column` the column of the first letter of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \param[out] column The column, counted from 0.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`; `column` is
     *         then left as it was.
     */
    static StdReturnType GetDisplayWordColumn(WordIdType word_id, byte& column);

    /*!
     * \brief Returns in `row` the row of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \param[out] row The row, counted from 0.
     * \return kEOk, or kENotOk if `word_id` is not a word, such as `kNumberOfWords`; `row` is then
     *         left as it was.
     */
    static StdReturnType GetDisplayWordRow(WordIdType word_id, byte& row);

    /*!
     * \brief Returns where the word `word_id` stands.
     *
     * \param[in] word_id The id of the word.
     * \return The column and the row of its first letter, and its length; a word of length 0 at
     *         column 0 of row 0 if `word_id` is not a word.
     */
    static DisplayWord GetDisplayWord(WordIdType word_id) {
        if(!IsWordIdValid(word_id)) { return DisplayWord{}; }
        return GetDisplayWordsTableElement(ToUnderlying(word_id));
    }

    /*!
     * \brief Returns the row of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \return The row, counted from 0, or 0 if `word_id` is not a word.
     */
    static byte GetDisplayWordRow(WordIdType word_id) { return GetDisplayWord(word_id).GetRow(); }

    /*!
     * \brief Returns the column of the first letter of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \return The column, counted from 0, or 0 if `word_id` is not a word.
     */
    static byte GetDisplayWordColumn(WordIdType word_id) { return GetDisplayWord(word_id).GetColumn(); }

    /*!
     * \brief Returns the length of the word `word_id`.
     *
     * \param[in] word_id The id of the word.
     * \return The number of its letters, or 0 if `word_id` is not a word.
     */
    static byte GetDisplayWordLength(WordIdType word_id) { return GetDisplayWord(word_id).GetLength(); }

    /*!
     * \brief Returns whether `word_id` is a word of the table, `kNone` included.
     *
     * \param[in] word_id The id.
     * \return True if it lies below `kNumberOfWords`.
     */
    static bool IsWordIdValid(WordIdType word_id) { return word_id < WordIdType::kNumberOfWords; }
};

#endif  // WORDCLOCK_SRC_DISPLAY_DISPLAYWORDS_H_
