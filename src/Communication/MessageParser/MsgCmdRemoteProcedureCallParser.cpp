// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What each remote procedure does, and the answer to one.
 */

#include "MsgCmdRemoteProcedureCallParser.h"

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "DisplayManager.h"
#include "ErrorMessage.h"
#include "Illuminance.h"
#include "Overlays.h"
#include "Persistence.h"
#include "Power.h"
#include "System.h"

void MsgCmdRemoteProcedureCallParser::SendAnswer() const {
  // Keep the whole response on one line, like every other command answer;
  // the command parser adds the terminating println() afterwards.
  Serial.print(F("RpcId="));
  Serial.print(ToUnderlying(rpc_id_));
  Serial.print(' ');
  // Last field before that terminating println(); no trailing separator space.
  // An unknown RPC id reports its own code; otherwise the execution result.
  if (answer_error_ != ErrorMessage::ErrorType::kNoError) {
    error_.Send(answer_error_, false);
  } else {
    error_.Send(return_value_, false);
  }
}

void MsgCmdRemoteProcedureCallParser::Process() {
  switch (rpc_id_) {
    case RpcIdType::kIlluminanceCalibrationMaxValue:
      return_value_ = Illuminance::GetInstance().StartCalibrationMaxValue();
      break;
    case RpcIdType::kIlluminanceCalibrationMinValue:
      return_value_ = Illuminance::GetInstance().StartCalibrationMinValue();
      break;
    case RpcIdType::kDisplayEnable:
      Display::GetInstance().Enable();
      break;
    case RpcIdType::kDisplayDisable:
      Display::GetInstance().Disable();
      break;
    case RpcIdType::kDisplayBrightnessIncrement:
      Display::GetInstance().IncrementBrightness();
      break;
    case RpcIdType::kDisplayBrightnessDecrement:
      Display::GetInstance().DecrementBrightness();
      break;
    case RpcIdType::kDisplayBrightnessAutomaticOn:
      Display::GetInstance().EnableBrightnessAutomatic();
      break;
    case RpcIdType::kDisplayBrightnessGammaCorrectionOff:
      Display::GetInstance().DisableBrightnessGammaCorrection();
      break;
    case RpcIdType::kDisplayBrightnessGammaCorrectionOn:
      Display::GetInstance().EnableBrightnessGammaCorrection();
      break;
    case RpcIdType::kDisplayBrightnessAutomaticOff:
      Display::GetInstance().DisableBrightnessAutomatic();
      break;
    case RpcIdType::kDisplayColorRedIncrement:
      Display::GetInstance().IncrementColorRed();
      break;
    case RpcIdType::kDisplayColorGreenIncrement:
      Display::GetInstance().IncrementColorGreen();
      break;
    case RpcIdType::kDisplayColorBlueIncrement:
      Display::GetInstance().IncrementColorBlue();
      break;
    case RpcIdType::kDisplayColorRedDecrement:
      Display::GetInstance().DecrementColorRed();
      break;
    case RpcIdType::kDisplayColorGreenDecrement:
      Display::GetInstance().DecrementColorGreen();
      break;
    case RpcIdType::kDisplayColorBlueDecrement:
      Display::GetInstance().DecrementColorBlue();
      break;
    case RpcIdType::kDisplayShow:
      return_value_ = Display::GetInstance().Show();
      break;
    case RpcIdType::kDisplayClear:
      Display::GetInstance().Clear();
      break;
    case RpcIdType::kDisplayTest:
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
    case RpcIdType::kPowerOn:
      if (!Power::IsSwitchFitted()) {
        answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent;
      } else {
        return_value_ = Power::GetInstance().SwitchSupplyOn();
      }
      break;
    case RpcIdType::kPowerOff:
      if (!Power::IsSwitchFitted()) {
        answer_error_ = ErrorMessage::ErrorType::kPowerSwitchAbsent;
      } else {
        return_value_ = Power::GetInstance().SwitchSupplyOff();
      }
      break;
    /*
     * These seven answer kENotOk where the display is busy with something else,
     * rather than doing it anyway: an overlay holding the display and an overlay
     * already showing are the two states in which "now" cannot be honoured, and a
     * caller that is told so can try again.
     */
    case RpcIdType::kClockRefresh:
      return_value_ = DisplayManager::GetInstance().RefreshClock();
      break;
    case RpcIdType::kAnimationStart:
      return_value_ = DisplayManager::GetInstance().StartAnimation();
      break;
    case RpcIdType::kAnimationAbort:
      return_value_ = DisplayManager::GetInstance().AbortAnimation();
      break;
    case RpcIdType::kOverlayDateShow:
      return_value_ = Overlays::GetInstance().ShowDateNow();
      break;
    case RpcIdType::kOverlayTemperatureShow:
      return_value_ = Overlays::GetInstance().ShowTemperatureNow();
      break;
    case RpcIdType::kOverlayTextShow:
      return_value_ = Overlays::GetInstance().ShowTextNow();
      break;
    case RpcIdType::kOverlayAbort:
      return_value_ = Overlays::GetInstance().Abort();
      break;
    case RpcIdType::kSettingsSave:
      return_value_ = Persistence::GetInstance().Save();
      break;
    case RpcIdType::kSettingsReset:
      return_value_ = Persistence::GetInstance().Reset();
      break;
    /*
     * Asks for a restart rather than carrying one out: the answer to this command
     * is sent after Process() returns, and a controller that restarted in here
     * would take that answer with it.
     */
    case RpcIdType::kSystemRestart:
      return_value_ = System::GetInstance().Restart();
      break;
    case RpcIdType::kTimeResynchronise:
      return_value_ = System::GetInstance().ResynchroniseTime();
      break;
    case RpcIdType::kNetworkReconnect:
      return_value_ = System::GetInstance().ReconnectNetwork();
      break;
    /*
     * These seven cannot be refused, which is why none of them takes a return
     * value: every one of them is a setting moved to its neighbour, and a setting
     * has no state it can be in that makes the next one unreachable. What they
     * ask of the display is left to the display - the clock's wording redraws through
     * DisplayManager's latch on the next task, and the animation the mode provides runs at
     * the next word change, the same as if a phone had sent the value.
     */
    case RpcIdType::kAnimationNext:
      Animations::GetInstance().NextAnimation();
      break;
    case RpcIdType::kAnimationPrevious:
      Animations::GetInstance().PreviousAnimation();
      break;
    case RpcIdType::kClockModeNext:
      Clock::GetInstance().NextMode();
      break;
    case RpcIdType::kDisplayToggle:
      Display::GetInstance().Toggle();
      break;
    case RpcIdType::kDisplayBrightnessAutomaticToggle:
      Display::GetInstance().ToggleBrightnessAutomatic();
      break;
    case RpcIdType::kDisplayBrightnessGammaCorrectionToggle:
      Display::GetInstance().ToggleBrightnessGammaCorrection();
      break;
    case RpcIdType::kDisplayColorReset:
      Display::GetInstance().ResetColor();
      break;
    default:
      // Unknown or missing RPC id (including RpcIdType::kNone): there is
      // nothing to execute, so report a specific error instead of Error=0.
      answer_error_ = ErrorMessage::ErrorType::kRpcIdUnknown;
      break;
  }
}
