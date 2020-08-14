/*Begining of Auto generated code by Atmel studio */
//#include <Arduino.h>
#include "Display.h"
#include "Animation.h"
#include "Clock.h"
#include "WS2812.h"
#include <util/delay.h>
#include "Timer.h"
#include "Communication.h"
#include "BH1750.h"

//volatile uint8_t* const PROGMEM port_to_mode_PGMdfg[] = {
    //NOT_A_PORT,
    //NOT_A_PORT,
    //&DDRB,
    //&DDRC,
    //&DDRD,
//};

/*End of auto generated code by Atmel studio */

//void initClock()
//{
    //OSC.CTRL |= OSC_RC8MEN_bm;
    //while(!(OSC.STATUS & OSC_RC8MRDY_bm));
    //CCP = CCP_IOREG_gc;
    //CLK.CTRL = CLK_SCLKSEL_RC8M_gc;
//}

//void initClock(void)
//{
    //OSC.CTRL |= OSC_RC32MEN_bm;
    //while(!(OSC.STATUS & OSC_RC32MRDY_bm));
    //CCP = CCP_IOREG_gc;
    //CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
//}

//DisplayCharacter WcDisplayCharacter;
//Animation wcAnimation;
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio

//Animation wcAnimation();
Communication wcCommunication;

void initClockPrescaler()
{
#if (F_CPU == 16000000L)
    CCP = CCP_IOREG_gc;
    CLK_PSCTRL = CLK_PSADIV_2_gc;
#elif (F_CPU == 8000000L)
    CCP = CCP_IOREG_gc;
    CLK_PSCTRL = CLK_PSADIV_4_gc;
#elif (F_CPU == 4000000L)
    CCP = CCP_IOREG_gc;
    CLK_PSCTRL = CLK_PSADIV_8_gc;
#endif
}

void setup() {
	Serial.begin(9600);
    initClockPrescaler();
    Display::getInstance().init();
    Display::getInstance().enableBrightnessAutomatic();
    BH1750::getInstance().setCalibrationValuesMaxValue(30000u);
    Display::getInstance().setBrightness(200u);

    //BH1750 bh1750;

    Timer myTimer;

	//cout << F("Hallo") << std::endl;

    //bh1750.changeMeasurementTime(0b10100111);
	
    //WS2812::getInstance().setBrightness(0);
    //WS2812::getInstance().show();
    //delay(100);
    //WS2812::getInstance().setBrightness(255);
    //wcDisplay.setWord(DisplayWords::WORD_ES);
    //wcDisplay.show();
  // put your setup code here, to run once:
  //WordClockDisplay.setChar(2, 0, 10, 20, 30);
  //WcDisplayCharacter.getChar(1,1, &Char);
  //WordClockDisplay.setCharacter(DISPLAY_CHARACTER_A_1);
  Display::getInstance().setWord(DisplayWords::WORD_ZEHN);
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
  SET_BIT(PORTC_OUT, 6);
}

//Font wcFont;

unsigned long Micros = 0;

void loop()
{
    //WS2812::getInstance().setPixels(0, 0, 0);
    //(WS2812::getInstance().setPixels(85, 85, 85);
    //WS2812::getInstance().setPixels(255, 255, 255);
    //WS2812::getInstance().setPixel(0, Brightness, 0, 0);
    //WS2812::getInstance().show();

    SET_BIT(PORTC_OUT, 6);
    delayMicroseconds(10);
    CLEAR_BIT(PORTC_OUT, 6);
    delayMicroseconds(10);

  //FontTahoma10x10::CharType Char;
  // put your main code here, to run repeatedly:
    Clock::getInstance().setClock(17,30);
	while(1) {
	    wcCommunication.task();
	}



    //volatile int Test = 5;
}
