/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       WS2812.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _WS2812_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "WS2812.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/
/* WS2812 parameter */
#define WS2812_ZERO_PULSE_DURATION_NS               350u
#define WS2812_ONE_PULSE_DURATION_NS                900u
#define WS2812_ZERO_PULSE_MAX_DURATION_NS           550u
#define WS2812_RESET_DURATION_NS                    50000UL

/* xmega hardware parameter */
#define WS2812_USART_SPI_BAUDRATE                   800000uL
#define WS2812_XCL_LUT_TRUTH_TABLE_VALUE            0xA0u

#define WS2812_BTC0_CYCLES_ONE_PULSE                (((F_CPU / 1000u) * WS2812_ONE_PULSE_DURATION_NS)  / 1000000u)
#define WS2812_BTC0_CYCLES_ZERO_PULSE               (((F_CPU / 1000u) * WS2812_ZERO_PULSE_DURATION_NS) / 1000000u)
#define WS2812_BTC0_CYCLES_HARDWARE_DELAY           4u
#define WS2812_BTC0_CYCLES_ONE_PULSE_CORRECTED      (WS2812_BTC0_CYCLES_ONE_PULSE - WS2812_BTC0_CYCLES_HARDWARE_DELAY)
#define WS2812_BTC0_CYCLES_ZERO_PULSE_CORRECTED     (WS2812_BTC0_CYCLES_ZERO_PULSE - WS2812_BTC0_CYCLES_HARDWARE_DELAY)
#define WS2812_BTC0_MIN_CYCLES_ZERO_PULSE           WS2812_BTC0_CYCLES_HARDWARE_DELAY
#define WS2812_DMA_PORT_OUTPUT_DELAY_NS             50000uL
#define WS2812_RESET_TIMER_DURATION_NS              (WS2812_RESET_DURATION_NS + WS2812_DMA_PORT_OUTPUT_DELAY_NS)

#define WS2812_XCL_PERCAPTL_VALUE                   WS2812_BTC0_CYCLES_ONE_PULSE_CORRECTED

/* 
    Xcl Cmpl max value is Xcl Percaptl.
*/
#if ((WS2812_BTC0_CYCLES_ONE_PULSE_CORRECTED - WS2812_BTC0_CYCLES_ZERO_PULSE_CORRECTED) > WS2812_XCL_PERCAPTL_VALUE)
    #define WS2812_XCL_CMPL_VALUE                   WS2812_XCL_PERCAPTL_VALUE
#else
    #define WS2812_XCL_CMPL_VALUE                   (WS2812_BTC0_CYCLES_ONE_PULSE_CORRECTED - WS2812_BTC0_CYCLES_ZERO_PULSE_CORRECTED)
#endif

/* 
    the only critical timing parameter is the minimum pulse length of zero "0" 
    warn or throw error if this timing can not be met with current F_CPU settings. 
*/
#define WS2812_ZERO_PULSE_DURATION_NS_CALC          ((WS2812_BTC0_MIN_CYCLES_ZERO_PULSE * 1000000u) / (F_CPU / 1000u))

#if (WS2812_ZERO_PULSE_DURATION_NS_CALC > WS2812_ZERO_PULSE_MAX_DURATION_NS)
    //#error "WS2812: sorry, the clock speed is too low. Did you set F_CPU correctly?"
#elif (WS2812_ZERO_PULSE_DURATION_NS_CALC > (WS2812_ZERO_PULSE_MAX_DURATION_NS - 100u))
    #warning "WS2812: The timing is critical and may only work on WS2812B, not on WS2812(S)."
    #warning "Please consider a higher clockspeed, if possible"
#endif

// missing in iox32e5.hl
#define USART_UCPHA_bm  0x02u  /* Clock Phase bit mask. */
#define USART_UCPHA_bp  1u     /* Clock Phase bit position. */
#define USART_UDORD_bm  0x04u  /* Data Order bit mask. */
#define USART_UDORD_bp  2u     /* Data Order bit position. */


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of WS2812
******************************************************************************************************************************************************/
/*! \brief          WS2812 Constructor
 *  \details        Instantiation of the WS2812 library
 *
 *  \return         -
******************************************************************************************************************************************************/
WS2812::WS2812() : State{STATE_UNINIT}, PixelsBuffer1{}, PixelsBuffer2{}
{
    pCurrentFrame = &PixelsBuffer1;
    pNextFrame = &PixelsBuffer2;

#if (WS2812_SUPPORT_DIMMING == STD_ON)
    Brightness = 255u;
#endif

#if (WS2812_RESET_TIMER == STD_ON)
    ResetTimer = 0u;
#endif
} /* WS2812 */


/******************************************************************************************************************************************************
  Destructor of WS2812
******************************************************************************************************************************************************/
WS2812::~WS2812()
{

} /* ~WS2812 */


/******************************************************************************************************************************************************
  Singleton Instance of WS2812
******************************************************************************************************************************************************/
WS2812& WS2812::getInstance()
{
    static WS2812 SingletonInstance;
    return SingletonInstance;
}


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
boolean WS2812::init(byte Pin)
{
    PortPinType PortPin;
    byte PinMask = digitalPinToBitMask(Pin);
    byte Port = digitalPinToPort(digitalPinToPort(Pin));

    if(isBitSet(PinMask, PORT_PIN_0)) { PortPin = PORT_PIN_0; } 
    else if(isBitSet(PinMask, PORT_PIN_4)) { PortPin = PORT_PIN_4; } 
    else { return E_NOT_OK; }

    if((PortPin == PORT_PIN_0 || PortPin == PORT_PIN_4) && (Port == PORT_C || Port == PORT_D)) {
        init(static_cast<PortType>(Port), PortPin);
        return E_OK;
    }
    return E_NOT_OK;
} /* init */


/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::init(PortType Port, PortPinType PortPin)
{
    if(Port == PORT_C) {
        initDma(&USARTC0, EDMA_CH_TRIGSRC_USARTC0_DRE_gc);
        initPort(&PORTC);
        initEventSystem(EVSYS_CHMUX_PORTC_PIN3_gc, EVSYS_CHMUX_PORTC_PIN1_gc);
        initUsart(&USARTC0);
        if(PortPin == PORT_PIN_0) { initXcl(XCL_LUT0OUTEN_PIN0_gc, XCL_PORTSEL_PC_gc); }
        else if(PortPin == PORT_PIN_4) { initXcl(XCL_LUT0OUTEN_PIN4_gc, XCL_PORTSEL_PC_gc); }
    }
    if(Port == PORT_D) {
        initDma(&USARTD0, EDMA_CH_TRIGSRC_USARTD0_DRE_gc);
        initPort(&PORTD);
        initUsart(&USARTD0);
        initEventSystem(EVSYS_CHMUX_PORTD_PIN3_gc, EVSYS_CHMUX_PORTD_PIN1_gc);
        if(PortPin == PORT_PIN_0) { initXcl(XCL_LUT0OUTEN_PIN0_gc, XCL_PORTSEL_PD_gc); }
        else if(PortPin == PORT_PIN_4) { initXcl(XCL_LUT0OUTEN_PIN4_gc, XCL_PORTSEL_PD_gc); }
    }
    State = STATE_IDLE;
} /* init */


/******************************************************************************************************************************************************
  show()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::show()
{
    if(State == STATE_IDLE) {

#if (WS2812_RESET_TIMER == STD_ON)
        if(isResetTimeElapsed() == false) return E_NOT_OK;
#endif

#if (WS2812_SUPPORT_DIMMING == STD_ON)
        State = STATE_BUSY;
        if(Brightness != 255u) {
            showNextFrameDimmed();
        } else {
            showNextFrame();
        }
#else
        showNextFrame();
#endif
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* show */


/******************************************************************************************************************************************************
  setPixels()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::setPixels(byte Red, byte Green, byte Blue)
{
    for(IndexType Index = 0u; Index < WS2812_NUMBER_OF_LEDS; Index++) {
        (*pNextFrame)[Index].setRed(Red);
        (*pNextFrame)[Index].setGreen(Green);
        (*pNextFrame)[Index].setGreen(Blue);
    }
} /* setPixels */


/******************************************************************************************************************************************************
  getPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::getPixel(IndexType Index, PixelType& Pixel) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Pixel = (*pNextFrame)[Index].getPixel();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixel */


/******************************************************************************************************************************************************
  getPixelRed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::getPixelRed(IndexType Index, byte& Red) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Red = (*pNextFrame)[Index].getRed();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixelRed */


/******************************************************************************************************************************************************
  getPixelGreen()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::getPixelGreen(IndexType Index, byte& Green) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Green = (*pNextFrame)[Index].getGreen();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixelGreen */


/******************************************************************************************************************************************************
  getPixelBlue()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::getPixelBlue(IndexType Index, byte& Blue) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Blue = (*pNextFrame)[Index].getBlue();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixelBlue */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::setPixel(IndexType Index, PixelType Pixel)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].setPixel(Pixel);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixel */


/******************************************************************************************************************************************************
  setPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::setPixel(IndexType Index, byte Red, byte Green, byte Blue)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].setPixel(Red, Green, Blue);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixel */


/******************************************************************************************************************************************************
  setPixelRed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::setPixelRed(IndexType Index, byte Red)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].setRed(Red);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixelRed */


/******************************************************************************************************************************************************
  setPixelGreen()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::setPixelGreen(IndexType Index, byte Green)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].setGreen(Green);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixelGreen */


/******************************************************************************************************************************************************
  setPixelBlue()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::setPixelBlue(IndexType Index, byte Blue)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].setBlue(Blue);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* setPixelBlue */


/******************************************************************************************************************************************************
  clearPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
StdReturnType WS2812::clearPixel(IndexType Index)
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        (*pNextFrame)[Index].clearPixel();
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* clearPixel */


#if (WS2812_SUPPORT_DIMMING == STD_ON)
/******************************************************************************************************************************************************
  getPixelDimmed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
StdReturnType WS2812::getPixelDimmed(IndexType Index, PixelType& Pixel) const
{
    if(Index < WS2812_NUMBER_OF_LEDS) {
        Pixel = getPixelDimmedFast(Index);
        return E_OK;
    } else {
        return E_NOT_OK;
    }
} /* getPixelDimmed */


/******************************************************************************************************************************************************
  getPixelDimmedFast()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
WS2812::PixelType WS2812::getPixelDimmedFast(IndexType Index) const
{
    PixelType Pixel;

    if(Brightness == 255u) {
        return getPixelFast(Index);
    } else {
        Pixel.Red = dimmColor(getPixelRedFast(Index));
        Pixel.Green = dimmColor(getPixelGreenFast(Index));
        Pixel.Blue = dimmColor(getPixelBlueFast(Index));
    }
    return Pixel;
} /* getPixelDimmedFast */


/******************************************************************************************************************************************************
  setBrightness()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::setBrightness(byte sBrightness, boolean GammaCorrection)
{
    if(GammaCorrection) {
        Brightness = GCorrection.getCorrectedValue(sBrightness / 2);
    } else {
        Brightness = sBrightness;
    }
} /* setBrightness */
#endif /* WS2812_SUPPORT_DIMMING */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  initUsart()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::initUsart(USART_t* pUsart)
{
    // Setup USART in master SPI mode 1, MSB first
    pUsart->BAUDCTRLA = (F_CPU / (2u * WS2812_USART_SPI_BAUDRATE)) - 1u;
    pUsart->BAUDCTRLB = 0u;
    pUsart->CTRLA = USART_DREINTLVL_OFF_gc | USART_RXCINTLVL_OFF_gc | USART_TXCINTLVL_OFF_gc;
    pUsart->CTRLC = USART_CMODE_MSPI_gc | USART_UCPHA_bm;
    pUsart->CTRLD = USART_DECTYPE_DATA_gc | USART_LUTACT_OFF_gc | USART_PECACT_OFF_gc;
    pUsart->CTRLB = USART_TXEN_bm;
} /* initUsart */


/******************************************************************************************************************************************************
  initPort()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::initPort(PORT_t* pPort)
{
    // Setup port pins for TxD, XCK
    pPort->PIN1CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_RISING_gc;    // XCK
    pPort->PIN3CTRL = PORT_OPC_TOTEM_gc | PORT_ISC_LEVEL_gc;     // TxD
    pPort->DIRSET = PIN1_bm | PIN3_bm;
} /* initUsart */


/******************************************************************************************************************************************************
  initDma()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::initDma(USART_t* pUsart, byte EdmaChTrigSrcUsartDreGc)
{
    // Setup EDMA channel 0(+1)
    EDMA.CTRL = EDMA_ENABLE_bm | EDMA_CHMODE_STD02_gc | EDMA_DBUFMODE_DISABLE_gc | EDMA_PRIMODE_CH0123_gc;
    EDMA.CH0.CTRLB = EDMA_CH_TRNINTLVL_LO_gc | EDMA_CH_ERRINTLVL_OFF_gc;
    EDMA.CH0.ADDRCTRL = EDMA_CH_RELOAD_NONE_gc | EDMA_CH_DIR_INC_gc;
    EDMA.CH0.DESTADDRCTRL = EDMA_CH_RELOAD_NONE_gc | EDMA_CH_DESTDIR_FIXED_gc;
    EDMA.CH0.DESTADDR = (uint16_t)&pUsart->DATA;
    EDMA.CH0.TRIGSRC = EdmaChTrigSrcUsartDreGc;
} /* initDma */


/******************************************************************************************************************************************************
  initEventSystem()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::initEventSystem(byte EvsysChmuxPortPin3Gc, byte EvsysChmuxPortPin1Gc)
{
    // Setup Event channel 0 to TxD (async)
    EVSYS.CH0MUX = EvsysChmuxPortPin3Gc;
    EVSYS.CH0CTRL = EVSYS_DIGFILT_1SAMPLE_gc;
    // Setup Event channel 6 to XCK rising edge
    EVSYS.CH6MUX = EvsysChmuxPortPin1Gc;
    EVSYS.CH6CTRL = EVSYS_DIGFILT_1SAMPLE_gc;
} /* initEventSystem */


/******************************************************************************************************************************************************
  initXcl()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::initXcl(byte Lut0OutenPinGc, byte XclPortselGc)
{
    // Setup XCL BTC0 timer to 1shot PWM generation
    XCL.CTRLE = XCL_CMDSEL_NONE_gc | XCL_TCSEL_BTC0_gc | XCL_CLKSEL_DIV1_gc;
    XCL.CTRLF = XCL_CMDEN_DISABLE_gc | XCL_MODE_1SHOT_gc;
    XCL.CTRLG = XCL_EVACTEN_bm | XCL_EVACT0_RESTART_gc | XCL_EVSRC_EVCH6_gc;
    // Output high time if data is 1 (from RESTART to falling edge of one-shot)
    XCL.PERCAPTL = WS2812_XCL_PERCAPTL_VALUE;
    // Output high time if data is 0 (from RESTART to rising edge of one-shot)
    XCL.CMPL = WS2812_XCL_CMPL_VALUE;
    // Setup XCL LUT
    // Setup glue logic for MUX
    XCL.CTRLA = Lut0OutenPinGc | XclPortselGc | XCL_LUTCONF_MUX_gc;
    XCL.CTRLB = XCL_IN3SEL_XCL_gc | XCL_IN2SEL_XCL_gc | XCL_IN1SEL_EVSYS_gc | XCL_IN0SEL_EVSYS_gc;
    // Async inputs, no delay
    XCL.CTRLC = XCL_EVASYSEL0_bm | XCL_DLY1CONF_DISABLE_gc;
    // LUT truth tables (only LUT1 is used)
    XCL.CTRLD = WS2812_XCL_LUT_TRUTH_TABLE_VALUE;
} /* initXcl */


/******************************************************************************************************************************************************
  showNextFrame()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::showNextFrame()
{
    // exclusive area start
    startDmaTransfer(pNextFrame);
    copyNextFrameToCurrentFrame();
    switchPixelsBufferPointer();
    // exclusive area end
} /* showNextFrame */

#if (WS2812_SUPPORT_DIMMING == STD_ON)
/******************************************************************************************************************************************************
  showNextFrameDimmed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::showNextFrameDimmed()
{
    // exclusive area start
    copyNextFrameToCurrentFrameDimmed();
    startDmaTransfer(pCurrentFrame);
    // exclusive area end
} /* showNextFrameDimmed */
#endif

/******************************************************************************************************************************************************
  startDmaTransfer()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::startDmaTransfer(pPixelsType pFrame)
{
    EDMA.CH0.ADDR = (uint16_t)pFrame;
    EDMA.CH0.TRFCNT = sizeof(PixelsType);
    EDMA.CH0.CTRLA = EDMA_CH_ENABLE_bm | EDMA_CH_SINGLE_bm; // Start DMA transfer to LEDs
} /* startDmaTransfer */


/******************************************************************************************************************************************************
  switchBufferPointer()
******************************************************************************************************************************************************/
/*! \brief          switch the two frame buffers
 *  \details        this function switches the two frame buffers PixelsBuffer1 and PixelsBuffer2
 *                  
 *  \return         -
******************************************************************************************************************************************************/
void WS2812::switchPixelsBufferPointer()
{
    pPixelsType pTmp = pCurrentFrame;
    pCurrentFrame = pNextFrame;
    pNextFrame = pTmp;
} /* switchBufferPointer */


#if (WS2812_RESET_TIMER == STD_ON)
/******************************************************************************************************************************************************
  isResetTimeElapsed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
boolean WS2812::isResetTimeElapsed()
{
    // check for timer overflow
    if(micros() < ResetTimer) {
        return ((micros() - (UINT64_MAX - ResetTimer)) > (WS2812_RESET_TIMER_DURATION_NS / 1000u));
    } else {
        return ((micros() - ResetTimer) > (WS2812_RESET_TIMER_DURATION_NS / 1000u));
    }
} /* isResetTimeElapsed */
#endif


#if (WS2812_SUPPORT_DIMMING == STD_ON)
/******************************************************************************************************************************************************
  dimmPixels()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::dimmPixels(pPixelsType pPixels) const
{
    for(IndexType Index = 0; Index < WS2812_NUMBER_OF_LEDS; Index++) {
        (*pPixels)[Index].setPixel(getPixelDimmedFast(Index));
    }
} /* dimmPixels */


/******************************************************************************************************************************************************
  dimmPixel()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
WS2812::PixelType WS2812::dimmPixel(byte Red, byte Green, byte Blue) const
{
    PixelType Pixel;
    Pixel.Red = dimmColor(Red);
    Pixel.Green = dimmColor(Green);
    Pixel.Blue = dimmColor(Blue);
    return Pixel;
} /* dimmPixel */


/******************************************************************************************************************************************************
  copyCurrentFrameToNextFrameDimmed()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
void WS2812::copyNextFrameToCurrentFrameDimmed()
{
    dimmPixels(pCurrentFrame);
} /* copyCurrentFrameToNextFrameDimmed */
#endif


/******************************************************************************************************************************************************
  F R I E N D   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  dmaIsr()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        
 *                  
 *  \return         -
 *****************************************************************************************************************************************************/
inline void dmaIsr()
{   // IS_BIT_CLEARED(EDMA.CH0.CTRLB, EDMA_CH_CHBUSY_bp)
    if(IS_BIT_SET(EDMA.CH0.CTRLB, EDMA_CH_TRNIF_bp)) {
        SET_BIT(EDMA.CH0.CTRLB, EDMA_CH_TRNIF_bp);
        WS2812::getInstance().State = WS2812::STATE_IDLE;

#if (WS2812_RESET_TIMER == STD_ON)
        WS2812::getInstance().startResetTimer();
#endif
    }
}


/******************************************************************************************************************************************************
  I S R   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Isr Dma
******************************************************************************************************************************************************/
ISR(EDMA_CH0_vect)
{
    dmaIsr();
}


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 