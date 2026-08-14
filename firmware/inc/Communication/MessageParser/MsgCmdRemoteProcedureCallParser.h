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
/**     \file       MsgCmdRemoteProcedureCallParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_
#define _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Illuminance.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Overlays.h"
#include "Persistence.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdRemoteProcedureCallParser configuration parameter */


/* MsgCmdRemoteProcedureCallParser parameter */
#define MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE           1u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdRemoteProcedureCallParser : public MsgParameterParser<MsgCmdRemoteProcedureCallParser, MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum RpcIdType
    {
        RPC_ID_NONE,
        RPC_ID_ILLUMINANCE_CALIBRATION_MAX_VALUE,
        RPC_ID_ILLUMINANCE_CALIBRATION_MIN_VALUE,
        RPC_ID_DISPLAY_ENABLE,
        RPC_ID_DISPLAY_DISABLE,
        RPC_ID_DISPLAY_SHOW,
        RPC_ID_DISPLAY_CLEAR,
        RPC_ID_DISPLAY_TEST,
        RPC_ID_DISPLAY_COLOR_RED_INCREMENT,
        RPC_ID_DISPLAY_COLOR_GREEN_INCREMENT,
        RPC_ID_DISPLAY_COLOR_BLUE_INCREMENT,
        RPC_ID_DISPLAY_COLOR_RED_DECREMENT,
        RPC_ID_DISPLAY_COLOR_GREEN_DECREMENT,
        RPC_ID_DISPLAY_COLOR_BLUE_DECREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_INCREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_DECREMENT,
        RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_ON,
        RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_OFF,
        RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_ON,
        RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_OFF,
        RPC_ID_POWER_ON,
        RPC_ID_POWER_OFF,
        /* Appended rather than sorted into the display group above: the ids are what a
           front end, this repository's own documentation and anybody's notes have written
           down, and inserting one would silently change what every id behind it does. */
        RPC_ID_CLOCK_REFRESH,
        RPC_ID_ANIMATION_START,
        RPC_ID_ANIMATION_ABORT,
        RPC_ID_OVERLAY_DATE_SHOW,
        RPC_ID_OVERLAY_TEMPERATURE_SHOW,
        RPC_ID_OVERLAY_TEXT_SHOW,
        RPC_ID_OVERLAY_ABORT,
        RPC_ID_SETTINGS_SAVE,
        RPC_ID_SETTINGS_RESET,
        /* Only so that the names in the message catalog can be counted against the
           procedures; never sent, and anything from here on lands in the default case. */
        RPC_ID_NUMBER_OF_PROCEDURES
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    RpcIdType RpcId{RPC_ID_NONE};
    StdReturnType ReturnValue{E_OK};
    ErrorMessage::ErrorType AnswerError{ErrorMessage::ERROR_NO_ERROR};
    static constexpr char RemoteProcedureShortName{'P'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(RemoteProcedureShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length); }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == RemoteProcedureShortName) { RpcId = static_cast<RpcIdType>(Argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdRemoteProcedureCallParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdRemoteProcedureCallParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const {
        // Keep the whole response on one line, like every other command answer;
        // the command parser adds the terminating println() afterwards.
        Serial.print(F("RpcId="));
        Serial.print(RpcId);
        Serial.print(' ');
        // Last field before that terminating println(); no trailing separator space.
        // An unknown RPC id reports its own code; otherwise the execution result.
        if(AnswerError != ErrorMessage::ERROR_NO_ERROR) { Error.send(AnswerError, false); }
        else { Error.send(ReturnValue, false); }
    }

    void process()
    {
        switch(RpcId) {
            case RPC_ID_ILLUMINANCE_CALIBRATION_MAX_VALUE :
                Illuminance::getInstance().startCalibrationMaxValue();
                break;
            case RPC_ID_ILLUMINANCE_CALIBRATION_MIN_VALUE :
                Illuminance::getInstance().startCalibrationMinValue();
                break;
            case RPC_ID_DISPLAY_ENABLE :
                Display::getInstance().enable();
                break;
            case RPC_ID_DISPLAY_DISABLE :
                Display::getInstance().disable();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_INCREMENT :
                Display::getInstance().incrementBrightness();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_DECREMENT :
                Display::getInstance().decrementBrightness();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_ON :
                Display::getInstance().enableBrightnessAutomatic();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_OFF :
                Display::getInstance().disableBrightnessGammaCorrection();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_GAMMA_CORRECTION_ON :
                Display::getInstance().enableBrightnessGammaCorrection();
                break;
            case RPC_ID_DISPLAY_BRIGHTNESS_AUTOMATIC_OFF :
                Display::getInstance().disableBrightnessAutomatic();
                break;
            case RPC_ID_DISPLAY_COLOR_RED_INCREMENT :
                Display::getInstance().incrementColorRed();
                break;
            case RPC_ID_DISPLAY_COLOR_GREEN_INCREMENT :
                Display::getInstance().incrementColorGreen();
                break;
            case RPC_ID_DISPLAY_COLOR_BLUE_INCREMENT :
                Display::getInstance().incrementColorBlue();
                break;
            case RPC_ID_DISPLAY_COLOR_RED_DECREMENT :
                Display::getInstance().decrementColorRed();
                break;
            case RPC_ID_DISPLAY_COLOR_GREEN_DECREMENT :
                Display::getInstance().decrementColorGreen();
                break;
            case RPC_ID_DISPLAY_COLOR_BLUE_DECREMENT :
                Display::getInstance().decrementColorBlue();
                break;
            case RPC_ID_DISPLAY_SHOW :
                ReturnValue = Display::getInstance().show();
                break;
            case RPC_ID_DISPLAY_CLEAR :
                Display::getInstance().clear();
                break;
            case RPC_ID_DISPLAY_TEST :
                Display::getInstance().test();
                break;
            // POWER_ON and POWER_OFF are reserved for the planned hardware switch
            // that cuts the 5 V supply of the LED stripes via a microcontroller
            // port (high side P-MOSFET, driven through an N-MOSFET, so the port is
            // active high and the pull down leaves the supply off after reset).
            // These ids are not display enable/disable, which have their own ids.
            //
            // The order matters once the port exists: data pulses must not reach
            // DIN while the supply is off. So POWER_OFF has to blank the stripes
            // over the data line first, wait for that transfer to complete, and
            // only then switch the supply off; POWER_ON switches the supply on and
            // resumes data output afterwards. Note that disable() already starts a
            // DMA transfer of its own (see WS2812::disablePixels), so it must not
            // be followed by a second show().
            //
            // Until the port exists the ids stay intentionally unimplemented.
            case RPC_ID_POWER_ON :
                break;
            case RPC_ID_POWER_OFF :
                break;
            /* These seven answer E_NOT_OK where the display is busy with something else,
               rather than doing it anyway: an overlay holding the display and an overlay
               already showing are the two states in which "now" cannot be honoured, and a
               caller that is told so can try again. */
            case RPC_ID_CLOCK_REFRESH :
                ReturnValue = DisplayManager::getInstance().refreshClock();
                break;
            case RPC_ID_ANIMATION_START :
                ReturnValue = DisplayManager::getInstance().startAnimation();
                break;
            case RPC_ID_ANIMATION_ABORT :
                ReturnValue = DisplayManager::getInstance().abortAnimation();
                break;
            case RPC_ID_OVERLAY_DATE_SHOW :
                ReturnValue = Overlays::getInstance().showDateNow();
                break;
            case RPC_ID_OVERLAY_TEMPERATURE_SHOW :
                ReturnValue = Overlays::getInstance().showTemperatureNow();
                break;
            case RPC_ID_OVERLAY_TEXT_SHOW :
                ReturnValue = Overlays::getInstance().showTextNow();
                break;
            case RPC_ID_OVERLAY_ABORT :
                ReturnValue = Overlays::getInstance().abort();
                break;
            case RPC_ID_SETTINGS_SAVE :
                ReturnValue = Persistence::getInstance().save();
                break;
            case RPC_ID_SETTINGS_RESET :
                ReturnValue = Persistence::getInstance().reset();
                break;
            default:
                // Unknown or missing RPC id (including RPC_ID_NONE): there is
                // nothing to execute, so report a specific error instead of Error=0.
                AnswerError = ErrorMessage::ERROR_RPC_ID_UNKNOWN;
                break;
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
