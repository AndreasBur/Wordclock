#include "WordclockMain.h"

#include "sim/RealTimeClock.h"

WordclockMain::WordclockMain()
{

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
}
