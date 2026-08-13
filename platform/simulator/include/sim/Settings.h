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
/**     \file       Settings.h
 *      \brief      Simulator settings window
 *
 *      \details    Holds the controls that stand in for hardware the PC does not
 *                  have, starting with the light sensor's illuminance. Kept out of
 *                  the pixel matrix window so that one stays the size of the
 *                  letter grid. Shown without blocking it, so the effect of a
 *                  control is visible on the grid while it is being changed.
 *
******************************************************************************************************************************************************/
#ifndef SETTINGS_H
#define SETTINGS_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include "StandardTypes.h"
#include "PixelsFrame.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   S E T T I N G S
******************************************************************************************************************************************************/
/* A dialog rather than a frame: that is what gets it centred over the main window
   instead of dropped wherever the compositor pleases, see the constructor. Shown with
   Show(), not ShowModal(), so it does not block the main window. */
class Settings : public wxDialog
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Own range, so the ids stay clear of the pixel matrix window's. */
    enum
    {
        ID_SLIDER_ILLUMINANCE = 2000,
        ID_STATIC_BOX_ILLUMINANCE
    };

    wxSlider* IlluminanceSlider;

    static constexpr int IlluminancePercentMin{0};
    static constexpr int IlluminancePercentMax{100};
    /* Full daylight, which leaves the brightness automatic at its brightest. */
    static constexpr int IlluminancePercentInitial{100};
    static constexpr int SliderWidth{200};
    static constexpr int Border{10};

    DECLARE_EVENT_TABLE()

    // functions
    Settings(wxWindow*, const wxString&);
    ~Settings();

    void OnClose(wxCloseEvent&);
    void OnIlluminance(wxCommandEvent&);

    wxBoxSizer* createSizerAll(wxWindow*);
    wxBoxSizer* createSizerIlluminance(wxWindow*);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Settings& getInstance() {
        static Settings* pSingletonInstance = new Settings(&PixelsFrame::getInstance(), _("Wordclock Settings"));
        return *pSingletonInstance;
    }

    // methods
    /* Brings the window up, whether it was never shown or only hidden. */
    void reveal() {
        Show();
        Raise();
    }
};

#endif // SETTINGS_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
