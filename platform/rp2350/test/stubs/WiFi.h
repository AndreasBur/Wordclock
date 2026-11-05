#ifndef _WIFI_H_
#define _WIFI_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <string>
enum { WIFI_STA, WIFI_AP };
enum { WL_CONNECTED, WL_IDLE_STATUS };
struct IPAddressStub { std::string toString() const { return ""; } };
/*
 * No setAutoReconnect and no reconnect(): this core has neither, which is why the backend
 * joins again instead. The stub leaves them out rather than accepting them silently, so
 * that a call added back by mistake fails to compile here as it would on the target.
 */
struct WiFiStub {
    void defaultLowPowerMode() { LowPower = true; }
    void noLowPowerMode() { LowPower = false; }
    void begin(const char* Ssid, const char* Password) {
        Joined = (Ssid == nullptr) ? "" : Ssid;
        JoinedWith = (Password == nullptr) ? "" : Password;
    }
    // Set by the test: arduino-pico answers WL_CONNECTED while the access point is open, too.
    int Status = WL_IDLE_STATUS;
    int status() { return Status; }
    IPAddressStub localIP() { return {}; }
    int RSSI() { return 0; }

    /* What the test looks at to tell the two ways of coming up apart. */
    std::string AccessPoint;
    std::string Joined;
    std::string JoinedWith;
    bool LowPower = false;
    int Mode = 0;
    void mode(int m) { Mode = m; }
    bool softAP(const char* Ssid) { AccessPoint = Ssid; return true; }
};
extern WiFiStub WiFi;
// NOLINTEND(readability-identifier-naming)
#endif
