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
    enum class CommandType {
        kNone = MSG_COMMAND_NONE_NUMBER,
        kRemoteProcedureCall,
        kDisplayColor,
        kDisplayBrightness,
        kDisplayPixel,
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        kOverlayDate,
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        kOverlayTemperature,
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        kOverlayText,
#endif
        kClockMode,
        kAnimation,
        kTime,
        kDate,
        kStatus,
        kNetwork,
        kNightSwitch,
        kColorCycle,
        kConsole
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const char kCommandParameterDelimiter{' '};
    ErrorMessage error_;
    const Message& incoming_message_;

    //private functions
    static void SendAnswer(CommandType command) {
        Serial.print(ToUnderlying(command));
        Serial.print(kCommandParameterDelimiter);
    }

    static bool IsDecimalDigit(char character) { return (character >= '0') && (character <= '9'); }
    static byte ToDigitValue(char character) { return static_cast<byte>(character - '0'); }

    /* Counted up digit by digit, and refused the moment the running number passes the
       last command rather than after the whole run: atoi returns an int, so on the AVR
       "65537" wrapped into 1 and was answered as a valid command. strtol reports that
       overflow and was what this did first, at 566 bytes of flash for a routine that
       parses bases, signs and a long - where a command number is one of seventeen. */
    static CommandType ToCommand(const char* message) {
        uint16_t number{0u};
        const char* digit = message;

        while(IsDecimalDigit(*digit)) {
            number = static_cast<uint16_t>((number * 10u) + ToDigitValue(*digit));
            if(number > ToUnderlying(CommandType::kConsole)) { return CommandType::kNone; }
            digit++;
        }
        /* Not one digit read: a message that is not a number at all, which is the same
           CommandType::kNone that a zero converts to on its own. */
        if(digit == message) { return CommandType::kNone; }

        return static_cast<CommandType>(number);
    }

    CommandType GetCommand() const { return ToCommand(incoming_message_.GetMessage()); }

    /* A command without a delimiter has no parameter, and the terminator is where that
       empty string lives. It used to be the last character instead, so "12" handed the
       parameter parser a "2" to read options out of - and an empty message asked for
       index Length() - 1, which is a strlen of zero minus one and not a position at all. */
    const char* GetParameter() const {
        const char* message = incoming_message_.GetMessage();
        size_t value_pos = incoming_message_.Find(kCommandParameterDelimiter);

        if(value_pos == Message::npos) { return &message[incoming_message_.Length()]; }
        else { return &message[value_pos]; }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdParser(const Message& message) : error_(), incoming_message_(message)  { }
    ~MsgCmdParser() { }

    // get methods


    // set methods

    // methods
    void Parse();

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
