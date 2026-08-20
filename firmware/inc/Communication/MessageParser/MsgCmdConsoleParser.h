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
/**     \file       MsgCmdConsoleParser.h
 *      \brief      Sets the password the web console asks for, or takes it away
 *
 *      \details    One option in, one answer out, and they are not the same field. The
 *                  password goes in and is never read back, exactly as the WiFi pass phrase
 *                  is not - what comes back is whether one is set at all, which is the only
 *                  thing somebody needs to know from the outside.
 *
 *                  Sent without a password, the command clears it and the console answers
 *                  everybody again. That is the way back for a clock whose password was
 *                  forgotten, and it is why this command is worth having on the serial line
 *                  rather than only in the console it locks: the console would ask for the
 *                  very password that is missing.
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_CONSOLE_PARSER_H_
#define _MSG_CMD_CONSOLE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "System.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
#define MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE             1u

/******************************************************************************************************************************************************
 *  C L A S S   M S G   C M D   C O N S O L E   P A R S E R
******************************************************************************************************************************************************/
class MsgCmdConsoleParser : public MsgParameterParser<MsgCmdConsoleParser, MSG_CMD_CONSOLE_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char PasswordShortName{'P'};
    static constexpr char ProtectedShortName{'A'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(PasswordShortName, MsgParameter::ARGUMENT_TYPE_STRING)
    };

    // functions
    void handleParameter(char ParameterShortName, byte Argument) {
        UNUSED(ParameterShortName); UNUSED(Argument);
    }
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length)
    {
        UNUSED(Length);

        if(ParameterShortName == PasswordShortName) { System::getInstance().setConsolePassword(Argument); }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdConsoleParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdConsoleParser() { }

    // methods
    /* Whether a password is set, and never which one. Last field before the command parser's
       terminating println(), so no trailing separator space. */
    void sendAnswer() const {
        sendAnswerParameter(ProtectedShortName,
                            static_cast<byte>(System::getInstance().isConsoleProtected() ? 1u : 0u), false);
    }

    /* Nothing deferred: the password is stored where it arrives, since there is no second
       field it has to agree with. */
    void process() const { }
};

#endif // _MSG_CMD_CONSOLE_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
