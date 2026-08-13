#ifndef _WIRE_H_
#define _WIRE_H_
#include <stdint.h>
#include <stddef.h>
struct TwoWire {
    bool begin(int, int, uint32_t) { return true; }
    void beginTransmission(uint8_t) {}
    size_t write(uint8_t) { return 1; }
    uint8_t endTransmission() { return 0; }
    size_t requestFrom(uint8_t, size_t) { return 0; }
    int read() { return 0; }
    int available() { return 0; }
};
extern TwoWire Wire;
#endif
