#include "WordclockMain.h"

#include "sim/RealTimeClock.h"

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

    //WcAnimation.setChar(0,0,'\xD6', AnimationFont::FONT_5X8);
    //WcAnimation.setChar(0,0,'B', AnimationFont::FONT_5X8);
    //WcAnimation.setCharWithShift('A', AnimationFont::FONT_10X10);
    //Text::getInstance().setTextWithShift(Text, Text::FONT_7X10);
    //WcDisplay.show();

    //WcTransformation.shiftRightFast(true);
    //WcTransformation.shiftRight(true);
    //WcTransformation.shiftUp(false);
    //WcTransformation.shiftDown(true);
    //Animations::getInstance().setAnimation(Animations::ANIMATION_ID_EXPLODE_WORDS);
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
    dateTime.setDateMonth(wxDT.GetMonth() + 1);   // wxDateTime months are 0-based (Jan=0); ClockDate expects 1-12
    dateTime.setDateDay(wxDT.GetDay());
    dateTime.setTimeHour(wxDT.GetHour());
    dateTime.setTimeMinute(wxDT.GetMinute());
    dateTime.setTimeSecond(wxDT.GetSecond());

    RealTimeClock::getInstance().setDateTime(dateTime);

    wcScheduler.task();

    // Push the current time on every minute change. Without an animation the
    // clock is drawn statically (setWord() only lights letters, so the grid is
    // cleared first, then the words are set and shown). With an animation
    // selected, Animations::setTime() kicks off the transition and the
    // scheduler's Animations task steps through and shows the frames.
    ClockTime time = RealTimeClock::getInstance().getTime();
    if(time.getMinute() != LastMinute) {
        LastMinute = time.getMinute();
        Animations& animations = Animations::getInstance();
        if(animations.getAnimation() == Animations::ANIMATION_ID_NONE) {
            Display::getInstance().clear();
            Clock::getInstance().setTime(time.getHour(), time.getMinute());
            Display::getInstance().show();
        } else {
            animations.setTime(time.getHour(), time.getMinute());
        }
    }
}
