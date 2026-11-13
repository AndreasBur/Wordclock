// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_PREFERENCES_H_
#define WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_PREFERENCES_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stddef.h>
#include <stdint.h>

#include <cstring>
#include <map>
#include <string>

// Keeps what was put into it, so a test can watch a round trip rather than only a call.
// One map per namespace, shared by every instance, which is what NVS is.
inline std::map<std::string, std::map<std::string, std::string>>& preferencesStore() {
  static std::map<std::string, std::map<std::string, std::string>> Store;
  return Store;
}

struct Preferences {
  std::string Namespace;

  size_t putString(char const* Key, char const* Value) {
    preferencesStore()[Namespace][Key] = Value;
    return strlen(Value);
  }
  /*
   * As the framework answers: the length nvs_get_str() reports, which counts the terminator,
   * so an empty value answers 1 and only a missing key, or one too long for Capacity, answers
   * 0. Counted without the terminator, as this stub did, an empty stored network name looked
   * like none, and the tests passed for code that joined a network with no name.
   */
  size_t getString(char const* Key, char* Value, size_t Capacity) {
    auto const& Keys = preferencesStore()[Namespace];
    auto const Found = Keys.find(Key);
    if ((Found == Keys.end()) || (Found->second.size() + 1u > Capacity)) {
      return 0u;
    }
    memcpy(Value, Found->second.c_str(), Found->second.size() + 1u);
    return Found->second.size() + 1u;
  }

  bool begin(char const* Name, bool = false) {
    Namespace = Name;
    return true;
  }
  void end() {}
  size_t getBytesLength(char const*) { return 0; }
  size_t getBytes(char const*, void*, size_t) { return 0; }
  size_t putBytes(char const*, void const*, size_t) { return 0; }
  /*
   * Really removes, because clearing the console password is a state a case has to be able
   * to reach - and "returns false and keeps the key" would make the cleared clock and the
   * protected one the same thing here.
   */
  bool remove(char const* Key) { return preferencesStore()[Namespace].erase(Key) > 0u; }
  bool isKey(char const* Key) { return preferencesStore()[Namespace].count(Key) > 0u; }
};
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_ESP32_TEST_STUBS_PREFERENCES_H_
