#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_
#include <stddef.h>
#include <stdint.h>
struct Preferences {
    bool begin(const char*, bool = false) { return true; }
    void end() {}
    size_t getBytesLength(const char*) { return 0; }
    size_t getBytes(const char*, void*, size_t) { return 0; }
    size_t putBytes(const char*, const void*, size_t) { return 0; }
    bool remove(const char*) { return false; }
    bool isKey(const char*) { return false; }
};
#endif
