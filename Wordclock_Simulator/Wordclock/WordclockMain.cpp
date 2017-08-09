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


WordclockDialog::WordclockDialog(wxDialog *dlg, const wxString &title) : wxDialog(dlg, -1, title), WcDisplay(255, 255, 255), WcClock(&WcDisplay, CLOCK_MODE_WESSI), WcAnimation(&WcDisplay), WcTransformation(&WcDisplay), Timer(this, TIMER_ID)
{
    Timer.Start(1000 * 5);
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

void WordclockDialog::OnTimer(wxTimerEvent& event)
{
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    WcDisplay.clearAllWords();
    //WcDisplay.clear();
    WcClock.show(Hour, Minute);
    WcDisplay.show();
}
