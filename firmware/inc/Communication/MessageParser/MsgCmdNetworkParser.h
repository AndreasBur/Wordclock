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
/**     \file       MsgCmdNetworkParser.h
 *      \brief      Which network the clock joins
 *
 *      \details    The credentials used to be compiled in, which made every change of
 *                  network a reflash. They are stored on the platform now and set through
 *                  this command, over whichever console can reach the clock - the serial
 *                  line, or the web console on the access point the clock opens when it
 *                  has no network to join.
 *
 *                  The pass phrase goes in and never comes back out: the answer names the
 *                  network and says whether one is configured at all. A console that
 *                  echoed it would put it in the scrollback of every browser that watched.
 *
 *                  Both are taken as they arrive and applied together, because half a pair
 *                  is a clock that cannot join anything.
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_NETWORK_PARSER_H_
#define _MSG_CMD_NETWORK_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "StringTools.h"
#include "System.h"

#include <string.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdNetworkParser parameter */
#define MSG_CMD_NETWORK_PARSER_PARAMETER_TABLE_SIZE                 2u

/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdNetworkParser : public MsgParameterParser<MsgCmdNetworkParser, MSG_CMD_NETWORK_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char SsidShortName{'S'};
    static constexpr char PasswordShortName{'P'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(SsidShortName, MsgParameter::ARGUMENT_TYPE_STRING),
        ParameterTableElementType(PasswordShortName, MsgParameter::ARGUMENT_TYPE_STRING)
    };

    char Ssid[System::SsidStringLength]{0u};
    char Password[System::PasswordStringLength]{0u};
    bool SsidGiven{false};
    bool PasswordGiven{false};
    StdReturnType ReturnValue{E_OK};

    // functions
    void handleParameter(char ParameterShortName, byte Argument) { UNUSED(ParameterShortName); UNUSED(Argument); }
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length)
    {
        if(ParameterShortName == SsidShortName) {
            StringTools::stringCopy(Ssid, Argument, minimum(Length, sizeof(Ssid)));
            trimTrailingBlanks(Ssid);
            SsidGiven = true;
        }
        if(ParameterShortName == PasswordShortName) {
            StringTools::stringCopy(Password, Argument, minimum(Length, sizeof(Password)));
            trimTrailingBlanks(Password);
            PasswordGiven = true;
        }
    }

    static constexpr size_t minimum(size_t Left, size_t Right) { return (Left < Right) ? Left : Right; }

    /* A string argument runs to the next option, so it carries the space that separates
       them. A network called "Home " is not what anybody typed, and joining it would fail
       in a way that names nothing - so a trailing blank is taken off here. It also means a
       name or a pass phrase cannot end in a space over this interface, which is written
       down in the command reference. */
    static void trimTrailingBlanks(char* String) {
        for(size_t Index = strlen(String); (Index > 0u) && (String[Index - 1u] == ' '); Index--) {
            String[Index - 1u] = STD_NULL_CHARACTER;
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdNetworkParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdNetworkParser() { }

    // methods
    /* Names the network and says whether there is one, and never the pass phrase. */
    void sendAnswer() const {
        char StoredSsid[System::SsidStringLength]{};

        /* An empty name is a clock with no network, which is the state the access point
           stands in for - the same empty field the status command answers with. */
        System::getInstance().getNetworkSsid(StoredSsid);

        sendAnswerParameter(SsidShortName, StoredSsid);
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        Error.send(ReturnValue, false);
    }

    /* Both halves or neither: a command that carried only one of them would either join a
       network with the previous pass phrase or store a pass phrase for the previous
       network, and both of those are a clock that goes quiet. */
    void process()
    {
        if(!SsidGiven && !PasswordGiven) { return; }

        if(!SsidGiven || !PasswordGiven) {
            ReturnValue = E_NOT_OK;
            return;
        }

        ReturnValue = System::getInstance().setNetworkCredentials(Ssid, Password);
    }
};

#endif // _MSG_CMD_NETWORK_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
