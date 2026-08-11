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
    wxString SendBuffer{""};
    /* What the firmware has printed since the last newline. Kept so a whole answer can be
       read back into readable form once it is complete, which single characters do not
       allow. */
    wxString OutputLine{""};

    wxTextCtrl* Output{nullptr};
    wxTextCtrl* Input{nullptr};

    // functions
    /* Not constexpr, unlike the firmware's singletons: wxString has no constexpr
       constructor. */
    SerialShim() { }
    ~SerialShim() { }

    /* Nothing to print into before the window has handed its controls over. Can only
       happen if the firmware runs before the window is built, which it does not, but a
       null pointer is not worth a crash. */
    bool isAttached() const { return (Output != nullptr) && (Input != nullptr); }

    void appendOutput(const wxString&);
    void finishOutputLine();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static SerialShim& getInstance() {
        static SerialShim SingletonInstance;
        return SingletonInstance;
    }

    // set methods
    /* Puts a line into the input field, ready to be looked over and sent from there. What
       the message builder uses; it deliberately does not send by itself. */
    void setInput(const wxString& Line) {
        if(isAttached()) { Input->SetValue(Line); }
    }

    // methods
    void attach(wxTextCtrl* sOutput, wxTextCtrl* sInput) {
        Output = sOutput;
        Input = sInput;
    }

    /* The Arduino side */
    void println() { finishOutputLine(); }
    void print(const char* Text) { appendOutput(Text); }
    void print(int Number) { appendOutput(wxString::Format(wxT("%i"), Number)); }
    void println(const char* Text) { print(Text); println(); }
    void println(int Number) { print(Number); println(); }
    /* A newline that arrives as a character has to close the line the same way, otherwise
       the line an answer is read back from would run on across it. */
    void print(char Char) {
        if(Char == '\n') { println(); }
        else { appendOutput(wxString(Char)); }
    }

    bool available() const { return !SendBuffer.IsEmpty(); }
    char read();

    /* The window side */
    void sendInput();
    void clearOutput() {
        if(isAttached()) { Output->Clear(); }
    }
};

#endif // SERIAL_SHIM_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
