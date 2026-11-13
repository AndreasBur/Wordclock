// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The parser of the animation command.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDANIMATIONPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDANIMATIONPARSER_H_

#include "Animations.h"
#include "Arduino.h"
#include "MsgParameterParser.h"
#include "Scheduler.h"
#include "StandardTypes.h"

/*! \brief The number of options in the parameter table of the animation command. */
#define MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE 4u

/*!
 * \brief The parser of command 9, the animation command: selects the animation of a word change and
 *        the mode that picks it, and sets the speed and the favourite flag of an animation.
 *
 * Its options:
 * - `-A<id>`, uint8: selects the animation, Animations::AnimationIdType, and makes it the current
 *   one at once; mode 0 runs it on each word change. An id from Animations::kNumberOfAnimations on
 *   is answered with `Error=4:A`.
 * - `-M<mode>`, uint8: sets the mode, Animations::ModeType. One from Animations::kNumberOfModes on
 *   is answered with `Error=4:M`.
 * - `-F<0|1>`, uint8: with any value but 0, makes the animation a favourite, one that the modes 1
 *   and 2 may pick; with 0, leaves it out. The flag of id 0, and taking the last favourite out,
 *   are answered with `Error=4:F`.
 * - `-S<speed>`, uint8: sets the speed of the animation, from 1 the slowest to 255 the fastest;
 *   at 0 it stops where it is.
 *
 * `-F` and `-S` address the animation of `-A`, or the selected one where the command has no `-A`.
 * Where `-A` is refused, they address the refused id and are refused as well. An option that is
 * refused leaves its setting as it was. An option the command does not carry is left alone, so
 * `9` alone is a query.
 *
 * MsgParameterParser reads the options by the parameter table and hands each value to the private
 * `HandleParameter()`, which keeps it for Process(). MsgCmdParser makes the parser and calls
 * Parse(), Process() and SendAnswer() in this order, so an error answered for an option stands in
 * the line before the fields of the answer.
 *
 * The number holds while every overlay is built in; one whose `OVERLAYS_SUPPORT_*` switch is
 * STD_OFF moves the commands behind it down by one.
 */
class MsgCmdAnimationParser
    : public MsgParameterParser<MsgCmdAnimationParser,
                                MSG_CMD_ANIMATION_PARSER_PARAMETER_TABLE_SIZE> {
 public:
  /*! \brief An animation id, as `-A` takes it: Animations::AnimationIdType. */
  using AnimationIdType = Animations::AnimationIdType;
  /*! \brief A mode, as `-M` takes it: Animations::ModeType. */
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

  static constexpr ParameterTableType kParameterTable PROGMEM{
      ParameterTableElementType(kAnimationOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kModeOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kSpeedOptionShortName, MsgParameter::ArgumentTypeType::kUint8),
      ParameterTableElementType(kFavouriteOptionShortName, MsgParameter::ArgumentTypeType::kUint8)};

  static void HandleParameter(char parameter_short_name, char const* argument,
                              PositionType length) {
    UNUSED(parameter_short_name);
    UNUSED(argument);
    UNUSED(length);
  }
  void HandleParameter(char parameter_short_name, byte argument) {
    if (parameter_short_name == kAnimationOptionShortName) {
      animation_id_ = static_cast<Animations::AnimationIdType>(argument);
      animation_given_ = true;
    }
    if (parameter_short_name == kModeOptionShortName) {
      mode_ = static_cast<Animations::ModeType>(argument);
      mode_given_ = true;
    }
    if (parameter_short_name == kSpeedOptionShortName) {
      speed_ = argument;
      speed_given_ = true;
    }
    if (parameter_short_name == kFavouriteOptionShortName) {
      favourite_ = (argument != 0u);
      favourite_given_ = true;
    }
  }

  static void SendAnswerAnimation(bool append_space) {
    SendAnswerParameter(kAnimationOptionShortName,
                        ToUnderlying(Animations::GetInstance().GetAnimation()), append_space);
  }

  static void SendAnswerMode(bool append_space) {
    SendAnswerParameter(kModeOptionShortName, ToUnderlying(Animations::GetInstance().GetMode()),
                        append_space);
  }

  static void SendAnswerSpeed(bool append_space) {
    AnimationIdType animation = Animations::GetInstance().GetAnimation();
    byte task_cycle = Animations::GetInstance().GetTaskCycle(animation);
    SendAnswerParameter(kSpeedOptionShortName, Scheduler::ConvertTaskCycleToSpeed(task_cycle),
                        append_space);
  }

  void SetAnimation() const {
    StdReturnType return_value = Animations::GetInstance().SetAnimation(animation_id_);
    error_.CheckReturnValueAndSend(kAnimationOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }

  void SetMode() const {
    StdReturnType return_value = Animations::GetInstance().SetMode(mode_);
    error_.CheckReturnValueAndSend(kModeOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }

  static void SendAnswerFavourite(bool append_space) {
    /* the flag of the selected animation, not of the one a mode is running */
    AnimationIdType animation = Animations::GetInstance().GetAnimation();
    byte favourite = Animations::GetInstance().IsFavourite(animation) ? 1u : 0u;
    SendAnswerParameter(kFavouriteOptionShortName, favourite, append_space);
  }

  void SetFavourite() const {
    StdReturnType return_value = Animations::GetInstance().SetFavourite(animation_id_, favourite_);
    error_.CheckReturnValueAndSend(kFavouriteOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }

  void SetClockTaskCycle() const {
    StdReturnType return_value = Animations::GetInstance().SetTaskCycle(
        animation_id_, Scheduler::ConvertSpeedToTaskCycle(speed_));
    error_.CheckReturnValueAndSend(kSpeedOptionShortName, return_value,
                                   ErrorMessage::ErrorType::kValueOutOfBounds);
  }

 public:
  /*!
   * \brief Makes the parser for the options in `parameter`.
   *
   * It takes the selected animation as it is now, which `-F` and `-S` address where the command
   * has no `-A`.
   *
   * \param[in] parameter The parameter string, terminated by a null character; the parser keeps the
   *                      pointer and reads it in Parse().
   */
  explicit MsgCmdAnimationParser(char const* parameter)
      : MsgParameterParser(kParameterTable, parameter),
        animation_id_(),
        mode_(),
        speed_(0u),
        favourite_(true),
        animation_given_(false),
        mode_given_(false),
        speed_given_(false),
        favourite_given_(false) {
    // preloaded with the current settings, so a command may set single options
    // without resetting the others
    animation_id_ = Animations::GetInstance().GetAnimation();
    mode_ = Animations::GetInstance().GetMode();
    speed_ =
        Scheduler::ConvertTaskCycleToSpeed(Animations::GetInstance().GetTaskCycle(animation_id_));
    favourite_ = Animations::GetInstance().IsFavourite(animation_id_);
  }

  /*! \brief Destroys the parser. */
  ~MsgCmdAnimationParser() = default;

  /*!
   * \brief Sends the answer: the fields that follow the command number.
   *
   * It sends `A=<id> M=<mode> F=<0|1> S=<speed>`: the selected animation, the mode, and whether
   * the selected animation is a favourite and at which speed it runs. A space follows each field
   * but the last; MsgCmdParser ends the line.
   */
  static void SendAnswer() {
    SendAnswerAnimation(true);
    SendAnswerMode(true);
    SendAnswerFavourite(true);
    SendAnswerSpeed(false);
  }

  /*!
   * \brief Completes the command once Parse() has read its options.
   *
   * It applies the options the command carried: `-A` first, as `-F` and `-S` address the
   * animation it selects, then `-M`, `-F` and `-S`. Each one that Animations refuses is answered
   * with `Error=4:<option>`.
   */
  void Process() const {
    /* the animation first, since -F and -S address the one it selects */
    if (animation_given_) {
      SetAnimation();
    }
    if (mode_given_) {
      SetMode();
    }
    if (favourite_given_) {
      SetFavourite();
    }
    if (speed_given_) {
      SetClockTaskCycle();
    }
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDANIMATIONPARSER_H_
