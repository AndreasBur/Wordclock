#include "Simulator.h"
#include "WS2812.h"
#include <array>

const wxString DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS] =
{
    wxT("E"),wxT("S"),wxT("K"),wxT("I"),wxT("S"),wxT("T"),wxT("L"),wxT("F"),wxT("Ü"),wxT("N"),wxT("F"),
    wxT("Z"),wxT("E"),wxT("H"),wxT("N"),wxT("Z"),wxT("W"),wxT("A"),wxT("N"),wxT("Z"),wxT("I"),wxT("G"),
    wxT("D"),wxT("R"),wxT("E"),wxT("I"),wxT("V"),wxT("I"),wxT("E"),wxT("R"),wxT("T"),wxT("E"),wxT("L"),
    wxT("T"),wxT("G"),wxT("N"),wxT("A"),wxT("C"),wxT("H"),wxT("V"),wxT("O"),wxT("R"),wxT("J"),wxT("M"),
    wxT("H"),wxT("A"),wxT("L"),wxT("B"),wxT("Q"),wxT("Z"),wxT("W"),wxT("Ö"),wxT("L"),wxT("F"),wxT("P"),
    wxT("Z"),wxT("W"),wxT("E"),wxT("I"),wxT("N"),wxT("S"),wxT("I"),wxT("E"),wxT("B"),wxT("E"),wxT("N"),
    wxT("K"),wxT("D"),wxT("R"),wxT("E"),wxT("I"),wxT("R"),wxT("H"),wxT("F"),wxT("Ü"),wxT("N"),wxT("F"),
    wxT("E"),wxT("L"),wxT("F"),wxT("N"),wxT("E"),wxT("U"),wxT("N"),wxT("V"),wxT("I"),wxT("E"),wxT("R"),
    wxT("W"),wxT("A"),wxT("C"),wxT("H"),wxT("T"),wxT("Z"),wxT("E"),wxT("H"),wxT("N"),wxT("R"),wxT("S"),
    wxT("B"),wxT("S"),wxT("E"),wxT("C"),wxT("H"),wxT("S"),wxT("F"),wxT("M"),wxT("U"),wxT("H"),wxT("R")
};


BEGIN_EVENT_TABLE(Simulator, wxFrame)
    EVT_CLOSE(Simulator::OnClose)
    EVT_BUTTON(ID_BUTTON_QUIT, Simulator::OnQuit)
    EVT_BUTTON(ID_BUTTON_ABOUT, Simulator::OnAbout)
END_EVENT_TABLE()


Simulator::Simulator(wxFrame *dlg, const wxString &title) : wxFrame(dlg, -1, title)
{
    Brightness = 255;

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    //this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxScrolledWindow* ScrolledWindow = new wxScrolledWindow(this);
    ScrolledWindows->
    wxBoxSizer* SizerAll = new wxBoxSizer(wxHORIZONTAL);

    SizerAll->
    SizerAll->Add(createSizerCharacters(), 1, wxALL|wxEXPAND, 10);
    //SizerAll->Add(createSizerButton(), 1, wxEXPAND, 5);
    SizerAll->Add(createSizerControl(), 0, wxALL|wxEXPAND, 10);

    this->SetSizer(SizerAll);
    this->Layout();
    SizerAll->Fit(this);
    Show();
}

wxBoxSizer* Simulator::createSizerCharacter(int Row)
{
    wxBoxSizer* SizerCharacter = new wxBoxSizer(wxHORIZONTAL);

    for(unsigned int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++)
    {
        Characters[Row][Column] = new wxStaticText(this, wxID_ANY, DisplayCharacters[Row][Column], wxDefaultPosition, wxDefaultSize, 0);
        Characters[Row][Column]->SetFont(wxFont(wxSize(40,40), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        SizerCharacter->Add(Characters[Row][Column], 1, wxALL|wxEXPAND, 5);
    }

    return SizerCharacter;
}

wxBoxSizer* Simulator::createSizerCharacters()
{
    wxBoxSizer* SizerCharacters = new wxBoxSizer(wxVERTICAL);

    for(unsigned int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        SizerCharacters->Add(createSizerCharacter(Row), 1, wxEXPAND, 5);
    }

    return SizerCharacters;
}

wxBoxSizer* Simulator::createSizerButton()
{
    wxBoxSizer* SizerButton = new wxBoxSizer(wxHORIZONTAL);
    wxButton* About = new wxButton(this, ID_BUTTON_ABOUT, wxT("&About"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* Quit = new wxButton(this, ID_BUTTON_QUIT, wxT("&Quit"), wxDefaultPosition, wxDefaultSize, 0);

    SizerButton->Add(About, 1, wxALL|wxEXPAND, 10);
    SizerButton->Add(Quit, 1, wxALL|wxEXPAND, 10);

    return SizerButton;
}

wxBoxSizer* Simulator::createSizerControl()
{
    wxStaticBox* StaticBox = new wxStaticBox(this, ID_STATIC_BOX, _T("Control"));
    wxStaticBoxSizer* SizerControl = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
    wxStaticText* OutputLabel = new wxStaticText(this, wxID_ANY, _T("Output"));
    wxStaticText* InputLabel = new wxStaticText(this, wxID_ANY, _T("Input"));
    wxButton* Send = new wxButton(this, ID_BUTTON_SEND, wxT("&Send"), wxDefaultPosition, wxDefaultSize, 0);
    wxButton* Clear = new wxButton(this, ID_BUTTON_CLEAR, wxT("&Clear"), wxDefaultPosition, wxDefaultSize, 0);

    wxTextCtrl* Output = new wxTextCtrl(this, ID_TEXT_CTRL_OUTPUT, _(""), wxDefaultPosition, wxSize(200, 200), wxTE_MULTILINE|wxTE_READONLY);
    wxTextCtrl* Input  = new wxTextCtrl(this, ID_TEXT_CTRL_INPUT, _(""), wxDefaultPosition, wxSize(200, 20));

    SizerControl->Add(OutputLabel, 0, wxLEFT | wxTOP | wxEXPAND, 10);
    SizerControl->Add(Output, 0, wxRight | wxLEFT, 10);
    SizerControl->Add(Clear, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);
    SizerControl->Add(InputLabel, 0, wxLEFT | wxTOP, 10);
    SizerControl->Add(Input, 0, wxRIGHT | wxLEFT, 10);
    SizerControl->Add(Send, 0, wxTOP | wxRIGHT | wxLEFT | wxEXPAND, 10);

    return SizerControl;
}

Simulator::~Simulator()
{

}

void Simulator::OnClose(wxCloseEvent &event)
{
    //wxTheApp->Exit();
    //wxTheApp->AddPendingEvent(wxCloseEvent());
    wxTheApp->GetTopWindow()->Close();
}

void Simulator::OnQuit(wxCommandEvent &event)
{
    //wxTheApp->Exit();
    //wxTheApp->AddPendingEvent(wxCloseEvent());
    wxTheApp->GetTopWindow()->Close();
}

void Simulator::OnAbout(wxCommandEvent &event)
{

}

StdReturnType Simulator::getPixel(byte Index, PixelType& Pixel) const
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < WS2812_NUMBER_OF_LEDS) {
        if(Characters[Row][Column]->GetForegroundColour() == wxColor(*wxBLACK)) {
            Pixel.Blue = 255;
            Pixel.Green = 255;
            Pixel.Red = 255;
        } else {
            Pixel.Blue = 0;
            Pixel.Green = 0;
            Pixel.Red = 0;
        }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

WS2812::PixelType Simulator::getPixelFast(byte Index) const
{
    PixelType Pixel;
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if(Characters[Row][Column]->GetForegroundColour() == wxColor(*wxBLACK)) {
        Pixel.Blue = 255;
        Pixel.Green = 255;
        Pixel.Red = 255;
    } else {
        Pixel.Blue = 0;
        Pixel.Green = 0;
        Pixel.Red = 0;
    }
    return Pixel;
}

StdReturnType Simulator::setPixel(byte Index, PixelType Pixel)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < WS2812_NUMBER_OF_LEDS) {
        if(Pixel.Red != 0 || Pixel.Green != 0 || Pixel.Blue != 0) {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxBLACK));
        } else {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

StdReturnType Simulator::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < WS2812_NUMBER_OF_LEDS) {
        if(Red != 0 || Green != 0 || Blue != 0) {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxBLACK));
        } else {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

void Simulator::setPixelFast(byte Index, PixelType Pixel)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if(Index >= WS2812_NUMBER_OF_LEDS ) {
        // set breakpoint to find index out of bounce calls
    }

    if(Pixel.Red != 0 || Pixel.Green != 0 || Pixel.Blue != 0) {
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxBLACK));
    } else {
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
    }
}

void Simulator::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if(Index >= WS2812_NUMBER_OF_LEDS ) {
        // set breakpoint to find index out of bounce calls
    }

    if(Red != 0 || Green != 0 || Blue != 0) {
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxBLACK));
    } else {
        Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
    }
}

void Simulator::clearPixels()
{
    setAllPixels(wxColour(*wxLIGHT_GREY));
}

void Simulator::setAllPixels(wxColor Color)
{
    for(unsigned int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            Characters[Row][Column]->SetForegroundColour(Color);
        }
    }
}

void Simulator::setBrightness(byte sBrightness, bool GammaCorrection)
{
    Brightness = sBrightness;

    if(Brightness == 0)
    {
        for(unsigned int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
            for(unsigned int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
                Colors[Row][Column] = Characters[Row][Column]->GetForegroundColour();
            }
        }
        setAllPixels(wxColour(*wxLIGHT_GREY));
    } else {
        for(unsigned int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
            for(unsigned int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
                Characters[Row][Column]->SetForegroundColour(Colors[Row][Column]);
            }
        }
    }
}

