#include "sim/PixelsFrame.h"
#include "sim/Settings.h"
#include "sim/MessageBuilder.h"
#include "sim/SerialShim.h"

#include "DisplayCharacters.h"

#if defined(PIXELS) && (defined(__APPLE__ ) || defined(__linux__))
# include "WordclockIcon.xpm"
#endif

namespace {

/* Taken from the firmware rather than kept here. There used to be a second copy of the
   letters in this file, which had to be edited alongside the firmware's table and the
   CharacterIdType enumeration that names the same positions - three places, and a
   mismatch shows up as one wrong letter on a panel nobody rereads.

   The firmware's table stores one byte per letter, so the umlauts are Latin-1 and need
   converting; wxString would otherwise read 0xDC as an invalid UTF-8 byte and drop it. */
wxString toCharacterLabel(char Character)
{
    return wxString(&Character, wxCSConv(wxFONTENCODING_ISO8859_1), 1u);
}

} // namespace



BEGIN_EVENT_TABLE(PixelsFrame, wxFrame)
    EVT_CLOSE(PixelsFrame::OnClose)
    EVT_BUTTON(ID_BUTTON_SEND, PixelsFrame::OnSend)
    EVT_BUTTON(ID_BUTTON_CLEAR, PixelsFrame::OnClear)
    EVT_MENU(wxID_PREFERENCES, PixelsFrame::OnSettings)
    EVT_BUTTON(ID_BUTTON_CREATE, PixelsFrame::OnMessage)
    EVT_MENU(ID_MENU_MESSAGE, PixelsFrame::OnMessage)
    EVT_MENU(wxID_ABOUT, PixelsFrame::OnAbout)
    EVT_MENU(wxID_EXIT, PixelsFrame::OnQuit)
END_EVENT_TABLE()


PixelsFrame::PixelsFrame(wxWindow* parent, const wxString &title) : wxFrame(parent, -1, title)
{
    SetIcon(wxICON(WordclockIcon));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    SetMenuBar(createMenuBar());

    wxScrolledWindow* ScrolledWindow = new wxScrolledWindow(this);
    wxBoxSizer* ContentSizer = createSizerAll(ScrolledWindow);
    ScrolledWindow->SetSizer(ContentSizer);
    ScrolledWindow->SetScrollRate(5, 5);

    // Lay the scrolled window out to fill the frame's client area.
    wxBoxSizer* FrameSizer = new wxBoxSizer(wxVERTICAL);
    FrameSizer->Add(ScrolledWindow, 1, wxEXPAND);
    SetSizer(FrameSizer);

    // A wxScrolledWindow advertises a tiny best size (it is meant to scroll),
    // so Fit() would shrink the frame and clip the letter grid. Size the frame
    // to the content's minimum instead. Note that wxWidgets/GTK only finalises
    // font metrics once the window is mapped, so GetMinSize() under-measures the
    // grid here; this initial size just avoids a tiny first paint.
    SetClientSize(ContentSizer->GetMinSize());
    ScrolledWindow->Show();

    // Re-fit once the window is on screen and the letter sizes are accurate, so
    // nothing gets clipped. Scrollbars still appear if the user shrinks it.
    CallAfter([this, ContentSizer] {
        SetClientSize(ContentSizer->GetMinSize());
    });
}

wxMenuBar* PixelsFrame::createMenuBar()
{
    wxMenu* MenuFile = new wxMenu();
    MenuFile->Append(wxID_PREFERENCES, _T("&Settings"));
    MenuFile->Append(ID_MENU_MESSAGE, _T("&Message"));
    MenuFile->AppendSeparator();
    MenuFile->Append(wxID_EXIT);

    wxMenu* MenuHelp = new wxMenu();
    MenuHelp->Append(wxID_ABOUT);

    wxMenuBar* MenuBar = new wxMenuBar();
    MenuBar->Append(MenuFile, _T("&File"));
    MenuBar->Append(MenuHelp, _T("&Help"));

    return MenuBar;
}

wxBoxSizer* PixelsFrame::createSizerAll(wxWindow* Parent)
{
    wxBoxSizer* SizerAll = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* SizerControl = createSizerControl(Parent);

    SizerAll->Add(createSizerCharacters(Parent), 1, wxALL|wxEXPAND, 10);
    SizerAll->Add(SizerControl, 0, wxALL|wxEXPAND, 10);

    return SizerAll;
}

wxBoxSizer* PixelsFrame::createSizerCharacters(wxWindow* Parent)
{
    wxBoxSizer* SizerCharacters = new wxBoxSizer(wxVERTICAL);

    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        // Proportion 0: rows keep their (square-cell) height instead of being
        // stretched vertically to fill the frame.
        SizerCharacters->Add(createSizerCharacter(Parent, Row), 0, wxALIGN_CENTER);
    }

    return SizerCharacters;
}

wxBoxSizer* PixelsFrame::createSizerCharacter(wxWindow* Parent, int Row)
{
    wxBoxSizer* SizerCharacter = new wxBoxSizer(wxHORIZONTAL);

    // Each letter lives in a fixed square cell so the grid reads as a real
    // matrix instead of vertically-stretched columns.
    static constexpr int CellSize = 44;

    const DisplayCharacters Letters;

    for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        const wxString Label = toCharacterLabel(Letters.getCharacterFast(static_cast<byte>(Column), static_cast<byte>(Row)));

        Characters[Row][Column] = new wxStaticText(Parent, wxID_ANY, Label, wxDefaultPosition, wxSize(CellSize, CellSize), wxALIGN_CENTRE_HORIZONTAL);
        Characters[Row][Column]->SetFont(wxFont(wxSize(40,40), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        Characters[Row][Column]->SetMinSize(wxSize(CellSize, CellSize));
        // Proportion 0 + centered: the cell stays square and never stretches.
        SizerCharacter->Add(Characters[Row][Column], 0, wxALIGN_CENTER);
    }

    return SizerCharacter;
}

wxBoxSizer* PixelsFrame::createSizerControl(wxWindow* Parent)
{
    wxStaticBox* StaticBox = new wxStaticBox(Parent, ID_STATIC_BOX, _T("Control"));
    wxStaticBoxSizer* SizerControl = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
    wxStaticText* OutputLabel = new wxStaticText(Parent, wxID_ANY, _T("Output"));
    wxStaticText* InputLabel = new wxStaticText(Parent, wxID_ANY, _T("Input"));
    /* Opens the message builder, which fills the input field below. */
    wxButton* Create = new wxButton(Parent, ID_BUTTON_CREATE, wxT("C&reate"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* Send = new wxButton(Parent, ID_BUTTON_SEND, wxT("&Send"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* Clear = new wxButton(Parent, ID_BUTTON_CLEAR, wxT("&Clear"), wxDefaultPosition, wxDefaultSize, 0);

    wxTextCtrl* Output = new wxTextCtrl(Parent, ID_TEXT_CTRL_OUTPUT, _(""), wxDefaultPosition, wxSize(200, 200), wxTE_MULTILINE|wxTE_READONLY);
    wxTextCtrl* Input  = new wxTextCtrl(Parent, ID_TEXT_CTRL_INPUT, _(""), wxDefaultPosition, wxSize(200, 20));

    SerialShim::getInstance().attach(Output, Input);

    SizerControl->Add(OutputLabel, 0, wxLEFT | wxTOP | wxEXPAND, 10);
    SizerControl->Add(Output, 1, wxRIGHT | wxLEFT | wxEXPAND, 10);
    SizerControl->Add(Clear, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);
    SizerControl->Add(InputLabel, 0, wxLEFT | wxTOP, 10);
    SizerControl->Add(Input, 0, wxRIGHT | wxLEFT, 10);
    SizerControl->Add(Create, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);
    SizerControl->Add(Send, 0, wxTOP | wxRIGHT | wxLEFT | wxBOTTOM | wxEXPAND, 10);

    return SizerControl;
}

PixelsFrame::~PixelsFrame()
{

}

void PixelsFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
    UNUSED(event);
}

void PixelsFrame::OnSend(wxCommandEvent &event)
{
    SerialShim::getInstance().sendInput();
    UNUSED(event);
}

void PixelsFrame::OnClear(wxCommandEvent &event)
{
    SerialShim::getInstance().clearOutput();
    UNUSED(event);
}

void PixelsFrame::OnSettings(wxCommandEvent &event)
{
    Settings::getInstance().reveal();
    UNUSED(event);
}

void PixelsFrame::OnMessage(wxCommandEvent &event)
{
    MessageBuilder::getInstance().reveal();
    UNUSED(event);
}

void PixelsFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
    UNUSED(event);
}

void PixelsFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(_("Copyright Andreas Burnickl\nWordclock Simulator"));
    UNUSED(event);
}

wxColour PixelsFrame::toColour(PixelType Pixel) const
{
    /* Only a pixel's brightness is rendered, as the grey level of its letter. */
    const byte Level = toLevel(getWeightedIntensity(getIntensity(Pixel)));

    return wxColour(Level, Level, Level);
}

void PixelsFrame::renderPixel(byte Row, byte Column)
{
    const Pixels& pixels = Pixels::getInstance();
    const byte Index = static_cast<byte>((Row * PIXELS_DISPLAY_NUMBER_OF_COLUMNS) + Column);
    const wxColour Colour = (pixels.getBrightness() == 0u) ? wxColour(*wxLIGHT_GREY)
                                                           : toColour(pixels.getPixelFast(Index));

    /* SetForegroundColour marks the label for repaint even when the colour does not
       change. Rendering now walks the whole grid instead of only the pixel that was
       written, so without this every pass would repaint all 110 letters. */
    if(Characters[Row][Column]->GetForegroundColour() == Colour) { return; }

    Characters[Row][Column]->SetForegroundColour(Colour);
}

void PixelsFrame::renderAllPixels()
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            renderPixel(Row, Column);
        }
    }
}

/* Called once per application tick rather than from inside every pixel write, so the
   window is repainted at most once per pass over the firmware's tasks - and not at all
   on the ticks where the firmware wrote nothing. */
void PixelsFrame::render()
{
    Pixels& pixels = Pixels::getInstance();

    if(!pixels.isDirty()) { return; }
    pixels.clearDirty();

    renderAllPixels();
    Refresh();
}
