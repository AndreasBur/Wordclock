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
/**     \file       MsgCmdParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_PARSER_H_
#define _MSG_CMD_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Message.h"
#include "ErrorMessage.h"
#include "Overlays.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdParser configuration parameter */


/* MsgCmdParser parameter */
# define MSG_COMMAND_NONE_NUMBER        0u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M S G   C M D   P A R S E R
******************************************************************************************************************************************************/
class MsgCmdParser
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum CommandType {
        COMMAND_NONE = MSG_COMMAND_NONE_NUMBER,
        COMMAND_REMOTE_PROCEDURE_CALL,
        COMMAND_DISPLAY_COLOR,
        COMMAND_DISPLAY_BRIGHTNESS,
        COMMAND_DISPLAY_PIXEL,
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        COMMAND_OVERLAY_DATE,
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        COMMAND_OVERLAY_TEMPERATURE,
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        COMMAND_OVERLAY_TEXT,
#endif
        COMMAND_CLOCK_MODE,
        COMMAND_ANIMATION,
        COMMAND_TIME,
        COMMAND_DATE,
        COMMAND_STATUS,
        COMMAND_NETWORK,
        COMMAND_NIGHT_SWITCH,
        COMMAND_COLOR_CYCLE,
        COMMAND_CONSOLE
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const char CommandParameterDelimiter{' '};
    ErrorMessage Error;
    const Message& IncomingMessage;

    //private functions
    static void sendAnswer(CommandType Command) {
        Serial.print(Command);
        Serial.print(CommandParameterDelimiter);
    }

    static bool isDecimalDigit(char Char) { return (Char >= '0') && (Char <= '9'); }
    static byte toDigitValue(char Char) { return static_cast<byte>(Char - '0'); }

    /* Counted up digit by digit, and refused the moment the running number passes the
       last command rather than after the whole run: atoi returns an int, so on the AVR
       "65537" wrapped into 1 and was answered as a valid command. strtol reports that
       overflow and was what this did first, at 566 bytes of flash for a routine that
       parses bases, signs and a long - where a command number is one of seventeen. */
    static CommandType toCommand(const char* Message) {
        uint16_t number{0u};
        const char* digit = Message;

        while(isDecimalDigit(*digit)) {
            number = static_cast<uint16_t>((number * 10u) + toDigitValue(*digit));
            if(number > COMMAND_CONSOLE) { return COMMAND_NONE; }
            digit++;
        }
        /* Not one digit read: a message that is not a number at all, which is the same
           COMMAND_NONE that a zero converts to on its own. */
        if(digit == Message) { return COMMAND_NONE; }

        return static_cast<CommandType>(number);
    }

    CommandType getCommand() const { return toCommand(IncomingMessage.getMessage()); }

    /* A command without a delimiter has no parameter, and the terminator is where that
       empty string lives. It used to be the last character instead, so "12" handed the
       parameter parser a "2" to read options out of - and an empty message asked for
       index length() - 1, which is a strlen of zero minus one and not a position at all. */
    const char* getParameter() const {
        const char* message = IncomingMessage.getMessage();
        size_t valuePos = IncomingMessage.find(CommandParameterDelimiter);

        if(valuePos == Message::npos) { return &message[IncomingMessage.length()]; }
        else { return &message[valuePos]; }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdParser(const Message& sMessage) : Error(), IncomingMessage(sMessage)  { }
    ~MsgCmdParser() { }

    // get methods


    // set methods

    // methods
    void parse();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
