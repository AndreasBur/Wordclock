// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Stands in for the Arduino Serial port.
 *
 * What the firmware prints ends up in a text control, and what is typed into another one is handed
 * back to it character by character, the way a real port delivers it. Arduino's Serial is a macro
 * for this instance.
 *
 * The two controls belong to the pixel matrix window, which lays them out and passes them here with
 * Attach(). Splitting it that way keeps the port's behaviour — the buffering, the line the answers
 * are read back from — out of a class whose subject is the letter grid.
 */

#ifndef SERIAL_SHIM_H
#define SERIAL_SHIM_H

#include <wx/wx.h>
#include "arduino/Types.h"

/*!
 * \brief The serial port of the simulator: the firmware's prints go into the window's output field,
 *        and what is sent from its input field, or injected by the web front end, is read back one
 *        character at a time.
 *
 * The Arduino side carries the names Arduino's Serial gives it, which the NOLINTs keep: the
 * firmware core is written against that API. Unlike Arduino's, the print functions return nothing,
 * available() returns a bool rather than a count, and read() returns a space rather than -1 when
 * nothing is waiting.
 *
 * The window side is Attach(), SetInput(), SendInput() and ClearOutput(). Everything runs on the
 * thread of the wx event loop: the firmware's tick, the web host's poll and the window's handlers
 * all run on it.
 */
class SerialShim
{
  private:
    /* What was typed and not yet read, drained one character at a time by read(). */
    wxString send_buffer_{""};
    /*
     * What the firmware has printed since the last newline. Kept so a whole answer can be
     * read back into readable form once it is complete, which single characters do not
     * allow.
     */
    wxString output_line_{""};

    wxTextCtrl* output_{nullptr};
    wxTextCtrl* input_{nullptr};

    // Nothing until the web front end installs itself, which is a clock with no browsers
    // watching and the case the simulator ran in until now.
    void (*line_sink_)(const char*){nullptr};

    // Not constexpr, unlike the firmware's singletons: wxString has no constexpr
    // constructor.
    SerialShim() { }
    ~SerialShim() { }

    /*
     * Nothing to print into before the window has handed its controls over. Can only
     * happen if the firmware runs before the window is built, which it does not, but a
     * null pointer is not worth a crash.
     */
    bool IsAttached() const { return (output_ != nullptr) && (input_ != nullptr); }

    void AppendOutput(const wxString& text);
    void FinishOutputLine();

  public:
    /*! \brief Returns the one serial port. */
    static SerialShim& GetInstance() {
        static SerialShim singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Puts `line` into the input field, ready to be looked over and sent from there.
     *
     * What the message builder uses; it deliberately does not send by itself. Nothing happens
     * before Attach().
     *
     * \param[in] line The line, without a line end.
     */
    void SetInput(const wxString& line) {
        if(IsAttached()) { input_->SetValue(line); }
    }

    /*!
     * \brief Takes over the two text controls the window lays out for the port.
     *
     * Until both are attached, the port prints nothing, collects no output line and sends nothing
     * from the input field.
     *
     * \param[in] output The multi-line field the firmware's output is written into.
     * \param[in] input The field a line is typed into and sent from.
     */
    void Attach(wxTextCtrl* output, wxTextCtrl* input) {
        output_ = output;
        input_ = input;
    }

    /*!
     * \brief Ends the line the firmware has been printing.
     *
     * The line goes to the line sink first, if one is set, and then, if the controls are
     * attached, a line end goes into the output field, followed by the readable form
     * MessageDecoder::Describe() makes of the line. The collected line is then emptied.
     */
    void println() { FinishOutputLine(); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Prints `text`.
     *
     * The text goes into the output field and into the line that println() ends. Before Attach(),
     * it is dropped.
     *
     * \param[in] text The text, terminated.
     * \warning `text` is converted into a wxString in the encoding of the current locale. A Latin-1
     *          byte, such as an umlaut of the text overlay, does not convert under a UTF-8 locale,
     *          and then the whole of `text` is lost, in the window and on the pages alike.
     */
    void print(const char* text) { AppendOutput(text); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Prints `number` in decimal.
     *
     * \param[in] number The number.
     */
    void print(int number) { AppendOutput(wxString::Format(wxT("%i"), number)); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Prints `text` and ends the line, as print() and println() do.
     *
     * \param[in] text The text, terminated.
     */
    void println(const char* text) { print(text); println(); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Prints `number` in decimal and ends the line, as print() and println() do.
     *
     * \param[in] number The number.
     */
    void println(int number) { print(number); println(); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Prints `character`.
     *
     * A newline ends the line as println() does. A newline that arrives as a character has to
     * close the line the same way, otherwise the line an answer is read back from would run on
     * across it.
     *
     * \param[in] character The character.
     * \warning A byte above 0x7F does not convert under a UTF-8 locale and is printed as `?`.
     */
    void print(char character) { // NOLINT(readability-identifier-naming)
        if(character == '\n') { println(); }
        else { AppendOutput(wxString(character)); }
    }

    /*! \brief Returns whether a character is waiting to be read. */
    bool available() const { return !send_buffer_.IsEmpty(); } // NOLINT(readability-identifier-naming)
    /*!
     * \brief Hands the firmware the next character that was sent or injected.
     *
     * Takes it off the front, the way a real port does, so a line arrives over as many calls as it
     * has characters. A character beyond Latin-1 comes back as the low byte of its code point.
     *
     * \return The character, or a space if there is nothing to read.
     */
    char read(); // NOLINT(readability-identifier-naming)

    /*!
     * \brief Appends `text` to what read() hands out, as though it had been typed.
     *
     * The second front end, the same arrangement the two networked backends have: a browser sends
     * command text over a web socket and it arrives here as though it had been typed. Appended
     * rather than assigned, unlike SendInput() - what is typed can wait for the field to clear,
     * but a command already on the wire has nowhere to wait.
     *
     * Called from the firmware's tick, because the simulator's server is polled from the same
     * timer rather than run in a task of its own. That is what lets this be a wxString at all: on
     * hardware the equivalent buffer is lock-free because a second task writes it.
     *
     * \param[in] text The text, as UTF-8; it need not be terminated. Text that is not valid UTF-8
     *                 adds nothing.
     * \param[in] length The number of bytes of `text`.
     */
    void Inject(const char* text, size_t length) {
        send_buffer_ += wxString::FromUTF8(text, length);
    }

    /*!
     * \brief A function a finished output line is handed to, terminated and without its line end.
     *
     * A plain function pointer, as on hardware.
     */
    using LineSinkType = void (*)(const char*);
    /*!
     * \brief Sets where a finished output line goes besides the output field.
     *
     * Set by the web host, so every answer reaches the browsers too. The sink is called whether or
     * not the controls are attached, but before Attach() the line it is handed is empty, as print()
     * collects nothing until then.
     *
     * \param[in] line_sink The function, or nullptr for none.
     * \warning The sink is handed the line as UTF-8. The one the web host installs passes it on to
     *          WebFrontend::BroadcastLine(), which reads a line as Latin-1 and widens every byte
     *          above 0x7F again, so a non-ASCII character that reaches the line arrives at a
     *          browser encoded twice.
     */
    void SetLineSink(LineSinkType line_sink) { line_sink_ = line_sink; }

    /*!
     * \brief Hands the line in the input field over to be read, with a newline behind it.
     *
     * Only while nothing is waiting: read() drains a line one character at a time, and a second
     * one written over it would be lost. A line sent while one is waiting is not taken; the field
     * keeps it. Nothing happens before Attach().
     */
    void SendInput();
    /*! \brief Empties the output field; nothing happens before Attach(). */
    void ClearOutput() {
        if(IsAttached()) { output_->Clear(); }
    }
};

#endif // SERIAL_SHIM_H
