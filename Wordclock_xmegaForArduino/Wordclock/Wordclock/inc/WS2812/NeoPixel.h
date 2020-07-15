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
/**     \file       NeoPixel.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _NEO_PIXEL_H_
#define _NEO_PIXEL_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* NeoPixel configuration parameter */
#define NEO_PIXEL_COLOR_OFFSET_GREEN                   0u
#define NEO_PIXEL_COLOR_OFFSET_RED                     1u
#define NEO_PIXEL_COLOR_OFFSET_BLUE                    2u

/* NeoPixel parameter */
#define NEO_PIXEL_NUMBER_OF_COLORS                     3u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   N E O P I X E L
******************************************************************************************************************************************************/
class NeoPixel
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
	using ColorType = byte;
  
    /* type which describes the structure of a pixel */
    struct NeoPixelType {
        ColorType Red;
        ColorType Green;
        ColorType Blue;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  private:
    using PixelType = std::array<byte, NEO_PIXEL_NUMBER_OF_COLORS>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    PixelType Pixel;

    // functions
    byte colorOffsetRed() const { return NEO_PIXEL_COLOR_OFFSET_RED; }
    byte colorOffsetGreen() const { return NEO_PIXEL_COLOR_OFFSET_GREEN; }
    byte colorOffsetBlue() const { return NEO_PIXEL_COLOR_OFFSET_BLUE; }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr NeoPixel() : Pixel{0u, 0u, 0u}
    {

    }

	// get methods
    ColorType getRed() const { return Pixel[colorOffsetRed()]; }
    ColorType getBlue() const { return Pixel[colorOffsetBlue()]; }
    ColorType getGreen() const { return Pixel[colorOffsetGreen()]; }
    NeoPixelType getPixel() const
    {
        NeoPixelType NeoPixel;
        NeoPixel.Red = Pixel[colorOffsetRed()];
        NeoPixel.Blue = Pixel[colorOffsetBlue()];
        NeoPixel.Green = Pixel[colorOffsetGreen()];
        return NeoPixel;
    }

	// set methods
    void setRed(ColorType Value) { Pixel[colorOffsetRed()] = Value; }
    void setBlue(ColorType Value) { Pixel[colorOffsetBlue()] = Value; }
    void setGreen(ColorType Value) { Pixel[colorOffsetGreen()] = Value; }
    void setPixel(NeoPixelType NeoPixel)
    {
         Pixel[colorOffsetRed()] = NeoPixel.Red;
         Pixel[colorOffsetBlue()] = NeoPixel.Blue;
         Pixel[colorOffsetGreen()] = NeoPixel.Green;
    }
    void setPixel(ColorType Red, ColorType Green, ColorType Blue)
    {
         Pixel[colorOffsetRed()] = Red;
         Pixel[colorOffsetBlue()] = Blue;
         Pixel[colorOffsetGreen()] = Green;
    }

	// methods
    void clearPixel() { setPixel(0u, 0u, 0u); }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
