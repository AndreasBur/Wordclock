/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Display.h"
#include "Animation.h"
#include "Clock.h"
#include "WS2812.h"
#include <util/delay.h>

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

Display wcDisplay(20, 20, 20);
Clock wcClock(&wcDisplay, Clock::MODE_WESSI);
Animation wcAnimation(&wcDisplay, &wcClock);
byte Brightness = 0;

void setup() {
    wcDisplay.init();
    WS2812::getInstance().setBrightness(0);
    //WS2812::getInstance().show();
    //delay(100);
    WS2812::getInstance().setBrightness(255);
    //wcDisplay.setWord(DisplayWords::WORD_ES);
    //wcDisplay.show();
  // put your setup code here, to run once:
  //WordClockDisplay.setChar(2, 0, 10, 20, 30);
  //WcDisplayCharacter.getChar(1,1, &Char);
  //WordClockDisplay.setCharacter(DISPLAY_CHARACTER_A_1);
  //WordClockDisplay.setWord(DISPLAY_WORD_DREIVIERTEL);
  //if (WordClockDisplay.clearAllWords() == E_OK) {
//		flag = 1;
  //} else {
//		flag = 0;
  //}
  //wcAnimation.setChar(0,0, 'T', AnimationFont::FONT_4X6);
  //wcClock.setClock(10,10);
  
  //wcAnimation.setAnimation(Animation::ANIMATION_CLOCK_DROP);
 
  //wcAnimation.setClock(12, 10);
  //WS2812::getInstance().setPixelFast(0, 180, 20, 50);
  //WS2812::getInstance().setPixelFast(1, 20, 180, 200);
  //WS2812::getInstance().setPixelFast(2, 50, 100, 80);

  SET_BIT(PORTC_DIRSET, 6);

}



//Font wcFont;

unsigned long Micros = 0;

void loop()
{
   //WS2812::getInstance().setPixels(0, 0, 0);
   //WS2812::getInstance().setPixels(255, 255, 255);
   WS2812::getInstance().setPixel(0, Brightness, 0, 0);
   WS2812::getInstance().show();
   Brightness++;
   if(Brightness > 10) Brightness = 0;

   delay(1000);


  //FontTahoma10x10::CharType Char;
  // put your main code here, to run repeatedly:
    //wcClock.setClock(17,30);
    

    //volatile int Test = 5;
}
