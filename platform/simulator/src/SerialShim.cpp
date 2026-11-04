// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Stands in for the Arduino Serial port.
 */

#define _SERIAL_SHIM_SOURCE_

#include "sim/SerialShim.h"
/* reads the firmware's answers back into readable lines */
#include "sim/MessageDecoder.h"

/*! \brief          hands the firmware the next character that was typed
 *  \details        Takes it off the front, the way a real port does, so a line arrives
 *                  over as many calls as it has characters.
 *
 *  \return         the character, or a space if there is nothing to read
 */
char SerialShim::read()
{
    char first_char{' '};

    if(!send_buffer_.IsEmpty()) {
        // The firmware's port is a byte wide, and what is typed into the console is
        // ASCII; the cast says that rather than leaving it to the conversion.
        first_char = static_cast<char>(send_buffer_.at(0).GetValue());
        send_buffer_.Remove(0, 1);
    }

    return first_char;
}

/*! \brief          hands the line in the input field over to be read
 *  \details        Only while nothing is waiting: read() drains a line one character at a
 *                  time, and a second one written over it would be lost.
 */
void SerialShim::SendInput()
{
    if(!IsAttached()) { return; }

    if(send_buffer_.IsEmpty()) {
        send_buffer_ = input_->GetValue() + _T("\n");
    }
}

void SerialShim::AppendOutput(const wxString& text)
{
    if(!IsAttached()) { return; }

    output_->AppendText(text);
    output_line_ += text;
}

/*! \brief          closes the line the firmware has been printing and reads it back
 *  \details        An answer gets its readable form underneath, indented to show it
 *                  belongs to the line above; the decoder brings its own layout. The raw
 *                  line stays: it is what actually went over the wire, which is what
 *                  matters when the protocol itself is in doubt. Anything the decoder does
 *                  not recognise — an error line, or plain text — passes through untouched.
 */
void SerialShim::FinishOutputLine()
{
    /*
     * The browsers first, and outside the attached check: they are watching whether or not a
     * window has handed its controls over, and an answer that reached one front end and not
     * the other is the confusing half.
     */
    if(line_sink_ != nullptr) { line_sink_(output_line_.utf8_str()); }

    if(IsAttached()) {
        output_->AppendText(_T("\n"));
        output_->AppendText(MessageDecoder::Describe(output_line_));
    }

    /*
     * Cleared whether or not anything displayed it. It used to be cleared only on the way
     * past the controls, so a run with none would have grown one line for the length of it -
     * harmless while nothing else read the line, and not once a socket does.
     */
    output_line_.Clear();
}
