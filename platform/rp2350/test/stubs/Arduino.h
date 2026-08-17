/* Stand-in for the arduino-pico core, enough of it to compile and link the backend on the
   host. Serial here is the *hardware* port: platform/rp2350/include/Arduino.h shadows this
   file, captures this object, and rebinds Serial to WordclockSerial. */
#ifndef _ARDUINO_H_
#define _ARDUINO_H_
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>

typedef uint8_t byte;
typedef bool boolean;

#define PROGMEM
#define F(x) reinterpret_cast<const __FlashStringHelper*>(x)
#define pgm_read_byte(x) (*(x))
#define memcpy_P(x,y,z) memcpy((x),(y),(z))
#define bitRead(v,b) (((v) >> (b)) & 0x01)

/* The Pico's default I2C0 pins, which is what the drivers ask for by name. */
#define SDA 4
#define SCL 5

class __FlashStringHelper;
char* itoa(int, char*, int);

/* The test drives Incoming and inspects Written. */
struct Stream {
    std::string Incoming;
    std::string Written;
    void begin(unsigned long) {}
    int available() { return static_cast<int>(Incoming.size()); }
    int read() { if(Incoming.empty()) return -1; int c = static_cast<unsigned char>(Incoming[0]); Incoming.erase(0,1); return c; }
    size_t write(uint8_t b) { Written.push_back(static_cast<char>(b)); return 1; }
    void flush() {}
};

/* What the backend asks the chip itself for. Named after the part on this core, where the
   ESP32's is named ESP. The test never restarts, so restart() only has to be linkable. */
struct Rp2040Class {
    uint32_t getFreeHeap() { return 0u; }
    void restart() {}
};
extern Rp2040Class rp2040;

/* Milliseconds since the start, which the test advances itself rather than measuring. */
extern unsigned long TestMillis;
unsigned long millis();
extern Stream Serial;
#endif
