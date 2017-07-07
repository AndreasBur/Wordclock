/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Display.h"
/*End of auto generated code by Atmel studio */
int flag;
Display WordClockDisplay(20, 20, 20);
//DisplayCharacter WcDisplayCharacter;

//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

void setup() {
  // put your setup code here, to run once:
  //WordClockDisplay.setChar(2, 0, 10, 20, 30);
  //WcDisplayCharacter.getChar(1,1, &Char);
  WordClockDisplay.setCharacter(DISPLAY_CHARACTER_A_1);
  WordClockDisplay.setWord(DISPLAY_WORD_DREIVIERTEL);
  if (WordClockDisplay.clearAllWords() == E_OK) {
		flag = 1;
  } else {
		flag = 0;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
