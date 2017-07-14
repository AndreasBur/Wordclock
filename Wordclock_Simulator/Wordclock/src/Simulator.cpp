#include "Simulator.h"
#include "WS2812.h"

const char DisplayCharacters[][DISPLAY_NUMBER_OF_COLUMNS + 1] =
{
    "ESKISTLFÜNF",
    "ZEHNZWANZIG",
    "DREIVIERTEL",
    "TGNACHVORJM",
    "HALBQZWÖLFP",
    "ZWEINSIEBEN",
    "KDREIRHFÜNF",
    "ELFNEUNVIER",
    "WACHTZEHNRS",
    "BSECHSFMUHR"
};


BEGIN_EVENT_TABLE(Simulator, wxDialog)
    EVT_CLOSE(Simulator::OnClose)
    EVT_BUTTON(idBtnQuit, Simulator::OnQuit)
    EVT_BUTTON(idBtnAbout, Simulator::OnAbout)
END_EVENT_TABLE()


Simulator::Simulator(wxDialog *dlg, const wxString &title) : wxDialog(dlg, -1, title)
{
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    wxBoxSizer* SizerCharacters[DISPLAY_NUMBER_OF_ROWS];
    wxBoxSizer* SizerAll;
    SizerAll = new wxBoxSizer(wxVERTICAL);

    for(int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {

        SizerCharacters[Row] = new wxBoxSizer(wxHORIZONTAL);

        for(int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            Characters[Row][Column] = new wxStaticText(this, wxID_ANY, DisplayCharacters[Row][Column], wxDefaultPosition, wxDefaultSize, 0);
            //Characters[Row][Column]->SetFont(wxFont(40, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
            Characters[Row][Column]->SetFont(wxFont(wxSize(40,40), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false));
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
            SizerCharacters[Row]->Add(Characters[Row][Column], 0, wxALL|wxEXPAND, 5);
        }
        SizerAll->Add(SizerCharacters[Row], 1, wxEXPAND, 5);
    }

    wxBoxSizer* SizerButton = new wxBoxSizer(wxHORIZONTAL);
    BtnAbout = new wxButton(this, idBtnAbout, wxT("&About"), wxDefaultPosition, wxDefaultSize, 0);
    BtnQuit = new wxButton(this, idBtnQuit, wxT("&Quit"), wxDefaultPosition, wxDefaultSize, 0);
    //m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);

    SizerButton->Add(BtnAbout, 1, wxALL|wxEXPAND, 10);
    SizerButton->AddSpacer(250);
    SizerButton->Add(BtnQuit, 1, wxALL|wxEXPAND, 10);

    SizerAll->Add(SizerButton, 1, wxEXPAND, 5);

    this->SetSizer(SizerAll);
    this->Layout();
    SizerAll->Fit(this);
}

Simulator::~Simulator()
{
    //dtor
}

void Simulator::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void Simulator::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void Simulator::OnAbout(wxCommandEvent &event)
{

}

stdReturnType Simulator::getPixel(byte Index, WS2812PixelType* Pixel)
{
    byte Row = Index / DISPLAY_NUMBER_OF_COLUMNS;
    byte Column = Index % DISPLAY_NUMBER_OF_COLUMNS;

    if (Index < WS2812_NUMBER_OF_LEDS) {
        if(Characters[Row][Column]->GetForegroundColour() == wxColor(*wxBLACK)) {
            Pixel->Blue = 255;
            Pixel->Green = 255;
            Pixel->Red = 255;
        } else {
            Pixel->Blue = 0;
            Pixel->Green = 0;
            Pixel->Red = 0;
        }
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}

stdReturnType Simulator::setPixel(byte Index, WS2812PixelType Pixel)
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

stdReturnType Simulator::setPixel(byte Index, byte Red, byte Green, byte Blue)
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

void Simulator::clearAllPixels()
{
    for(int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        }
    }
}
