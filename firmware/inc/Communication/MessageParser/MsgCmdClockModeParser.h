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
/**     \file       MsgCmdClockModeParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_CLOCK_MODE_PARSER_H_
#define _MSG_CMD_CLOCK_MODE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Clock.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdClockModeParser configuration parameter */


/* MsgCmdClockModeParser parameter */
#define MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE           2u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdClockModeParser : public MsgParameterParser<MsgCmdClockModeParser, MSG_CMD_CLOCK_MODE_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using ModeType = Clock::ModeType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;

    static constexpr char kModeOptionShortName{'M'};
    static constexpr char kShowItIsOptionShortName{'I'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kModeOptionShortName, MsgParameter::kArgumentTypeUint8),
        ParameterTableElementType(kShowItIsOptionShortName, MsgParameter::kArgumentTypeUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kModeOptionShortName) {
            SetMode(static_cast<ModeType>(argument));
        }
        if(parameter_short_name == kShowItIsOptionShortName) {
            Clock::GetInstance().SetShowItIsPermanently(argument != 0u);
        }
    }

    void SetMode(ModeType mode) const
    {
        StdReturnType return_value = Clock::GetInstance().SetMode(mode);
        error_.CheckReturnValueAndSend(kModeOptionShortName, return_value, ErrorMessage::kErrorValueOutOfBounds);
    }

    void Show() const
    {
        StdReturnType return_value = Clock::GetInstance().Show();
        error_.CheckReturnValueAndSend(ErrorMessage::kApiClockShow, return_value, ErrorMessage::kErrorDisplayPending);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdClockModeParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter) { }
    ~MsgCmdClockModeParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer() {
        SendAnswerParameter(kModeOptionShortName, Clock::GetInstance().GetMode());
        /* Last field before the command parser's terminating println(), so no trailing
           separator space. */
        SendAnswerParameter(kShowItIsOptionShortName, static_cast<byte>(Clock::GetInstance().GetShowItIsPermanently() ? 1u : 0u), false);
    }

    void Process() const { Show(); }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
