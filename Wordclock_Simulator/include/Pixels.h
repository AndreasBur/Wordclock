#ifndef PIXELS_H
#define PIXELS_H

#include <wx/wx.h>
#include <wx/button.h>
#include "StandardTypes.h"
#include "Pixel.h"

#define PIXELS_NUMBER_OF_LEDS                           110
#define PIXELS_SUPPORT_DIMMING                          STD_ON
#define PIXELS_IS_SINGLETON                             STD_ON

#define PIXELS_DISPLAY_NUMBER_OF_ROWS                   10u
#define PIXELS_DISPLAY_NUMBER_OF_COLUMNS                11u
#define PIXELS_NUMBER_OF_PIXELS                         (PIXELS_DISPLAY_NUMBER_OF_ROWS * PIXELS_DISPLAY_NUMBER_OF_COLUMNS)

class Pixels : public wxFrame
{
    public:
        static Pixels& getInstance() {
            static Pixels* pSingletonInstance = new Pixels(0L, _("Wordclock Pixels"));
            return *pSingletonInstance;
        }

        using PixelType = Pixel;
# if (PIXELS_NUMBER_OF_PIXELS < 255u)
        using IndexType = byte;
# else
        using IndexType = uint16_t;
# endif

        using SizerCharactersType = std::array<wxBoxSizer*, PIXELS_DISPLAY_NUMBER_OF_ROWS>;

        // get methods
        byte getBrightness() const { return Brightness; }
        StdReturnType getPixel(byte, PixelType&) const;
        PixelType getPixelFast(byte) const;
        void setPixels(PixelType);

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
        bool isIndexValid(IndexType Index) const { return Index < PIXELS_NUMBER_OF_PIXELS; }
        void init(byte sPin) { Pin = sPin; }
        void enablePixels() { setBrightness(255); }
        void disablePixels() { setBrightness(0); }
        void clearPixels();
        StdReturnType show() {
            Refresh();
            return E_OK;
        }

        void println() { Output->AppendText(_T("\n")); }
        void print(const char* Text) { Output->AppendText(Text); }
        void print(int Number) { Output->AppendText(wxString::Format(wxT("%i"), Number)); }
        void println(const char* Text) { print(Text); println(); }
        void println(int Number) { print(Number); println(); }
        void print(char Char) { Output->AppendText(Char); }

        bool available() { return !SendBuffer.IsEmpty(); }
        char read();

    protected:


    private:
        enum
        {
            ID_BUTTON_QUIT = 1000,
            ID_BUTTON_ABOUT,
            //ID_BUTTON_CREATE_INPUT,
            ID_BUTTON_CLEAR,
            ID_BUTTON_SEND,
            ID_TEXT_CTRL_OUTPUT,
            ID_TEXT_CTRL_INPUT,
            ID_STATIC_BOX
        };

        wxString SendBuffer{""};
        wxTextCtrl* Output;
        wxTextCtrl* Input;

        wxStaticText* Characters[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS];
        wxColor Colors[PIXELS_DISPLAY_NUMBER_OF_ROWS][PIXELS_DISPLAY_NUMBER_OF_COLUMNS];
        byte Pin{0};
        byte Brightness{255};

        DECLARE_EVENT_TABLE()

        // functions
        Pixels(wxWindow*, const wxString&);
        ~Pixels();
        void OnClose(wxCloseEvent&);
        void OnClear(wxCommandEvent&);
        void OnAbout(wxCommandEvent&);
        //void OnCreateInput(wxCommandEvent&);
        void OnSend(wxCommandEvent&);
        void OnQuit(wxCommandEvent&);
        void setPixels(wxColour);
        wxBoxSizer* createSizerAll(wxWindow*);
        wxBoxSizer* createSizerCharacters(wxWindow*);
        wxBoxSizer* createSizerCharacter(wxWindow*, int Row);
        wxBoxSizer* createSizerButton(wxWindow*);
        wxBoxSizer* createSizerControl(wxWindow*);
};

#endif // PIXELS_H



