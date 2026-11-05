// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator LED matrix window.
 *
 * wxWidgets frame that renders the Pixels buffer as the 10x11 character grid. It is the only part
 * of the pixel stand-in that needs a GUI, which is why it is a class of its own: the firmware talks
 * to Pixels, and Pixels knows nothing about this.
 *
 * It also lays out the serial output and input controls, but only lays them out: what they mean
 * belongs to SerialShim, which they are handed to.
 */

#ifndef PIXELS_FRAME_H
#define PIXELS_FRAME_H

#include <wx/wx.h>
#include <wx/button.h>
#include <cmath>
#include <array>
#include "StandardTypes.h"
#include "Pixels.h"

/*!
 * \brief The window of the simulator: the letter grid, drawn from Pixels, and the serial port's
 *        output and input fields with their buttons.
 *
 * A lit letter is drawn in a grey between the unlit letter colour and black, by the brightness of
 * its pixel; the hue is not shown. Its menu opens Settings and MessageBuilder.
 */
class PixelsFrame : public wxFrame
{
  public:
    using PixelType = Pixels::PixelType; /*!< A pixel of the buffer the window draws. */
    /*! \brief An array of one box sizer per row of the grid. */
    using SizerCharactersType = std::array<wxBoxSizer*, PIXELS_DISPLAY_NUMBER_OF_ROWS>;

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

    /*
     * The brightest channel decides how far the letter is pulled from the unlit colour
     * towards black. Colour itself cannot be shown on a light background — a white LED
     * at full would be invisible — so only the brightness is rendered.
     */
    static byte GetIntensity(PixelType pixel) {
        byte intensity = pixel.GetRed();
        if(pixel.GetGreen() > intensity) { intensity = pixel.GetGreen(); }
        if(pixel.GetBlue() > intensity) { intensity = pixel.GetBlue(); }
        return intensity;
    }

    /*
     * Weighted rather than proportional. Unlit letters sit at kUnlitLevel and not at
     * black, which leaves the dark end of the range squeezed: proportionally, a pixel at
     * 3 percent would land five levels off the unlit colour and be invisible, while on
     * real LEDs it is clearly visible against LEDs that are truly off. The square root
     * spreads the low end, which is also closer to how the eye reads brightness.
     */
    static byte GetWeightedIntensity(byte intensity) {
        return static_cast<byte>(std::sqrt(static_cast<double>(intensity) * kIntensityMaxValue));
    }

    // Grey level of the letter, which the weighted intensity pulls from kUnlitLevel
    // towards black.
    static constexpr byte ToLevel(byte weighted_intensity) {
        return static_cast<byte>(kUnlitLevel - ((kUnlitLevel * weighted_intensity) / kIntensityMaxValue));
    }

    PixelsFrame(wxWindow* parent, const wxString& title);
    ~PixelsFrame();
    void OnClose(wxCloseEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    void OnSettings(wxCommandEvent& event);
    void OnMessage(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    static wxColour ToColour(PixelType pixel);
    void RenderPixel(byte row, byte column);
    void RenderAllPixels();
    static wxMenuBar* CreateMenuBar();
    wxBoxSizer* CreateSizerAll(wxWindow* parent);
    wxBoxSizer* CreateSizerCharacters(wxWindow* parent);
    wxBoxSizer* CreateSizerCharacter(wxWindow* parent, int row);
    static wxBoxSizer* CreateSizerControl(wxWindow* parent);

  public:
    /*!
     * \brief Returns the one window, which the first call creates, without a parent and hidden.
     *
     * Creating it attaches the output and input fields to SerialShim.
     *
     * \return The window.
     * \note Closing the window destroys it, through File, Quit or the frame's own close button, and
     *       nothing creates it again; GetInstance() must not be called after that.
     */
    static PixelsFrame& GetInstance() {
        static PixelsFrame* p_singleton_instance = new PixelsFrame(0L, _("Wordclock Pixels"));
        return *p_singleton_instance;
    }

    /*!
     * \brief Redraws the letter grid from Pixels, if the firmware wrote to it since the last time.
     *
     * Called once per application tick rather than from inside every pixel write, so the window
     * is repainted at most once per pass over the firmware's tasks - and not at all on the ticks
     * where the firmware wrote nothing.
     *
     * It draws nothing while Pixels::IsOutputSuspended() is true, and leaves the buffer dirty, so
     * that the supply coming back redraws it. Otherwise it clears the dirty flag of Pixels and
     * gives every letter the grey of its pixel; at a brightness of 0 every letter is drawn unlit.
     */
    void Render();
};

#endif // PIXELS_FRAME_H
