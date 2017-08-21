/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Display.h"
#include "Animation.h"
#include "Clock.h"
/*End of auto generated code by Atmel studio */
int flag;
Display WordClockDisplay(20, 20, 20);
Clock wcClock(&WordClockDisplay, CLOCK_MODE_WESSI);
//DisplayCharacter WcDisplayCharacter;
//Animation wcAnimation;
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

void setup() {
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
  //wcAnimation.setChar(0,0, 'T', ANIMATION_FONT_4X6);
  wcClock.setClock(17,30);
}

void loop() {
  // put your main code here, to run repeatedly:
}
