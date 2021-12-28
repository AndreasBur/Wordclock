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
    enum ErrorType {
        ERROR_NO_ERROR,
        ERROR_MESSAGE_TOO_LONG,
        ERROR_WRONG_COMMAND,
        ERROR_PARAMETER_UNKNOWN,
        ERROR_VALUE_OUT_OF_BOUNCE,
        ERROR_NO_VALUE_GIVEN,
        ERROR_DISPLAY_PENDING,
        ERROR_UNKNOWN
    };

    enum ApiType {
        API_NONE,
        API_DISPLAY_SHOW,
        API_DISPLAY_WRITE_PIXEL,
        API_ANIMATION_SHOW,
        API_CLOCK_SHOW,
        API_ANIMATION_SET_ANIMATION,
        API_ANIMATION_SET_CLOCK_TASK_CYCLE,
        //API_CLOCK_SET_MODE,
        //API_OVERLAY_SET_PERIOD,
        //API_OVERLAY_SET_ENDURANCE
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    //static constexpr char ErrorOptionCharDelimiter{':'};

    // functions
    void sendSpace() const { Serial.print(' '); }
    void sendColon() const { Serial.print(':'); }
    void sendEqual() const { Serial.print('='); }

    void send(ApiType Api) const
    {
        Serial.print(F("Api"));
        sendEqual();
        Serial.print(Api);
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
    void send(ErrorType Error, bool AppendSpace = true) const
    {
        Serial.print(F("Error"));
        sendEqual();
        Serial.print(Error);
        if(AppendSpace) {  }
    }

    void send(char OptionShortName, ErrorType Error, bool AppendSpace = true) const
    {
        send(Error, false);
        sendColon();
        Serial.print(OptionShortName);
        if(AppendSpace) { sendSpace(); }
    }

    void send(MsgParameter Parameter, ErrorType Error, bool AppendSpace = true) const
    {
        send(Parameter.getOptionShortName(), Error, AppendSpace);
    }

    void send(StdReturnType ReturnValue) const
    {
        if(ReturnValue == E_OK) { send(ERROR_NO_ERROR); }
        else { send(ERROR_UNKNOWN); }
    }

//    void send(ApiType Api, StdReturnType ReturnValue, bool AppendSpace = true) const
//    {
//        Serial.print(':');
//        send(ReturnValue);
//        if(AppendSpace) { sendSpace(); }
//    }

    void checkReturnValueAndSend(ApiType Api, StdReturnType ReturnValue, ErrorType Error, bool AppendSpace = true) const
    {
        if(ReturnValue == E_NOT_OK) {
            send(Api);
            sendColon();
            send(Error);
            if(AppendSpace) { sendSpace(); }
        }
    }

        void checkReturnValueAndSend(char OptionShortName, StdReturnType ReturnValue, ErrorType Error, bool AppendSpace = true) const
    {
        if(ReturnValue == E_NOT_OK) {
            send(OptionShortName, Error, AppendSpace);
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
