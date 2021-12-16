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
/**     \file       MsgCmdOverlayTemperatureParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_
#define _MSG_CMD_OVERLAY_TEMPERATURE_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgCmdOverlayParser.h"
#include "Overlays.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdOverlayTemperatureParser configuration parameter */


/* MsgCmdOverlayTemperatureParser parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdOverlayTemperatureParser : public MsgCmdOverlayParser<MsgCmdOverlayTemperatureParser>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgCmdOverlayParser;


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdOverlayTemperatureParser(const char* Parameter) : MsgCmdOverlayParser(Parameter) { }
    ~MsgCmdOverlayTemperatureParser() { }

    // get methods

    // set methods

    // methods
    byte getPeriodInMinutes() const { return Overlays::getInstance().getTemperaturePeriodInMinutes(); }
    byte getEnduranceInSeconds() const { return Overlays::getInstance().getTemperatureEnduranceInSeconds(); }
    byte getMonth() const { return Overlays::getInstance().getTemperatureMonth(); }
    byte getDay() const { return Overlays::getInstance().getTemperatureDay(); }
    byte getValidInDays() const { return Overlays::getInstance().getTemperatureValidInDays(); }
    byte getIsActive() const { return Overlays::getInstance().getTemperatureIsActive(); }

    void setPeriodInMinutes(byte PeriodInMinutes) { Overlays::getInstance().setTemperaturePeriodInMinutes(PeriodInMinutes); }
    void setEnduranceInSeconds(byte EnduranceInSeconds) {Overlays::getInstance().setTemperatureEnduranceInSeconds(EnduranceInSeconds); }
    void setMonth(byte Month) { Overlays::getInstance().setTemperatureMonth(Month); }
    void setDay(byte Day) { Overlays::getInstance().setTemperatureDay(Day); }
    void setValidInDays(byte ValidInDays) { Overlays::getInstance().setTemperatureValidInDays(ValidInDays); }
    void setIsActive(bool IsActive) { Overlays::getInstance().setTemperatureIsActive(IsActive); }
    void setText(const char* Text, LengthType Length) { }

    void sendAnswerText(bool AppendSpace) const {}

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
