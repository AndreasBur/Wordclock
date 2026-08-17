#ifndef _WIFI_NTP_H_
#define _WIFI_NTP_H_
/* The servers half of what the ESP32's configTzTime() does in one call. The test looks at
   whether it was asked at all, because the moment it is asked is the difference that
   matters on this core: before the link there is nothing to resolve the names over. */
struct NTPClassStub {
    int Started = 0;
    void begin(const char*, const char*, int = 3600) { Started++; }
};
extern NTPClassStub NTP;
#endif
