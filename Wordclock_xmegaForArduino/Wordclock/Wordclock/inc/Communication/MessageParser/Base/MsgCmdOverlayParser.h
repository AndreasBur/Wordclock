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
/**     \file       MsgCmdOverlayParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_OVERLAY_PARSER_H_
#define _MSG_CMD_OVERLAY_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "MsgCmdBaseOverlayParser.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdOverlayParser configuration parameter */


/* MsgCmdOverlayParser parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M S G   C M D   O V E R L A Y   P A R S E R
******************************************************************************************************************************************************/
template <typename Derived> class MsgCmdOverlayParser
    : public MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>, public MsgCmdBaseOverlayParser
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using MsgParameterParserType = MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;
    using PositionType = typename MsgParameterParserType::PositionType;
    using LengthType = StringTools::LengthType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    // functions
    constexpr MsgCmdOverlayParser(const char* Parameter) : MsgParameterParserType(ParameterTable, Parameter) { }
    ~MsgCmdOverlayParser() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser<MsgCmdOverlayParser<Derived>, MSG_CMD_BASE_OVERLAY_PARSER_PARAMETER_TABLE_SIZE>;

    // functions
    Derived& underlying() { return static_cast<Derived&>(*this); }
    Derived const& underlying() const { return static_cast<Derived const&>(*this); }

    void handleParameter(char ParameterShortName, byte Argument) {
        if(ParameterShortName == PeriodOptionShortName) { underlying().setPeriodInMinutes(Argument); }
        if(ParameterShortName == EnduranceOptionShortName) { underlying().setEnduranceInSeconds(Argument); }
        if(ParameterShortName == MonthOptionShortName) { underlying().setMonth(Argument); }
        if(ParameterShortName == ValidOptionShortName) { underlying().setValidInDays(Argument); }
        if(ParameterShortName == ActiveOptionShortName) { underlying().setIsActive(static_cast<bool>(Argument)); }
        if(ParameterShortName == TextSpeedOptionShortName) { underlying().setTextSpeed(Argument); }
    }

    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) {
        if(ParameterShortName == TextOptionShortName) { underlying().setText(Argument, Length); }
    }

    void sendAnswerPeriod(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(PeriodOptionShortName,  underlying().getPeriodInMinutes(), AppendSpace); }
    void sendAnswerEndurance(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(EnduranceOptionShortName, underlying().getEnduranceInSeconds(), AppendSpace); }
    void sendAnswerMonth(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(MonthOptionShortName, underlying().getMonth(), AppendSpace); }
    void sendAnswerDay(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(MonthOptionShortName, underlying().getDay(), AppendSpace); }
    void sendAnswerValid(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(ValidOptionShortName, underlying().getValidInDays(), AppendSpace); }
    void sendAnswerActive(bool AppendSpace) const { MsgParameterParserType::sendAnswerParameter(ActiveOptionShortName, underlying().getIsActive(), AppendSpace); }
    void sendAnswerText(bool AppendSpace) const { underlying().sendAnswerText(AppendSpace); }
    void sendAnswerSpeedText(bool AppendSpace) const { underlying().sendAnswerSpeedText(AppendSpace); }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:


    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        sendAnswerPeriod(true);
        sendAnswerEndurance(true);
        sendAnswerMonth(true);
        sendAnswerDay(true);
        sendAnswerValid(true);
        sendAnswerText(true);
        sendAnswerSpeedText(true);
        sendAnswerActive(false);
    }

    void process() const {  }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
