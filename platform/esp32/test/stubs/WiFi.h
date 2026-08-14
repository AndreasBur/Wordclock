#ifndef _WIFI_H_
#define _WIFI_H_
#include <string>
enum { WIFI_STA, WIFI_AP };
enum { WL_CONNECTED, WL_IDLE_STATUS };
struct IPAddressStub { std::string toString() const { return ""; } };
struct WiFiStub {
    void setAutoReconnect(bool) {}
    void setSleep(bool) {}
    void begin(const char*, const char*) {}
    int status() { return WL_IDLE_STATUS; }
    IPAddressStub localIP() { return {}; }
    int RSSI() { return 0; }
    bool reconnect() { return false; }
    /* What the test looks at to tell the two ways of coming up apart. */
    std::string AccessPoint;
    int Mode = 0;
    void mode(int m) { Mode = m; }
    bool softAP(const char* Ssid) { AccessPoint = Ssid; return true; }
};
extern WiFiStub WiFi;
#endif
