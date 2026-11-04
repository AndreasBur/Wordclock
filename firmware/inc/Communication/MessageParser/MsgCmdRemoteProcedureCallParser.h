// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the remote procedure call command, with the ids of the procedures.
 */

#ifndef _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_
#define _MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_H_

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

/*! \brief The number of options in the parameter table of the remote procedure call command. */
#define MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE           1u

/*!
 * \brief The parser of command 1, the remote procedure call command: runs one procedure of the
 *        clock by its id.
 *
 * Its option:
 * - `-P<id>`, uint8: the procedure, RpcIdType.
 *
 * The answer is `RpcId=<id> Error=<error>`, and carries no value:
 * - `Error=0`, ErrorMessage::ErrorType::kNoError: the procedure was carried out.
 * - `Error=8`, ErrorMessage::ErrorType::kUnknown: the procedure was refused. Only the procedures
 *   1, 2 and 22 to 33 are ever refused; each id says when.
 * - `Error=7`, ErrorMessage::ErrorType::kRpcIdUnknown: the id is not a procedure, as 0 is, and
 *   nothing was run. A command without `-P` has the id 0.
 * - `Error=9`, ErrorMessage::ErrorType::kPowerSwitchAbsent: the procedures 20 and 21 on a clock
 *   whose supply switch is not fitted, which is every backend but the simulator.
 *
 * An id above 255 is refused by MsgParameterParser with `Error=4:P`, and the id stays 0: `1 -P300`
 * answers `1 Error=4:P RpcId=0 Error=7`.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 */
class MsgCmdRemoteProcedureCallParser : public MsgParameterParser<MsgCmdRemoteProcedureCallParser, MSG_CMD_REMOTE_PROCEDURE_CALL_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    /*!
     * \brief The procedures, by the id `-P` takes.
     *
     * A procedure that can be refused answers `Error=8` when it is; the others always answer
     * `Error=0`.
     */
    enum class RpcIdType
    {
        kNone, /*!< No procedure: answered with `Error=7`. */
        /*!
         * \brief Takes the illuminance the light sensor reads now as the bright bound of the
         *        automatic brightness; refused while the sensor is not ready.
         */
        kIlluminanceCalibrationMaxValue,
        /*!
         * \brief Takes the illuminance the light sensor reads now as the dark bound of the
         *        automatic brightness; refused while the sensor is not ready.
         */
        kIlluminanceCalibrationMinValue,
        kDisplayEnable, /*!< Switches the display on: its pixels show again. */
        kDisplayDisable, /*!< Switches the display off; its pixels keep their colours. */
        kDisplayShow, /*!< Marks the display's buffer to be sent. */
        kDisplayClear, /*!< Turns every pixel off. */
        kDisplayTest, /*!< Lights every pixel in the colour of the display. */
        kDisplayColorRedIncrement, /*!< Raises the red value of the colour by one, up to 255. */
        kDisplayColorGreenIncrement, /*!< Raises the green value of the colour by one, up to 255. */
        kDisplayColorBlueIncrement, /*!< Raises the blue value of the colour by one, up to 255. */
        kDisplayColorRedDecrement, /*!< Lowers the red value of the colour by one, down to 0. */
        kDisplayColorGreenDecrement, /*!< Lowers the green value of the colour by one, down to 0. */
        kDisplayColorBlueDecrement, /*!< Lowers the blue value of the colour by one, down to 0. */
        kDisplayBrightnessIncrement, /*!< Raises the brightness asked for by one, up to 255. */
        kDisplayBrightnessDecrement, /*!< Lowers the brightness asked for by one, down to 0. */
        kDisplayBrightnessAutomaticOn, /*!< Has the brightness follow the light sensor. */
        kDisplayBrightnessAutomaticOff, /*!< Stops the brightness following the light sensor. */
        kDisplayBrightnessGammaCorrectionOn, /*!< Switches the gamma correction on. */
        kDisplayBrightnessGammaCorrectionOff, /*!< Switches the gamma correction off. */
        /*! \brief Asks for the LED strip's supply to come on; `Error=9` without a supply switch. */
        kPowerOn,
        /*! \brief Asks for the LED strip's supply to go off; `Error=9` without a supply switch. */
        kPowerOff,
        /*
         * Appended rather than sorted into the display group above: the ids are what a
         * front end, this repository's own documentation and anybody's notes have written
         * down, and inserting one would silently change what every id behind it does.
         */
        /*!
         * \brief Draws the current time again, now and without an animation; refused while an
         *        overlay shows.
         */
        kClockRefresh,
        /*!
         * \brief Runs the selected animation on the current time now; refused while an overlay
         *        shows.
         */
        kAnimationStart,
        /*!
         * \brief Ends a running animation and draws the current time again; refused while an
         *        overlay shows.
         */
        kAnimationAbort,
        /*!
         * \brief Shows the date overlay now rather than at its next period; refused while it is
         *        switched off or an overlay shows.
         */
        kOverlayDateShow,
        /*!
         * \brief Shows the temperature overlay now rather than at its next period; refused while it
         *        is switched off, the clock chip has no reading, or an overlay shows.
         */
        kOverlayTemperatureShow,
        /*!
         * \brief Shows the text overlay now rather than at its next period; refused while it is
         *        switched off or an overlay shows.
         */
        kOverlayTextShow,
        kOverlayAbort, /*!< Ends the overlay that shows; refused when none shows. */
        /*!
         * \brief Writes the settings to the store now; refused when the store does not take them.
         */
        kSettingsSave,
        /*!
         * \brief Puts every setting back to how a new clock has it, and empties the store; refused
         *        when the store is not emptied, though the settings are reset all the same.
         */
        kSettingsReset,
        /*!
         * \brief Restarts the controller on a later tick, after the answer; refused in the
         *        simulator.
         */
        kSystemRestart,
        /*!
         * \brief Asks the network for the time again; refused while the clock has not joined a
         *        network, and always on the AVR and in the simulator.
         */
        kTimeResynchronise,
        /*!
         * \brief Joins the network again; refused where the backend cannot, and always on the AVR
         *        and in the simulator.
         */
        kNetworkReconnect,
        /*
         * For a control that cannot name a value, only ask for one further - see the
         * increment and decrement pairs above, which are the same eight for the colour
         * and the brightness.
         */
        /*! \brief Selects the animation after the selected one; after the last comes id 0. */
        kAnimationNext,
        /*! \brief Selects the animation before the selected one; before id 0 comes the last. */
        kAnimationPrevious,
        kClockModeNext, /*!< Switches to the next regional mode, from the last to the first. */
        kDisplayToggle, /*!< Switches the display off if it is on, and on if it is off. */
        /*!
         * \brief Has the brightness follow the light sensor if it does not, and stops it if it
         *        does.
         */
        kDisplayBrightnessAutomaticToggle,
        /*! \brief Switches the gamma correction on if it is off, and off if it is on. */
        kDisplayBrightnessGammaCorrectionToggle,
        kDisplayColorReset, /*!< Sets the colour back to white. */
        /*!
         * \brief The number of procedures; not a procedure.
         *
         * Only so that the names in the message catalog can be counted against the procedures;
         * never sent, and anything from here on lands in the default case of Process(), answered
         * with `Error=7`.
         */
        kNumberOfProcedures
    };

    /*! \brief The number of procedures, RpcIdType::kNumberOfProcedures. */
    static constexpr byte kNumberOfProcedures{static_cast<byte>(RpcIdType::kNumberOfProcedures)};

  private:
    friend class MsgParameterParser;
    RpcIdType rpc_id_{RpcIdType::kNone};
    StdReturnType return_value_{kEOk};
    ErrorMessage::ErrorType answer_error_{ErrorMessage::ErrorType::kNoError};
    static constexpr char kRemoteProcedureShortName{'P'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kRemoteProcedureShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kRemoteProcedureShortName) { rpc_id_ = static_cast<RpcIdType>(argument); }
    }

  public:
    /*!
     * \brief Makes the parser for the options in `parameter`.
     *
     * \param[in] parameter The parameter string, terminated by a null character; the parser keeps
     *                      the pointer and reads it in Parse().
     */
    constexpr MsgCmdRemoteProcedureCallParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }

    /*! \brief Destroys the parser. */
    ~MsgCmdRemoteProcedureCallParser() { }

    /*!
     * \brief Sends the answer: the fields that follow the command number.
     *
     * It sends `RpcId=<id> Error=<error>`: the id of `-P`, and how the procedure went, as the class
     * describes. A space follows each field but the last; MsgCmdParser ends the line.
     */
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

    /*!
     * \brief Completes the command once Parse() has read its options.
     *
     * It runs the procedure of `-P`, and keeps for SendAnswer() whether it was carried out. An id
     * that is not a procedure runs nothing.
     */
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
            /*
             * The switch in the strip's 5 V, not the display's enable and disable on ids 3
             * and 4 - those set the brightness to zero and leave a powered strip drawing its
             * quiescent current. Both pairs exist and neither stands in for the other.
             *
             * Only asked for here. Cutting the supply means blanking the strip over the data
             * line and waiting for that frame to be gone, and the frame is not gone when this
             * returns: nothing is transmitted until the application's tick renders, and the
             * transmission outlives that call as well. Power carries the order out over the
             * ticks that follow, the way SYSTEM_RESTART leaves the restart to one.
             *
             * A board built without the switch has the high side MOSFET bridged and nothing
             * to act on, which is its own answer rather than kENotOk: the caller asked for
             * something this clock does not have, and "unknown" would not say so. The circuit
             * is written down in platform/esp32/README.md, under "Switching the strip's
             * supply".
             */
            case RpcIdType::kPowerOn :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOn(); }
                break;
            case RpcIdType::kPowerOff :
                if(!Power::IsSwitchFitted()) { answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent; }
                else { return_value_ = Power::GetInstance().SwitchSupplyOff(); }
                break;
            /*
             * These seven answer kENotOk where the display is busy with something else,
             * rather than doing it anyway: an overlay holding the display and an overlay
             * already showing are the two states in which "now" cannot be honoured, and a
             * caller that is told so can try again.
             */
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
            /*
             * Asks for a restart rather than carrying one out: the answer to this command
             * is sent after Process() returns, and a controller that restarted in here
             * would take that answer with it.
             */
            case RpcIdType::kSystemRestart :
                return_value_ = System::GetInstance().Restart();
                break;
            case RpcIdType::kTimeResynchronise :
                return_value_ = System::GetInstance().ResynchroniseTime();
                break;
            case RpcIdType::kNetworkReconnect :
                return_value_ = System::GetInstance().ReconnectNetwork();
                break;
            /*
             * These seven cannot be refused, which is why none of them takes a return
             * value: every one of them is a setting moved to its neighbour, and a setting
             * has no state it can be in that makes the next one unreachable. What they
             * ask of the display is left to the display - the clock's wording redraws
             * through DisplayManager's latch on the next task, and the selected animation
             * runs at the next word change, the same as if a phone had sent the value.
             */
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
