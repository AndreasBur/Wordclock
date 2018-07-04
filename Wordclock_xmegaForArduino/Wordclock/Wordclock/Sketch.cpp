﻿/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Display.h"
#include "Animation.h"
#include "Clock.h"
#include "FontTahoma10x10.h"
#include "array.h"


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
}

Display wcDisplay(20, 20, 20);
Clock wcClock(&wcDisplay, Clock::MODE_WESSI);
Animation wcAnimation(&wcDisplay, &wcClock);
FontTahoma10x10 Font;

void loop()
{


  //FontTahoma10x10::CharType Char;
  // put your main code here, to run repeatedly:
    wcClock.setClock(17,30);
    wcDisplay.show();

    FontTahoma10x10::FontCharType Char;
    //Char = Font.getCharFast(5);
    Font.getChar(5, Char);

}