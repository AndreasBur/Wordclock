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

/*
 * Closes the line the firmware has been printing and reads it back. An answer gets its
 * readable form underneath, indented to show it belongs to the line above; the decoder brings
 * its own layout. The raw line stays: it is what actually went over the wire, which is what
 * matters when the protocol itself is in doubt. Anything the decoder does not recognise, such
 * as plain text, passes through untouched.
 */
void SerialShim::FinishOutputLine()
{
    /*
     * The browsers first, and outside the attached check: they are watching whether or not a
     * window has handed its controls over, and an answer that reached one front end and not
     * the other is the confusing half. AppendOutput() collects nothing before the controls
     * are attached, though, so until then the line handed over is empty.
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
