#ifndef WORDCLOCKAPP_H
#define WORDCLOCKAPP_H

#include <wx/app.h>
#include <wx/timer.h>
#include "WordclockMain.h"

class WordclockApp : public wxApp
{
    public:
        WordclockApp();
        bool OnInit() override;
        void OnTimer(wxTimerEvent& event);
        void OnClose(wxCloseEvent& event);

    private:
        WordclockMain wordclock_;
        wxTimer timer_;
        wxDECLARE_EVENT_TABLE();

        enum {
            kTimerId = 1500
        };
};

#endif // WORDCLOCKAPP_H
