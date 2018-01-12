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


WordclockDialog::WordclockDialog(wxDialog *dlg, const wxString &title) : wxDialog(dlg, -1, title), WcDisplay(255, 255, 255), WcClock(&WcDisplay, Clock::MODE_WESSI), WcAnimation(&WcDisplay, &WcClock), WcTransformation(&WcDisplay), Timer(this, TIMER_ID)
{
    //Timer.Start(1000 * 2);
    Timer.Start(300);
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    WcClock.setClock(Hour, Minute);

    //WcDisplay.setPixelRowFast(5, 0xFFFF);
    WcDisplay.show();

    //WcAnimation.setChar(0,0,219, ANIMATION_FONT_6X10);
    //WcAnimation.setChar(0,0,'A', ANIMATION_FONT_6X10);

    //WcTransformation.shiftRightFast(true);
    //WcTransformation.shiftRight(true);
    //WcTransformation.shiftUp(false);
    //WcTransformation.shiftDown(true);
    WcAnimation.setAnimation(Animation::ANIMATION_WIPE);
    WcAnimation.setClock(12, 00);
    //WcClock.show();
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
    Clock::ClockWordsType NewTimeWords;
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();

    WcAnimation.task();
    WcAnimation.show();

    WcClock.getClockWords(Hour, Minute, &NewTimeWords);

    if(NewTimeWords != CurrentTimeWords) {
        CurrentTimeWords = NewTimeWords;
        //WcDisplay.clearAllWords();
        //WcDisplay.clear();
        //WcClock.setClock(Hour, Minute);
        //WcClock.show();
    }
}
