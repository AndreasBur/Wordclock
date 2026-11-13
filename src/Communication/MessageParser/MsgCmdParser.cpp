// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How a command is told apart and handed to its parser.
 */

#include "MsgCmdParser.h"

#include "MsgCmdAnimationParser.h"
#include "MsgCmdClockModeParser.h"
#include "MsgCmdColorCycleParser.h"
#include "MsgCmdConsoleParser.h"
#include "MsgCmdDateParser.h"
#include "MsgCmdDisplayBrightnessParser.h"
#include "MsgCmdDisplayColorParser.h"
#include "MsgCmdDisplayPixelParser.h"
#include "MsgCmdNetworkParser.h"
#include "MsgCmdNightSwitchParser.h"
#include "MsgCmdRemoteProcedureCallParser.h"
#include "MsgCmdStatusParser.h"
#include "MsgCmdTimeParser.h"
#include "Overlays.h"
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
#include "MsgCmdOverlayDateParser.h"
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
#include "MsgCmdOverlayTemperatureParser.h"
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
#include "MsgCmdOverlayTextParser.h"
#endif

namespace {
/*
 * Every command parser shares the same lifecycle: construct from the parameter
 * string, then parse -> Process -> SendAnswer. This helper runs that sequence so
 * the dispatch below stays a flat table instead of a copy-pasted if/else chain.
 */
template <typename ParserType>
void DispatchCommand(char const* parameter) {
  ParserType parser(parameter);
  parser.Parse();
  parser.Process();
  parser.SendAnswer();
}
}  // namespace

void MsgCmdParser::Parse() {
  CommandType command = GetCommand();
  char const* parameter = GetParameter();

  if (command != CommandType::kNone) SendAnswer(command);

  switch (command) {
    case CommandType::kRemoteProcedureCall:
      DispatchCommand<MsgCmdRemoteProcedureCallParser>(parameter);
      break;
    case CommandType::kDisplayColor:
      DispatchCommand<MsgCmdDisplayColorParser>(parameter);
      break;
    case CommandType::kDisplayBrightness:
      DispatchCommand<MsgCmdDisplayBrightnessParser>(parameter);
      break;
    case CommandType::kDisplayPixel:
      DispatchCommand<MsgCmdDisplayPixelParser>(parameter);
      break;
    case CommandType::kAnimation:
      DispatchCommand<MsgCmdAnimationParser>(parameter);
      break;
    case CommandType::kClockMode:
      DispatchCommand<MsgCmdClockModeParser>(parameter);
      break;
    case CommandType::kTime:
      DispatchCommand<MsgCmdTimeParser>(parameter);
      break;
    case CommandType::kDate:
      DispatchCommand<MsgCmdDateParser>(parameter);
      break;
    case CommandType::kStatus:
      DispatchCommand<MsgCmdStatusParser>(parameter);
      break;
    case CommandType::kNetwork:
      DispatchCommand<MsgCmdNetworkParser>(parameter);
      break;
    case CommandType::kNightSwitch:
      DispatchCommand<MsgCmdNightSwitchParser>(parameter);
      break;
    case CommandType::kColorCycle:
      DispatchCommand<MsgCmdColorCycleParser>(parameter);
      break;
    case CommandType::kConsole:
      DispatchCommand<MsgCmdConsoleParser>(parameter);
      break;
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    case CommandType::kOverlayDate:
      DispatchCommand<MsgCmdOverlayDateParser>(parameter);
      break;
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    case CommandType::kOverlayTemperature:
      DispatchCommand<MsgCmdOverlayTemperatureParser>(parameter);
      break;
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    case CommandType::kOverlayText:
      DispatchCommand<MsgCmdOverlayTextParser>(parameter);
      break;
#endif
    default:
      error_.Send(ErrorMessage::ErrorType::kWrongCommand, false);
      break;
  }

  Serial.println();
}
