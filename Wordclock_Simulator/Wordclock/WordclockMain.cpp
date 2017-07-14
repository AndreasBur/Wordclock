/***************************************************************
 * Name:      WordclockMain.cpp
 * Purpose:   Code for Application Frame
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

#include "WordclockMain.h"

wxBEGIN_EVENT_TABLE(WordclockDialog, wxDialog)
    EVT_TIMER(TIMER_ID, WordclockDialog::OnTimer)
wxEND_EVENT_TABLE()


WordclockDialog::WordclockDialog() : WcDisplay(255, 255, 255), WcClock(&WcDisplay, CLOCK_MODE_WESSI), WcAnimation(&WcDisplay), Timer(this, TIMER_ID)
{
    Timer.Start(1000 * 60);
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    WcClock.show(Hour, Minute);
    WcDisplay.show();
}


WordclockDialog::~WordclockDialog()
{

}

void WordclockDialog::OnTimer(wxTimerEvent& event)
{
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    WcDisplay.clearAllWords();
    WcClock.show(Hour, Minute);
    WcDisplay.show();
}
