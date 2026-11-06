// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How a line of the serial port becomes a command.
 */

#define _COMMUNICATION_SOURCE_

#include "Communication.h"
#include "StandardTypes.h"
#include "MsgCmdParser.h"

void Communication::Task()
{
    if(IsMessageComplete()) {
        /*
         * A line that did not fit is refused whole, with one error. It used to be answered with
         * an error per character dropped and then run cut short: 7 -T and fifty letters set the
         * overlay text to its first forty-five.
         */
        if(is_message_too_long_) {
            // Outside the command parser's line framing, so it is a line of its own.
            error_.Send(ErrorMessage::ErrorType::kMessageTooLong, false);
            Serial.println();
        } else {
            MsgCmdParser cmd_parser(incoming_message_);
            cmd_parser.Parse();
        }
        incoming_message_.Clear();
        is_message_too_long_ = false;
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
        if(incoming_message_.AddChar(in_char) == kENotOk) { is_message_too_long_ = true; }
    }
}
