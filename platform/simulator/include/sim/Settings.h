// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator settings window.
 *
 * Holds the controls that stand in for hardware the PC does not have, starting with the light
 * sensor's illuminance. Kept out of the pixel matrix window so that one stays the size of the
 * letter grid. Shown without blocking it, so the effect of a control is visible on the grid while
 * it is being changed.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <wx/wx.h>
#include "StandardTypes.h"
#include "PixelsFrame.h"

/*
 * A dialog rather than a frame: that is what gets it centred over the main window
 * instead of dropped wherever the compositor pleases, see the constructor. Shown with
 * Show(), not ShowModal(), so it does not block the main window.
 */
class Settings : public wxDialog
{
  private:
    /* Own range, so the ids stay clear of the pixel matrix window's. */
    enum
    {
        kIdSliderIlluminance = 2000,
        kIdStaticBoxIlluminance,
        kIdSliderTemperature,
        kIdCheckboxTemperatureConnected,
        kIdStaticBoxTemperature
    };

    wxSlider* illuminance_slider_;
    wxSlider* temperature_slider_;
    wxCheckBox* temperature_connected_check_box_;

    static constexpr int kIlluminancePercentMin{0};
    static constexpr int kIlluminancePercentMax{100};
    /* Full daylight, which leaves the brightness automatic at its brightest. */
    static constexpr int kIlluminancePercentInitial{100};
    // Whole degrees Celsius; the overlay shows a tenth, which the slider does not need to
    // reach for what it is here to try out. Well inside what the chip can measure.
    static constexpr int kTemperatureCelsiusMin{-20};
    static constexpr int kTemperatureCelsiusMax{50};
    static constexpr int kTemperatureCelsiusInitial{21};
    static constexpr int kSliderWidth{200};
    static constexpr int kBorder{10};

    DECLARE_EVENT_TABLE()

    // functions
    Settings(wxWindow*, const wxString&);
    ~Settings();

    void OnClose(wxCloseEvent&);
    void OnIlluminance(wxCommandEvent&);
    void OnTemperature(wxCommandEvent&);
    void OnTemperatureConnected(wxCommandEvent&);

    void ApplyTemperature();

    wxBoxSizer* CreateSizerAll(wxWindow*);
    wxBoxSizer* CreateSizerIlluminance(wxWindow*);
    wxBoxSizer* CreateSizerTemperature(wxWindow*);

  public:
    static Settings& GetInstance() {
        static Settings* p_singleton_instance = new Settings(&PixelsFrame::GetInstance(), _("Wordclock Settings"));
        return *p_singleton_instance;
    }

    // methods
    /* Brings the window up, whether it was never shown or only hidden. */
    void Reveal() {
        Show();
        Raise();
    }
};

#endif // SETTINGS_H
