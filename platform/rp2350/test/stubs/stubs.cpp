// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Definitions for the framework stand-ins in stubs/, so the backend links on the host.
 * Only the objects and calls that no test wants to observe live here; a test that needs to
 * see what the backend handed over defines that call itself.
 */
#include <Arduino.h>
#include <LEAmDNS.h>
#include <LittleFS.h>
#include <PicoOTA.h>
#include <WiFi.h>
#include <WiFiNTP.h>
#include <Wire.h>

TwoWire Wire;
Rp2040Class rp2040;

unsigned long TestMillis = 0u;                 // NOLINT(runtime/int)
unsigned long millis() { return TestMillis; }  // NOLINT(runtime/int)
MDNSStub MDNS;
WiFiStub WiFi;
NTPClassStub NTP;
LittleFSStub LittleFS;
PicoOtaStub picoOTA;
