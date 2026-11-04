// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the animation command.
 */

#ifndef _MSG_CMD_ANIMATION_PARSER_H_
#define _MSG_CMD_ANIMATION_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Animations.h"
#include "Scheduler.h"

/* MsgCmdAnimationParser configuration parameter */

/* MsgCmdAnimationParser parameter */
#define MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE           4u

class MsgCmdAnimationParser : public MsgParameterParser<MsgCmdAnimationParser, MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE>
{
  public:
    using AnimationIdType = Animations::AnimationIdType;
    using ModeType = Animations::ModeType;

  private:
    friend class MsgParameterParser;
    AnimationIdType animation_id_{Animations::AnimationIdType::kNone};
    ModeType mode_{Animations::ModeType::kFixed};
    byte speed_{0u};
    bool favourite_{true};

    /*
     * Which options the message actually carried. The values above are preloaded with
     * the current settings so a message may set single options, but writing all of them
     * back would do harm: -F and -S address whatever -A points at, so a preloaded value
     * would land on the newly selected animation instead of the one it was read from.
     * A message without options also has to stay a pure query.
     */
    bool animation_given_{false};
    bool mode_given_{false};
    bool speed_given_{false};
    bool favourite_given_{false};

    static constexpr char kAnimationOptionShortName{'A'};
    static constexpr char kModeOptionShortName{'M'};
    static constexpr char kSpeedOptionShortName{'S'};
    static constexpr char kFavouriteOptionShortName{'F'};

    static constexpr ParameterTableType kParameterTable PROGMEM {
        ParameterTableElementType(kAnimationOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kModeOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kSpeedOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
        ParameterTableElementType(kFavouriteOptionShortName, MsgParameter::ArgumentTypeType::kUint8)
    };

    // functions
    static void HandleParameter(char parameter_short_name, const char* argument, PositionType length) { UNUSED(parameter_short_name); UNUSED(argument); UNUSED(length); }
    void HandleParameter(char parameter_short_name, byte argument)
    {
        if(parameter_short_name == kAnimationOptionShortName) {
            animation_id_ = static_cast<Animations::AnimationIdType>(argument);
            animation_given_ = true;
        }
        if(parameter_short_name == kModeOptionShortName) {
            mode_ = static_cast<Animations::ModeType>(argument);
            mode_given_ = true;
        }
        if(parameter_short_name == kSpeedOptionShortName) {
             speed_ = argument;
             speed_given_ = true;
        }
        if(parameter_short_name == kFavouriteOptionShortName) {
            favourite_ = (argument != 0u);
            favourite_given_ = true;
        }
    }

    static void SendAnswerAnimation(bool append_space) {
        SendAnswerParameter(kAnimationOptionShortName, ToUnderlying(Animations::GetInstance().GetAnimation()), append_space);
    }

    static void SendAnswerMode(bool append_space) {
        SendAnswerParameter(kModeOptionShortName, ToUnderlying(Animations::GetInstance().GetMode()), append_space);
    }

    static void SendAnswerSpeed(bool append_space) {
        AnimationIdType animation = Animations::GetInstance().GetAnimation();
        byte task_cycle = Animations::GetInstance().GetTaskCycle(animation);
        SendAnswerParameter(kSpeedOptionShortName, Scheduler::ConvertTaskCycleToSpeed(task_cycle), append_space);
    }

    void SetAnimation() const
    {
        StdReturnType return_value = Animations::GetInstance().SetAnimation(animation_id_);
        error_.CheckReturnValueAndSend(kAnimationOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetMode() const
    {
        StdReturnType return_value = Animations::GetInstance().SetMode(mode_);
        error_.CheckReturnValueAndSend(kModeOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    static void SendAnswerFavourite(bool append_space) {
        /* the flag of the selected animation, not of the one a mode is running */
        AnimationIdType animation = Animations::GetInstance().GetAnimation();
        byte favourite = Animations::GetInstance().IsFavourite(animation) ? 1u : 0u;
        SendAnswerParameter(kFavouriteOptionShortName, favourite, append_space);
    }

    void SetFavourite() const
    {
        StdReturnType return_value = Animations::GetInstance().SetFavourite(animation_id_, favourite_);
        error_.CheckReturnValueAndSend(kFavouriteOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

    void SetClockTaskCycle() const
    {
        StdReturnType return_value = Animations::GetInstance().SetTaskCycle(animation_id_, Scheduler::ConvertSpeedToTaskCycle(speed_));
        error_.CheckReturnValueAndSend(kSpeedOptionShortName, return_value, ErrorMessage::ErrorType::kValueOutOfBounds);
    }

  public:
    MsgCmdAnimationParser(const char* parameter) : MsgParameterParser(kParameterTable, parameter), animation_id_(), mode_(), speed_(0u), favourite_(true),
        animation_given_(false), mode_given_(false), speed_given_(false), favourite_given_(false)
    {
        // preloaded with the current settings, so a command may set single options
        // without resetting the others
        animation_id_ = Animations::GetInstance().GetAnimation();
        mode_ = Animations::GetInstance().GetMode();
        speed_ = Scheduler::ConvertTaskCycleToSpeed(Animations::GetInstance().GetTaskCycle(animation_id_));
        favourite_ = Animations::GetInstance().IsFavourite(animation_id_);
    }

    ~MsgCmdAnimationParser() { }

    // get methods

    // set methods

    // methods
    static void SendAnswer()
    {
        SendAnswerAnimation(true);
        SendAnswerMode(true);
        SendAnswerFavourite(true);
        SendAnswerSpeed(false);
    }

    void Process() const
    {
        /* the animation first, since -F and -S address the one it selects */
        if(animation_given_) { SetAnimation(); }
        if(mode_given_) { SetMode(); }
        if(favourite_given_) { SetFavourite(); }
        if(speed_given_) { SetClockTaskCycle(); }
    }
};

#endif
