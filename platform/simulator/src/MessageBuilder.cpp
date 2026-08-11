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
/**     \file       MessageBuilder.cpp
 *      \brief      Puts a serial message together from a command and its options
 *
 *      \details
 *
******************************************************************************************************************************************************/
#define _MESSAGE_BUILDER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/MessageBuilder.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
BEGIN_EVENT_TABLE(MessageBuilder, wxDialog)
    EVT_CLOSE(MessageBuilder::OnClose)
    EVT_CHOICE(ID_CHOICE_COMMAND, MessageBuilder::OnCommand)
    EVT_BUTTON(ID_BUTTON_INSERT, MessageBuilder::OnInsert)
    /* All option rows share one handler; the preview does not care which of them moved. */
    EVT_COMMAND_RANGE(ID_CHECKBOX_OPTION_FIRST, ID_CHECKBOX_OPTION_LAST, wxEVT_CHECKBOX, MessageBuilder::OnOptionUsed)
    EVT_COMMAND_RANGE(ID_TEXT_OPTION_FIRST, ID_TEXT_OPTION_LAST, wxEVT_TEXT, MessageBuilder::OnOptionChanged)
    EVT_COMMAND_RANGE(ID_CHOICE_OPTION_FIRST, ID_CHOICE_OPTION_LAST, wxEVT_CHOICE, MessageBuilder::OnOptionChanged)
END_EVENT_TABLE()


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of MessageBuilder
******************************************************************************************************************************************************/
MessageBuilder::MessageBuilder(wxWindow* parent, const wxString &title) : wxDialog(parent, wxID_ANY, title)
{
    /* Not SetSizerAndFit(): that also pins the minimum size to the height fitted here,
       and the window then cannot shrink again when a command with fewer options is
       picked. showOptionsOfSelectedCommand() sizes it instead, on every change. */
    SetSizer(createSizerAll(this));

    showOptionsOfSelectedCommand();
    CentreOnParent();
}


/******************************************************************************************************************************************************
  Destructor of MessageBuilder
******************************************************************************************************************************************************/
MessageBuilder::~MessageBuilder()
{

} /* ~MessageBuilder */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  createSizerAll()
******************************************************************************************************************************************************/
wxBoxSizer* MessageBuilder::createSizerAll(wxWindow* Parent)
{
    wxStaticBox* StaticBox = new wxStaticBox(Parent, ID_STATIC_BOX_MESSAGE, _T("Command"));
    wxStaticBoxSizer* SizerAll = new wxStaticBoxSizer(StaticBox, wxVERTICAL);

    CommandChoice = new wxChoice(Parent, ID_CHOICE_COMMAND);
    for(byte Index = 0u; Index < MessageCatalog::getNumberOfCommands(); Index++) {
        const MessageCatalog::CommandType& Command = MessageCatalog::getCommand(Index);
        CommandChoice->Append(wxString::Format(wxT("%u  %s"), Command.Number, Command.Label));
    }
    CommandChoice->SetSelection(0);

    SizerAll->Add(CommandChoice, 0, wxALL | wxEXPAND, Border);

    /* Every row is built once, for the command with the longest option list, and later
       only shown or hidden. Creating and destroying them per selection would be the same
       widgets over and over, with more to go wrong. */
    for(byte Index = 0u; Index < MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS; Index++) {
        wxBoxSizer* SizerOption = new wxBoxSizer(wxHORIZONTAL);

        OptionRows[Index].Use = new wxCheckBox(Parent, ID_CHECKBOX_OPTION_FIRST + Index, _T(""));
        OptionRows[Index].Value = new wxTextCtrl(Parent, ID_TEXT_OPTION_FIRST + Index, _T(""),
                                                 wxDefaultPosition, wxSize(ValueWidth, -1));
        OptionRows[Index].Choice = new wxChoice(Parent, ID_CHOICE_OPTION_FIRST + Index,
                                                wxDefaultPosition, wxSize(ChoiceWidth, -1));

        SizerOption->Add(OptionRows[Index].Use, 1, wxALIGN_CENTER_VERTICAL);
        SizerOption->Add(OptionRows[Index].Value, 0, wxALIGN_CENTER_VERTICAL);
        SizerOption->Add(OptionRows[Index].Choice, 0, wxALIGN_CENTER_VERTICAL);

        SizerAll->Add(SizerOption, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, Border);
    }

    wxStaticText* PreviewLabel = new wxStaticText(Parent, wxID_ANY, _T("Message"));
    Preview = new wxTextCtrl(Parent, ID_TEXT_CTRL_PREVIEW, _T(""), wxDefaultPosition,
                             wxSize(PreviewWidth, -1), wxTE_READONLY);
    wxButton* Insert = new wxButton(Parent, ID_BUTTON_INSERT, wxT("&Insert"));

    SizerAll->Add(PreviewLabel, 0, wxLEFT | wxRIGHT, Border);
    SizerAll->Add(Preview, 0, wxALL | wxEXPAND, Border);
    SizerAll->Add(Insert, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, Border);

    return SizerAll;
} /* createSizerAll */


/******************************************************************************************************************************************************
  showOptionsOfSelectedCommand()
******************************************************************************************************************************************************/
/*! \brief          brings the option rows in line with the selected command
 *  \details        Rows beyond the command's option list are hidden. The ones in use are
 *                  reset rather than left as they were, so an argument typed for a
 *                  different command cannot be carried over into this one under a short
 *                  name that happens to sit at the same position.
 *
 *  \return         -
******************************************************************************************************************************************************/
void MessageBuilder::showOptionsOfSelectedCommand()
{
    const MessageCatalog::CommandType& Command = getSelectedCommand();

    for(byte Index = 0u; Index < MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS; Index++) {
        const bool Used = Index < Command.NumberOfOptions;

        bool Named = false;

        if(Used) {
            const MessageCatalog::OptionType& Option = Command.Options[Index];
            Named = Option.ValueNames != nullptr;

            OptionRows[Index].Use->SetLabel(wxString::Format(wxT("-%c  %s"), Option.ShortName, Option.Label));
            OptionRows[Index].Use->SetValue(false);

            if(Named) {
                OptionRows[Index].Choice->Clear();
                for(byte Name = 0u; Name < Option.NumberOfValueNames; Name++) {
                    OptionRows[Index].Choice->Append(wxString::Format(wxT("%u  %s"),
                                                                      Option.Minimum + Name,
                                                                      Option.ValueNames[Name]));
                }
                OptionRows[Index].Choice->SetSelection(0);
            } else {
                OptionRows[Index].Value->ChangeValue(_T(""));
                if(Option.Argument == MessageCatalog::ARGUMENT_TYPE_STRING) {
                    OptionRows[Index].Value->SetHint(_T("text"));
                } else {
                    OptionRows[Index].Value->SetHint(wxString::Format(wxT("%u..%u"), Option.Minimum, Option.Maximum));
                }
            }
        }

        OptionRows[Index].Use->Show(Used);
        OptionRows[Index].Value->Show(Used && !Named);
        OptionRows[Index].Choice->Show(Used && Named);
        /* Every row starts unticked, so every argument starts greyed out. */
        enableOptionValue(Index, false);
    }

    /* Resizes the window to the rows that are left and moves its minimum along, so it
       follows the content in both directions. Leaving it at the height of the longest
       option list instead put a command with one option above 360 pixels of nothing. */
    Layout();
    GetSizer()->SetSizeHints(this);
    updatePreview();
} /* showOptionsOfSelectedCommand */


/******************************************************************************************************************************************************
  buildMessage()
******************************************************************************************************************************************************/
/*! \brief          assembles the message from the selected command and its ticked options
 *  \details        Same shape the parsers expect: the command number, then one
 *                  "-<short name><argument>" per option. A command on its own is a
 *                  request for the current settings, so no option needs to be ticked.
 *
 *  \return         the message, without the newline the Serial shim adds when it is sent
******************************************************************************************************************************************************/
wxString MessageBuilder::buildMessage() const
{
    const MessageCatalog::CommandType& Command = getSelectedCommand();
    wxString Message = wxString::Format(wxT("%u"), Command.Number);

    for(byte Index = 0u; Index < Command.NumberOfOptions; Index++) {
        if(!OptionRows[Index].Use->IsChecked()) { continue; }

        const MessageCatalog::OptionType& Option = Command.Options[Index];
        wxString Argument;

        if(Option.ValueNames != nullptr) {
            /* The names run upwards from Minimum, so the position in the dropdown is the
               offset from it — what goes on the wire stays the number. */
            Argument = wxString::Format(wxT("%u"), Option.Minimum + OptionRows[Index].Choice->GetSelection());
        } else {
            Argument = OptionRows[Index].Value->GetValue();
        }

        Message += wxString::Format(wxT(" -%c%s"), Option.ShortName, Argument);
    }

    return Message;
} /* buildMessage */


/******************************************************************************************************************************************************
  OnCommand()
******************************************************************************************************************************************************/
void MessageBuilder::OnCommand(wxCommandEvent &event)
{
    showOptionsOfSelectedCommand();
    UNUSED(event);
} /* OnCommand */


/******************************************************************************************************************************************************
  OnOptionUsed()
******************************************************************************************************************************************************/
/*! \brief          follows a ticked or unticked option
 *  \details        The row is taken from the event's id, which is what the contiguous
 *                  ranges of the option widgets are for.
 *
 *  \return         -
******************************************************************************************************************************************************/
void MessageBuilder::OnOptionUsed(wxCommandEvent &event)
{
    const byte Index = static_cast<byte>(event.GetId() - ID_CHECKBOX_OPTION_FIRST);

    enableOptionValue(Index, OptionRows[Index].Use->IsChecked());
    updatePreview();
} /* OnOptionUsed */


/******************************************************************************************************************************************************
  OnOptionChanged()
******************************************************************************************************************************************************/
void MessageBuilder::OnOptionChanged(wxCommandEvent &event)
{
    updatePreview();
    UNUSED(event);
} /* OnOptionChanged */


/******************************************************************************************************************************************************
  OnInsert()
******************************************************************************************************************************************************/
/*! \brief          hands the message to the input field, without sending it
 *  \details        Deliberately not sent: it can be corrected or extended by hand there
 *                  first, and Send stays the one place a message leaves for the firmware.
 *                  The window closes with it, since what there was to do here is done —
 *                  hidden rather than destroyed, so the next Create brings back the
 *                  command that was selected.
 *
 *  \return         -
******************************************************************************************************************************************************/
void MessageBuilder::OnInsert(wxCommandEvent &event)
{
    Pixels::getInstance().setInput(buildMessage());
    Hide();
    UNUSED(event);
} /* OnInsert */


/******************************************************************************************************************************************************
  OnClose()
******************************************************************************************************************************************************/
void MessageBuilder::OnClose(wxCloseEvent &event)
{
    /* Hide rather than destroy: getInstance() keeps handing out this very dialog, so a
       destroyed one would leave the singleton pointing at freed memory. The parent
       window destroys it when the application exits. */
    Hide();
    UNUSED(event);
} /* OnClose */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
