// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How a line of the serial port becomes a command.
 */

#define _COMMUNICATION_SOURCE_

#include "Communication.h"
#include "MsgCmdParser.h"

void Communication::Task()
{
    if(IsMessageComplete()) {
        MsgCmdParser cmd_parser(incoming_message_);
        cmd_parser.Parse();
        incoming_message_.Clear();
        state_ = StateType::kMessageIncomplete;
    } else {
        AddMessagePart();
    }
}

void Communication::AddMessagePart()
{
    while (Serial.available()) {
        // get the new byte from uart
        char in_char = static_cast<char>(Serial.read());
        // The end-of-message marker terminates the message but is not part of
        // it, so stop before appending it to the buffer.
        if(in_char == kEndOfMessageChar) { state_ = StateType::kMessageComplete; break; }
        if(incoming_message_.AddChar(in_char) == kENotOk) {
            // Emitted while reading, outside the command parser's line framing,
            // so terminate it as its own line without a trailing separator space.
            error_.Send(ErrorMessage::ErrorType::kMessageTooLong, false);
            Serial.println();
        }
    }
}
