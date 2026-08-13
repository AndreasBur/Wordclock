/* Definitions for the framework stand-ins in stubs/, so the backend links on the host.
   Only the objects and calls that no test wants to observe live here; a test that needs to
   see what the backend handed over defines that call itself. */
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <Wire.h>
#include <freertos/task.h>

TwoWire Wire;
MDNSStub MDNS;
WiFiStub WiFi;

TickType_t xTaskGetTickCount() { return 0; }
void vTaskDelayUntil(TickType_t*, TickType_t) { }
