#include "sim/Pixels.h"

StdReturnType Pixels::getPixel(byte Index, PixelType& Pixel) const
{
    if(!isIndexValid(Index)) { return E_NOT_OK; }

    Pixel = PixelBuffer[toRow(Index)][toColumn(Index)];
    return E_OK;
}

Pixels::PixelType Pixels::getPixel(byte Index) const
{
    /* An index that is not on the strip has no pixel, and an unlit one is the answer no
       caller can mistake for an LED that is on. */
    if(!isIndexValid(Index)) { return PixelType{}; }

    return PixelBuffer[toRow(Index)][toColumn(Index)];
}

StdReturnType Pixels::setPixel(byte Index, PixelType Pixel)
{
    if(!isIndexValid(Index)) { return E_NOT_OK; }

    PixelBuffer[toRow(Index)][toColumn(Index)] = Pixel;
    Dirty = true;
    return E_OK;
}

StdReturnType Pixels::setPixel(byte Index, byte Red, byte Green, byte Blue)
{
    return setPixel(Index, PixelType(Red, Green, Blue));
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
