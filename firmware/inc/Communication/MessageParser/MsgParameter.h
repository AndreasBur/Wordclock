// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief One option of a command: its short name and the type of its argument.
 */

#ifndef _MSG_PARAMETER_H_
#define _MSG_PARAMETER_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* MsgParameter configuration parameter */

/* MsgParameter parameter */

class MsgParameter
{
  public:
    enum class ArgumentTypeType {
        kNone,
        kString,
        kChar,
        kUint8,
        kUint16,
        kUint32,
        kUint64,
        kInt8,
        kInt16,
        kInt32,
        kInt64,
        kFloat,
        kDouble,
    };

  private:
    char option_short_name_;
    ArgumentTypeType argument_type_;

  protected:

  public:
    constexpr MsgParameter(char option_short_name, ArgumentTypeType argument_type)
    : option_short_name_(option_short_name), argument_type_(argument_type) { }

    constexpr MsgParameter() : option_short_name_(' '), argument_type_(ArgumentTypeType::kNone) { }

    // get methods
    char GetOptionShortName() const { return option_short_name_; }
    ArgumentTypeType GetArgumentType() const { return argument_type_; }

    // set methods
    void SetOptionShortName(char option_short_name) { option_short_name_ = option_short_name; }
    void SetArgumentType(ArgumentTypeType argument_type) { argument_type_ = argument_type; }

    // methods
};

#endif
