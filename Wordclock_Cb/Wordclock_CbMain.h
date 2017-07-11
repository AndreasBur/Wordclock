/***************************************************************
 * Name:      Wordclock_CbMain.h
 * Purpose:   Defines Application Frame
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCK_CBMAIN_H
#define WORDCLOCK_CBMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "Wordclock_CbApp.h"


#include <wx/button.h>
#include <wx/statline.h>

#define DISPLAY_NUMBER_OF_ROWS                  10
#define DISPLAY_NUMBER_OF_COLUMNS               11

class Wordclock_CbDialog: public wxDialog
{
    public:
        Wordclock_CbDialog(wxDialog *dlg, const wxString& title);
        ~Wordclock_CbDialog();

    protected:
        enum
        {
            idBtnQuit = 1000,
            idBtnAbout
        };
        wxStaticText* Characters[10][11];
        wxStaticText* m_staticText1;

        wxButton* BtnAbout;
        wxStaticLine* m_staticline1;
        wxButton* BtnQuit;

    private:
        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()
};

#endif // WORDCLOCK_CBMAIN_H
