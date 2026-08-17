/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       main.cpp
 *      \brief      Entry point of the RP2350 application
 *
 *      \details    The counterpart of the simulator's WordclockApp: it starts the network
 *                  and the time synchronisation, then ticks the firmware at the scheduler's
 *                  interval and hands the finished frame to the strip.
 *
 *                  Nothing here blocks waiting for the network. The clock has to come up
 *                  and answer commands whether or not there is a WiFi to join, so the
 *                  connection and the first SNTP answer are simply awaited in the
 *                  background while the firmware already runs.
 *
 *                  The tick is kept against a running deadline rather than an RTOS's
 *                  absolute wake-up, because this core runs loop() on the bare metal. The
 *                  arithmetic below does what vTaskDelayUntil() does on the ESP32, and for
 *                  the same reason.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* The framework headers come first, ahead of this platform's Arduino.h: that one binds
   Serial to WordclockSerial, and the macro would otherwise reach into them. */
#include <WiFi.h>

#include "Arduino.h"

#include "Pixels.h"
#include "RealTimeClock.h"
#include "Scheduler.h"
#include "System.h"
#include "TimeSync.h"
#include "WebInterface.h"
#include "WordclockConfiguration.h"
#include "WordclockMain.h"

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

WordclockMain Wordclock;

/* The tick the whole firmware is timed against: every module's task cycle counts in this
   unit, so it is read from the scheduler rather than repeated as a number here. */
const uint32_t TaskIntervalMs{Scheduler::getTaskIntervalMs()};
uint32_t NextTickMs{0u};

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* What the clock came up as, said once: the network it is joining, or the one it opened
   because it has none. System does not print this itself - it also runs from the command
   that changes the credentials, and a line printed there would land inside that command's
   answer. */
void reportNetwork()
{
    char Ssid[System::SsidStringLength]{};

    if(System::getInstance().getNetworkSsid(Ssid) == E_OK) {
        Serial.print(F("WiFi: joining "));
        Serial.println(Ssid);
    } else {
        Serial.print(F("WiFi: no network configured, console on the access point "));
        Serial.println(SYSTEM_ACCESS_POINT_SSID);
    }
}

/* Reported from the tick rather than from setup(), because both of these finish long after
   setup() has returned. Until the second one appears the display holds its default date,
   which is the one thing a first bring-up needs to be able to tell apart from a dead
   strip. */
void reportProgress()
{
    static bool ConnectionReported{false};
    static bool TimeReported{false};

    if(!ConnectionReported && (WiFi.status() == WL_CONNECTED)) {
        ConnectionReported = true;
        /* Here rather than in setup(): the server names are resolved when this is called,
           so before the link there is nothing to resolve them over. */
        WordclockTime::startTimeServers();
        Serial.print(F("WiFi: connected, address "));
        Serial.println(WiFi.localIP().toString().c_str());
    }

    if(!TimeReported && RealTimeClock::isSystemTimeSet()) {
        TimeReported = true;
        Serial.println(F("Time: synchronised"));
    }
}

/* Sleeps until the next tick is due, and gives up on the schedule rather than chasing it
   when a tick overran. Chasing would run the following ticks back to back to catch up,
   which is the one thing the fixed interval exists to prevent: animation speeds and the
   sensor's sampling rate all count in ticks, and a burst of them plays an animation at
   several times its speed.
 *
 * The difference is taken as a signed value so that the millisecond counter's wrap, which
 * a clock reaches after seven weeks, passes without a tick that waits for another seven. */
void waitForNextTick()
{
    NextTickMs += TaskIntervalMs;

    const int32_t RemainingMs = static_cast<int32_t>(NextTickMs - millis());

    if(RemainingMs > 0) { delay(static_cast<uint32_t>(RemainingMs)); }
    else                { NextTickMs = millis(); }
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  setup()
******************************************************************************************************************************************************/
void setup()
{
    Serial.begin(WORDCLOCK_SERIAL_BAUDRATE);

    Wordclock.init();

    /* Which network, and whether there is one at all, is System's answer now: what was
       stored over the console, what was compiled in, or - when neither - an access point
       of its own, so that the console the credentials are entered through can be reached
       without a cable. */
    System::getInstance().startNetwork();
    reportNetwork();
    /* The zone rule only, which is what makes every conversion in RealTimeClock local time.
       The servers cannot follow it here as they do on the ESP32 - see TimeSync.h - so they
       are started from the tick, at the moment the link comes up. */
    WordclockTime::applyTimeZone();
    /* Before the network is up on purpose: the server listens on whatever address arrives
       later, and starting it here keeps the order in setup() the same whether there is a
       WiFi or not. */
    WebInterface::getInstance().begin();

    NextTickMs = millis();
} /* setup */


/******************************************************************************************************************************************************
  loop()
******************************************************************************************************************************************************/
/*! \brief          Drives the firmware at the scheduler's interval
 *  \details        Timed against a running deadline rather than a delay of the interval,
 *                  so the work inside a tick does not stretch it: animation speeds, the
 *                  sensor's sampling rate and the serial poll rate all count in this
 *                  interval, and a drifting one silently rescales all three.
******************************************************************************************************************************************************/
void loop()
{
    Wordclock.task();

    /* After the firmware, not from inside its pixel writes: the strip gets whatever the
       buffer holds once the whole pass over the tasks is done. */
    Pixels::getInstance().render();

    /* After the strip, so a watching browser is shown the frame that went out rather than
       one the firmware is still assembling. */
    WebInterface::getInstance().broadcastFrame();

    reportProgress();

    /* Last, so a restart asked for by a command takes place once that command's answer has
       reached both the wire and the browser. */
    System::getInstance().performPendingRestart();

    waitForNextTick();
} /* loop */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
