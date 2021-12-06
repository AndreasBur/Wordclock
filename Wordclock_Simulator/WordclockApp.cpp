#include "WordclockApp.h"

IMPLEMENT_APP(WordclockApp);

wxBEGIN_EVENT_TABLE(WordclockApp, wxApp)
    EVT_CLOSE(WordclockApp::OnClose)
    EVT_TIMER(TIMER_ID, WordclockApp::OnTimer)
wxEND_EVENT_TABLE()

WordclockApp::WordclockApp() : Timer(this, TIMER_ID)
{
    Timer.Start(50);
}

bool WordclockApp::OnInit()
{
    Pixels::getInstance().Show();
    return true;
}

void WordclockApp::OnClose(wxCloseEvent &event)
{
    Timer.Stop();
}

void WordclockApp::OnTimer(wxTimerEvent& event)
{
    Wordclock.task();
}
