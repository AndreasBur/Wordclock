/***************************************************************
 * Name:      WordclockMain.h
 * Purpose:   Defines Application Frame
 * Author:     ()
 * Created:   2017-07-14
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "WordclockApp.h"

#include "Clock.h"
#include "Display.h"
#include "Animation.h"
#include "Transformation.h"

class WordclockDialog : public wxDialog
{
    public:
        WordclockDialog(wxDialog *dlg, const wxString& title);
        ~WordclockDialog();

        void OnTimer(wxTimerEvent& event);

    protected:

    private:
        Display WcDisplay;
        Clock WcClock;
        Animation WcAnimation;
        Transformation WcTransformation;
        wxDateTime Time;
        wxTimer Timer;
        enum {
            TIMER_ID = 1500
        };
        wxDECLARE_EVENT_TABLE();
};

#endif // WORDCLOCKMAIN_H
