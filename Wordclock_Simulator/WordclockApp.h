/***************************************************************
 * Name:      WordclockApp.h
 * Purpose:   Defines Application Class
 * Author:     ()
 * Created:   2017-07-14
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCKAPP_H
#define WORDCLOCKAPP_H

#include <wx/app.h>
#include <wx/timer.h>
#include "WordclockMain.h"

class WordclockApp : public wxApp
{
    public:
        WordclockApp();
        virtual bool OnInit();
        void OnTimer(wxTimerEvent& event);
        void OnClose(wxCloseEvent& event);

    private:
        WordclockMain Wordclock;
        wxTimer Timer;
        wxDECLARE_EVENT_TABLE();

        enum {
            TIMER_ID = 1500
        };
};

#endif // WORDCLOCKAPP_H
