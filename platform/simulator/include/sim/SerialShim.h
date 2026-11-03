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
/**     \file       SerialShim.h
 *      \brief      Stands in for the Arduino Serial port
 *
 *      \details    What the firmware prints ends up in a text control, and what is
 *                  typed into another one is handed back to it character by character,
 *                  the way a real port delivers it. Arduino's Serial is #defined to
 *                  this instance.
 *
 *                  The two controls belong to the pixel matrix window, which lays them
 *                  out and passes them here with attach(). Splitting it that way keeps
 *                  the port's behaviour — the buffering, the line the answers are read
 *                  back from — out of a class whose subject is the letter grid.
 *
******************************************************************************************************************************************************/
#ifndef SERIAL_SHIM_H
#define SERIAL_SHIM_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include "arduino/Types.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   S E R I A L   S H I M
******************************************************************************************************************************************************/
class SerialShim
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* What was typed and not yet read, drained one character at a time by read(). */
    wxString send_buffer_{""};
    /* What the firmware has printed since the last newline. Kept so a whole answer can be
       read back into readable form once it is complete, which single characters do not
       allow. */
    wxString output_line_{""};

    wxTextCtrl* output_{nullptr};
    wxTextCtrl* input_{nullptr};

    /* Nothing until the web front end installs itself, which is a clock with no browsers
       watching and the case the simulator ran in until now. */
    void (*line_sink_)(const char*){nullptr};

    // functions
    /* Not constexpr, unlike the firmware's singletons: wxString has no constexpr
       constructor. */
    SerialShim() { }
    ~SerialShim() { }

    /* Nothing to print into before the window has handed its controls over. Can only
       happen if the firmware runs before the window is built, which it does not, but a
       null pointer is not worth a crash. */
    bool IsAttached() const { return (output_ != nullptr) && (input_ != nullptr); }

    void AppendOutput(const wxString&);
    void FinishOutputLine();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static SerialShim& GetInstance() {
        static SerialShim singleton_instance;
        return singleton_instance;
    }

    // set methods
    /* Puts a line into the input field, ready to be looked over and sent from there. What
       the message builder uses; it deliberately does not send by itself. */
    void SetInput(const wxString& line) {
        if(IsAttached()) { input_->SetValue(line); }
    }

    // methods
    void Attach(wxTextCtrl* output, wxTextCtrl* input) {
        output_ = output;
        input_ = input;
    }

    /* The Arduino side, under the names Arduino's Serial gives it, which the NOLINTs keep:
       the firmware core is written against that API. */
    void println() { FinishOutputLine(); } // NOLINT(readability-identifier-naming)
    void print(const char* text) { AppendOutput(text); } // NOLINT(readability-identifier-naming)
    void print(int number) { AppendOutput(wxString::Format(wxT("%i"), number)); } // NOLINT(readability-identifier-naming)
    void println(const char* text) { print(text); println(); } // NOLINT(readability-identifier-naming)
    void println(int number) { print(number); println(); } // NOLINT(readability-identifier-naming)
    /* A newline that arrives as a character has to close the line the same way, otherwise
       the line an answer is read back from would run on across it. */
    void print(char character) { // NOLINT(readability-identifier-naming)
        if(character == '\n') { println(); }
        else { AppendOutput(wxString(character)); }
    }

    bool available() const { return !send_buffer_.IsEmpty(); } // NOLINT(readability-identifier-naming)
    char read(); // NOLINT(readability-identifier-naming)

    /* The second front end, the same arrangement the two networked backends have: a browser
       sends command text over a web socket and it arrives here as though it had been typed.
       Appended rather than assigned, unlike sendInput() below - what is typed can wait for
       the field to clear, but a command already on the wire has nowhere to wait.

       Called from the firmware's tick, because the simulator's server is polled from the same
       timer rather than run in a task of its own. That is what lets this be a wxString at all:
       on hardware the equivalent buffer is lock-free because a second task writes it. */
    void Inject(const char* text, size_t length) {
        send_buffer_ += wxString::FromUTF8(text, length);
    }

    /* Where a finished output line goes besides the text control. Set by the web front end so
       every answer reaches the browsers too; a plain function pointer, as on hardware. */
    using LineSinkType = void (*)(const char*);
    void SetLineSink(LineSinkType line_sink) { line_sink_ = line_sink; }

    /* The window side */
    void SendInput();
    void ClearOutput() {
        if(IsAttached()) { output_->Clear(); }
    }
};

#endif // SERIAL_SHIM_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
