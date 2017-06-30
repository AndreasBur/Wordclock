/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>
#include "Display.h"
#include "Display_Character.h"
#include "Display_Clock.h"
/*End of auto generated code by Atmel studio */

//Display WordClockDisplay;
//DisplayCharacter WcDisplayCharacter;

char Char;
Display WcDisplay;
DisplayClock WcDisplayClock;
int I = 0;

//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

void setup() {
  // put your setup code here, to run once:
  //WordClockDisplay.setChar(2, 0, 10, 20, 30);
  //WcDisplayCharacter.getChar(1,1, &Char);
 
}

void loop() {
  // put your main code here, to run repeatedly:
   WcDisplayClock.showClock(10,I);
   I++;
   WcDisplay.show();
}
