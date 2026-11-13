// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator settings window.
 */

#include "sim/Settings.h"
/* the simulated light sensor the illuminance slider feeds */
#include "sim/BH1750.h"
/* and the simulated clock chip behind the temperature overlay */
#include "sim/DS3231.h"

BEGIN_EVENT_TABLE(Settings, wxDialog)
  EVT_CLOSE(Settings::OnClose)
  EVT_SLIDER(kIdSliderIlluminance, Settings::OnIlluminance)
  EVT_SLIDER(kIdSliderTemperature, Settings::OnTemperature)
  EVT_CHECKBOX(kIdCheckboxTemperatureConnected, Settings::OnTemperatureConnected)
END_EVENT_TABLE()

Settings::Settings(wxWindow* parent, wxString const& title) : wxDialog(parent, wxID_ANY, title) {
  // No content whose size only settles once the window is mapped, unlike the letter
  // grid in Pixels, so fitting right here is enough — no CallAfter() needed.
  SetSizerAndFit(CreateSizerAll(this));

  /*
   * Under X11 this places the window. Under Wayland a client cannot position its own
   * toplevel at all, so a frame lands wherever the compositor puts it; what centres it
   * there is being a dialog that is transient for the main window, which the compositor
   * then places over its parent. Hence wxDialog and not wxFrame. Only on construction,
   * so a window the user has dragged somewhere stays put when it is closed and opened
   * again.
   */
  CentreOnParent();
}

Settings::~Settings() {}

wxBoxSizer* Settings::CreateSizerAll(wxWindow* parent) {
  wxBoxSizer* sizer_all = new wxBoxSizer(wxVERTICAL);

  sizer_all->Add(CreateSizerIlluminance(parent), 0, wxALL | wxEXPAND, kBorder);
  sizer_all->Add(CreateSizerTemperature(parent), 0, wxALL | wxEXPAND, kBorder);

  return sizer_all;
}

wxBoxSizer* Settings::CreateSizerIlluminance(wxWindow* parent) {
  wxStaticBox* static_box = new wxStaticBox(parent, kIdStaticBoxIlluminance, _T("Light sensor"));
  wxStaticBoxSizer* sizer_illuminance = new wxStaticBoxSizer(static_box, wxVERTICAL);
  wxStaticText* illuminance_label = new wxStaticText(parent, wxID_ANY, _T("Illuminance (%)"));

  // Stands in for the light sensor, which has nothing to measure here. Only has an
  // effect while the brightness automatic is on (command 3 -A1).
  illuminance_slider_ =
      new wxSlider(parent, kIdSliderIlluminance, kIlluminancePercentInitial, kIlluminancePercentMin,
                   kIlluminancePercentMax, wxDefaultPosition, wxSize(kSliderWidth, -1),
                   wxSL_HORIZONTAL | wxSL_LABELS);

  sizer_illuminance->Add(illuminance_label, 0, wxLEFT | wxTOP, kBorder);
  sizer_illuminance->Add(illuminance_slider_, 0, wxALL | wxEXPAND, kBorder);

  return sizer_illuminance;
}

/*
 * Builds the group that stands in for the clock chip's thermometer. The check box is what the
 * slider alone could not express: a clock built without the chip, and every clock until the
 * chip has answered once. The temperature overlay stays away in that state, which is the
 * behaviour that has to be tryable here. It starts cleared for the same reason - the simulator
 * has no chip until somebody says it has.
 */
wxBoxSizer* Settings::CreateSizerTemperature(wxWindow* parent) {
  wxStaticBox* static_box = new wxStaticBox(parent, kIdStaticBoxTemperature, _T("Clock chip"));
  wxStaticBoxSizer* sizer_temperature = new wxStaticBoxSizer(static_box, wxVERTICAL);
  wxStaticText* temperature_label = new wxStaticText(parent, wxID_ANY, _T("Temperature (°C)"));

  temperature_connected_check_box_ =
      new wxCheckBox(parent, kIdCheckboxTemperatureConnected, _T("Sensor connected"));
  temperature_connected_check_box_->SetValue(false);

  temperature_slider_ =
      new wxSlider(parent, kIdSliderTemperature, kTemperatureCelsiusInitial, kTemperatureCelsiusMin,
                   kTemperatureCelsiusMax, wxDefaultPosition, wxSize(kSliderWidth, -1),
                   wxSL_HORIZONTAL | wxSL_LABELS);

  sizer_temperature->Add(temperature_connected_check_box_, 0, wxLEFT | wxTOP, kBorder);
  sizer_temperature->Add(temperature_label, 0, wxLEFT | wxTOP, kBorder);
  sizer_temperature->Add(temperature_slider_, 0, wxALL | wxEXPAND, kBorder);

  return sizer_temperature;
}

void Settings::OnClose(wxCloseEvent& event) {
  /*
   * Hide rather than destroy: GetInstance() keeps handing out this very dialog, so a
   * destroyed one would leave the singleton pointing at freed memory. The parent
   * window destroys it when the application exits.
   */
  Hide();
  UNUSED(event);
}

void Settings::OnIlluminance(wxCommandEvent& event) {
  /*
   * The slider is a percentage of the calibration maximum, which is the top of what the
   * brightness automatic spreads between. The sensor's own range would put every useful
   * room in the bottom percent of the slider.
   */
  BH1750::IlluminanceType const maximum = BH1750_CALIBRATION_MAX_DEFAULT_LX_VALUE;
  BH1750::SetSimulatedIlluminance(static_cast<BH1750::IlluminanceType>(
      (maximum / kIlluminancePercentMax) * illuminance_slider_->GetValue()));
  UNUSED(event);
}

void Settings::OnTemperature(wxCommandEvent& event) {
  ApplyTemperature();
  UNUSED(event);
}

void Settings::OnTemperatureConnected(wxCommandEvent& event) {
  ApplyTemperature();
  UNUSED(event);
}

/*
 * Hands the dialled-in reading to the simulated clock chip. Both controls end here, because
 * between them they describe one state: a chip that is there and reads this much, or no chip at
 * all. Moving the slider while the box is cleared therefore changes nothing yet, and ticking the
 * box afterwards puts that value in - which is the order somebody trying the overlay out will
 * use it in.
 */
void Settings::ApplyTemperature() {
  if (!temperature_connected_check_box_->IsChecked()) {
    DS3231::ClearSimulatedTemperature();
    return;
  }

  DS3231::SetSimulatedTemperature(static_cast<DS3231::TemperatureType>(
      temperature_slider_->GetValue() * DS3231::kTenthsPerDegree));
}
