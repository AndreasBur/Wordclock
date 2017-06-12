#include <WS2812.h>

WS2812 LED(12); // 1 LED

void setup() {

  //Serial.begin(9600);
}

void loop() {
  
  //LED.setBrightness(11);
  LED.setPixel(0, 50, 50, 50);
  LED.show();

  delay(1000); // Wait 500 ms
}
