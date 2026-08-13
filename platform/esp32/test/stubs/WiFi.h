#ifndef _WIFI_H_
#define _WIFI_H_
#include <string>
enum { WIFI_STA };
enum { WL_CONNECTED, WL_IDLE_STATUS };
struct IPAddressStub { std::string toString() const { return ""; } };
struct WiFiStub {
    void mode(int) {}
    void setAutoReconnect(bool) {}
    void setSleep(bool) {}
    void begin(const char*, const char*) {}
    int status() { return WL_IDLE_STATUS; }
    IPAddressStub localIP() { return {}; }
};
extern WiFiStub WiFi;
#endif
