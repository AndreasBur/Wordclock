// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Puts a serial message together from a command and its options.
 */

#define _MESSAGE_BUILDER_SOURCE_

#include "sim/MessageBuilder.h"
#include "sim/SerialShim.h"

BEGIN_EVENT_TABLE(MessageBuilder, wxDialog)
    EVT_CLOSE(MessageBuilder::OnClose)
    EVT_COMBOBOX(kIdChoiceCommand, MessageBuilder::OnCommand)
    EVT_BUTTON(kIdButtonInsert, MessageBuilder::OnInsert)
    /* All option rows share one handler; the preview does not care which of them moved. */
    EVT_COMMAND_RANGE(kIdCheckboxOptionFirst, kIdCheckboxOptionLast, wxEVT_CHECKBOX, MessageBuilder::OnOptionUsed)
    EVT_COMMAND_RANGE(kIdTextOptionFirst, kIdTextOptionLast, wxEVT_TEXT, MessageBuilder::OnOptionChanged)
    EVT_COMMAND_RANGE(kIdChoiceOptionFirst, kIdChoiceOptionLast, wxEVT_COMBOBOX, MessageBuilder::OnOptionChanged)
END_EVENT_TABLE()

MessageBuilder::MessageBuilder(wxWindow* parent, const wxString &title) : wxDialog(parent, wxID_ANY, title)
{
    /*
     * Not SetSizerAndFit(): that also pins the minimum size to the height fitted here,
     * and the window then cannot shrink again when a command with fewer options is
     * picked. ShowOptionsOfSelectedCommand() sizes it instead, on every change.
     */
    SetSizer(CreateSizerAll(this));

    ShowOptionsOfSelectedCommand();
    CentreOnParent();
}

MessageBuilder::~MessageBuilder()
{
}

wxBoxSizer* MessageBuilder::CreateSizerAll(wxWindow* parent)
{
    wxStaticBox* static_box = new wxStaticBox(parent, kIdStaticBoxMessage, _T("Command"));
    wxStaticBoxSizer* sizer_all = new wxStaticBoxSizer(static_box, wxVERTICAL);

    // wxCB_READONLY so it behaves as the wxChoice it replaces: pick from the list, do not
    // type into it.
    command_choice_ = new wxOwnerDrawnComboBox(parent, kIdChoiceCommand, wxEmptyString,
                                             wxDefaultPosition, wxDefaultSize,
                                             0, nullptr, wxCB_READONLY);
    command_choice_->SetPopupMaxHeight(kPopupMaxHeight);
    for(byte index = 0u; index < MessageCatalog::GetNumberOfCommands(); index++) {
        const MessageCatalog::CommandType& command = MessageCatalog::GetCommand(index);
        command_choice_->Append(wxString::Format(wxT("%u  %s"), command.number, command.label));
    }
    command_choice_->SetSelection(0);

    sizer_all->Add(command_choice_, 0, wxALL | wxEXPAND, kBorder);

    /*
     * Every row is built once, for the command with the longest option list, and later
     * only shown or hidden. Creating and destroying them per selection would be the same
     * widgets over and over, with more to go wrong.
     */
    for(byte index = 0u; index < MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS; index++) {
        wxBoxSizer* sizer_option = new wxBoxSizer(wxHORIZONTAL);

        option_rows_[index].use = new wxCheckBox(parent, kIdCheckboxOptionFirst + index, _T(""));
        option_rows_[index].value = new wxTextCtrl(parent, kIdTextOptionFirst + index, _T(""),
                                                 wxDefaultPosition, wxSize(kValueWidth, -1));
        option_rows_[index].choice = new wxOwnerDrawnComboBox(parent, kIdChoiceOptionFirst + index,
                                                            wxEmptyString, wxDefaultPosition,
                                                            wxSize(kChoiceWidth, -1),
                                                            0, nullptr, wxCB_READONLY);
        option_rows_[index].choice->SetPopupMaxHeight(kPopupMaxHeight);

        sizer_option->Add(option_rows_[index].use, 1, wxALIGN_CENTER_VERTICAL);
        sizer_option->Add(option_rows_[index].value, 0, wxALIGN_CENTER_VERTICAL);
        sizer_option->Add(option_rows_[index].choice, 0, wxALIGN_CENTER_VERTICAL);

        sizer_all->Add(sizer_option, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, kBorder);
    }

    wxStaticText* preview_label = new wxStaticText(parent, wxID_ANY, _T("Message"));
    preview_ = new wxTextCtrl(parent, kIdTextCtrlPreview, _T(""), wxDefaultPosition,
                             wxSize(kPreviewWidth, -1), wxTE_READONLY);
    wxButton* insert = new wxButton(parent, kIdButtonInsert, wxT("&Insert"));

    sizer_all->Add(preview_label, 0, wxLEFT | wxRIGHT, kBorder);
    sizer_all->Add(preview_, 0, wxALL | wxEXPAND, kBorder);
    sizer_all->Add(insert, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, kBorder);

    return sizer_all;
}

/*! \brief          brings the option rows in line with the selected command
 *  \details        Rows beyond the command's option list are hidden. The ones in use are
 *                  reset rather than left as they were, so an argument typed for a
 *                  different command cannot be carried over into this one under a short
 *                  name that happens to sit at the same position.
 *
 *  \return         -
 */
void MessageBuilder::ShowOptionsOfSelectedCommand()
{
    const MessageCatalog::CommandType& command = GetSelectedCommand();

    for(byte index = 0u; index < MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS; index++) {
        const bool exists = index < command.number_of_options;
        /*
         * A read-only field is answered, never sent, so it gets no row of its own. Its
         * name is still in the catalog, which is what lets the decoder put a label on it
         * when the answer comes back.
         */
        const bool used = exists && !command.options[index].read_only;

        bool named = false;

        /*
         * Cleared for every option this command has, not only for the ones that get a row:
         * a checkbox left ticked from a previous command would otherwise put its option
         * into the message although nothing shows it.
         */
        if(exists) { option_rows_[index].use->SetValue(false); }

        if(used) {
            const MessageCatalog::OptionType& option = command.options[index];
            named = option.value_names != nullptr;

            option_rows_[index].use->SetLabel(wxString::Format(wxT("-%c  %s"), option.short_name, option.label));

            if(named) {
                option_rows_[index].choice->Clear();
                for(byte name = 0u; name < option.number_of_value_names; name++) {
                    option_rows_[index].choice->Append(wxString::Format(wxT("%u  %s"),
                                                                      option.minimum + name,
                                                                      option.value_names[name]));
                }
                option_rows_[index].choice->SetSelection(0);
            } else {
                option_rows_[index].value->ChangeValue(_T(""));
                if(option.argument == MessageCatalog::ArgumentType::kString) {
                    option_rows_[index].value->SetHint(_T("text"));
                } else {
                    option_rows_[index].value->SetHint(wxString::Format(wxT("%u..%u"), option.minimum, option.maximum));
                }
            }
        }

        option_rows_[index].use->Show(used);
        option_rows_[index].value->Show(used && !named);
        option_rows_[index].choice->Show(used && named);
        /* Every row starts unticked, so every argument starts greyed out. */
        EnableOptionValue(index, false);
    }

    /*
     * Resizes the window to the rows that are left and moves its minimum along, so it
     * follows the content in both directions. Fixing it at the height of the longest
     * option list would leave a command with one option above 360 pixels of nothing.
     */
    Layout();
    GetSizer()->SetSizeHints(this);
    UpdatePreview();
}

/*! \brief          assembles the message from the selected command and its ticked options
 *  \details        Same shape the parsers expect: the command number, then one
 *                  "-<short name><argument>" per option. A command on its own is a
 *                  request for the current settings, so no option needs to be ticked.
 *
 *  \return         the message, without the newline the Serial shim adds when it is sent
 */
wxString MessageBuilder::BuildMessage() const
{
    const MessageCatalog::CommandType& command = GetSelectedCommand();
    wxString message = wxString::Format(wxT("%u"), command.number);

    for(byte index = 0u; index < command.number_of_options; index++) {
        if(!option_rows_[index].use->IsChecked()) { continue; }

        const MessageCatalog::OptionType& option = command.options[index];
        wxString argument;

        if(option.value_names != nullptr) {
            // The names run upwards from Minimum, so the position in the dropdown is the
            // offset from it — what goes on the wire stays the number.
            argument = wxString::Format(wxT("%u"), option.minimum + option_rows_[index].choice->GetSelection());
        } else {
            argument = option_rows_[index].value->GetValue();
        }

        message += wxString::Format(wxT(" -%c%s"), option.short_name, argument);
    }

    return message;
}

void MessageBuilder::OnCommand(wxCommandEvent &event)
{
    ShowOptionsOfSelectedCommand();
    UNUSED(event);
}

/*! \brief          follows a ticked or unticked option
 *  \details        The row is taken from the event's id, which is what the contiguous
 *                  ranges of the option widgets are for.
 *
 *  \return         -
 */
void MessageBuilder::OnOptionUsed(wxCommandEvent &event)
{
    const byte index = static_cast<byte>(event.GetId() - kIdCheckboxOptionFirst);

    EnableOptionValue(index, option_rows_[index].use->IsChecked());
    UpdatePreview();
}

void MessageBuilder::OnOptionChanged(wxCommandEvent &event)
{
    UpdatePreview();
    UNUSED(event);
}

/*! \brief          hands the message to the input field, without sending it
 *  \details        Deliberately not sent: it can be corrected or extended by hand there
 *                  first, and Send stays the one place a message leaves for the firmware.
 *                  The window closes with it, since what there was to do here is done —
 *                  hidden rather than destroyed, so the next Create brings back the
 *                  command that was selected.
 *
 *  \return         -
 */
void MessageBuilder::OnInsert(wxCommandEvent &event)
{
    SerialShim::GetInstance().SetInput(BuildMessage());
    Hide();
    UNUSED(event);
}

void MessageBuilder::OnClose(wxCloseEvent &event)
{
    /*
     * Hide rather than destroy: GetInstance() keeps handing out this very dialog, so a
     * destroyed one would leave the singleton pointing at freed memory. The parent
     * window destroys it when the application exits.
     */
    Hide();
    UNUSED(event);
}
