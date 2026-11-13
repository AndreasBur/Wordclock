// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_WIFI_H_
#define WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_WIFI_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <string>
enum { WIFI_STA, WIFI_AP };
enum { WL_CONNECTED, WL_IDLE_STATUS };
struct IPAddressStub {
  std::string toString() const { return ""; }
};
struct WiFiStub {
  void setAutoReconnect(bool) {}
  void setSleep(bool) {}
  void begin(char const* Ssid, char const* Password) {
    Joined = Ssid;
    JoinedWith = Password;
  }
  int status() { return WL_IDLE_STATUS; }
  IPAddressStub localIP() { return {}; }
  int RSSI() { return 0; }
  bool reconnect() {
    Reconnects++;
    return true;
  }
  /* What the test looks at to tell the two ways of coming up apart. */
  std::string AccessPoint;
  std::string Joined;
  std::string JoinedWith;
  int Reconnects = 0;
  int Mode = 0;
  void mode(int m) { Mode = m; }
  bool softAP(char const* Ssid) {
    AccessPoint = Ssid;
    return true;
  }
};
extern WiFiStub WiFi;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_WIFI_H_
