#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <wx/wx.h>
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


class Simulator : public wxDialog
{
    public:
        Simulator(wxDialog *dlg, const wxString& title);
        virtual ~Simulator();


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

#endif // SIMULATOR_H



