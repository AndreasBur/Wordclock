#ifndef _WIRE_H_
#define _WIRE_H_
#include <stdint.h>
#include <stddef.h>
#include <vector>
/* Answers reads out of one buffer and keeps everything that was written, so a driver's
   register addressing can be checked rather than only its arithmetic. The buffer is
   consumed as it is read, which is what lets a test answer a sequence of transfers - the
   status register and then the time block - with one prepared response. A test that
   prepares nothing gets the silent bus the drivers have to cope with. */
struct TwoWire {
    std::vector<uint8_t> Response;
    std::vector<uint8_t> Written;
    size_t ReadPosition{0u};

    void respondWith(std::vector<uint8_t> Bytes) { Response = Bytes; ReadPosition = 0u; }
    size_t remaining() const { return Response.size() - ReadPosition; }

    bool begin(int, int, uint32_t) { return true; }
    void beginTransmission(uint8_t Address) { Written.push_back(Address); }
    size_t write(uint8_t Value) { Written.push_back(Value); return 1; }
    uint8_t endTransmission() { return 0; }
    size_t requestFrom(uint8_t, size_t Length) { return (remaining() < Length) ? 0u : Length; }
    int read() { return (ReadPosition < Response.size()) ? Response[ReadPosition++] : 0; }
    int available() { return static_cast<int>(remaining()); }
};
extern TwoWire Wire;
#endif
