#include "WordclockMain.h"

#include "RealTimeClock.h"

WordclockMain::WordclockMain()
{
    //Timer.Start(1000);
    strcpy(Text, "Andreas");
    //Timer.Start(100);
    Time = wxDateTime::Now();
    //int Hour = Time.GetHour();
    //int Minute = Time.GetMinute();
    //Clock::getInstance().setTime(10, 15);
    //Display::getInstance().setWord(DisplayWords::WORD_UHR);
    //Display::getInstance().setWord(DisplayWords::WORD_HOUR_ACHT);
    //Clock::getInstance().setTime(Hour, Minute);

    //WcDisplay.setPixelRowFast(5, 0xFFFF);
    //Display::getInstance().show();

    //WcAnimation.setChar(0,0,'�', AnimationFont::FONT_5X8);
    //WcAnimation.setChar(0,0,'B', AnimationFont::FONT_5X8);
    //WcAnimation.setCharWithShift('A', AnimationFont::FONT_10X10);
    //Text::getInstance().setTextWithShift(Text, Text::FONT_7X10);
    //WcDisplay.show();

    //WcTransformation.shiftRightFast(true);
    //WcTransformation.shiftRight(true);
    //WcTransformation.shiftUp(false);
    //WcTransformation.shiftDown(true);
    //Animations::getInstance().setAnimation(Animations::ANIMATION_EXPLODE);
    //Animations::getInstance().setTime(Hour, Minute);
    //WcClock.show();
}

WordclockMain::~WordclockMain()
{

}

void WordclockMain::task()
{
    wxDateTime wxDT = wxDateTime::Now();
    ClockDateTime dateTime;

    dateTime.setDateYear(wxDT.GetYear());
    dateTime.setDateMonth(wxDT.GetMonth());
    dateTime.setDateDay(wxDT.GetDay());
    dateTime.setTimeHour(wxDT.GetHour());
    dateTime.setTimeMinute(wxDT.GetMinute());
    dateTime.setTimeSecond(wxDT.GetSecond());

    RealTimeClock::getInstance().setDateTime(dateTime);



    wcScheduler.task();

     //Clock::getInstance().getClockWords(Hour, Minute, NewTimeWords);

    //if(NewTimeWords != CurrentTimeWords) {
        //CurrentTimeWords = NewTimeWords;
        //WcDisplay.clearAllWords();
        //WcDisplay.clear();
        //WcClock.setClock(Hour, Minute);
        //WcClock.show();
    //}
}
