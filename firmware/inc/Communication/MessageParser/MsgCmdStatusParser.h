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
 *                  ERROR_PARAMETER_UNKNOWN by the base parser, which is what a caller
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
#include "Temperature.h"
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

    static constexpr char VersionShortName{'V'};
    static constexpr char IlluminanceShortName{'I'};
    static constexpr char TemperatureShortName{'T'};

    static constexpr ParameterTableType ParameterTable PROGMEM { };

    // functions
    /* Never called - the table above has nothing to match an option against - but the base
       parser instantiates both, so both have to exist. */
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length); }
    void handleParameter(char ParameterShortName, byte Argument) { UNUSED(ParameterShortName); UNUSED(Argument); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdStatusParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdStatusParser() { }

    // methods
    void sendAnswer() const {
        char TemperatureString[Temperature::StringLength];
        /* An empty field where there is no reading, rather than a zero that reads like
           one. Temperature::getTemperatureString() writes the terminator either way. */
        Temperature::getInstance().getTemperatureString(TemperatureString);

        sendAnswerParameter(VersionShortName, WORDCLOCK_VERSION);
        sendAnswerParameter(IlluminanceShortName, Illuminance::getInstance().getIlluminance());
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        sendAnswerParameter(TemperatureShortName, TemperatureString, false);
    }

    void process() const { }
};

#endif // _MSG_CMD_STATUS_PARSER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
