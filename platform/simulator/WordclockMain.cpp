#include "WordclockMain.h"

#include "sim/RealTimeClock.h"

namespace {

ClockDateTime getSystemDateTime()
{
    wxDateTime wxDT = wxDateTime::Now();
    ClockDateTime dateTime;

    dateTime.setDateYear(wxDT.GetYear());
    dateTime.setDateMonth(wxDT.GetMonth() + 1);   // wxDateTime months are 0-based (Jan=0); ClockDate expects 1-12
    dateTime.setDateDay(wxDT.GetDay());
    dateTime.setTimeHour(wxDT.GetHour());
    dateTime.setTimeMinute(wxDT.GetMinute());
    dateTime.setTimeSecond(wxDT.GetSecond());
    return dateTime;
}

} // namespace

WordclockMain::WordclockMain()
{
    RealTimeClock::getInstance().setDateTime(getSystemDateTime());
}

WordclockMain::~WordclockMain()
{

}

void WordclockMain::task()
{
    RealTimeClock::getInstance().task();
    wcScheduler.task();
}
