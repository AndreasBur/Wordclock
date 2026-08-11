#include "sim/Pixels.h"
#include "sim/Settings.h"
#include "sim/MessageBuilder.h"
#include "sim/SerialShim.h"
#include <array>

#if defined(PIXELS) && (defined(__APPLE__ ) || defined(__linux__))
# include "WordclockIcon.xpm"
#endif

const wxString DisplayCharacters[][PIXELS_DISPLAY_NUMBER_OF_COLUMNS]
{
    {wxT("E"),wxT("S"),wxT("K"),wxT("I"),wxT("S"),wxT("T"),wxT("L"),wxT("F"),wxT("Ü"),wxT("N"),wxT("F")},
    {wxT("Z"),wxT("E"),wxT("H"),wxT("N"),wxT("Z"),wxT("W"),wxT("A"),wxT("N"),wxT("Z"),wxT("I"),wxT("G")},
    {wxT("D"),wxT("R"),wxT("E"),wxT("I"),wxT("V"),wxT("I"),wxT("E"),wxT("R"),wxT("T"),wxT("E"),wxT("L")},
    {wxT("T"),wxT("G"),wxT("N"),wxT("A"),wxT("C"),wxT("H"),wxT("V"),wxT("O"),wxT("R"),wxT("J"),wxT("M")},
    {wxT("H"),wxT("A"),wxT("L"),wxT("B"),wxT("Q"),wxT("Z"),wxT("W"),wxT("Ö"),wxT("L"),wxT("F"),wxT("P")},
    {wxT("Z"),wxT("W"),wxT("E"),wxT("I"),wxT("N"),wxT("S"),wxT("I"),wxT("E"),wxT("B"),wxT("E"),wxT("N")},
    {wxT("K"),wxT("D"),wxT("R"),wxT("E"),wxT("I"),wxT("R"),wxT("H"),wxT("F"),wxT("Ü"),wxT("N"),wxT("F")},
    {wxT("E"),wxT("L"),wxT("F"),wxT("N"),wxT("E"),wxT("U"),wxT("N"),wxT("V"),wxT("I"),wxT("E"),wxT("R")},
    {wxT("W"),wxT("A"),wxT("C"),wxT("H"),wxT("T"),wxT("Z"),wxT("E"),wxT("H"),wxT("N"),wxT("R"),wxT("S")},
    {wxT("B"),wxT("S"),wxT("E"),wxT("C"),wxT("H"),wxT("S"),wxT("F"),wxT("M"),wxT("U"),wxT("H"),wxT("R")}
};



BEGIN_EVENT_TABLE(Pixels, wxFrame)
    EVT_CLOSE(Pixels::OnClose)
    EVT_BUTTON(ID_BUTTON_SEND, Pixels::OnSend)
    //EVT_BUTTON(ID_BUTTON_CREATE_INPUT, Pixels::OnCreateInput)
    EVT_BUTTON(ID_BUTTON_CLEAR, Pixels::OnClear)
    EVT_MENU(wxID_PREFERENCES, Pixels::OnSettings)
    EVT_BUTTON(ID_BUTTON_CREATE, Pixels::OnMessage)
    EVT_MENU(ID_MENU_MESSAGE, Pixels::OnMessage)
    EVT_MENU(wxID_ABOUT, Pixels::OnAbout)
    EVT_MENU(wxID_EXIT, Pixels::OnQuit)
END_EVENT_TABLE()


Pixels::Pixels(wxWindow* parent, const wxString &title) : wxFrame(parent, -1, title)
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

wxMenuBar* Pixels::createMenuBar()
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

wxBoxSizer* Pixels::createSizerAll(wxWindow* Parent)
{
    wxBoxSizer* SizerAll = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* SizerControl = createSizerControl(Parent);

    SizerAll->Add(createSizerCharacters(Parent), 1, wxALL|wxEXPAND, 10);
    SizerAll->Add(SizerControl, 0, wxALL|wxEXPAND, 10);

    return SizerAll;
}

wxBoxSizer* Pixels::createSizerCharacters(wxWindow* Parent)
{
    wxBoxSizer* SizerCharacters = new wxBoxSizer(wxVERTICAL);

    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        // Proportion 0: rows keep their (square-cell) height instead of being
        // stretched vertically to fill the frame.
        SizerCharacters->Add(createSizerCharacter(Parent, Row), 0, wxALIGN_CENTER);
    }

    return SizerCharacters;
}

wxBoxSizer* Pixels::createSizerCharacter(wxWindow* Parent, int Row)
{
    wxBoxSizer* SizerCharacter = new wxBoxSizer(wxHORIZONTAL);

    // Each letter lives in a fixed square cell so the grid reads as a real
    // matrix instead of vertically-stretched columns.
    static constexpr int CellSize = 44;

    for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
        Characters[Row][Column] = new wxStaticText(Parent, wxID_ANY, DisplayCharacters[Row][Column], wxDefaultPosition, wxSize(CellSize, CellSize), wxALIGN_CENTRE_HORIZONTAL);
        Characters[Row][Column]->SetFont(wxFont(wxSize(40,40), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        Characters[Row][Column]->SetMinSize(wxSize(CellSize, CellSize));
        // Proportion 0 + centered: the cell stays square and never stretches.
        SizerCharacter->Add(Characters[Row][Column], 0, wxALIGN_CENTER);
    }

    return SizerCharacter;
}

wxBoxSizer* Pixels::createSizerControl(wxWindow* Parent)
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

Pixels::~Pixels()
{

}

void Pixels::OnClose(wxCloseEvent &event)
{
    //wxTheApp->Exit();
    //wxTheApp->AddPendingEvent(wxCloseEvent());
    //wxTheApp->GetTopWindow()->Destroy();
    //wxTheApp->GetTopWindow()->Close();
    Destroy();
    UNUSED(event);
}

//void Pixels::OnCreateInput(wxCommandEvent &event)
//{
//
//}

void Pixels::OnSend(wxCommandEvent &event)
{
    SerialShim::getInstance().sendInput();
    UNUSED(event);
}

void Pixels::OnClear(wxCommandEvent &event)
{
    SerialShim::getInstance().clearOutput();
    UNUSED(event);
}

void Pixels::OnSettings(wxCommandEvent &event)
{
    Settings::getInstance().reveal();
    UNUSED(event);
}

void Pixels::OnMessage(wxCommandEvent &event)
{
    MessageBuilder::getInstance().reveal();
    UNUSED(event);
}

void Pixels::OnQuit(wxCommandEvent &event)
{
    //wxTheApp->Exit();
    //wxTheApp->AddPendingEvent(wxCloseEvent());
    //wxTheApp->GetTopWindow()->Destroy();
    Destroy();
    UNUSED(event);
}

void Pixels::OnAbout(wxCommandEvent &event)
{
    wxMessageBox(_("Copyright Andreas Burnickl\nWordclock Simulator"));
    UNUSED(event);
}


StdReturnType Pixels::getPixel(byte Index, PixelType& Pixel) const
{
    byte Row = Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < PIXELS_NUMBER_OF_LEDS) {
        Pixel = PixelBuffer[Row][Column];
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

Pixels::PixelType Pixels::getPixelFast(byte Index) const
{
    byte Row = Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS;

    return PixelBuffer[Row][Column];
}

wxColour Pixels::toColour(PixelType Pixel) const
{
    /* Only a pixel's brightness is rendered, as the grey level of its letter. */
    const byte Level = toLevel(getWeightedIntensity(getIntensity(Pixel)));

    return wxColour(Level, Level, Level);
}

void Pixels::renderPixel(byte Row, byte Column)
{
    if(Brightness == 0u) { Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY)); }
    else { Characters[Row][Column]->SetForegroundColour(toColour(PixelBuffer[Row][Column])); }
}

void Pixels::renderAllPixels()
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            renderPixel(Row, Column);
        }
    }
}

StdReturnType Pixels::setPixel(byte Index, PixelType Pixel)
{
    byte Row = Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < PIXELS_NUMBER_OF_LEDS) {
        PixelBuffer[Row][Column] = Pixel;
        renderPixel(Row, Column);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

StdReturnType Pixels::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    return setPixel(Index, PixelType(Red, Green, Blue));
}

void Pixels::setPixelFast(byte Index, PixelType Pixel)
{
    byte Row = Index / PIXELS_DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % PIXELS_DISPLAY_NUMBER_OF_COLUMNS;

    if(Index >= PIXELS_NUMBER_OF_LEDS) {
        // set breakpoint to find index out of bounce calls
    }

    wxASSERT(Index < PIXELS_NUMBER_OF_LEDS);

    PixelBuffer[Row][Column] = Pixel;
    renderPixel(Row, Column);
}

void Pixels::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    setPixelFast(Index, PixelType(Red, Green, Blue));
}

void Pixels::clearPixels()
{
    setPixels(PixelType(0u, 0u, 0u));
}

void Pixels::setPixels(wxColor Color)
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            Characters[Row][Column]->SetForegroundColour(Color);
        }
    }
}

void Pixels::setPixels(PixelType Pixel)
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            PixelBuffer[Row][Column] = Pixel;
        }
    }
    renderAllPixels();
}

void Pixels::setBrightness(byte sBrightness, bool GammaCorrection)
{
    /* Master output on and off, driven by enablePixels() and disablePixels(). The
       buffer is left alone, so switching back on restores the picture without having to
       copy the label colours away first. */
    Brightness = sBrightness;
    renderAllPixels();

    UNUSED(GammaCorrection);
}

