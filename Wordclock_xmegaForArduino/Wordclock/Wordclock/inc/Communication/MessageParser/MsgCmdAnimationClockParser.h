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
    friend class MsgParameterParser;
    AnimationClockType AnimationClock{AnimationClock::ANIMATION_CLOCK_NONE};
    byte Speed{0u};

    static constexpr char AnimationOptionShortName{'A'};
    static constexpr char SpeedOptionShortName{'S'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(AnimationOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(SpeedOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    byte convertSpeedToTaskCycle(byte Speed) const { return UINT8_MAX - Speed; }
    byte convertTaskCycleToSpeed(byte TaskCylce) const { return UINT8_MAX - TaskCylce; }

    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == AnimationOptionShortName) {
            AnimationClock = static_cast<Animation::AnimationClockType>(Argument);
        }
        if(ParameterShortName == SpeedOptionShortName) {
             Speed = Argument;
        }
    }

    void sendAnswerAnimation() const { sendAnswerParameter(AnimationOptionShortName, Animation::getInstance().getAnimation()); }
    void sendAnswerSpeed() const {
        AnimationClockType animation = Animation::getInstance().getAnimation();
        byte taskCylce = Animation::getInstance().getClockTaskCycle(animation);
        sendAnswerParameter(SpeedOptionShortName, convertTaskCycleToSpeed(taskCylce), false);
    }

    void setAnimation() const
    {
        StdReturnType returnValue = Animation::getInstance().setAnimation(AnimationClock);
        Error.checkReturnValueAndSend(ErrorMessage::API_ANIMATION_SET_ANIMATION, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

    void setClockTaskCylce() const
    {
        StdReturnType returnValue = Animation::getInstance().setClockTaskCylce(AnimationClock, convertSpeedToTaskCycle(Speed));
        Error.checkReturnValueAndSend(ErrorMessage::API_ANIMATION_SET_CLOCK_TASK_CYCLE, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNCE);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    MsgCmdAnimationClockParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter), AnimationClock(), Speed(0u)
    {
        AnimationClock = Animation::getInstance().getAnimation();
        Speed = convertTaskCycleToSpeed(Animation::getInstance().getClockTaskCycle(AnimationClock));
    }

    ~MsgCmdAnimationClockParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        sendAnswerAnimation();
        sendAnswerSpeed();
    }

    void process() const
    {
        setAnimation();
        setClockTaskCylce();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
