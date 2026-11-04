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

/* Communication configuration parameter */
#define COMMUNICATION_TASK_CYCLE                         10u

/* Communication parameter */

class Communication
{
  public:
    enum class StateType {
        kMessageComplete,
        kMessageIncomplete,
    };

  private:
    Communication() : error_(), incoming_message_(), state_(StateType::kMessageIncomplete) { }
    ~Communication() { }

    static constexpr byte kTaskCycle{COMMUNICATION_TASK_CYCLE};
    static constexpr char kEndOfMessageChar{'\n'};
    ErrorMessage error_;
    Message incoming_message_;
    StateType state_;

    // private functions
    void AddMessagePart();

  public:
    static Communication& GetInstance() {
        static Communication singleton_instance;
        return singleton_instance;
    }

    // get methods
    static constexpr char GetEndOfMessageChar() { return kEndOfMessageChar; }
    static constexpr byte GetTaskCycle() { return kTaskCycle; }
    StateType GetState() const { return state_; }

    // set methods

    // methods
    void Task();
    bool IsMessageComplete() const { return state_ == StateType::kMessageComplete; }
};

#endif
