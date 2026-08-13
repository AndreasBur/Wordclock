#include "sim/Pixels.h"
#include <cassert>

StdReturnType Pixels::getPixel(byte Index, PixelType& Pixel) const
{
    if(Index >= PIXELS_NUMBER_OF_LEDS) { return E_NOT_OK; }

    Pixel = PixelBuffer[toRow(Index)][toColumn(Index)];
    return E_OK;
}

Pixels::PixelType Pixels::getPixelFast(byte Index) const
{
    return PixelBuffer[toRow(Index)][toColumn(Index)];
}

StdReturnType Pixels::setPixel(byte Index, PixelType Pixel)
{
    if(Index >= PIXELS_NUMBER_OF_LEDS) { return E_NOT_OK; }

    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
    return E_OK;
}

StdReturnType Pixels::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    return setPixel(Index, PixelType(Red, Green, Blue));
}

/* assert rather than wxASSERT, which would pop a dialog - and there is no GUI here to
   pop it in. Set a breakpoint on it to find the caller that ran off the end. */
void Pixels::setPixelFast(byte Index, PixelType Pixel)
{
    assert(Index < PIXELS_NUMBER_OF_LEDS);

    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
}

void Pixels::setPixelFast(byte Index, byte Red, byte Green, byte Blue)
{
    setPixelFast(Index, PixelType(Red, Green, Blue));
}

void Pixels::setPixels(PixelType Pixel)
{
    for(unsigned int Row = 0; Row < PIXELS_DISPLAY_NUMBER_OF_ROWS; Row++) {
        for(unsigned int Column = 0; Column < PIXELS_DISPLAY_NUMBER_OF_COLUMNS; Column++) {
            PixelBuffer[Row][Column] = Pixel;
        }
    }
    Dirty = true;
}

void Pixels::setBrightness(byte sBrightness, bool GammaCorrection)
{
    /* Master output on and off, driven by enablePixels() and disablePixels(). The
       buffer is left alone, so switching back on restores the picture without having to
       copy it away first. */
    Brightness = sBrightness;
    Dirty = true;

    UNUSED(GammaCorrection);
}
