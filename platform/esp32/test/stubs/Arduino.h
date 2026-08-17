/* Stand-in for the ESP32 Arduino core, enough of it to compile and link the backend on
   the host. Serial here is the *hardware* port: platform/esp32/include/Arduino.h shadows
   this file, captures this object, and rebinds Serial to WordclockSerial. */
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
#define SDA 8
#define SCL 9
/* Enough of the core's digital IO for the supply switch to link. The test never reads a pin
   back, so what a write did is not recorded: the switch's own state is kept in PowerSwitch and
   that is what a case would ask. */
#define OUTPUT 1
#define HIGH 1
#define LOW 0
void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
class __FlashStringHelper;
char* itoa(int, char*, int);
void configTzTime(const char*, const char*, const char* = nullptr);

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

/* What the backend asks the chip itself for. The test never restarts, so restart() only
   has to be linkable. */
struct EspClass {
    uint32_t getFreeHeap() { return 0u; }
    void restart() {}
};
extern EspClass ESP;

/* Milliseconds since the start, which the test advances itself rather than measuring. */
extern unsigned long TestMillis;
unsigned long millis();
extern Stream Serial;
#endif
