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

    static constexpr char kSsidShortName{'S'};
    static constexpr char kPasswordShortName{'P'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kSsidShortName, MsgParameter::ArgumentTypeType::kString),
        ParameterTableElementType(kPasswordShortName, MsgParameter::ArgumentTypeType::kString)
    };

    char ssid_[System::kSsidStringLength]{0u};
    char password_[System::kPasswordStringLength]{0u};
    bool ssid_given_{false};
    bool password_given_{false};
    StdReturnType return_value_{kEOk};

    // functions
    static void HandleParameter(char parameter_short_name, byte argument) { UNUSED(parameter_short_name); UNUSED(argument); }
    void HandleParameter(char parameter_short_name, const char* argument, PositionType length)
    {
        if(parameter_short_name == kSsidShortName) {
            StringTools::StringCopy(ssid_, argument, Minimum(length, sizeof(ssid_)));
            TrimTrailingBlanks(ssid_);
            ssid_given_ = true;
        }
        if(parameter_short_name == kPasswordShortName) {
            StringTools::StringCopy(password_, argument, Minimum(length, sizeof(password_)));
            TrimTrailingBlanks(password_);
            password_given_ = true;
        }
    }

    static constexpr size_t Minimum(size_t left, size_t right) { return (left < right) ? left : right; }

    /*
     * A string argument runs to the next option, so it carries the space that separates
     * them. A network called "Home " is not what anybody typed, and joining it would fail
     * in a way that names nothing - so a trailing blank is taken off here. It also means a
     * name or a pass phrase cannot end in a space over this interface, which is written
     * down in the command reference.
     */
    static void TrimTrailingBlanks(char* string) {
        for(size_t index = strlen(string); (index > 0u) && (string[index - 1u] == ' '); index--) {
            string[index - 1u] = STD_NULL_CHARACTER;
        }
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdNetworkParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdNetworkParser() { }

    // methods
    /* Names the network and says whether there is one, and never the pass phrase. */
    void SendAnswer() const {
        char stored_ssid[System::kSsidStringLength]{};

        // An empty name is a clock with no network, which is the state the access point
        // stands in for - the same empty field the status command answers with.
        System::GetInstance().GetNetworkSsid(stored_ssid);

        SendAnswerParameter(kSsidShortName, stored_ssid);
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        error_.Send(return_value_, false);
    }

    /*
     * Both halves or neither: a command that carried only one of them would either join a
     * network with the previous pass phrase or store a pass phrase for the previous
     * network, and both of those are a clock that goes quiet.
     */
    void Process()
    {
        if(!ssid_given_ && !password_given_) { return; }

        if(!ssid_given_ || !password_given_) {
            return_value_ = kENotOk;
            return;
        }

        return_value_ = System::GetInstance().SetNetworkCredentials(ssid_, password_);
    }
};

#endif // _MSG_CMD_NETWORK_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
