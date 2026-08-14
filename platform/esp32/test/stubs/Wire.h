#ifndef _WIRE_H_
#define _WIRE_H_
#include <stdint.h>
#include <stddef.h>
#include <vector>
/* Answers a read with whatever a test put into Response, and keeps what was written, so a
   driver's register addressing can be checked rather than only its arithmetic. A test that
   sets nothing gets the silent bus the drivers have to cope with. */
struct TwoWire {
    std::vector<uint8_t> Response;
    std::vector<uint8_t> Written;
    size_t ReadPosition{0u};

    bool begin(int, int, uint32_t) { return true; }
    void beginTransmission(uint8_t Address) { Written.push_back(Address); }
    size_t write(uint8_t Value) { Written.push_back(Value); return 1; }
    uint8_t endTransmission() { return 0; }
    size_t requestFrom(uint8_t, size_t Length) {
        ReadPosition = 0u;
        return (Response.size() < Length) ? 0u : Length;
    }
    int read() { return (ReadPosition < Response.size()) ? Response[ReadPosition++] : 0; }
    int available() { return static_cast<int>(Response.size() - ReadPosition); }
};
extern TwoWire Wire;
#endif
