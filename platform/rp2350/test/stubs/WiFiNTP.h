#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIFINTP_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIFINTP_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
/*
 * The servers half of what the ESP32's configTzTime() does in one call. The test looks at
 * whether it was asked at all, because the moment it is asked is the difference that
 * matters on this core: before the link there is nothing to resolve the names over.
 */
struct NTPClassStub {
  int Started = 0;
  void begin(char const*, char const*, int = 3600) { Started++; }
};
extern NTPClassStub NTP;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIFINTP_H_
