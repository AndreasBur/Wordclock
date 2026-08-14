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
 *      \brief      Entry point of the ESP32 application
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
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* The framework headers come first, ahead of this platform's Arduino.h: that one binds
   Serial to WordclockSerial, and the macro would otherwise reach into them. */
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Arduino.h"

#include "Pixels.h"
#include "RealTimeClock.h"
#include "Scheduler.h"
#include "System.h"
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
const TickType_t TaskInterval{pdMS_TO_TICKS(Scheduler::getTaskIntervalMs())};
TickType_t LastWakeTime{0};

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* Installs the zone rule together with the servers, which is what makes every conversion
   in RealTimeClock local time. Safe to do before the link is up: the request is repeated
   until an answer arrives. */
void startTimeSync()
{
    configTzTime(WORDCLOCK_TIMEZONE, WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
}

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
        Serial.print(F("WiFi: connected, address "));
        Serial.println(WiFi.localIP().toString().c_str());
    }

    if(!TimeReported && RealTimeClock::isSystemTimeSet()) {
        TimeReported = true;
        Serial.println(F("Time: synchronised"));
    }
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
    startTimeSync();
    /* Before the network is up on purpose: the server listens on whatever address arrives
       later, and starting it here keeps the order in setup() the same whether there is a
       WiFi or not. */
    WebInterface::getInstance().begin();

    LastWakeTime = xTaskGetTickCount();
} /* setup */


/******************************************************************************************************************************************************
  loop()
******************************************************************************************************************************************************/
/*! \brief          Drives the firmware at the scheduler's interval
 *  \details        Timed against an absolute wake-up rather than a delay, so the work
 *                  inside a tick does not stretch the interval: animation speeds, the
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

    vTaskDelayUntil(&LastWakeTime, TaskInterval);
} /* loop */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
