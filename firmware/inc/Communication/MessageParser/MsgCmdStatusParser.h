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
/**     \file       MsgCmdStatusParser.h
 *      \brief      What the clock can say about itself
 *
 *      \details    Everything a remote procedure call cannot answer. Its answer is
 *                  "RpcId=<id> Error=<code>" and carries no value, so the readings that
 *                  were reachable nowhere else - what the light sensor measures, what the
 *                  clock chip measures, which firmware is running - are a command with the
 *                  same query semantics as every other one instead.
 *
 *                  It is read-only, and its parameter table is empty rather than filled
 *                  with fields that are ignored: an option sent to it is answered with
 *                  ErrorType::kParameterUnknown by the base parser, which is what a caller
 *                  trying to set a reading should hear. The field names still exist - in
 *                  the message catalog, marked read-only - so a front end can read the
 *                  answer back through them without offering them as inputs.
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_STATUS_PARSER_H_
#define _MSG_CMD_STATUS_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Illuminance.h"
#include "System.h"
#include "Temperature.h"
#include "Uptime.h"
#include "Version.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdStatusParser parameter */
/* Empty: the command takes nothing, see the file header. */
#define MSG_CMD_STATUS_PARSER_PARAMETER_TABLE_SIZE                  0u

/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdStatusParser : public MsgParameterParser<MsgCmdStatusParser, MSG_CMD_STATUS_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char kVersionShortName{'V'};
    static constexpr char kUptimeDaysShortName{'U'};
    static constexpr char kUptimeHoursShortName{'H'};
    static constexpr char kUptimeMinutesShortName{'N'};
    static constexpr char kIlluminanceShortName{'I'};
    static constexpr char kTemperatureShortName{'T'};
    static constexpr char kAddressShortName{'A'};
    static constexpr char kLinkQualityShortName{'Q'};
    static constexpr char kFreeMemoryShortName{'M'};

    static constexpr ParameterTableType kParameterTable PROGMEM { };

    // functions
    // Never called - the table above has nothing to match an option against - but the base
    // parser instantiates both, so both have to exist.
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    static void HandleParameter(char parameter_short_name, byte argument) { UNUSED(parameter_short_name); UNUSED(argument); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdStatusParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdStatusParser() { }

    /*
     * Every field that can be absent is sent empty rather than as a zero that reads like a
     * value: no chip, no network, no answer. The getters write the terminator either way,
     * so what is sent after a refusal is the empty string.
     */
    static void SendAnswerNumber(char short_name, StdReturnType return_value, uint16_t value, bool append_space = true) {
        if(return_value == kEOk) { SendAnswerParameter(short_name, value, append_space); }
        else { SendAnswerParameter(short_name, "", append_space); }
    }

    // methods
    static void SendAnswer() {
        const System& system = System::GetInstance();

        char temperature_string[Temperature::kStringLength];
        char address_string[System::kAddressStringLength];
        char link_quality_string[System::kLinkQualityStringLength];
        uint16_t free_memory{0u};

        /*
         * Every value is fetched before anything is sent, rather than inside the call that
         * sends it: an out parameter and the return code that says whether it was written
         * are two arguments of the same call, and which of them is evaluated first is not
         * the compiler's to be trusted with - it reported a zero uptime where a fetched
         * one was right beside it.
         */
        Temperature::GetInstance().GetTemperatureString(temperature_string);
        const StdReturnType free_memory_return = system.GetFreeMemoryInKibibytes(free_memory);
        const StdReturnType address_return = system.GetNetworkAddress(address_string);
        const StdReturnType link_quality_return = system.GetLinkQuality(link_quality_string);

        SendAnswerParameter(kVersionShortName, WORDCLOCK_VERSION);
        // Counted in the core rather than asked of the platform, so there is no return code
        // to weigh: every build has an uptime, and it is always an answer.
        const Uptime& uptime = Uptime::GetInstance();
        SendAnswerParameter(kUptimeDaysShortName, uptime.GetDays());
        SendAnswerParameter(kUptimeHoursShortName, uptime.GetHours());
        SendAnswerParameter(kUptimeMinutesShortName, uptime.GetMinutes());
        SendAnswerParameter(kIlluminanceShortName, Illuminance::GetInstance().GetIlluminance());
        SendAnswerParameter(kTemperatureShortName, temperature_string);
        SendAnswerParameter(kAddressShortName, (address_return == kEOk) ? address_string : "");
        SendAnswerParameter(kLinkQualityShortName, (link_quality_return == kEOk) ? link_quality_string : "");
        // Last field before the command parser's terminating println(), so no trailing
        // separator space.
        SendAnswerNumber(kFreeMemoryShortName, free_memory_return, free_memory, false);
    }

    static void Process() { }
};

#endif // _MSG_CMD_STATUS_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
