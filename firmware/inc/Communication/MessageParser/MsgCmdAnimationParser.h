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
/**     \file       MsgCmdAnimationParser.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_CMD_ANIMATION_PARSER_H_
#define _MSG_CMD_ANIMATION_PARSER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Animations.h"
#include "Scheduler.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* MsgCmdAnimationParser configuration parameter */


/* MsgCmdAnimationParser parameter */
#define MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE           4u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class MsgCmdAnimationParser : public MsgParameterParser<MsgCmdAnimationParser, MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE>
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using AnimationIdType = Animations::AnimationIdType;
    using ModeType = Animations::ModeType;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    friend class MsgParameterParser;
    AnimationIdType AnimationId{Animations::ANIMATION_ID_NONE};
    ModeType Mode{Animations::MODE_FIXED};
    byte Speed{0u};
    bool Favourite{true};

    static constexpr char AnimationOptionShortName{'A'};
    static constexpr char ModeOptionShortName{'M'};
    static constexpr char SpeedOptionShortName{'S'};
    static constexpr char FavouriteOptionShortName{'F'};

    static constexpr ParameterTableType ParameterTable PROGMEM {
        ParameterTableElementType(AnimationOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(ModeOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(SpeedOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8),
        ParameterTableElementType(FavouriteOptionShortName, MsgParameter::ARGUMENT_TYPE_UINT8)
    };

    // functions
    void handleParameter(char ParameterShortName, const char* Argument, PositionType Length) { UNUSED(ParameterShortName); UNUSED(Argument); UNUSED(Length); }
    void handleParameter(char ParameterShortName, byte Argument)
    {
        if(ParameterShortName == AnimationOptionShortName) {
            AnimationId = static_cast<Animations::AnimationIdType>(Argument);
        }
        if(ParameterShortName == ModeOptionShortName) {
            Mode = static_cast<Animations::ModeType>(Argument);
        }
        if(ParameterShortName == SpeedOptionShortName) {
             Speed = Argument;
        }
        if(ParameterShortName == FavouriteOptionShortName) {
            Favourite = (Argument != 0u);
        }
    }

    void sendAnswerAnimation(bool AppendSpace) const {
        sendAnswerParameter(AnimationOptionShortName, Animations::getInstance().getAnimation(), AppendSpace);
    }

    void sendAnswerMode(bool AppendSpace) const {
        sendAnswerParameter(ModeOptionShortName, Animations::getInstance().getMode(), AppendSpace);
    }

    void sendAnswerSpeed(bool AppendSpace) const {
        AnimationIdType animation = Animations::getInstance().getAnimation();
        byte taskCycle = Animations::getInstance().getTaskCycle(animation);
        sendAnswerParameter(SpeedOptionShortName, Scheduler::convertTaskCycleToSpeed(taskCycle), AppendSpace);
    }

    void setAnimation() const
    {
        StdReturnType returnValue = Animations::getInstance().setAnimation(AnimationId);
        Error.checkReturnValueAndSend(AnimationOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNDS);
    }

    void setMode() const
    {
        StdReturnType returnValue = Animations::getInstance().setMode(Mode);
        Error.checkReturnValueAndSend(ModeOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNDS);
    }

    void sendAnswerFavourite(bool AppendSpace) const {
        /* the flag of the selected animation, not of the one a mode is running */
        AnimationIdType animation = Animations::getInstance().getAnimation();
        byte favourite = Animations::getInstance().isFavourite(animation) ? 1u : 0u;
        sendAnswerParameter(FavouriteOptionShortName, favourite, AppendSpace);
    }

    void setFavourite() const
    {
        StdReturnType returnValue = Animations::getInstance().setFavourite(AnimationId, Favourite);
        Error.checkReturnValueAndSend(FavouriteOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNDS);
    }

    void setClockTaskCycle() const
    {
        StdReturnType returnValue = Animations::getInstance().setTaskCycle(AnimationId, Scheduler::convertSpeedToTaskCycle(Speed));
        Error.checkReturnValueAndSend(SpeedOptionShortName, returnValue, ErrorMessage::ERROR_VALUE_OUT_OF_BOUNDS);
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    MsgCmdAnimationParser(const char* Parameter) : MsgParameterParser(ParameterTable, Parameter), AnimationId(), Mode(), Speed(0u), Favourite(true)
    {
        /* preloaded with the current settings, so a command may set single options
           without resetting the others */
        AnimationId = Animations::getInstance().getAnimation();
        Mode = Animations::getInstance().getMode();
        Speed = Scheduler::convertTaskCycleToSpeed(Animations::getInstance().getTaskCycle(AnimationId));
        Favourite = Animations::getInstance().isFavourite(AnimationId);
    }

    ~MsgCmdAnimationParser() { }

    // get methods

    // set methods

    // methods
    void sendAnswer() const
    {
        sendAnswerAnimation(true);
        sendAnswerMode(true);
        sendAnswerFavourite(true);
        sendAnswerSpeed(false);
    }

    void process() const
    {
        setAnimation();
        setMode();
        setFavourite();
        setClockTaskCycle();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
