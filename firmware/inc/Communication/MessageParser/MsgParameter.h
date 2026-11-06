// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief One option of a command: its short name and the type of its argument.
 */

#ifndef _MSG_PARAMETER_H_
#define _MSG_PARAMETER_H_


/*!
 * \brief One option of a command: its short name, the character behind the `-`, and the type of its
 *        argument.
 *
 * A command parser's parameter table holds one for each option, and MsgParameterParser reads the
 * options by it. ErrorMessage takes one to name the option in an error, `Error=4:A`. The
 * constructors are constexpr, so the tables are built when the firmware is compiled.
 */
class MsgParameter
{
  public:
    /*!
     * \brief The type of an option's argument, which decides how MsgParameterParser reads it.
     *
     * The parser reads only the types that its switches build in, such as
     * MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8; it skips an option of another type without an
     * answer, as it skips one of kNone.
     */
    enum class ArgumentTypeType {
        kNone, /*!< No argument: the parser skips the option without an answer. */
        kString, /*!< A string, up to the next `-` or the end of the parameter string. */
        kChar, /*!< A `char`, read as a number in base 10, as the integer types are. */
        kUint8, /*!< An unsigned integer, `uint8_t`. */
        kUint16, /*!< An unsigned integer, `uint16_t`. */
        kUint32, /*!< An unsigned integer, `uint32_t`. */
        kUint64, /*!< An unsigned integer, `uint64_t`. */
        kInt8, /*!< A signed integer, `int8_t`. */
        kInt16, /*!< A signed integer, `int16_t`. */
        kInt32, /*!< A signed integer, `int32_t`. */
        kInt64, /*!< A signed integer, `int64_t`. */
        kFloat, /*!< A floating-point number, `float`. */
        kDouble, /*!< A floating-point number, `double`. */
    };

  private:
    char option_short_name_;
    ArgumentTypeType argument_type_;

  protected:

  public:
    /*!
     * \brief Makes the option `option_short_name` with an argument of type `argument_type`.
     *
     * \param[in] option_short_name The short name, the character behind the `-`.
     * \param[in] argument_type The type of the argument.
     */
    constexpr MsgParameter(char option_short_name, ArgumentTypeType argument_type)
    : option_short_name_(option_short_name), argument_type_(argument_type) { }

    /*! \brief Makes an option with a space as its short name and no argument, kNone. */
    constexpr MsgParameter() : option_short_name_(' '), argument_type_(ArgumentTypeType::kNone) { }

    /*! \brief Returns the short name, the character behind the `-`. */
    char GetOptionShortName() const { return option_short_name_; }
    /*! \brief Returns the type of the argument. */
    ArgumentTypeType GetArgumentType() const { return argument_type_; }

    /*!
     * \brief Sets the short name.
     *
     * \param[in] option_short_name The short name, the character behind the `-`.
     */
    void SetOptionShortName(char option_short_name) { option_short_name_ = option_short_name; }

    /*!
     * \brief Sets the type of the argument.
     *
     * \param[in] argument_type The type of the argument.
     */
    void SetArgumentType(ArgumentTypeType argument_type) { argument_type_ = argument_type; }
};

#endif
