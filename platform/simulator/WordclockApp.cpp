#include "WordclockApp.h"
#include "sim/PixelsFrame.h"
#include "sim/WebHost.h"

#include "WebFrontend.h"

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
    /* After the window, so the address it prints has somewhere to be printed. A port that is
       taken is not fatal - the window is still worth having, which is what begin() answers. */
    WebHost::getInstance().begin();
    return true;
}

void WordclockApp::OnClose(wxCloseEvent &event)
{
    Timer.Stop();
    WebHost::getInstance().end();
    UNUSED(event);
}

void WordclockApp::OnTimer(wxTimerEvent& event)
{
    /* Before the firmware, so a command that arrived since the last tick is in the port by
       the time the parser reads it - one tick earlier than taking it afterwards would be. */
    WebHost::getInstance().task();

    Wordclock.task();
    /* After the firmware, not from inside its pixel writes: the window shows whatever
       the buffer holds once the whole pass over the tasks is done. */
    PixelsFrame::getInstance().render();
    /* And after the window, on the same buffer it just drew, so both front ends show the
       frame the tick produced rather than one of them showing the previous one. */
    WebFrontend::getInstance().broadcastFrame();
    UNUSED(event);
}
