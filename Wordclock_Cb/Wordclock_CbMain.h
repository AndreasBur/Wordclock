/***************************************************************
 * Name:      Wordclock_CbMain.h
 * Purpose:   Defines Application Frame
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef WORDCLOCK_CBMAIN_H
#define WORDCLOCK_CBMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "Wordclock_CbApp.h"


#include <wx/button.h>
#include <wx/statline.h>

#include "StandardTypes.h"

#define DISPLAY_NUMBER_OF_ROWS                  10
#define DISPLAY_NUMBER_OF_COLUMNS               11

/* type which describes the structure of a pixel */
typedef struct {
    byte Red;
    byte Green;
    byte Blue;
} WS2812PixelType;

class Wordclock_CbDialog: public wxDialog
{
    public:
        Wordclock_CbDialog(wxDialog *dlg, const wxString& title);
        ~Wordclock_CbDialog();

        // get methods
        //byte getBrightness() { return Brightness; }
        stdReturnType getPixel(byte, WS2812PixelType*);
        //stdReturnType getPixelDimmed(byte, WS2812PixelType*);

        // set methods
        //stdReturnType setBrightness(byte);
        //stdReturnType setPin(byte);
        stdReturnType setPixel(byte, WS2812PixelType);
        stdReturnType setPixel(byte, byte, byte, byte);

        // methods
        //void init();
        void clearAllPixels();
        //stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        void show() { Refresh(); }

    protected:
        enum
        {
            idBtnQuit = 1000,
            idBtnAbout
        };

        wxStaticText* Characters[DISPLAY_NUMBER_OF_ROWS][DISPLAY_NUMBER_OF_COLUMNS];

        wxButton* BtnAbout;
        wxButton* BtnQuit;

    private:
        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()
};

#endif // WORDCLOCK_CBMAIN_H
