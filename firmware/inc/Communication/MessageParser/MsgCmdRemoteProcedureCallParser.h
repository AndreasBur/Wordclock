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
    enum class RpcIdType
    {
        kNone,
        kIlluminanceCalibrationMaxValue,
        kIlluminanceCalibrationMinValue,
        kDisplayEnable,
        kDisplayDisable,
        kDisplayShow,
        kDisplayClear,
        kDisplayTest,
        kDisplayColorRedIncrement,
        kDisplayColorGreenIncrement,
        kDisplayColorBlueIncrement,
        kDisplayColorRedDecrement,
        kDisplayColorGreenDecrement,
        kDisplayColorBlueDecrement,
        kDisplayBrightnessIncrement,
        kDisplayBrightnessDecrement,
        kDisplayBrightnessAutomaticOn,
        kDisplayBrightnessAutomaticOff,
        kDisplayBrightnessGammaCorrectionOn,
        kDisplayBrightnessGammaCorrectionOff,
        kPowerOn,
        kPowerOff,
        /* Appended rather than sorted into the display group above: the ids are what a
           front end, this repository's own documentation and anybody's notes have written
           down, and inserting one would silently change what every id behind it does. */
        kClockRefresh,
        kAnimationStart,
        kAnimationAbort,
        kOverlayDateShow,
        kOverlayTemperatureShow,
        kOverlayTextShow,
        kOverlayAbort,
        kSettingsSave,
        kSettingsReset,
        kSystemRestart,
        kTimeResynchronise,
        kNetworkReconnect,
        /* For a control that cannot name a value, only ask for one further - see the
           increment and decrement pairs above, which are the same eight for the colour
           and the brightness. */
        kAnimationNext,
        kAnimationPrevious,
        kClockModeNext,
        kDisplayToggle,
        kDisplayBrightnessAutomaticToggle,
        kDisplayBrightnessGammaCorrectionToggle,
        kDisplayColorReset,
        /* Only so that the names in the message catalog can be counted against the
           procedures; never sent, and anything from here on lands in the default case. */
        kNumberOfProcedures
    };
    static constexpr byte kNumberOfProcedures{static_cast<byte>(RpcIdType::kNumberOfProcedures)};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    RpcIdType rpc_id_{RpcIdType::kNone};
    StdReturnType return_value_{kEOk};
    ErrorMessage::ErrorType answer_error_{ErrorMessage::ErrorType::kNoError};
    static constexpr char kRemoteProcedureShortName{'P'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kRemoteProcedureShortName, MsgParameter::ArgumentTypeType::kUint8)
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
        Serial.print(ToUnderlying(rpc_id_));
        Serial.print(' ');
        // Last field before that terminating println(); no trailing separator space.
        // An unknown RPC id reports its own code; otherwise the execution result.
        if(answer_error_ != ErrorMessage::ErrorType::kNoError) { error_.Send(answer_error_, false); }
        else { error_.Send(return_value_, false); }
    }

    void Process()
    {
        switch(rpc_id_) {
            case RpcIdType::kIlluminanceCalibrationMaxValue :
                return_value_ = Illuminance::GetInstance().StartCalibrationMaxValue();
                break;
            case RpcIdType::kIlluminanceCalibrationMinValue :
                return_value_ = Illuminance::GetInstance().StartCalibrationMinValue();
                break;
            case RpcIdType::kDisplayEnable :
                Display::GetInstance().Enable();
                break;
            case RpcIdType::kDisplayDisable :
                Display::GetInstance().Disable();
                break;
            case RpcIdType::kDisplayBrightnessIncrement :
                Display::GetInstance().IncrementBrightness();
                break;
            case RpcIdType::kDisplayBrightnessDecrement :
                Display::GetInstance().DecrementBrightness();
                break;
            case RpcIdType::kDisplayBrightnessAutomaticOn :
                Display::GetInstance().EnableBrightnessAutomatic();
                break;
            case RpcIdType::kDisplayBrightnessGammaCorrectionOff :
                Display::GetInstance().DisableBrightnessGammaCorrection();
                break;
            case RpcIdType::kDisplayBrightnessGammaCorrectionOn :
                Display::GetInstance().EnableBrightnessGammaCorrection();
                break;
            case RpcIdType::kDisplayBrightnessAutomaticOff :
                Display::GetInstance().DisableBrightnessAutomatic();
                break;
            case RpcIdType::kDisplayColorRedIncrement :
                Display::GetInstance().IncrementColorRed();
                break;
            case RpcIdType::kDisplayColorGreenIncrement :
                Display::GetInstance().IncrementColorGreen();
                break;
            case RpcIdType::kDisplayColorBlueIncrement :
                Display::GetInstance().IncrementColorBlue();
                break;
            case RpcIdType::kDisplayColorRedDecrement :
                Display::GetInstance().DecrementColorRed();
                break;
            case RpcIdType::kDisplayColorGreenDecrement :
                Display::GetInstance().DecrementColorGreen();
                break;
            case RpcIdType::kDisplayColorBlueDecrement :
                Display::GetInstance().DecrementColorBlue();
                break;
            case RpcIdType::kDisplayShow :
                return_value_ = Display::GetInstance().Show();
                break;
            case RpcIdType::kDisplayClear :
                Display::GetInstance().Clear();
                break;
            case RpcIdType::kDisplayTest :
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
            case RpcIdType::kPowerOn :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOn(); }
                break;
            case RpcIdType::kPowerOff :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOff(); }
                break;
            /* These seven answer kENotOk where the display is busy with something else,
               rather than doing it anyway: an overlay holding the display and an overlay
               already showing are the two states in which "now" cannot be honoured, and a
               caller that is told so can try again. */
            case RpcIdType::kClockRefresh :
                return_value_ = DisplayManager::GetInstance().RefreshClock();
                break;
            case RpcIdType::kAnimationStart :
                return_value_ = DisplayManager::GetInstance().StartAnimation();
                break;
            case RpcIdType::kAnimationAbort :
                return_value_ = DisplayManager::GetInstance().AbortAnimation();
                break;
            case RpcIdType::kOverlayDateShow :
                return_value_ = Overlays::GetInstance().ShowDateNow();
                break;
            case RpcIdType::kOverlayTemperatureShow :
                return_value_ = Overlays::GetInstance().ShowTemperatureNow();
                break;
            case RpcIdType::kOverlayTextShow :
                return_value_ = Overlays::GetInstance().ShowTextNow();
                break;
            case RpcIdType::kOverlayAbort :
                return_value_ = Overlays::GetInstance().Abort();
                break;
            case RpcIdType::kSettingsSave :
                return_value_ = Persistence::GetInstance().Save();
                break;
            case RpcIdType::kSettingsReset :
                return_value_ = Persistence::GetInstance().Reset();
                break;
            /* Asks for a restart rather than carrying one out: the answer to this command
               is sent after Process() returns, and a controller that restarted in here
               would take that answer with it. */
            case RpcIdType::kSystemRestart :
                return_value_ = System::GetInstance().Restart();
                break;
            case RpcIdType::kTimeResynchronise :
                return_value_ = System::GetInstance().ResynchroniseTime();
                break;
            case RpcIdType::kNetworkReconnect :
                return_value_ = System::GetInstance().ReconnectNetwork();
                break;
            /* These seven cannot be refused, which is why none of them takes a return
               value: every one of them is a setting moved to its neighbour, and a setting
               has no state it can be in that makes the next one unreachable. What they
               ask of the display is left to the display - the clock's wording redraws
               through DisplayManager's latch on the next task, and the selected animation
               runs at the next word change, the same as if a phone had sent the value. */
            case RpcIdType::kAnimationNext :
                Animations::GetInstance().NextAnimation();
                break;
            case RpcIdType::kAnimationPrevious :
                Animations::GetInstance().PreviousAnimation();
                break;
            case RpcIdType::kClockModeNext :
                Clock::GetInstance().NextMode();
                break;
            case RpcIdType::kDisplayToggle :
                Display::GetInstance().Toggle();
                break;
            case RpcIdType::kDisplayBrightnessAutomaticToggle :
                Display::GetInstance().ToggleBrightnessAutomatic();
                break;
            case RpcIdType::kDisplayBrightnessGammaCorrectionToggle :
                Display::GetInstance().ToggleBrightnessGammaCorrection();
                break;
            case RpcIdType::kDisplayColorReset :
                Display::GetInstance().ResetColor();
                break;
            default:
                // Unknown or missing RPC id (including RpcIdType::kNone): there is
                // nothing to execute, so report a specific error instead of Error=0.
                answer_error_ = ErrorMessage::ErrorType::kRpcIdUnknown;
                break;
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
