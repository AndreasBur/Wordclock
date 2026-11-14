// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Entry point of the ESP32 application.
 *
 * The counterpart of the simulator's WordclockApp: it starts the network and the time
 * synchronisation, then ticks the firmware at the scheduler's interval and hands the finished frame
 * to the strip.
 *
 * Nothing here blocks waiting for the network. The clock has to come up and answer commands whether
 * or not there is a WiFi to join, so the connection and the first SNTP answer are simply awaited in
 * the background while the firmware already runs.
 */

// clang-format off
// The framework headers come first, ahead of this platform's Arduino.h: that one binds
// Serial to WordclockSerial, and the macro would otherwise reach into them.
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
// clang-format on

namespace {
WordclockMain wordclock;

// The tick the whole firmware is timed against: every module's task cycle counts in this
// unit, so it is read from the scheduler rather than repeated as a number here. The C cast is
// FreeRTOS's, inside pdMS_TO_TICKS: the conversion is the kernel's own, rounding included.
// NOLINTNEXTLINE(clang-diagnostic-old-style-cast)
TickType_t const kTaskInterval{pdMS_TO_TICKS(Scheduler::GetTaskIntervalMs())};
TickType_t last_wake_time{0};

/*
 * Installs the zone rule together with the servers, which is what makes every conversion
 * in RealTimeClock local time. Safe to do before the link is up: the request is repeated
 * until an answer arrives.
 */
void StartTimeSync() {
  configTzTime(WORDCLOCK_TIMEZONE, WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
}

/*
 * What the clock came up as, said once: the network it is joining, or the one it opened
 * because it has none. System does not print this itself - it also runs from the command
 * that changes the credentials, and a line printed there would land inside that command's
 * answer.
 */
void ReportNetwork() {
  char ssid[System::kSsidStringLength]{};

  if (System::GetInstance().GetNetworkSsid(ssid) == kEOk) {
    Serial.print(F("WiFi: joining "));
    Serial.println(ssid);
  } else {
    Serial.print(F("WiFi: no network configured, console on the access point "));
    Serial.println(SYSTEM_ACCESS_POINT_SSID);
  }
}

/*
 * Reported from the tick rather than from setup(), because both of these finish long after
 * setup() has returned. Until the second one appears the display holds its default date,
 * which is the one thing a first bring-up needs to be able to tell apart from a dead
 * strip.
 */
void ReportProgress() {
  static bool connection_reported{false};
  static bool time_reported{false};

  if (!connection_reported && (WiFi.status() == WL_CONNECTED)) {
    connection_reported = true;
    Serial.print(F("WiFi: connected, address "));
    Serial.println(WiFi.localIP().toString().c_str());
  }

  if (!time_reported && RealTimeClock::IsSystemTimeSet()) {
    time_reported = true;
    Serial.println(F("Time: synchronised"));
  }
}

}  // namespace

/*!
 * \brief Brings the clock up: the serial port, the firmware, the network, the time synchronisation
 *        and the web server, in that order.
 *
 * The Arduino core calls it once, before the first loop(). It waits for none of them: the network
 * is joined, or the access point opened, and the first SNTP answer awaited in the background while
 * the firmware already runs. It says on the console which network the clock is joining, or which
 * access point it opened.
 */
void setup() {
  Serial.begin(WORDCLOCK_SERIAL_BAUDRATE);

  wordclock.Init();

  /*
   * Which network, and whether there is one at all, is System's answer now: what was
   * stored over the console, what was compiled in, or - when neither - an access point
   * of its own, so that the console the credentials are entered through can be reached
   * without a cable.
   */
  System::GetInstance().StartNetwork();
  ReportNetwork();
  StartTimeSync();
  /*
   * Before the network is up on purpose: the server listens on whatever address arrives
   * later, and starting it here keeps the order in setup() the same whether there is a
   * WiFi or not.
   */
  WebInterface::GetInstance().Begin();

  last_wake_time = xTaskGetTickCount();
}

/*!
 * \brief Runs one tick and waits for the next, one tick every Scheduler::GetTaskIntervalMs()
 *        milliseconds.
 *
 * The Arduino core calls it over and over. A tick runs the firmware's tasks, hands the buffer to
 * the strip with Pixels::Render(), has WebInterface::BroadcastFrame() send the display to watching
 * browsers, reports the network's and the time's progress on the console once each, and carries
 * out a restart that was asked for, in that order.
 *
 * Timed against an absolute wake-up rather than a delay, so the work inside a tick does not
 * stretch the interval: animation speeds, the sensor's sampling rate and the serial poll rate all
 * count in this interval, and a drifting one silently rescales all three.
 */
void loop() {
  wordclock.Task();

  // After the firmware, not from inside its pixel writes: the strip gets whatever the
  // buffer holds once the whole pass over the tasks is done.
  Pixels::GetInstance().Render();

  // After the strip, so a watching browser is shown the frame that went out rather than
  // one the firmware is still assembling.
  WebInterface::GetInstance().BroadcastFrame();

  ReportProgress();

  // Last, so a restart asked for by a command takes place once that command's answer has
  // reached both the wire and the browser.
  System::GetInstance().PerformPendingRestart();

  vTaskDelayUntil(&last_wake_time, kTaskInterval);
}
