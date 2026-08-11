#ifndef WORDCLOCKMAIN_H
#define WORDCLOCKMAIN_H


#include "Scheduler.h"

/* Adapts the wxWidgets side to the firmware: it hands the host's time to the real time
   clock and drives the scheduler. What reaches the display is the DisplayManager's
   decision, so nothing of that lives here. */
class WordclockMain
{
    public:
        WordclockMain();
        ~WordclockMain();

        void task();

    protected:

    private:
        Scheduler wcScheduler;
        enum {
            TIMER_ID = 1500
        };
};

#endif // WORDCLOCKMAIN_H
