/***************************************************************
 * Name:      Wordclock_CbApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "Wordclock_CbApp.h"
#include "Wordclock_CbMain.h"

#include "Clock.h"
#include "Display.h"
#include "Animation.h"

IMPLEMENT_APP(Wordclock_CbApp);

bool Wordclock_CbApp::OnInit()
{
    Display* WcDisplay = new Display(255,255,255);

    Clock* WcClock = new Clock(WcDisplay, CLOCK_MODE_WESSI);

    wxDateTime Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();

    //WcClock->show(Hour, Minute);
    //WcDisplay->clear();

    Animation* WcAnimation = new Animation(WcDisplay);


    WcAnimation->setChar(0, 0, 219, ANIMATION_FONT_6X8);
    //WcAnimation->setChar(0, 0, '2', ANIMATION_FONT_6X10);

    WcDisplay->show();




    //Wordclock_CbDialog* dlg = new Wordclock_CbDialog(0L, _("wxWidgets Application Template"));
    //dlg->SetIcon(wxICON(aaaa)); // To Set App Icon
    //dlg->Show();
    return true;
}
