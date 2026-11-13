#ifndef WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESPMDNS_H_
#define WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESPMDNS_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
struct MDNSStub {
    bool begin(const char*) { return true; }
    void addService(const char*, const char*, unsigned short) {}
};
extern MDNSStub MDNS;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_ESPMDNS_H_
