/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "WS2812.h"
//#include "array.h"

WS2812& ws2812 = WS2812::getInstance();

//#define ws2812 WS2812::getInstance()

//volatile uint8_t* const PROGMEM port_to_mode_PGMdfg[] = {
    //NOT_A_PORT,
    //NOT_A_PORT,
    //&DDRB,
    //&DDRC,
    //&DDRD,
//};

/*End of auto generated code by Atmel studio */

//DisplayCharacter WcDisplayCharacter;
//Animation wcAnimation;
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

void setup() {
    ws2812.init(WS2812::PORT_C, WS2812::PIN_0);
    ws2812.setBrightness(50);
    ws2812.show();
}

//Font wcFont;

void loop()
{
    if(ws2812.getState() == WS2812::STATE_IDLE) {
        //ws2812.show();
    }
}
