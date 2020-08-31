#include "WordclockMain.h"

WordclockMain::WordclockMain()
{
    //Timer.Start(1000);
    strcpy(Text, "geht gut GUT");
    //Timer.Start(100);
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();
    //Clock::getInstance().setTime(10, 15);
    //Display::getInstance().setWord(DisplayWords::WORD_UHR);
    //Display::getInstance().setWord(DisplayWords::WORD_HOUR_ACHT);
    Clock::getInstance().setTime(Hour, Minute);

    //WcDisplay.setPixelRowFast(5, 0xFFFF);
    //Display::getInstance().show();

    //WcAnimation.setChar(0,0,'Ö', AnimationFont::FONT_5X8);
    //WcAnimation.setChar(0,0,'B', AnimationFont::FONT_5X8);
    //WcAnimation.setCharWithShift('A', AnimationFont::FONT_10X10);
    //WcAnimation.setTextWithShift(Text, AnimationFont::FONT_7X10);
    //WcDisplay.show();

    //WcTransformation.shiftRightFast(true);
    //WcTransformation.shiftRight(true);
    //WcTransformation.shiftUp(false);
    //WcTransformation.shiftDown(true);
    Animation::getInstance().setAnimation(AnimationClock::ANIMATION_CLOCK_IMPLODE);
    Animation::getInstance().setTime(Hour, Minute);
    //WcClock.show();
}


WordclockMain::~WordclockMain()
{

}

void WordclockMain::task()
{
    ClockWords NewTimeWords;
    Time = wxDateTime::Now();
    int Hour = Time.GetHour();
    int Minute = Time.GetMinute();

    Animation::getInstance().task();

//    if(WcAnimation.getState() == Animation::STATE_IDLE) {
//        Time = wxDateTime::Now();
//        int Hour = Time.GetHour();
//        int Minute = Time.GetMinute();
//        //WcDisplay.clear();
    //WcAnimation.setClock(Hour, Minute);
//    }
    wcCommunication.task();

    Display::getInstance().show();

     //Clock::getInstance().getClockWords(Hour, Minute, NewTimeWords);

    //if(NewTimeWords != CurrentTimeWords) {
        //CurrentTimeWords = NewTimeWords;
        //WcDisplay.clearAllWords();
        //WcDisplay.clear();
        //WcClock.setClock(Hour, Minute);
        //WcClock.show();
    //}
}
