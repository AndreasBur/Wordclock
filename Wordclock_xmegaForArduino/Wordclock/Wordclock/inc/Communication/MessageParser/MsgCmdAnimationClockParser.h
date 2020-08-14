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
/**     \file       MsgCmdAnimationClockParser.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_ANIMATION_CLOCK_PARSER_H_
#define _MSG_CMD_ANIMATION_CLOCK_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Animation.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdAnimationClockParser configuration parameter */


/* MsgCmdAnimationClockParser parameter */
#define MSG_CMD_ANIMATION_CLOCK_PARSER_PARAMETER_TABLE_SIZE           2u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdAnimationClockParser : public MsgParameterParser<MsgCmdAnimationClockParser, MSG_CMD_ANIMATION_CLOCK_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using AnimationClockType = Animation::AnimationClockType;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    AnimationClockType AnimationClock;
    byte Speed;
        
    friend class MsgParameterParser;
    static constexpr char AnimationOptionShortName{'A'};
    static constexpr char SpeedOptionShortName{'S'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(AnimationOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(SpeedOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };
    
    // functions// functions
    byte convertSpeedToTaskCycle(byte Speed) { return UINT8_MAX - Speed; }
    
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == AnimationOptionShortName) {
            Animation::getInstance().setAnimation(static_cast<Animation::AnimationClockType>(Argument));
        }
        if(ParameterShortName == SpeedOptionShortName) {
             Animation::getInstance().setClockTaskCylce(convertSpeedToTaskCycle(Argument));
        }
    }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgCmdAnimationClockParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter) { }
    ~MsgCmdAnimationClockParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer()
    {
        Serial.print(ModeOptionShortName);
        Serial.print(OptionArgumentDelimiter);
        Serial.print(Display::getInstance().getColorRed());
    }
    
    void process() { }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
