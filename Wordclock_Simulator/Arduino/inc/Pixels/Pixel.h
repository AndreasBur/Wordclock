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
/**     \file       Pixel.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _PIXEL_H_
#define _PIXEL_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Pixel configuration parameter */
#define PIXEL_COLOR_OFFSET_GREEN                   0u
#define PIXEL_COLOR_OFFSET_RED                     1u
#define PIXEL_COLOR_OFFSET_BLUE                    2u

/* Pixel parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   N E O P I X E L
******************************************************************************************************************************************************/
class Pixel
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using ColorType = byte;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  private:
    static constexpr byte NumberOfColors{3u};
    using PixelRawType = std::array<byte, NumberOfColors>;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    PixelRawType PixelRaw;
    
    static constexpr byte ColorMaxValue{255u};
    static constexpr byte ColorOffsetRed{PIXEL_COLOR_OFFSET_RED};
    static constexpr byte ColorOffsetGreen{PIXEL_COLOR_OFFSET_GREEN};
    static constexpr byte ColorOffsetBlue{PIXEL_COLOR_OFFSET_BLUE};
    
    // functions
    //ColorType getColorRed() const { return PixelRaw[ColorOffsetRed]; }
    //ColorType getColorGreen() const { return PixelRaw[ColorOffsetGreen]; }
    //ColorType getColorBlue() const { return PixelRaw[ColorOffsetBlue]; }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Pixel() : PixelRaw{0u, 0u, 0u} { }
    constexpr Pixel(ColorType ColorRed, ColorType ColorGreen, ColorType ColorBlue)
        : PixelRaw{ColorRed, ColorGreen, ColorBlue} { }
    ~Pixel() { }

    // get methods
    static constexpr byte getNumberOfColors() { return NumberOfColors; }
    ColorType getRed() const { return PixelRaw[ColorOffsetRed]; }
    ColorType getBlue() const { return PixelRaw[ColorOffsetBlue]; }
    ColorType getGreen() const { return PixelRaw[ColorOffsetGreen]; }

    // set methods
    void setRed(ColorType Value) { PixelRaw[ColorOffsetRed] = Value; }
    void setBlue(ColorType Value) { PixelRaw[ColorOffsetBlue] = Value; }
    void setGreen(ColorType Value) { PixelRaw[ColorOffsetGreen] = Value; }

    void setPixel(ColorType Red, ColorType Green, ColorType Blue)
    {
         PixelRaw[ColorOffsetRed] = Red;
         PixelRaw[ColorOffsetBlue] = Blue;
         PixelRaw[ColorOffsetGreen] = Green;
    }

    // methods
    void clearPixel() { setPixel(0u, 0u, 0u); }
        
    void incrementColorRed() { if(PixelRaw[ColorOffsetRed] < ColorMaxValue) PixelRaw[ColorOffsetRed]++; }
    void incrementColorGreen() { if(PixelRaw[ColorOffsetGreen] < ColorMaxValue) PixelRaw[ColorOffsetGreen]++; }
    void incrementColorBlue() { if(PixelRaw[ColorOffsetBlue] < ColorMaxValue) PixelRaw[ColorOffsetBlue]++; }
        
    void decrementColorRed() { if(PixelRaw[ColorOffsetRed] > 0u) PixelRaw[ColorOffsetRed]--; }
    void decrementColorGreen() { if(PixelRaw[ColorOffsetGreen] > 0u) PixelRaw[ColorOffsetGreen]--; }
    void decrementColorBlue() { if(PixelRaw[ColorOffsetBlue] > 0u) PixelRaw[ColorOffsetBlue]--; }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
