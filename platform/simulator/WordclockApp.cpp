#include "WordclockApp.h"
#include "sim/PixelsFrame.h"
#include "sim/WebHost.h"

#include "WebFrontend.h"

IMPLEMENT_APP(WordclockApp);

wxBEGIN_EVENT_TABLE(WordclockApp, wxApp)
    EVT_CLOSE(WordclockApp::OnClose)
    EVT_TIMER(kTimerId, WordclockApp::OnTimer)
wxEND_EVENT_TABLE()

WordclockApp::WordclockApp() : timer_(this, kTimerId)
{
    timer_.Start(Scheduler::GetTaskIntervalMs());
}

bool WordclockApp::OnInit()
{
    PixelsFrame::GetInstance().Show();
    /* After the window, so the address it prints has somewhere to be printed. A port that is
       taken is not fatal - the window is still worth having, which is what Begin() answers. */
    WebHost::GetInstance().Begin();
    return true;
}

void WordclockApp::OnClose(wxCloseEvent &event)
{
    timer_.Stop();
    WebHost::GetInstance().End();
    UNUSED(event);
}

void WordclockApp::OnTimer(wxTimerEvent& event)
{
    /* Before the firmware, so a command that arrived since the last tick is in the port by
       the time the parser reads it - one tick earlier than taking it afterwards would be. */
    WebHost::GetInstance().Task();

    wordclock_.Task();
    /* After the firmware, not from inside its pixel writes: the window shows whatever
       the buffer holds once the whole pass over the tasks is done. */
    PixelsFrame::GetInstance().Render();
    /* And after the window, on the same buffer it just drew, so both front ends show the
       frame the tick produced rather than one of them showing the previous one. */
    WebFrontend::GetInstance().BroadcastFrame();
    UNUSED(event);
}
