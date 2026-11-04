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
/**     \file       ErrorMessage.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _ERROR_MESSAGE_H_
#define _ERROR_MESSAGE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameter.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* ErrorMessage configuration parameter */


/* ErrorMessage parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   E R R O R   M E S S A G E
******************************************************************************************************************************************************/
class ErrorMessage
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
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

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
