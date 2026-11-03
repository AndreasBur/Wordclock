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
#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "Overlays.h"
#include "Persistence.h"
#include "Power.h"
#include "System.h"

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
        kRpcIdNone,
        kRpcIdIlluminanceCalibrationMaxValue,
        kRpcIdIlluminanceCalibrationMinValue,
        kRpcIdDisplayEnable,
        kRpcIdDisplayDisable,
        kRpcIdDisplayShow,
        kRpcIdDisplayClear,
        kRpcIdDisplayTest,
        kRpcIdDisplayColorRedIncrement,
        kRpcIdDisplayColorGreenIncrement,
        kRpcIdDisplayColorBlueIncrement,
        kRpcIdDisplayColorRedDecrement,
        kRpcIdDisplayColorGreenDecrement,
        kRpcIdDisplayColorBlueDecrement,
        kRpcIdDisplayBrightnessIncrement,
        kRpcIdDisplayBrightnessDecrement,
        kRpcIdDisplayBrightnessAutomaticOn,
        kRpcIdDisplayBrightnessAutomaticOff,
        kRpcIdDisplayBrightnessGammaCorrectionOn,
        kRpcIdDisplayBrightnessGammaCorrectionOff,
        kRpcIdPowerOn,
        kRpcIdPowerOff,
        /* Appended rather than sorted into the display group above: the ids are what a
           front end, this repository's own documentation and anybody's notes have written
           down, and inserting one would silently change what every id behind it does. */
        kRpcIdClockRefresh,
        kRpcIdAnimationStart,
        kRpcIdAnimationAbort,
        kRpcIdOverlayDateShow,
        kRpcIdOverlayTemperatureShow,
        kRpcIdOverlayTextShow,
        kRpcIdOverlayAbort,
        kRpcIdSettingsSave,
        kRpcIdSettingsReset,
        kRpcIdSystemRestart,
        kRpcIdTimeResynchronise,
        kRpcIdNetworkReconnect,
        /* For a control that cannot name a value, only ask for one further - see the
           increment and decrement pairs above, which are the same eight for the colour
           and the brightness. */
        kRpcIdAnimationNext,
        kRpcIdAnimationPrevious,
        kRpcIdClockModeNext,
        kRpcIdDisplayToggle,
        kRpcIdDisplayBrightnessAutomaticToggle,
        kRpcIdDisplayBrightnessGammaCorrectionToggle,
        kRpcIdDisplayColorReset,
        /* Only so that the names in the message catalog can be counted against the
           procedures; never sent, and anything from here on lands in the default case. */
        kRpcIdNumberOfProcedures
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    RpcIdType rpc_id_{kRpcIdNone};
    StdReturnType return_value_{kEOk};
    ErrorMessage::ErrorType answer_error_{ErrorMessage::kErrorNoError};
    static constexpr char kRemoteProcedureShortName{'P'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kRemoteProcedureShortName, MsgParameter::kArgumentTypeUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kRemoteProcedureShortName) { rpc_id_ = static_cast<RpcIdType>(argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdRemoteProcedureCallParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdRemoteProcedureCallParser() { }

    // get methods

    // set methods

    // methods
    void SendAnswer() const {
        // Keep the whole response on one line, like every other command answer;
        // the command parser adds the terminating println() afterwards.
        Serial.print(F("RpcId="));
        Serial.print(rpc_id_);
        Serial.print(' ');
        // Last field before that terminating println(); no trailing separator space.
        // An unknown RPC id reports its own code; otherwise the execution result.
        if(answer_error_ != ErrorMessage::kErrorNoError) { error_.Send(answer_error_, false); }
        else { error_.Send(return_value_, false); }
    }

    void Process()
    {
        switch(rpc_id_) {
            case kRpcIdIlluminanceCalibrationMaxValue :
                return_value_ = Illuminance::GetInstance().StartCalibrationMaxValue();
                break;
            case kRpcIdIlluminanceCalibrationMinValue :
                return_value_ = Illuminance::GetInstance().StartCalibrationMinValue();
                break;
            case kRpcIdDisplayEnable :
                Display::GetInstance().Enable();
                break;
            case kRpcIdDisplayDisable :
                Display::GetInstance().Disable();
                break;
            case kRpcIdDisplayBrightnessIncrement :
                Display::GetInstance().IncrementBrightness();
                break;
            case kRpcIdDisplayBrightnessDecrement :
                Display::GetInstance().DecrementBrightness();
                break;
            case kRpcIdDisplayBrightnessAutomaticOn :
                Display::GetInstance().EnableBrightnessAutomatic();
                break;
            case kRpcIdDisplayBrightnessGammaCorrectionOff :
                Display::GetInstance().DisableBrightnessGammaCorrection();
                break;
            case kRpcIdDisplayBrightnessGammaCorrectionOn :
                Display::GetInstance().EnableBrightnessGammaCorrection();
                break;
            case kRpcIdDisplayBrightnessAutomaticOff :
                Display::GetInstance().DisableBrightnessAutomatic();
                break;
            case kRpcIdDisplayColorRedIncrement :
                Display::GetInstance().IncrementColorRed();
                break;
            case kRpcIdDisplayColorGreenIncrement :
                Display::GetInstance().IncrementColorGreen();
                break;
            case kRpcIdDisplayColorBlueIncrement :
                Display::GetInstance().IncrementColorBlue();
                break;
            case kRpcIdDisplayColorRedDecrement :
                Display::GetInstance().DecrementColorRed();
                break;
            case kRpcIdDisplayColorGreenDecrement :
                Display::GetInstance().DecrementColorGreen();
                break;
            case kRpcIdDisplayColorBlueDecrement :
                Display::GetInstance().DecrementColorBlue();
                break;
            case kRpcIdDisplayShow :
                return_value_ = Display::GetInstance().Show();
                break;
            case kRpcIdDisplayClear :
                Display::GetInstance().Clear();
                break;
            case kRpcIdDisplayTest :
                Display::GetInstance().Test();
                break;
            /* The switch in the strip's 5 V, not the display's enable and disable on ids 3
               and 4 - those set the brightness to zero and leave a powered strip drawing its
               quiescent current. Both pairs exist and neither stands in for the other.

               Only asked for here. Cutting the supply means blanking the strip over the data
               line and waiting for that frame to be gone, and the frame is not gone when this
               returns: nothing is transmitted until the application's tick renders, and the
               transmission outlives that call as well. Power carries the order out over the
               ticks that follow, the way SYSTEM_RESTART leaves the restart to one.

               A board built without the switch has the high side MOSFET bridged and nothing
               to act on, which is its own answer rather than kENotOk: the caller asked for
               something this clock does not have, and "unknown" would not say so. The circuit
               is written down in platform/esp32/README.md, under "Switching the strip's
               supply". */
            case kRpcIdPowerOn :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::kErrorPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOn(); }
                break;
            case kRpcIdPowerOff :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::kErrorPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOff(); }
                break;
            /* These seven answer E_NOT_OK where the display is busy with something else,
               rather than doing it anyway: an overlay holding the display and an overlay
               already showing are the two states in which "now" cannot be honoured, and a
               caller that is told so can try again. */
            case kRpcIdClockRefresh :
                return_value_ = DisplayManager::GetInstance().RefreshClock();
                break;
            case kRpcIdAnimationStart :
                return_value_ = DisplayManager::GetInstance().StartAnimation();
                break;
            case kRpcIdAnimationAbort :
                return_value_ = DisplayManager::GetInstance().AbortAnimation();
                break;
            case kRpcIdOverlayDateShow :
                return_value_ = Overlays::GetInstance().ShowDateNow();
                break;
            case kRpcIdOverlayTemperatureShow :
                return_value_ = Overlays::GetInstance().ShowTemperatureNow();
                break;
            case kRpcIdOverlayTextShow :
                return_value_ = Overlays::GetInstance().ShowTextNow();
                break;
            case kRpcIdOverlayAbort :
                return_value_ = Overlays::GetInstance().Abort();
                break;
            case kRpcIdSettingsSave :
                return_value_ = Persistence::GetInstance().Save();
                break;
            case kRpcIdSettingsReset :
                return_value_ = Persistence::GetInstance().Reset();
                break;
            /* Asks for a restart rather than carrying one out: the answer to this command
               is sent after Process() returns, and a controller that restarted in here
               would take that answer with it. */
            case kRpcIdSystemRestart :
                return_value_ = System::GetInstance().Restart();
                break;
            case kRpcIdTimeResynchronise :
                return_value_ = System::GetInstance().ResynchroniseTime();
                break;
            case kRpcIdNetworkReconnect :
                return_value_ = System::GetInstance().ReconnectNetwork();
                break;
            /* These seven cannot be refused, which is why none of them takes a return
               value: every one of them is a setting moved to its neighbour, and a setting
               has no state it can be in that makes the next one unreachable. What they
               ask of the display is left to the display - the clock's wording redraws
               through DisplayManager's latch on the next task, and the selected animation
               runs at the next word change, the same as if a phone had sent the value. */
            case kRpcIdAnimationNext :
                Animations::GetInstance().NextAnimation();
                break;
            case kRpcIdAnimationPrevious :
                Animations::GetInstance().PreviousAnimation();
                break;
            case kRpcIdClockModeNext :
                Clock::GetInstance().NextMode();
                break;
            case kRpcIdDisplayToggle :
                Display::GetInstance().Toggle();
                break;
            case kRpcIdDisplayBrightnessAutomaticToggle :
                Display::GetInstance().ToggleBrightnessAutomatic();
                break;
            case kRpcIdDisplayBrightnessGammaCorrectionToggle :
                Display::GetInstance().ToggleBrightnessGammaCorrection();
                break;
            case kRpcIdDisplayColorReset :
                Display::GetInstance().ResetColor();
                break;
            default:
                // Unknown or missing RPC id (including RPC_ID_NONE): there is
                // nothing to execute, so report a specific error instead of Error=0.
                answer_error_ = ErrorMessage::kErrorRpcIdUnknown;
                break;
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
