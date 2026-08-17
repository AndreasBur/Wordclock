#ifndef _LEAMDNS_H_
#define _LEAMDNS_H_
struct MDNSStub {
    bool begin(const char*) { return true; }
    void addService(const char*, const char*, unsigned short) {}
};
extern MDNSStub MDNS;
#endif
