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
/**     \file       Settings.cpp
 *      \brief      Simulator settings window
 *
 *      \details
 *
******************************************************************************************************************************************************/
#define _SETTINGS_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/Settings.h"
/* the simulated light sensor the illuminance slider feeds */
#include "sim/BH1750.h"
/* and the simulated clock chip behind the temperature overlay */
#include "sim/DS3231.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
BEGIN_EVENT_TABLE(Settings, wxDialog)
    EVT_CLOSE(Settings::OnClose)
    EVT_SLIDER(ID_SLIDER_ILLUMINANCE, Settings::OnIlluminance)
    EVT_SLIDER(ID_SLIDER_TEMPERATURE, Settings::OnTemperature)
    EVT_CHECKBOX(ID_CHECKBOX_TEMPERATURE_CONNECTED, Settings::OnTemperatureConnected)
END_EVENT_TABLE()


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of Settings
******************************************************************************************************************************************************/
Settings::Settings(wxWindow* parent, const wxString &title) : wxDialog(parent, wxID_ANY, title)
{
    /* No content whose size only settles once the window is mapped, unlike the letter
       grid in Pixels, so fitting right here is enough — no CallAfter() needed. */
    SetSizerAndFit(createSizerAll(this));

    /* Under X11 this places the window. Under Wayland a client cannot position its own
       toplevel at all, so a frame lands wherever the compositor puts it; what centres it
       there is being a dialog that is transient for the main window, which the compositor
       then places over its parent. Hence wxDialog and not wxFrame. Only on construction,
       so a window the user has dragged somewhere stays put when it is closed and opened
       again. */
    CentreOnParent();
}


/******************************************************************************************************************************************************
  Destructor of Settings
******************************************************************************************************************************************************/
Settings::~Settings()
{

} /* ~Settings */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  createSizerAll()
******************************************************************************************************************************************************/
wxBoxSizer* Settings::createSizerAll(wxWindow* Parent)
{
    wxBoxSizer* SizerAll = new wxBoxSizer(wxVERTICAL);

    SizerAll->Add(createSizerIlluminance(Parent), 0, wxALL | wxEXPAND, Border);
    SizerAll->Add(createSizerTemperature(Parent), 0, wxALL | wxEXPAND, Border);

    return SizerAll;
} /* createSizerAll */


/******************************************************************************************************************************************************
  createSizerIlluminance()
******************************************************************************************************************************************************/
wxBoxSizer* Settings::createSizerIlluminance(wxWindow* Parent)
{
    wxStaticBox* StaticBox = new wxStaticBox(Parent, ID_STATIC_BOX_ILLUMINANCE, _T("Light sensor"));
    wxStaticBoxSizer* SizerIlluminance = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
    wxStaticText* IlluminanceLabel = new wxStaticText(Parent, wxID_ANY, _T("Illuminance (%)"));

    /* Stands in for the light sensor, which has nothing to measure here. Only has an
       effect while the brightness automatic is on (command 3 -A1). */
    IlluminanceSlider = new wxSlider(Parent, ID_SLIDER_ILLUMINANCE, IlluminancePercentInitial,
                                     IlluminancePercentMin, IlluminancePercentMax,
                                     wxDefaultPosition, wxSize(SliderWidth, -1),
                                     wxSL_HORIZONTAL | wxSL_LABELS);

    SizerIlluminance->Add(IlluminanceLabel, 0, wxLEFT | wxTOP, Border);
    SizerIlluminance->Add(IlluminanceSlider, 0, wxALL | wxEXPAND, Border);

    return SizerIlluminance;
} /* createSizerIlluminance */


/******************************************************************************************************************************************************
  createSizerTemperature()
******************************************************************************************************************************************************/
/*! \brief          Builds the group that stands in for the clock chip's thermometer
 *  \details        The check box is what the slider alone could not express: a clock built
 *                  without the chip, and every clock until the chip has answered once. The
 *                  temperature overlay stays away in that state, which is the behaviour
 *                  that has to be tryable here. It starts cleared for the same reason -
 *                  the simulator has no chip until somebody says it has.
******************************************************************************************************************************************************/
wxBoxSizer* Settings::createSizerTemperature(wxWindow* Parent)
{
    wxStaticBox* StaticBox = new wxStaticBox(Parent, ID_STATIC_BOX_TEMPERATURE, _T("Clock chip"));
    wxStaticBoxSizer* SizerTemperature = new wxStaticBoxSizer(StaticBox, wxVERTICAL);
    wxStaticText* TemperatureLabel = new wxStaticText(Parent, wxID_ANY, _T("Temperature (°C)"));

    TemperatureConnectedCheckBox = new wxCheckBox(Parent, ID_CHECKBOX_TEMPERATURE_CONNECTED, _T("Sensor connected"));
    TemperatureConnectedCheckBox->SetValue(false);

    TemperatureSlider = new wxSlider(Parent, ID_SLIDER_TEMPERATURE, TemperatureCelsiusInitial,
                                     TemperatureCelsiusMin, TemperatureCelsiusMax,
                                     wxDefaultPosition, wxSize(SliderWidth, -1),
                                     wxSL_HORIZONTAL | wxSL_LABELS);

    SizerTemperature->Add(TemperatureConnectedCheckBox, 0, wxLEFT | wxTOP, Border);
    SizerTemperature->Add(TemperatureLabel, 0, wxLEFT | wxTOP, Border);
    SizerTemperature->Add(TemperatureSlider, 0, wxALL | wxEXPAND, Border);

    return SizerTemperature;
} /* createSizerTemperature */


/******************************************************************************************************************************************************
  OnClose()
******************************************************************************************************************************************************/
void Settings::OnClose(wxCloseEvent &event)
{
    /* Hide rather than destroy: getInstance() keeps handing out this very dialog, so a
       destroyed one would leave the singleton pointing at freed memory. The parent
       window destroys it when the application exits. */
    Hide();
    UNUSED(event);
} /* OnClose */


/******************************************************************************************************************************************************
  OnIlluminance()
******************************************************************************************************************************************************/
void Settings::OnIlluminance(wxCommandEvent &event)
{
    /* The slider is a percentage of the calibration maximum, which is what the
       brightness automatic divides by */
    const BH1750::IlluminanceType Maximum = BH1750_ILLUMINANCE_MAX_LX_VALUE;
    BH1750::setSimulatedIlluminance(static_cast<BH1750::IlluminanceType>((Maximum / IlluminancePercentMax) * IlluminanceSlider->GetValue()));
    UNUSED(event);
} /* OnIlluminance */


/******************************************************************************************************************************************************
  OnTemperature()
******************************************************************************************************************************************************/
void Settings::OnTemperature(wxCommandEvent &event)
{
    applyTemperature();
    UNUSED(event);
} /* OnTemperature */


/******************************************************************************************************************************************************
  OnTemperatureConnected()
******************************************************************************************************************************************************/
void Settings::OnTemperatureConnected(wxCommandEvent &event)
{
    applyTemperature();
    UNUSED(event);
} /* OnTemperatureConnected */


/******************************************************************************************************************************************************
  applyTemperature()
******************************************************************************************************************************************************/
/*! \brief          Hands the dialled-in reading to the simulated clock chip
 *  \details        Both controls end here, because between them they describe one state:
 *                  a chip that is there and reads this much, or no chip at all. Moving the
 *                  slider while the box is cleared therefore changes nothing yet, and
 *                  ticking the box afterwards puts that value in - which is the order
 *                  somebody trying the overlay out will use it in.
******************************************************************************************************************************************************/
void Settings::applyTemperature()
{
    if(!TemperatureConnectedCheckBox->IsChecked()) {
        DS3231::clearSimulatedTemperature();
        return;
    }

    DS3231::setSimulatedTemperature(static_cast<DS3231::TemperatureType>(TemperatureSlider->GetValue() * DS3231::TenthsPerDegree));
} /* applyTemperature */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
