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

/* Message configuration parameter */
#define MESSAGE_LENGTH    50u

/* Message parameter */

class Message
{
  public:
      static constexpr size_t kMessageLength{MESSAGE_LENGTH};
      static constexpr size_t npos = -1;
      static constexpr char kNullCharacter{STD_NULL_CHARACTER};

  private:
    char buffer_[kMessageLength];

  public:
    Message() { Clear(); }
    ~Message() { }

    // get methods
    const char* GetMessage() const { return buffer_; }

    // set methods

    // methods
    void Clear() { buffer_[0u] = kNullCharacter; }
    bool IsEmpty() const { return buffer_[0u] == kNullCharacter; }

    size_t Length() const
    {
        return strlen(buffer_);
    }

    size_t Find(const char* string, size_t position = 0u) const
    {
        const char* finding = strstr(&buffer_[position], string);
        if(finding == nullptr) { return npos; }
        else { return finding - buffer_; }
    }

    bool Contains(char character) const { return strchr(buffer_, character); }

    size_t Find(char character, size_t position = 0u) const
    {
        const char* finding = strchr(&buffer_[position], character);
        if(finding == nullptr) { return npos; }
        else { return finding - buffer_; }
    }

    StdReturnType AddChar(char character)
    {
        byte message_length = strlen(buffer_);
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
