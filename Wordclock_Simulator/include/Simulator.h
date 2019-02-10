#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <wx/wx.h>
#include <wx/button.h>
#include <wx/statline.h>

#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayCharacters.h"


#define DISPLAY_NUMBER_OF_ROWS                  DISPLAY_CHARACTERS_NUMBER_OF_ROWS
#define DISPLAY_NUMBER_OF_COLUMNS               DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
#define SIMULATOR


class Simulator : public wxFrame
{
    public:
        /* type which describes the structure of a pixel */
        typedef struct {
            byte Red;
            byte Green;
            byte Blue;
        } PixelType;

        Simulator(wxFrame *dlg, const wxString& title);
        virtual ~Simulator();

        // get methods
        byte getBrightness() const { return Brightness; }
        stdReturnType getPixel(byte, PixelType&) const;
        PixelType getPixelFast(byte) const;
        //stdReturnType getPixelDimmed(byte, WS2812PixelType*);

        // set methods
        void setBrightness(byte, bool = false);
        //stdReturnType setPin(byte);
        stdReturnType setPixel(byte, PixelType);
        stdReturnType setPixel(byte, byte, byte, byte);
        stdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        void setPixelFast(byte, PixelType);
        void setPixelFast(byte, byte, byte, byte);
        void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

        // methods
        void init(byte sPin) { Pin = sPin; }
        void enablePixels() { setBrightness(255); }
        void disablePixels() { setBrightness(0); }
        void clearPixels();
        //void setAllPixels(PixelType) {}
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
        wxColor Colors[DISPLAY_NUMBER_OF_ROWS][DISPLAY_NUMBER_OF_COLUMNS];
        byte Pin;
        byte Brightness;

        wxButton* BtnAbout;
        wxButton* BtnQuit;

        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()

        // functions
        void setAllPixels(wxColour);
};

#endif // SIMULATOR_H



