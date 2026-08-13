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
/**     \file       MessageBuilder.h
 *      \brief      Puts a serial message together from a command and its options
 *
 *      \details    Offers the commands the firmware understands by name, with an
 *                  option row each, and writes the result into the pixel matrix
 *                  window's input field. It does not send: that stays with the Send
 *                  button there, so a built message can still be edited by hand
 *                  first.
 *
 *                  Named MessageBuilder rather than Message, which the firmware
 *                  already uses for an incoming message.
 *
******************************************************************************************************************************************************/
#ifndef MESSAGE_BUILDER_H
#define MESSAGE_BUILDER_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include <array>
#include "StandardTypes.h"
#include "PixelsFrame.h"
#include "MessageCatalog.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M E S S A G E   B U I L D E R
******************************************************************************************************************************************************/
/* A dialog rather than a frame, so the compositor centres it over the main window.
   Shown with Show(), not ShowModal(), so the input field it writes to stays reachable. */
class MessageBuilder : public wxDialog
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Own range, clear of the other windows'. The option rows get contiguous ranges of
       their own, which is what lets one handler serve all of them. */
    enum
    {
        ID_STATIC_BOX_MESSAGE = 3000,
        ID_CHOICE_COMMAND,
        ID_TEXT_CTRL_PREVIEW,
        ID_BUTTON_INSERT,

        ID_CHECKBOX_OPTION_FIRST = 3100,
        ID_CHECKBOX_OPTION_LAST = ID_CHECKBOX_OPTION_FIRST + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1,

        ID_TEXT_OPTION_FIRST = 3200,
        ID_TEXT_OPTION_LAST = ID_TEXT_OPTION_FIRST + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1,

        ID_CHOICE_OPTION_FIRST = 3300,
        ID_CHOICE_OPTION_LAST = ID_CHOICE_OPTION_FIRST + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1
    };

    /* One row per option of the selected command: whether it is part of the message at
       all, and with which argument. Both argument widgets exist in every row and only
       the one the option calls for is shown — a dropdown where the catalog names the
       values, a text field for a plain number or a string. Building them once and
       hiding one keeps the rows reusable across commands. */
    struct OptionRowType {
        wxCheckBox* Use;
        wxTextCtrl* Value;
        wxChoice* Choice;
    };

    wxChoice* CommandChoice;
    std::array<OptionRowType, MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS> OptionRows;
    wxTextCtrl* Preview;

    static constexpr int ValueWidth{80};
    /* Wider than the plain value field: it has to hold names like "Illuminance
       calibration maximum". */
    static constexpr int ChoiceWidth{240};
    static constexpr int PreviewWidth{300};
    static constexpr int Border{10};

    DECLARE_EVENT_TABLE()

    // functions
    MessageBuilder(wxWindow*, const wxString&);
    ~MessageBuilder();

    void OnClose(wxCloseEvent&);
    void OnCommand(wxCommandEvent&);
    void OnOptionUsed(wxCommandEvent&);
    void OnOptionChanged(wxCommandEvent&);
    void OnInsert(wxCommandEvent&);

    /* An option that is not ticked contributes nothing to the message, so its argument is
       greyed out rather than left looking as though it did. */
    void enableOptionValue(byte Index, bool Enabled) {
        OptionRows[Index].Value->Enable(Enabled);
        OptionRows[Index].Choice->Enable(Enabled);
    }

    wxBoxSizer* createSizerAll(wxWindow*);

    const MessageCatalog::CommandType& getSelectedCommand() const {
        return MessageCatalog::getCommand(static_cast<byte>(CommandChoice->GetSelection()));
    }

    void showOptionsOfSelectedCommand();
    wxString buildMessage() const;
    void updatePreview() { Preview->SetValue(buildMessage()); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    /* Parented to the pixel matrix window, so closing that one takes this along and the
       application can exit. */
    static MessageBuilder& getInstance() {
        static MessageBuilder* pSingletonInstance = new MessageBuilder(&PixelsFrame::getInstance(), _("Wordclock Message"));
        return *pSingletonInstance;
    }

    // methods
    void reveal() {
        Show();
        Raise();
        /* Again once the window is on screen. A size set while it was still hidden did
           not always reach the window manager under XWayland, which left the frame at
           the height of a different command's option list. */
        GetSizer()->SetSizeHints(this);
    }
};

#endif // MESSAGE_BUILDER_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
