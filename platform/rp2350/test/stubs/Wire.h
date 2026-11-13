#ifndef WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIRE_H_
#define WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIRE_H_
/* Under the framework's names, because it stands in for the framework's header. */
// NOLINTBEGIN(readability-identifier-naming)
#include <stdint.h>
#include <stddef.h>
#include <vector>
/*
 * Answers reads out of one buffer and keeps everything that was written, so a driver's
 * register addressing can be checked rather than only its arithmetic. The buffer is
 * consumed as it is read, which is what lets a test answer a sequence of transfers - the
 * status register and then the time block - with one prepared response. A test that
 * prepares nothing gets the silent bus the drivers have to cope with.
 *
 * The pin setters are separate calls here, unlike the ESP32's begin(sda, scl, freq), and
 * a pin they will not take ends in panic() on the target - so the stub records them and
 * can be made to refuse.
 */
struct TwoWire {
    std::vector<uint8_t> Response;
    std::vector<uint8_t> Written;
    size_t ReadPosition{0u};
    int Sda{-1};
    int Scl{-1};
    uint32_t Clock{0u};
    bool PinsAccepted{true};
    // What the core's begin() sets the clock to, over whatever was set before it.
    static constexpr uint32_t DefaultClock{100000u};

    void respondWith(std::vector<uint8_t> Bytes) { Response = Bytes; ReadPosition = 0u; }
    size_t remaining() const { return Response.size() - ReadPosition; }

    bool setSDA(int Pin) { Sda = Pin; return PinsAccepted; }
    bool setSCL(int Pin) { Scl = Pin; return PinsAccepted; }
    void setClock(uint32_t Hz) { Clock = Hz; }
    void begin() { Clock = DefaultClock; }

    void beginTransmission(uint8_t Address) { Written.push_back(Address); }
    size_t write(uint8_t Value) { Written.push_back(Value); return 1; }
    uint8_t endTransmission() { return 0; }
    size_t requestFrom(uint8_t, size_t Length) { return (remaining() < Length) ? 0u : Length; }
    int read() { return (ReadPosition < Response.size()) ? Response[ReadPosition++] : 0; }
    int available() { return static_cast<int>(remaining()); }
};
extern TwoWire Wire;
// NOLINTEND(readability-identifier-naming)
#endif  // WORDCLOCK_PLATFORM_RP2350_TEST_STUBS_WIRE_H_
