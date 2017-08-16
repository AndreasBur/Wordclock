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
    EVT_CLOSE(WordclockDialog::OnClose)
    EVT_TIMER(TIMER_ID, WordclockDialog::OnTimer)
wxEND_EVENT_TABLE()


WordclockDialog::WordclockDialog(wxDialog *dlg, const wxString &title) : wxDialog(dlg, -1, title), WcDisplay(255, 255, 255), WcClock(&WcDisplay, CLOCK_MODE_WESSI), WcAnimation(&WcDisplay), WcTransformation(&WcDisplay), Timer(this, TIMER_ID)
{
    Timer.Start(1000 * 1);
    //Timer.Start(10);
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    WcClock.show(Hour, Minute);
    //WcDisplay.setPixelRowFast(5, 0xFFFF);
    //WcDisplay.show();

    //WcAnimation.setChar(0,0,219, ANIMATION_FONT_6X10);
    //WcAnimation.setChar(0,0,'A', ANIMATION_FONT_6X10);

    //WcTransformation.shiftLeft(true);
    //WcTransformation.shiftRight(true);
    //WcTransformation.shiftUp(false);
    //WcTransformation.shiftDown(true);
    WcDisplay.show();
}


WordclockDialog::~WordclockDialog()
{

}

void WordclockDialog::OnClose(wxCloseEvent &event)
{
    Timer.Stop();
    Destroy();
}

void WordclockDialog::OnTimer(wxTimerEvent& event)
{
    ClockWordsType NewTimeWords;
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();

    WcClock.getClockWords(Hour, Minute, &NewTimeWords);

    if( NewTimeWords.HourWords[0] != CurrentTimeWords.HourWords[0]      ||
        NewTimeWords.HourWords[1] != CurrentTimeWords.HourWords[1]      ||
        NewTimeWords.MinuteWords[0] != CurrentTimeWords.MinuteWords[0]  ||
        NewTimeWords.MinuteWords[1] != CurrentTimeWords.MinuteWords[1]  ||
        NewTimeWords.MinuteWords[2] != CurrentTimeWords.MinuteWords[2]  )
    {
        CurrentTimeWords = NewTimeWords;
        WcDisplay.clearAllWords();
        //WcDisplay.clear();
        WcClock.show(Hour, Minute);
        WcDisplay.show();
    }
}
