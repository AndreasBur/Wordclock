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
/**     \file       PixelsFrame.h
 *      \brief      Simulator LED matrix window
 *
 *      \details    wxWidgets frame that renders the Pixels buffer as the 10x11
 *                  character grid. It is the only part of the pixel stand-in that needs
 *                  a GUI, which is why it is a class of its own: the firmware talks to
 *                  Pixels, and Pixels knows nothing about this.
 *
 *                  It also lays out the serial output and input controls, but only
 *                  lays them out: what they mean belongs to SerialShim, which they are
 *                  handed to.
 *
******************************************************************************************************************************************************/
#ifndef PIXELS_FRAME_H
#define PIXELS_FRAME_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include <wx/button.h>
#include <cmath>
#include <array>
#include "StandardTypes.h"
#include "Pixels.h"

/******************************************************************************************************************************************************
 *  C L A S S   P I X E L S   F R A M E
******************************************************************************************************************************************************/
class PixelsFrame : public wxFrame
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using PixelType = Pixels::PixelType;
    using SizerCharactersType = std::array<wxBoxSizer*, PIXELS_DISPLAY_NUMBER_OF_ROWS>;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    enum
    {
        kIdButtonClear = 1000,
        kIdButtonSend,
        kIdTextCtrlOutput,
        kIdTextCtrlInput,
        kIdStaticBox,
        kIdButtonCreate,
        /* No stock wx id fits the message builder, unlike Settings, About and Quit. */
        kIdMenuMessage
    };

    wxStaticText* characters_[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS];

    static constexpr byte kIntensityMaxValue{255u};
    static constexpr byte kUnlitLevel{192u};             /* wxLIGHT_GREY */

    DECLARE_EVENT_TABLE()

    // functions
    /* The brightest channel decides how far the letter is pulled from the unlit colour
       towards black. Colour itself cannot be shown on a light background — a white LED
       at full would be invisible — so only the brightness is rendered. */
    static byte GetIntensity(PixelType pixel) {
        byte intensity = pixel.GetRed();
        if(pixel.GetGreen() > intensity) { intensity = pixel.GetGreen(); }
        if(pixel.GetBlue() > intensity) { intensity = pixel.GetBlue(); }
        return intensity;
    }

    /* Weighted rather than proportional. Unlit letters sit at kUnlitLevel and not at
       black, which leaves the dark end of the range squeezed: proportionally, a pixel at
       3 percent would land five levels off the unlit colour and be invisible, while on
       real LEDs it is clearly visible against LEDs that are truly off. The square root
       spreads the low end, which is also closer to how the eye reads brightness. */
    static byte GetWeightedIntensity(byte intensity) {
        return static_cast<byte>(std::sqrt(static_cast<double>(intensity) * kIntensityMaxValue));
    }

    /* Grey level of the letter, which the weighted intensity pulls from kUnlitLevel
       towards black. */
    static constexpr byte ToLevel(byte weighted_intensity) {
        return static_cast<byte>(kUnlitLevel - ((kUnlitLevel * weighted_intensity) / kIntensityMaxValue));
    }

    PixelsFrame(wxWindow*, const wxString&);
    ~PixelsFrame();
    void OnClose(wxCloseEvent&);
    void OnClear(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnSend(wxCommandEvent&);
    void OnSettings(wxCommandEvent&);
    void OnMessage(wxCommandEvent&);
    void OnQuit(wxCommandEvent&);
    static wxColour ToColour(PixelType);
    void RenderPixel(byte row, byte column);
    void RenderAllPixels();
    static wxMenuBar* CreateMenuBar();
    wxBoxSizer* CreateSizerAll(wxWindow*);
    wxBoxSizer* CreateSizerCharacters(wxWindow*);
    wxBoxSizer* CreateSizerCharacter(wxWindow*, int row);
    static wxBoxSizer* CreateSizerControl(wxWindow*);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static PixelsFrame& GetInstance() {
        static PixelsFrame* p_singleton_instance = new PixelsFrame(0L, _("Wordclock Pixels"));
        return *p_singleton_instance;
    }

    // methods
    void Render();
};

#endif // PIXELS_FRAME_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
