// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The serial port's task, which collects a line and hands it to the command parser.
 */

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "Arduino.h"
#include "Message.h"
#include "ErrorMessage.h"

/*!
 * \brief The scheduler ticks between two calls of Communication::Task(), so 10 * 10 ms = 100 ms.
 */
#define COMMUNICATION_TASK_CYCLE                         10u

/*!
 * \brief The serial port's task: collects a line from Serial and hands it to MsgCmdParser.
 *
 * A line ends with the end-of-message character, the newline, which is not part of it. Every
 * other character is added to the line, a carriage return included. The line is collected in a
 * Message, which holds MESSAGE_LENGTH - 1 characters.
 *
 * The characters come from Serial. On the ESP32 and the RP2350 that is WordclockSerial, which
 * also hands out the characters WebFrontend::InjectCommand() injected; in the simulator it is
 * SerialShim, which hands out what is typed into its window and what is injected.
 */
class Communication
{
  public:
    /*! \brief Whether the line in the buffer is complete. */
    enum class StateType {
        kMessageComplete,   /*!< The line has ended; the next Task() parses it. */
        kMessageIncomplete, /*!< The line is still being read. */
    };

  private:
    Communication() : error_(), incoming_message_(), state_(StateType::kMessageIncomplete), is_message_too_long_(false) { }
    ~Communication() { }

    static constexpr byte kTaskCycle{COMMUNICATION_TASK_CYCLE};
    static constexpr char kEndOfMessageChar{'\n'};
    ErrorMessage error_;
    Message incoming_message_;
    StateType state_;
    // A character of the line was dropped, so the line is not parsed when it ends.
    bool is_message_too_long_;

    void AddMessagePart();

  public:
    /*! \brief Returns the one communication of the firmware. */
    static Communication& GetInstance() {
        static Communication singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the character that ends a line, the newline. */
    static constexpr char GetEndOfMessageChar() { return kEndOfMessageChar; }

    /*! \brief Returns the scheduler ticks between two calls of Task(), COMMUNICATION_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*! \brief Returns whether the line in the buffer is complete. */
    StateType GetState() const { return state_; }

    /*!
     * \brief Reads the characters the port has, or parses the line once it is complete.
     *
     * A call does one of the two. While the line is incomplete, it reads every character
     * Serial has, up to and including the end-of-message character; the characters after that
     * stay in the port for the call after the parse. Once the line is complete, the next call
     * has MsgCmdParser parse and answer it, empties the buffer, and reads nothing.
     *
     * A line longer than MESSAGE_LENGTH - 1 characters is refused whole: at its end-of-message
     * character it is answered with `Error=1`, ErrorMessage::ErrorType::kMessageTooLong, on a line
     * of its own, and not parsed.
     *
     * An empty line is parsed as well, and answered with `Error=2`,
     * ErrorMessage::ErrorType::kWrongCommand.
     */
    void Task();

    /*! \brief Returns whether the line is complete and waits for the next Task() to parse it. */
    bool IsMessageComplete() const { return state_ == StateType::kMessageComplete; }
};

#endif
