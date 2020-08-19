#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <wx/wx.h>
#include <wx/button.h>

#include "StandardTypes.h"
#include "Arduino.h"
#include "DisplayCharacters.h"


#define DISPLAY_NUMBER_OF_ROWS                  DISPLAY_CHARACTERS_NUMBER_OF_ROWS
#define DISPLAY_NUMBER_OF_COLUMNS               DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS
#define SIMULATOR


class Simulator : public wxFrame
{
    public:
        static Simulator& getInstance() {
            static Simulator* pSingletonInstance = new Simulator(0L, _("Wordclock Simulator"));
            return *pSingletonInstance;
        }

        /* type which describes the structure of a pixel */
        typedef struct {
            byte Red;
            byte Green;
            byte Blue;
        } PixelType;

        using SizerCharactersType = std::array<wxBoxSizer*, DISPLAY_NUMBER_OF_ROWS>;

        // get methods
        byte getBrightness() const { return Brightness; }
        StdReturnType getPixel(byte, PixelType&) const;
        PixelType getPixelFast(byte) const;
        //StdReturnType getPixelDimmed(byte, WS2812PixelType*);

        // set methods
        void setBrightness(byte, bool = false);
        //StdReturnType setPin(byte);
        StdReturnType setPixel(byte, PixelType);
        StdReturnType setPixel(byte, byte, byte, byte);
        StdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        void setPixelFast(byte, PixelType);
        void setPixelFast(byte, byte, byte, byte);
        void clearPixelFast(byte Index) { setPixelFast(Index, 0, 0, 0); }

        // methods
        void init(byte sPin) { Pin = sPin; }
        void enablePixels() { setBrightness(255); }
        void disablePixels() { setBrightness(0); }
        void clearPixels();
        //void setAllPixels(PixelType) {}
        //StdReturnType clearPixel(byte Index) { return setPixel(Index, 0, 0, 0); }
        StdReturnType show() {
            Refresh();
            return E_OK;
        }


    protected:


    private:

        enum
        {
            ID_BUTTON_QUIT = 1000,
            ID_BUTTON_ABOUT,
            ID_BUTTON_SEND,
            ID_TEXT_CTRL_OUTPUT,
            ID_TEXT_CTRL_INPUT,
            ID_STATIC_BOX
        };

        wxStaticText* Characters[DISPLAY_NUMBER_OF_ROWS][DISPLAY_NUMBER_OF_COLUMNS];
        wxColor Colors[DISPLAY_NUMBER_OF_ROWS][DISPLAY_NUMBER_OF_COLUMNS];
        byte Pin;
        byte Brightness;
//
//        wxButton* BtnAbout;
//        wxButton* BtnQuit;
//        wxButton* BtnSend;
//
//        wxTextCtrl* Output;
//        wxTextCtrl* Input;

        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        DECLARE_EVENT_TABLE()

        // functions
        Simulator(wxFrame *dlg, const wxString& title);
        virtual ~Simulator();
        void setAllPixels(wxColour);
        wxBoxSizer* createSizerCharacters();
        wxBoxSizer* createSizerCharacter(int Row);
        wxBoxSizer* createSizerButton();
        wxBoxSizer* createSizerControl();
};

#endif // SIMULATOR_H



