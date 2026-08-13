#include "WordclockApp.h"
#include "sim/PixelsFrame.h"

IMPLEMENT_APP(WordclockApp);

wxBEGIN_EVENT_TABLE(WordclockApp, wxApp)
    EVT_CLOSE(WordclockApp::OnClose)
    EVT_TIMER(TIMER_ID, WordclockApp::OnTimer)
wxEND_EVENT_TABLE()

WordclockApp::WordclockApp() : Timer(this, TIMER_ID)
{
    Timer.Start(Scheduler::getTaskIntervalMs());
}

bool WordclockApp::OnInit()
{
    PixelsFrame::getInstance().Show();
    return true;
}

void WordclockApp::OnClose(wxCloseEvent &event)
{
    Timer.Stop();
    UNUSED(event);
}

void WordclockApp::OnTimer(wxTimerEvent& event)
{
    Wordclock.task();
    /* After the firmware, not from inside its pixel writes: the window shows whatever
       the buffer holds once the whole pass over the tasks is done. */
    PixelsFrame::getInstance().render();
    UNUSED(event);
}
