/***************************************************************
 * Name:      WordclockMain.h
 * Purpose:   Defines Application Frame
 * Author:     ()
 * Created:   2017-07-14
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "Clock.h"
#include "Display.h"
#include "Animation.h"
#include "AnimationClock.h"
#include "Transformation.h"

class WordclockMain
{
    public:
        WordclockMain();
        ~WordclockMain();

        void task();

    protected:

    private:
        //Display WcDisplay;
        //Clock WcClock;
        //Animation WcAnimation;
        //Transformation WcTransformation;
        wxDateTime Time;

        char Text[25];
        ClockWords CurrentTimeWords;
        enum {
            TIMER_ID = 1500
        };
};

#endif // WORDCLOCKMAIN_H
