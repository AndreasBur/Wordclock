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
/**     \file       Pixels.h
 *      \brief      Simulator LED matrix window (replaces the WS2812 pixel strip)
 *
 *      \details    wxWidgets frame that renders the 10x11 character grid as
 *                  coloured cells. Mirrors the public API of the hardware pixel
 *                  driver (get/set/clear pixel, brightness, show) and also backs
 *                  the Arduino Serial shim via its print/read methods.
 *
******************************************************************************************************************************************************/
#ifndef PIXELS_H
#define PIXELS_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include <wx/button.h>
#include <cmath>
#include "StandardTypes.h"
#include "Pixel.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
#define PIXELS_NUMBER_OF_LEDS                           110
#define PIXELS_SUPPORT_DIMMING                          STD_ON
#define PIXELS_IS_SINGLETON                             STD_ON

#define PIXELS_DISPLAY_NUMBER_OF_ROWS                   10u
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS                11u
#define PIXELS_NUMBER_OF_PIXELS                         (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   P I X E L S
******************************************************************************************************************************************************/
class Pixels : public wxFrame
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using PixelType = Pixel;
# if (PIXELS_NUMBER_OF_PIXELS < 255u)
    using IndexType = byte;
# else
    using IndexType = uint16_t;
# endif
    using SizerCharactersType = std::array<wxBoxSizer*, PIXELS_DISPLAY_NUMBER_OF_ROWS>;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    enum
    {
        ID_BUTTON_QUIT = 1000,
        ID_BUTTON_ABOUT,
        ID_BUTTON_CLEAR,
        ID_BUTTON_SEND,
        ID_TEXT_CTRL_OUTPUT,
        ID_TEXT_CTRL_INPUT,
        ID_SLIDER_ILLUMINANCE,
        ID_STATIC_BOX
    };

    wxString SendBuffer{""};
    wxTextCtrl* Output;
    wxTextCtrl* Input;
    wxSlider* IlluminanceSlider;

    wxStaticText* Characters[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS];
    PixelType PixelBuffer[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS];
    byte Pin{0};
    byte Brightness{255};

    static constexpr byte IntensityMaxValue{255u};
    static constexpr byte UnlitLevel{192u};             /* wxLIGHT_GREY */

    DECLARE_EVENT_TABLE()

    // functions
    /* The brightest channel decides how far the letter is pulled from the unlit colour
       towards black. Colour itself cannot be shown on a light background — a white LED
       at full would be invisible — so only the brightness is rendered. */
    static byte getIntensity(PixelType Pixel) {
        byte Intensity = Pixel.getRed();
        if(Pixel.getGreen() > Intensity) { Intensity = Pixel.getGreen(); }
        if(Pixel.getBlue() > Intensity) { Intensity = Pixel.getBlue(); }
        return Intensity;
    }

    /* Weighted rather than proportional. Unlit letters sit at UnlitLevel and not at
       black, which leaves the dark end of the range squeezed: proportionally, a pixel at
       3 percent would land five levels off the unlit colour and be invisible, while on
       real LEDs it is clearly visible against LEDs that are truly off. The square root
       spreads the low end, which is also closer to how the eye reads brightness. */
    static byte getWeightedIntensity(byte Intensity) {
        return static_cast<byte>(std::sqrt(static_cast<double>(Intensity) * IntensityMaxValue));
    }

    /* Grey level of the letter, which the weighted intensity pulls from UnlitLevel
       towards black. */
    static constexpr byte toLevel(byte WeightedIntensity) {
        return static_cast<byte>(UnlitLevel - ((UnlitLevel * WeightedIntensity) / IntensityMaxValue));
    }

    Pixels(wxWindow*, const wxString&);
    ~Pixels();
    void OnClose(wxCloseEvent&);
    void OnClear(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnSend(wxCommandEvent&);
    void OnIlluminance(wxCommandEvent&);
    void OnQuit(wxCommandEvent&);
    void setPixels(wxColour);
    wxColour toColour(PixelType) const;
    void renderPixel(byte Row, byte Column);
    void renderAllPixels();
    wxBoxSizer* createSizerAll(wxWindow*);
    wxBoxSizer* createSizerCharacters(wxWindow*);
    wxBoxSizer* createSizerCharacter(wxWindow*, int Row);
    wxBoxSizer* createSizerButton(wxWindow*);
    wxBoxSizer* createSizerControl(wxWindow*);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Pixels& getInstance() {
        static Pixels* pSingletonInstance = new Pixels(0L, _("Wordclock Pixels"));
        return *pSingletonInstance;
    }

    // get methods
    byte getBrightness() const { return Brightness; }
    StdReturnType getPixel(byte, PixelType&) const;
    PixelType getPixelFast(byte) const;

    // set methods
    void setPixels(PixelType);
    void setBrightness(byte, bool = false);
    StdReturnType setPixel(byte, PixelType);
    StdReturnType setPixel(byte, byte, byte, byte);
    StdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
    void setPixelFast(byte, PixelType);
    void setPixelFast(byte, byte, byte, byte);
    void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

    // methods
    bool isIndexValid(IndexType Index) const { return Index < PIXELS_NUMBER_OF_PIXELS; }
    void init(byte sPin) { Pin = sPin; }
    void enablePixels() { setBrightness(255); }
    void disablePixels() { setBrightness(0); }
    void clearPixels();
    StdReturnType show() {
        Refresh();
        return E_OK;
    }

    // Serial shim (Arduino Serial is #defined to this instance)
    void println() { Output->AppendText(_T("\n")); }
    void print(const char* Text) { Output->AppendText(Text); }
    void print(int Number) { Output->AppendText(wxString::Format(wxT("%i"), Number)); }
    void println(const char* Text) { print(Text); println(); }
    void println(int Number) { print(Number); println(); }
    void print(char Char) { Output->AppendText(Char); }

    bool available() { return !SendBuffer.IsEmpty(); }
    char read();
};

#endif // PIXELS_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
