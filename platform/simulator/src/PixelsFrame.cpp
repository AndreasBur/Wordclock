// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The window of the simulator: the letter grid, rendered from the pixel buffer.
 */

#include "sim/PixelsFrame.h"
#include "sim/Settings.h"
#include "sim/MessageBuilder.h"
#include "sim/SerialShim.h"

#include "DisplayCharacters.h"

#if defined(PIXELS) && (defined(__APPLE__ ) || defined(__linux__))
# include "WordclockIcon.xpm"
#endif

namespace {
/*
 * Taken from the firmware rather than kept here. There used to be a second copy of the
 * letters in this file, which had to be edited alongside the firmware's table and the
 * CharacterIdType enumeration that names the same positions - three places, and a
 * mismatch shows up as one wrong letter on a panel nobody rereads.
 *
 * The firmware's table stores one byte per letter, so the umlauts are Latin-1 and need
 * converting; wxString would otherwise read 0xDC as an invalid UTF-8 byte and drop it.
 */
wxString ToCharacterLabel(char character)
{
    return wxString(&character, wxCSConv(wxFONTENCODING_ISO8859_1), 1u);
}

} // namespace

BEGIN_EVENT_TABLE(PixelsFrame, wxFrame)
    EVT_CLOSE(PixelsFrame::OnClose)
    EVT_BUTTON(kIdButtonSend, PixelsFrame::OnSend)
    EVT_BUTTON(kIdButtonClear, PixelsFrame::OnClear)
    EVT_MENU(wxID_PREFERENCES, PixelsFrame::OnSettings)
    EVT_BUTTON(kIdButtonCreate, PixelsFrame::OnMessage)
    EVT_MENU(kIdMenuMessage, PixelsFrame::OnMessage)
    EVT_MENU(wxID_ABOUT, PixelsFrame::OnAbout)
    EVT_MENU(wxID_EXIT, PixelsFrame::OnQuit)
END_EVENT_TABLE()

PixelsFrame::PixelsFrame(wxWindow* parent, const wxString &title) : wxFrame(parent, -1, title)
{
    SetIcon(wxICON(WordclockIcon));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    SetMenuBar(CreateMenuBar());

    wxScrolledWindow* scrolled_window = new wxScrolledWindow(this);
    wxBoxSizer* content_sizer = CreateSizerAll(scrolled_window);
    scrolled_window->SetSizer(content_sizer);
    scrolled_window->SetScrollRate(5, 5);

    // Lay the scrolled window out to fill the frame's client area.
    wxBoxSizer* frame_sizer = new wxBoxSizer(wxVERTICAL);
    frame_sizer->Add(scrolled_window, 1, wxEXPAND);
    SetSizer(frame_sizer);

    // A wxScrolledWindow advertises a tiny best size (it is meant to scroll),
    // so Fit() would shrink the frame and clip the letter grid. Size the frame
    // to the content's minimum instead. Note that wxWidgets/GTK only finalises
    // font metrics once the window is mapped, so GetMinSize() under-measures the
    // grid here; this initial size just avoids a tiny first paint.
    SetClientSize(content_sizer->GetMinSize());
    scrolled_window->Show();

    // Re-fit once the window is on screen and the letter sizes are accurate, so
    // nothing gets clipped. Scrollbars still appear if the user shrinks it.
    CallAfter([this, content_sizer] {
        SetClientSize(content_sizer->GetMinSize());
    });
}

wxMenuBar* PixelsFrame::CreateMenuBar()
{
    wxMenu* menu_file = new wxMenu();
    menu_file->Append(wxID_PREFERENCES, _T("&Settings"));
    menu_file->Append(kIdMenuMessage, _T("&Message"));
    menu_file->AppendSeparator();
    menu_file->Append(wxID_EXIT);

    wxMenu* menu_help = new wxMenu();
    menu_help->Append(wxID_ABOUT);

    wxMenuBar* menu_bar = new wxMenuBar();
    menu_bar->Append(menu_file, _T("&File"));
    menu_bar->Append(menu_help, _T("&Help"));

    return menu_bar;
}

wxBoxSizer* PixelsFrame::CreateSizerAll(wxWindow* parent)
{
    wxBoxSizer* sizer_all = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* sizer_control = CreateSizerControl(parent);

    sizer_all->Add(CreateSizerCharacters(parent), 1, wxALL|wxEXPAND, 10);
    sizer_all->Add(sizer_control, 0, wxALL|wxEXPAND, 10);

    return sizer_all;
}

wxBoxSizer* PixelsFrame::CreateSizerCharacters(wxWindow* parent)
{
    wxBoxSizer* sizer_characters = new wxBoxSizer(wxVERTICAL);

    for(unsigned int row = 0; row < PIXELS_DISPLAY_NUMBER_OF_ROWS; row++) {
        // Proportion 0: rows keep their (square-cell) height instead of being
        // stretched vertically to fill the frame.
        sizer_characters->Add(CreateSizerCharacter(parent, static_cast<int>(row)), 0, wxALIGN_CENTER);
    }

    return sizer_characters;
}

wxBoxSizer* PixelsFrame::CreateSizerCharacter(wxWindow* parent, int row)
{
    wxBoxSizer* sizer_character = new wxBoxSizer(wxHORIZONTAL);

    // Each letter lives in a fixed square cell so the grid reads as a real
    // matrix instead of vertically-stretched columns.
    static constexpr int kCellSize = 44;

    const DisplayCharacters letters;

    for(unsigned int column = 0; column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; column++) {
        const wxString label = ToCharacterLabel(letters.GetCharacter(static_cast<byte>(column), static_cast<byte>(row)));

        characters_[row][column] = new wxStaticText(parent, wxID_ANY, label, wxDefaultPosition, wxSize(kCellSize, kCellSize), wxALIGN_CENTRE_HORIZONTAL);
        characters_[row][column]->SetFont(wxFont(wxSize(40,40), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
        characters_[row][column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        characters_[row][column]->SetMinSize(wxSize(kCellSize, kCellSize));
        // Proportion 0 + centered: the cell stays square and never stretches.
        sizer_character->Add(characters_[row][column], 0, wxALIGN_CENTER);
    }

    return sizer_character;
}

wxBoxSizer* PixelsFrame::CreateSizerControl(wxWindow* parent)
{
    wxStaticBox* static_box = new wxStaticBox(parent, kIdStaticBox, _T("Control"));
    wxStaticBoxSizer* sizer_control = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxStaticText* output_label = new wxStaticText(parent, wxID_ANY, _T("Output"));
    wxStaticText* input_label = new wxStaticText(parent, wxID_ANY, _T("Input"));
    /* Opens the message builder, which fills the input field below. */
    wxButton* create = new wxButton(parent, kIdButtonCreate, wxT("C&reate"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* send = new wxButton(parent, kIdButtonSend, wxT("&Send"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* clear = new wxButton(parent, kIdButtonClear, wxT("&Clear"), wxDefaultPosition, wxDefaultSize, 0);

    static constexpr int kConsoleWidth = 300;

    wxTextCtrl* output = new wxTextCtrl(parent, kIdTextCtrlOutput, _(""), wxDefaultPosition,
                                        wxSize(kConsoleWidth, 200), wxTE_MULTILINE|wxTE_READONLY|wxTE_DONTWRAP);
    wxTextCtrl* input  = new wxTextCtrl(parent, kIdTextCtrlInput, _(""), wxDefaultPosition, wxSize(kConsoleWidth, 20));

    SerialShim::GetInstance().Attach(output, input);

    sizer_control->Add(output_label, 0, wxLEFT | wxTOP | wxEXPAND, 10);
    sizer_control->Add(output, 1, wxRIGHT | wxLEFT | wxEXPAND, 10);
    sizer_control->Add(clear, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);
    sizer_control->Add(input_label, 0, wxLEFT | wxTOP, 10);
    sizer_control->Add(input, 0, wxRIGHT | wxLEFT | wxEXPAND, 10);
    sizer_control->Add(create, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);
    sizer_control->Add(send, 0, wxTOP | wxRIGHT | wxLEFT | wxBOTTOM | wxEXPAND, 10);

    return sizer_control;
}

PixelsFrame::~PixelsFrame()
{
}

void PixelsFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
    UNUSED(event);
}

// wxWidgets event tables require a non-static member function.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void PixelsFrame::OnSend(wxCommandEvent &event)
{
    SerialShim::GetInstance().SendInput();
    UNUSED(event);
}

// wxWidgets event tables require a non-static member function.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void PixelsFrame::OnClear(wxCommandEvent &event)
{
    SerialShim::GetInstance().ClearOutput();
    UNUSED(event);
}

// wxWidgets event tables require a non-static member function.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void PixelsFrame::OnSettings(wxCommandEvent &event)
{
    Settings::GetInstance().Reveal();
    UNUSED(event);
}

// wxWidgets event tables require a non-static member function.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void PixelsFrame::OnMessage(wxCommandEvent &event)
{
    MessageBuilder::GetInstance().Reveal();
    UNUSED(event);
}

void PixelsFrame::OnQuit(wxCommandEvent &event)
{
    // Through the close event, as the frame's close button goes, so that the application
    // stops its timer first; Destroy() sends none.
    Close();
    UNUSED(event);
}

// wxWidgets event tables require a non-static member function.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void PixelsFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(_("Copyright Andreas Burnickl\nWordclock Simulator"));
    UNUSED(event);
}

wxColour PixelsFrame::ToColour(PixelType pixel)
{
    /* Only a pixel's brightness is rendered, as the grey level of its letter. */
    const byte level = ToLevel(GetWeightedIntensity(GetIntensity(pixel)));

    return wxColour(level, level, level);
}

void PixelsFrame::RenderPixel(byte row, byte column)
{
    const Pixels& pixels = Pixels::GetInstance();
    const byte index = static_cast<byte>((row * PIXELS_DISPLAY_NUMBER_OF_COLUMNS) + column);
    const wxColour colour = (pixels.GetBrightness() == 0u) ? wxColour(*wxLIGHT_GREY)
                                                           : ToColour(pixels.GetPixel(index));

    /*
     * SetForegroundColour marks the label for repaint even when the colour does not
     * change. Rendering now walks the whole grid instead of only the pixel that was
     * written, so without this every pass would repaint all 110 letters.
     */
    if(characters_[row][column]->GetForegroundColour() == colour) { return; }

    characters_[row][column]->SetForegroundColour(colour);
}

void PixelsFrame::RenderAllPixels()
{
    for(unsigned int row = 0; row < PIXELS_DISPLAY_NUMBER_OF_ROWS; row++) {
        for(unsigned int column = 0; column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; column++) {
            RenderPixel(row, column);
        }
    }
}

void PixelsFrame::Render()
{
    Pixels& pixels = Pixels::GetInstance();

    /*
     * Before the dirty check rather than after it: while the strip has no supply nothing
     * may be drawn, and the buffer has to stay dirty so that the supply coming back
     * redraws it. What stands in the window until then is the black frame the switch-off
     * sequence sends on purpose, which is also what the wall would show.
     */
    if(pixels.IsOutputSuspended()) { return; }

    if(!pixels.IsDirty()) { return; }
    pixels.ClearDirty();

    RenderAllPixels();
    Refresh();
}
