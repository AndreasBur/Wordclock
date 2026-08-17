#ifndef _WIFI_H_
#define _WIFI_H_
#include <string>
enum { WIFI_STA, WIFI_AP };
enum { WL_CONNECTED, WL_IDLE_STATUS };
struct IPAddressStub { std::string toString() const { return ""; } };
/* No setAutoReconnect and no reconnect(): this core has neither, which is why the backend
   joins again instead. The stub leaves them out rather than accepting them silently, so
   that a call added back by mistake fails to compile here as it would on the target. */
struct WiFiStub {
    void defaultLowPowerMode() { LowPower = true; }
    void noLowPowerMode() { LowPower = false; }
    void begin(const char* Ssid, const char*) { Joined = (Ssid == nullptr) ? "" : Ssid; }
    int status() { return WL_IDLE_STATUS; }
    IPAddressStub localIP() { return {}; }
    int RSSI() { return 0; }

    /* What the test looks at to tell the two ways of coming up apart. */
    std::string AccessPoint;
    std::string Joined;
    bool LowPower = false;
    int Mode = 0;
    void mode(int m) { Mode = m; }
    bool softAP(const char* Ssid) { AccessPoint = Ssid; return true; }
};
extern WiFiStub WiFi;
#endif
