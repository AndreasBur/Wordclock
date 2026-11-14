// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Puts a serial message together from a command and its options.
 *
 * Offers the commands the firmware understands by name, with an option row each, and writes the
 * result into the pixel matrix window's input field. It does not send: that stays with the Send
 * button there, so a built message can still be edited by hand first.
 *
 * Named MessageBuilder rather than Message, which the firmware already uses for an incoming
 * message.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEBUILDER_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEBUILDER_H_

#include <wx/odcombo.h>
#include <wx/wx.h>

#include <array>

#include "MessageCatalog.h"
#include "PixelsFrame.h"
#include "StandardTypes.h"

/*!
 * \brief The message builder: a window that puts a command together from MessageCatalog and writes
 *        it into the input field of the serial port.
 *
 * It offers every command of the catalog by number and label, and a row per option the command
 * can be sent with: a box that puts the option into the message, and its argument, picked by
 * name where the catalog names the values and typed otherwise. An option that is only answered,
 * never sent, gets no row. The message is shown as it is built, and Insert hands it to
 * SerialShim::SetInput() and hides the window.
 *
 * A dialog rather than a frame, so the compositor centres it over the main window. Shown with
 * Show(), not ShowModal(), so the input field it writes to stays reachable. Closing it hides it.
 */
// wx deletes a window through wxWindow*, so the destructor is virtual, and it is not public, so
// that nothing deletes the window but wx itself, after Destroy().
// NOLINTNEXTLINE(cppcoreguidelines-virtual-class-destructor)
class MessageBuilder : public wxDialog {
 private:
  // Own range, clear of the other windows'. The option rows get contiguous ranges of
  // their own, which is what lets one handler serve all of them.
  enum {
    kIdStaticBoxMessage = 3000,
    kIdChoiceCommand,
    kIdTextCtrlPreview,
    kIdButtonInsert,

    kIdCheckboxOptionFirst = 3100,
    kIdCheckboxOptionLast = kIdCheckboxOptionFirst + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1,

    kIdTextOptionFirst = 3200,
    kIdTextOptionLast = kIdTextOptionFirst + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1,

    kIdChoiceOptionFirst = 3300,
    kIdChoiceOptionLast = kIdChoiceOptionFirst + MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS - 1
  };

  /*
   * One row per option of the selected command: whether it is part of the message at
   * all, and with which argument. Both argument widgets exist in every row and only
   * the one the option calls for is shown — a dropdown where the catalog names the
   * values, a text field for a plain number or a string. Building them once and
   * hiding one keeps the rows reusable across commands.
   */
  struct OptionRowType {
    wxCheckBox* use;
    wxTextCtrl* value;
    wxOwnerDrawnComboBox* choice;
  };

  wxOwnerDrawnComboBox* command_choice_{nullptr};
  std::array<OptionRowType, MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS> option_rows_{};
  wxTextCtrl* preview_{nullptr};

  static constexpr int kValueWidth{80};
  // Wider than the plain value field: it has to hold names like "Illuminance
  // calibration maximum".
  static constexpr int kChoiceWidth{240};
  /*
   * What a popup may grow to before it scrolls instead. wxChoice had no such knob: its
   * popup is a GTK one, and GTK decides when to switch on its scroll arrows from the
   * monitor's work area - which is right on a normal desktop and wrong under WSLg, where
   * the list simply ran off the bottom of the screen and the entries past it could not be
   * reached at all. wxOwnerDrawnComboBox draws its own popup, so this is a number rather
   * than a hope. Ten rows or so, which is the RPC list's thirty-three in three screens
   * rather than one screen and a cliff.
   */
  static constexpr int kPopupMaxHeight{240};
  static constexpr int kPreviewWidth{300};
  static constexpr int kBorder{10};

  DECLARE_EVENT_TABLE()

  MessageBuilder(wxWindow* parent, wxString const& title);
  ~MessageBuilder() override;

  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnOptionUsed(wxCommandEvent& event);
  void OnOptionChanged(wxCommandEvent& event);
  void OnInsert(wxCommandEvent& event);

  // An option that is not ticked contributes nothing to the message, so its argument is
  // greyed out rather than left looking as though it did.
  void EnableOptionValue(byte index, bool enabled) {
    option_rows_[index].value->Enable(enabled);
    option_rows_[index].choice->Enable(enabled);
  }

  wxBoxSizer* CreateSizerAll(wxWindow* parent);

  MessageCatalog::CommandType const& GetSelectedCommand() const {
    return MessageCatalog::GetCommand(static_cast<byte>(command_choice_->GetSelection()));
  }

  // Labels a row with its option and fills the argument widget the option calls for, a dropdown
  // of its names or a text field with its range as the hint.
  static void FillOptionRow(OptionRowType const& row, MessageCatalog::OptionType const& option);
  void ShowOptionsOfSelectedCommand();
  wxString BuildMessage() const;
  void UpdatePreview() { preview_->SetValue(BuildMessage()); }

 public:
  /*!
   * \brief Returns the one message builder, which the first call creates, hidden and centred over
   *        the window of PixelsFrame, with the first command of the catalog selected.
   *
   * Parented to the pixel matrix window, so closing that one takes this along and the
   * application can exit.
   *
   * \return The message builder.
   */
  static MessageBuilder& GetInstance() {
    static MessageBuilder* p_singleton_instance =
        new MessageBuilder(&PixelsFrame::GetInstance(), _("Wordclock Message"));
    return *p_singleton_instance;
  }

  /*!
   * \brief Brings the window up, whether it was never shown or only hidden.
   *
   * The command and the options stay as they were left.
   */
  void Reveal() {
    Show();
    Raise();
    /*
     * Again once the window is on screen. A size set while it was still hidden did
     * not always reach the window manager under XWayland, which left the frame at
     * the height of a different command's option list.
     */
    GetSizer()->SetSizeHints(this);
  }
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEBUILDER_H_
