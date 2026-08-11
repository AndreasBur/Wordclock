#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H


#include "Clock.h"
#include "Display.h"
#include "Animations.h"
#include "Transformation.h"
#include "Communication.h"
#include "Scheduler.h"

class WordclockMain
{
    public:
        WordclockMain();
        ~WordclockMain();

        void task();

    protected:

    private:
        //Communication wcCommunication;
        Scheduler wcScheduler;
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
