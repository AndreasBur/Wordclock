// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The error codes and API ids of an error answer, and how it is sent.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_ERRORMESSAGE_H_
#define WORDCLOCK_SRC_COMMUNICATION_ERRORMESSAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "MsgParameter.h"

/*!
 * \brief The error answer: writes `Error=` and an error code to Serial, alone, with the short name
 *        of the option it is about, or behind the id of the call that refused.
 *
 * Every function is static and writes straight to Serial; the object holds nothing. The answer
 * takes one of three forms, CODE being the number of the ErrorType, NAME the short name of the
 * option and ID the number of the ApiType:
 * - `Error=CODE`, from Send(ErrorType, bool) and Send(StdReturnType, bool);
 * - `Error=CODE:NAME`, from Send(char, ErrorType, bool), Send(MsgParameter, ErrorType, bool) and
 *   CheckReturnValueAndSend(char, StdReturnType, ErrorType, bool);
 * - `Api=ID:Error=CODE`, from CheckReturnValueAndSend(ApiType, StdReturnType, ErrorType, bool).
 *
 * None of them ends the line; the caller does, the command parser once the whole answer is out.
 */
class ErrorMessage {
 public:
  /*!
   * \brief The error codes, sent as the number behind `Error=`.
   *
   * kPowerSwitchAbsent is appended behind kUnknown rather than sorted in beside its
   * neighbours, for the reason the remote procedure ids are appended too: these numbers are
   * what an answer carries and what this repository's own documentation writes down, and
   * inserting one would silently change what every code behind it means.
   */
  enum class ErrorType {
    /*! \brief 0: success, which the remote procedure call and the network command answer. */
    kNoError,
    /*! \brief 1: a character arrived for a line that already fills the buffer. */
    kMessageTooLong,
    /*! \brief 2: the line does not start with the number of a command. */
    kWrongCommand,
    /*! \brief 3: the command has no option of that short name. */
    kParameterUnknown,
    /*! \brief 4: a value too large for its type, or one the setting refuses. */
    kValueOutOfBounds,
    /*! \brief 5: an option that takes a number, given without one. */
    kNoValueGiven,
    /*! \brief 6: the display or the clock did not show the change. */
    kDisplayPending,
    /*! \brief 7: a remote procedure call whose id names no procedure, as with no `-P`. */
    kRpcIdUnknown,
    /*! \brief 8: a remote procedure call or the network command was not carried out. */
    kUnknown,
    /*! \brief 9: procedure 20 or 21 on a clock built without the power switch. */
    kPowerSwitchAbsent,
  };

  /*! \brief The calls an `Api=ID:Error=CODE` answer names, by the number behind `Api=`. */
  enum class ApiType {
    kNone,                       /*!< 0: no call; no command sends it. */
    kDisplayShow,                /*!< 1: Display::Show(), sent by commands 2, 3 and 4. */
    kDisplayWritePixel,          /*!< 2: writing a pixel; no command sends it. */
    kAnimationShow,              /*!< 3: showing an animation; no command sends it. */
    kClockShow,                  /*!< 4: Clock::Show(), sent by the clock mode command. */
    kAnimationSetAnimation,      /*!< 5: selecting an animation; no command sends it. */
    kAnimationSetClockTaskCycle, /*!< 6: an animation's task cycle; no command sends it. */
  };

 private:
  static void SendSpace() { Serial.print(' '); }
  static void SendColon() { Serial.print(':'); }
  static void SendEqual() { Serial.print('='); }

  static void Send(ApiType api) {
    Serial.print(F("Api"));
    SendEqual();
    Serial.print(ToUnderlying(api));
  }

 public:
  /*! \brief Makes an error answer, which holds nothing. */
  constexpr ErrorMessage() {}
  /*! \brief Destroys the error answer. */
  ~ErrorMessage() = default;

  /*!
   * \brief Sends `Error=CODE` for `error`.
   *
   * \param[in] error The error.
   * \param[in] append_space True to send a space behind it, for a field that follows.
   */
  static void Send(ErrorType error, bool append_space = true) {
    Serial.print(F("Error"));
    SendEqual();
    Serial.print(ToUnderlying(error));
    if (append_space) {
      SendSpace();
    }
  }

  /*!
   * \brief Sends `Error=CODE:NAME` for `error` about the option `option_short_name`.
   *
   * \param[in] option_short_name The short name of the option.
   * \param[in] error The error.
   * \param[in] append_space True to send a space behind it, for a field that follows.
   */
  static void Send(char option_short_name, ErrorType error, bool append_space = true) {
    Send(error, false);
    SendColon();
    Serial.print(option_short_name);
    if (append_space) {
      SendSpace();
    }
  }

  /*!
   * \brief Sends `Error=CODE:NAME` for `error` about the option `parameter`.
   *
   * \param[in] parameter The option, of which only its short name is sent.
   * \param[in] error The error.
   * \param[in] append_space True to send a space behind it, for a field that follows.
   */
  static void Send(MsgParameter parameter, ErrorType error, bool append_space = true) {
    Send(parameter.GetOptionShortName(), error, append_space);
  }

  /*!
   * \brief Sends `Error=0` for kEOk and `Error=8`, ErrorType::kUnknown, for anything else.
   *
   * \param[in] return_value What the call to answer for returned.
   * \param[in] append_space True to send a space behind it, for a field that follows.
   */
  static void Send(StdReturnType return_value, bool append_space = true) {
    if (return_value == kEOk) {
      Send(ErrorType::kNoError, append_space);
    } else {
      Send(ErrorType::kUnknown, append_space);
    }
  }

  /*!
   * \brief Sends `Api=ID:Error=CODE` for `api` and `error` if `return_value` is kENotOk, and
   *        nothing otherwise.
   *
   * \param[in] api The call that returned `return_value`.
   * \param[in] return_value What the call returned.
   * \param[in] error The error to send for kENotOk.
   * \param[in] append_space True to send a second space behind it; see the note.
   *
   * \note A space always follows the code, and `append_space` true adds a second one.
   */
  static void CheckReturnValueAndSend(ApiType api, StdReturnType return_value, ErrorType error,
                                      bool append_space = true) {
    if (return_value == kENotOk) {
      Send(api);
      SendColon();
      Send(error);
      if (append_space) {
        SendSpace();
      }
    }
  }

  /*!
   * \brief Sends `Error=CODE:NAME` for `error` about the option `option_short_name` if
   *        `return_value` is kENotOk, and nothing otherwise.
   *
   * \param[in] option_short_name The short name of the option the call set.
   * \param[in] return_value What the call returned.
   * \param[in] error The error to send for kENotOk.
   * \param[in] append_space True to send a space behind it, for a field that follows.
   */
  static void CheckReturnValueAndSend(char option_short_name, StdReturnType return_value,
                                      ErrorType error, bool append_space = true) {
    if (return_value == kENotOk) {
      Send(option_short_name, error, append_space);
    }
  }
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_ERRORMESSAGE_H_
