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
    PixelRawType PixelRaw{0u, 0u, 0u};

    static constexpr byte ColorMaxValue{255u};
    static constexpr byte ColorOffsetRed{PIXEL_COLOR_OFFSET_RED};
    static constexpr byte ColorOffsetGreen{PIXEL_COLOR_OFFSET_GREEN};
    static constexpr byte ColorOffsetBlue{PIXEL_COLOR_OFFSET_BLUE};

    // functions
    static constexpr ColorType dimmColor(ColorType Color, byte Brightness) {
        return static_cast<ColorType>((static_cast<uint16_t>(Color) * (Brightness + 1u)) >> 8u);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Pixel() { }
    /* Goes through setPixel() rather than initialising the array in argument order,
       which is what it used to do: the raw array is ordered by the ColorOffset
       constants (green first, for the WS2812 wire order), so a positional initialiser
       put red where getRed() does not look for it. Nothing visible came of it while
       every colour built this way was grey or black, and the simulator could not have
       shown it either - it renders brightness, not hue. On the strip it swaps red and
       green. Keeping setPixel() as the only place that knows the layout is what stops
       that from coming back. */
    constexpr Pixel(ColorType ColorRed, ColorType ColorGreen, ColorType ColorBlue) {
        setPixel(ColorRed, ColorGreen, ColorBlue);
    }
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

    /* constexpr so the constructor can use it, and thereby be the only place in the
       class that maps a colour name onto a raw index. */
    constexpr void setPixel(ColorType Red, ColorType Green, ColorType Blue) {
         PixelRaw[ColorOffsetRed] = Red;
         PixelRaw[ColorOffsetBlue] = Blue;
         PixelRaw[ColorOffsetGreen] = Green;
    }

    // methods
    void clearPixel() { setPixel(0u, 0u, 0u); }

    /* dimms all three colors, ColorMaxValue keeps the pixel unchanged. Dimms the
       color as a whole, so the hue is preserved and only the brightness changes. */
    void dimmPixel(byte Brightness) {
        setPixel(dimmColor(getRed(), Brightness), dimmColor(getGreen(), Brightness), dimmColor(getBlue(), Brightness));
    }

    void incrementRed() { if(PixelRaw[ColorOffsetRed] < ColorMaxValue) PixelRaw[ColorOffsetRed]++; }
    void incrementGreen() { if(PixelRaw[ColorOffsetGreen] < ColorMaxValue) PixelRaw[ColorOffsetGreen]++; }
    void incrementBlue() { if(PixelRaw[ColorOffsetBlue] < ColorMaxValue) PixelRaw[ColorOffsetBlue]++; }

    void decrementRed() { if(PixelRaw[ColorOffsetRed] > 0u) PixelRaw[ColorOffsetRed]--; }
    void decrementGreen() { if(PixelRaw[ColorOffsetGreen] > 0u) PixelRaw[ColorOffsetGreen]--; }
    void decrementBlue() { if(PixelRaw[ColorOffsetBlue] > 0u) PixelRaw[ColorOffsetBlue]--; }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
