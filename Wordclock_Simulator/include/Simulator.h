#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <wx/wx.h>
#include <wx/button.h>
#include <wx/statline.h>

#include "StandardTypes.h"
#include "Arduino.h"


#define DISPLAY_NUMBER_OF_ROWS                  10
#define DISPLAY_NUMBER_OF_COLUMNS               11
#define SIMULATOR


/* type which describes the structure of a pixel */
typedef struct {
    byte Red;
    byte Green;
    byte Blue;
} WS2812PixelType;


class Simulator : public wxFrame
{
    public:
        Simulator(wxFrame *dlg, const wxString& title);
        virtual ~Simulator();


        // get methods
        //byte getBrightness() { return Brightness; }
        stdReturnType getPixel(byte, WS2812PixelType*) const;
        WS2812PixelType getPixelFast(byte) const;
        //stdReturnType getPixelDimmed(byte, WS2812PixelType*);

        // set methods
        //stdReturnType setBrightness(byte);
        //stdReturnType setPin(byte);
        stdReturnType setPixel(byte, WS2812PixelType);
        stdReturnType setPixel(byte, byte, byte, byte);
        stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        void setPixelFast(byte, WS2812PixelType);
        void setPixelFast(byte, byte, byte, byte);
        void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

        // methods
        //void init();
        void clearAllPixels();
        //stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        void show() { Refresh(); }


    protected:


    private:

        enum
        {
            idBtnQuit = 1000,
            idBtnAbout
        };

        wxStaticText* Characters[DISPLAY_NUMBER_OF_ROWS][DISPLAY_NUMBER_OF_COLUMNS];

        wxButton* BtnAbout;
        wxButton* BtnQuit;

        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()
};

#endif // SIMULATOR_H



