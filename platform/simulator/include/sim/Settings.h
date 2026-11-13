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

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SETTINGS_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SETTINGS_H_

#include <wx/wx.h>
#include "StandardTypes.h"
#include "PixelsFrame.h"

/*!
 * \brief The settings window: the controls that stand in for hardware the PC does not have, the
 *        light sensor's illuminance and the clock chip's temperature.
 *
 * The illuminance slider sets the simulated reading of BH1750, as a percentage of
 * BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE. The temperature slider and the Sensor connected box
 * set the simulated reading of DS3231, or clear it while the box is not ticked, which it is not
 * at first.
 *
 * A dialog rather than a frame: that is what gets it centred over the main window instead of
 * dropped wherever the compositor pleases, see the constructor. Shown with Show(), not ShowModal(),
 * so it does not block the main window. Closing it hides it.
 */
class Settings : public wxDialog {
 private:
  /* Own range, so the ids stay clear of the pixel matrix window's. */
  enum {
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

  Settings(wxWindow* parent, wxString const& title);
  ~Settings();

  void OnClose(wxCloseEvent& event);
  void OnIlluminance(wxCommandEvent& event);
  void OnTemperature(wxCommandEvent& event);
  void OnTemperatureConnected(wxCommandEvent& event);

  void ApplyTemperature();

  wxBoxSizer* CreateSizerAll(wxWindow* parent);
  wxBoxSizer* CreateSizerIlluminance(wxWindow* parent);
  wxBoxSizer* CreateSizerTemperature(wxWindow* parent);

 public:
  /*!
   * \brief Returns the one settings window, which the first call creates, hidden and centred over
   *        the window of PixelsFrame, its parent.
   *
   * \return The settings window.
   */
  static Settings& GetInstance() {
    static Settings* p_singleton_instance =
        new Settings(&PixelsFrame::GetInstance(), _("Wordclock Settings"));
    return *p_singleton_instance;
  }

  /*! \brief Brings the window up, whether it was never shown or only hidden. */
  void Reveal() {
    Show();
    Raise();
  }
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_SETTINGS_H_
