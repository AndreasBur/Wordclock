/***************************************************************
 * Name:      WordclockApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2017-07-14
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "WordclockApp.h"

IMPLEMENT_APP(WordclockApp);

wxBEGIN_EVENT_TABLE(WordclockApp, wxApp)
    EVT_CLOSE(WordclockApp::OnClose)
    EVT_TIMER(TIMER_ID, WordclockApp::OnTimer)
wxEND_EVENT_TABLE()

WordclockApp::WordclockApp() : Timer(this, TIMER_ID)
{
    Timer.Start(1000);
}

bool WordclockApp::OnInit()
{
    Simulator::getInstance().Show();
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
