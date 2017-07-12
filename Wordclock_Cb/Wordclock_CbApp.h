/***************************************************************
 * Name:      Wordclock_CbApp.h
 * Purpose:   Defines Application Class
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCK_CBAPP_H
#define WORDCLOCK_CBAPP_H

#include <wx/app.h>

//class Display;
//class Clock;

class Wordclock_CbApp : public wxApp
{
    private:
        //Display* WcDisplay;
        //Clock* WcClock;


    public:
        virtual bool OnInit();
};

#endif // WORDCLOCK_CBAPP_H
