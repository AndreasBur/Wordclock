#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#include <map>
#include <string>

/* Keeps what was put into it, so a test can watch a round trip rather than only a call.
   One map per namespace, shared by every instance, which is what NVS is. */
inline std::map<std::string, std::map<std::string, std::string>>& preferencesStore()
{
    static std::map<std::string, std::map<std::string, std::string>> Store;
    return Store;
}

struct Preferences {
    std::string Namespace;

    size_t putString(const char* Key, const char* Value) {
        preferencesStore()[Namespace][Key] = Value;
        return strlen(Value);
    }
    size_t getString(const char* Key, char* Value, size_t Capacity) {
        const auto& Keys = preferencesStore()[Namespace];
        const auto Found = Keys.find(Key);
        if((Found == Keys.end()) || (Found->second.size() >= Capacity)) { Value[0] = '\0'; return 0u; }
        strcpy(Value, Found->second.c_str());
        return Found->second.size();
    }

    bool begin(const char* Name, bool = false) { Namespace = Name; return true; }
    void end() {}
    size_t getBytesLength(const char*) { return 0; }
    size_t getBytes(const char*, void*, size_t) { return 0; }
    size_t putBytes(const char*, const void*, size_t) { return 0; }
    /* Really removes, because clearing the console password is a state a case has to be able
       to reach - and "returns false and keeps the key" would make the cleared clock and the
       protected one the same thing here. */
    bool remove(const char* Key) { return preferencesStore()[Namespace].erase(Key) > 0u; }
    bool isKey(const char*) { return false; }
};
#endif
