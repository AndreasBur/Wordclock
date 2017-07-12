/***************************************************************
 * Name:      Wordclock_CbMain.cpp
 * Purpose:   Code for Application Frame
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "Wordclock_CbMain.h"
#include "WS2812.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

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


BEGIN_EVENT_TABLE(Wordclock_CbDialog, wxDialog)
    EVT_CLOSE(Wordclock_CbDialog::OnClose)
    EVT_BUTTON(idBtnQuit, Wordclock_CbDialog::OnQuit)
    EVT_BUTTON(idBtnAbout, Wordclock_CbDialog::OnAbout)
END_EVENT_TABLE()

Wordclock_CbDialog::Wordclock_CbDialog(wxDialog *dlg, const wxString &title)
    : wxDialog(dlg, -1, title)
{    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
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


Wordclock_CbDialog::~Wordclock_CbDialog()
{
}

void Wordclock_CbDialog::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void Wordclock_CbDialog::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void Wordclock_CbDialog::OnAbout(wxCommandEvent &event)
{

}

stdReturnType Wordclock_CbDialog::getPixel(byte Index, WS2812PixelType* Pixel)
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

stdReturnType Wordclock_CbDialog::setPixel(byte Index, WS2812PixelType Pixel)
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

stdReturnType Wordclock_CbDialog::setPixel(byte Index, byte Red, byte Green, byte Blue)
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

void Wordclock_CbDialog::clearAllPixels()
{
    for(int Row = 0; Row < DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(int Column = 0; Column < DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            Characters[Row][Column]->SetForegroundColour(wxColour(*wxLIGHT_GREY));
        }
    }
}
