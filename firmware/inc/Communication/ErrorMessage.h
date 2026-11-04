// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The error codes and API ids of an error answer, and how it is sent.
 */

#ifndef _ERROR_MESSAGE_H_
#define _ERROR_MESSAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameter.h"

/* ErrorMessage configuration parameter */

/* ErrorMessage parameter */

class ErrorMessage
{
  public:
    enum class ErrorType {
        kNoError,
        kMessageTooLong,
        kWrongCommand,
        kParameterUnknown,
        kValueOutOfBounds,
        kNoValueGiven,
        kDisplayPending,
        kRpcIdUnknown,
        kUnknown,
        /*
         * Appended behind kUnknown rather than sorted in beside its neighbours, for the
         * reason the remote procedure ids are appended too: these numbers are what an answer
         * carries and what this repository's own documentation writes down, and inserting one
         * would silently change what every code behind it means.
         */
        kPowerSwitchAbsent,
    };

    enum class ApiType {
        kNone,
        kDisplayShow,
        kDisplayWritePixel,
        kAnimationShow,
        kClockShow,
        kAnimationSetAnimation,
        kAnimationSetClockTaskCycle,
    };

  private:
    // functions
    static void SendSpace() { Serial.print(' '); }
    static void SendColon() { Serial.print(':'); }
    static void SendEqual() { Serial.print('='); }

    static void Send(ApiType api)
    {
        Serial.print(F("Api"));
        SendEqual();
        Serial.print(ToUnderlying(api));
    }

  public:
    constexpr ErrorMessage() { }
    ~ErrorMessage() { }

    // get methods

    // set methods

    // methods
    static void Send(ErrorType error, bool append_space = true)
    {
        Serial.print(F("Error"));
        SendEqual();
        Serial.print(ToUnderlying(error));
        if(append_space) { SendSpace(); }
    }

    static void Send(char option_short_name, ErrorType error, bool append_space = true)
    {
        Send(error, false);
        SendColon();
        Serial.print(option_short_name);
        if(append_space) { SendSpace(); }
    }

    static void Send(MsgParameter parameter, ErrorType error, bool append_space = true)
    {
        Send(parameter.GetOptionShortName(), error, append_space);
    }

    static void Send(StdReturnType return_value, bool append_space = true)
    {
        if(return_value == kEOk) { Send(ErrorType::kNoError, append_space); }
        else { Send(ErrorType::kUnknown, append_space); }
    }

    static void CheckReturnValueAndSend(ApiType api, StdReturnType return_value, ErrorType error, bool append_space = true)
    {
        if(return_value == kENotOk) {
            Send(api);
            SendColon();
            Send(error);
            if(append_space) { SendSpace(); }
        }
    }

        static void CheckReturnValueAndSend(char option_short_name, StdReturnType return_value, ErrorType error, bool append_space = true)
    {
        if(return_value == kENotOk) {
            Send(option_short_name, error, append_space);
        }
    }
};

#endif
