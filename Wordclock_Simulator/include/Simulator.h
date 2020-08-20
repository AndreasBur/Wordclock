#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <wx/wx.h>
#include <wx/button.h>

#include "StandardTypes.h"
//#include "DisplayCharacters.h"


#define SIMULATOR_DISPLAY_NUMBER_OF_ROWS                  11u
#define SIMULATOR_DISPLAY_NUMBER_OF_COLUMNS               10u


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

        using SizerCharactersType = std::array<wxBoxSizer*, SIMULATOR_DISPLAY_NUMBER_OF_ROWS>;

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

        void println() { Output->AppendText(_T("\n")); }
        void print(const char* Text) { Output->AppendText(Text); }
        void print(int Number) { Output->AppendText(wxString::Format(wxT("%i"), Number)); }
        void println(const char* Text) { print(Text); println(); }
        void println(int Number) { print(Number); println(); }

        bool available() { return false; }
        char read() { return ' '; }

    protected:


    private:
        enum
        {
            ID_BUTTON_QUIT = 1000,
            ID_BUTTON_ABOUT,
            ID_BUTTON_CLEAR,
            ID_BUTTON_SEND,
            ID_TEXT_CTRL_OUTPUT,
            ID_TEXT_CTRL_INPUT,
            ID_STATIC_BOX
        };

        wxTextCtrl* Output;
        //wxTextCtrl* Output;

        wxStaticText* Characters[SIMULATOR_DISPLAY_NUMBER_OF_ROWS][SIMULATOR_DISPLAY_NUMBER_OF_COLUMNS];
        wxColor Colors[SIMULATOR_DISPLAY_NUMBER_OF_ROWS][SIMULATOR_DISPLAY_NUMBER_OF_COLUMNS];
        byte Pin{0};
        byte Brightness{255};

        void OnClose(wxCloseEvent&);
        void OnClear(wxCommandEvent&);
        void OnAbout(wxCommandEvent&);
        void OnQuit(wxCommandEvent&);
        DECLARE_EVENT_TABLE()

        // functions
        Simulator(wxWindow*, const wxString&);
        virtual ~Simulator();
        void setAllPixels(wxColour);
        wxBoxSizer* createSizerAll(wxWindow*);
        wxBoxSizer* createSizerCharacters(wxWindow*);
        wxBoxSizer* createSizerCharacter(wxWindow*, int Row);
        wxBoxSizer* createSizerButton(wxWindow*);
        wxBoxSizer* createSizerControl(wxWindow*);
};

#endif // SIMULATOR_H



