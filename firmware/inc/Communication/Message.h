// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief A line from the serial port: a fixed buffer to add characters to and search.
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

#define MESSAGE_LENGTH    50u /*!< The size of a line's buffer, its terminator included. */

/*!
 * \brief A line from the serial port: a buffer of MESSAGE_LENGTH characters that holds a terminated
 *        string of up to MESSAGE_LENGTH - 1 of them.
 *
 * AddChar() appends one character at a time and refuses one once the buffer is full. Find() and
 * Contains() search the line, and GetMessage() hands it out as a string.
 */
class Message
{
  public:
      static constexpr size_t kMessageLength{MESSAGE_LENGTH}; /*!< The size of the buffer. */
      static constexpr size_t npos{static_cast<size_t>(-1)}; /*!< What Find() returns for no finding, SIZE_MAX. */
      static constexpr char kNullCharacter{STD_NULL_CHARACTER}; /*!< The terminator. */

  private:
    char buffer_[kMessageLength];

  public:
    /*! \brief Makes an empty line. */
    Message() { Clear(); }
    /*! \brief Destroys the line. */
    ~Message() = default;

    /*! \brief Returns the line, a terminated string. */
    const char* GetMessage() const { return buffer_; }

    /*! \brief Empties the line. */
    void Clear() { buffer_[0u] = kNullCharacter; }
    /*! \brief Returns whether the line is empty. */
    bool IsEmpty() const { return buffer_[0u] == kNullCharacter; }

    /*! \brief Returns the number of characters of the line, without its terminator. */
    size_t Length() const
    {
        return strlen(buffer_);
    }

    /*!
     * \brief Returns where `string` first occurs in the line, searched from `position` on.
     *
     * \param[in] string The string to find, terminated.
     * \param[in] position Where the search starts, counted from 0.
     * \return The position of the finding's first character, counted from the start of the line, or
     *         npos if there is none.
     * \pre `position` is at most Length(); it is not checked.
     */
    size_t Find(const char* string, size_t position = 0u) const
    {
        const char* finding = strstr(&buffer_[position], string);
        if(finding == nullptr) { return npos; }
        else { return static_cast<size_t>(finding - buffer_); }
    }

    /*!
     * \brief Returns whether `character` occurs in the line.
     *
     * \param[in] character The character to find.
     * \return True if it does. Always true for kNullCharacter, as the terminator is found.
     */
    bool Contains(char character) const { return strchr(buffer_, character); }

    /*!
     * \brief Returns where `character` first occurs in the line, searched from `position` on.
     *
     * \param[in] character The character to find.
     * \param[in] position Where the search starts, counted from 0.
     * \return The position of the finding, counted from the start of the line, or npos if there is
     *         none.
     * \pre `position` is at most Length(); it is not checked.
     */
    size_t Find(char character, size_t position = 0u) const
    {
        const char* finding = strchr(&buffer_[position], character);
        if(finding == nullptr) { return npos; }
        else { return static_cast<size_t>(finding - buffer_); }
    }

    /*!
     * \brief Appends `character` to the line.
     *
     * Any character is taken. A kNullCharacter ends the line where it is, so it adds nothing.
     *
     * \param[in] character The character.
     * \return kEOk, or kENotOk if the line already holds MESSAGE_LENGTH - 1 characters; the line is
     *         then left as it was.
     */
    StdReturnType AddChar(char character)
    {
        const size_t message_length{strlen(buffer_)};
        // check for free space in message buffer
        if(message_length < (MESSAGE_LENGTH - 1u))
        {
            // save new char
            buffer_[message_length] = character;
            buffer_[message_length + 1u] = kNullCharacter;
            return kEOk;
        } else {
            return kENotOk;
        }
    }
};

#endif
