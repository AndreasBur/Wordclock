#include "WordclockMain.h"

#include "Persistence.h"
#include "sim/PowerSwitch.h"
#include "sim/RealTimeClock.h"

namespace {

ClockDateTime GetSystemDateTime()
{
    wxDateTime wx_dt = wxDateTime::Now();
    ClockDateTime date_time;

    date_time.SetDateYear(wx_dt.GetYear());
    date_time.SetDateMonth(wx_dt.GetMonth() + 1);   // wxDateTime months are 0-based (Jan=0); ClockDate expects 1-12
    date_time.SetDateDay(wx_dt.GetDay());
    date_time.SetTimeHour(wx_dt.GetHour());
    date_time.SetTimeMinute(wx_dt.GetMinute());
    date_time.SetTimeSecond(wx_dt.GetSecond());
    return date_time;
}

} // namespace

WordclockMain::WordclockMain()
{
    RealTimeClock::GetInstance().SetDateTime(GetSystemDateTime());
    /* Nothing to set up here - there is no port - but the call is made anyway, so that the
       one backend the others are mirrored from shows where it belongs. On hardware it is what
       brings the strip's 5 V up, and it has to come before the data line is claimed. */
    PowerSwitch::GetInstance().Init();
    /* Before the first task, so the window's first paint already shows the restored
       colour and mode rather than the defaults for a tick. */
    Persistence::GetInstance().Load();
}

WordclockMain::~WordclockMain()
{

}

void WordclockMain::Task()
{
    RealTimeClock::GetInstance().Task();
    wc_scheduler_.Task();
}
