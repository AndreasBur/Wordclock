// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The base of the command parsers, which reads the options by a parameter table.
 */

#ifndef _MSG_PARAMETER_PARSER_H_
#define _MSG_PARAMETER_PARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "ErrorMessage.h"
#include "MsgParameter.h"
#include "StringTools.h"

#include <type_traits>
#include "Communication.h"
#include <array>

/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kChar. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_CHAR                     STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kUint8. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8                    STD_ON
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kUint16. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT16                   STD_ON
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kUint32. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT32                   STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kUint64. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT64                   STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kInt8. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT8                     STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kInt16. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT16                    STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kInt32. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT32                    STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kInt64. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT64                    STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kFloat. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_FLOAT                    STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kDouble. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_DOUBLE                   STD_OFF
/*! \brief STD_ON reads the arguments of type MsgParameter::ArgumentTypeType::kString. */
#define MSG_PARAMETER_PARSER_ARGUMENT_TYPE_STRING                   STD_ON

/*!
 * \brief The base of the command parsers: reads the options of a command by its parameter table and
 *        hands each value to the derived parser.
 *
 * A command arrives as `9 -A3 -M1`, and MsgCmdParser hands the parser of command 9 the rest of the
 * message from its first space on, ` -A3 -M1`: the parameter string. Parse() reads it option by
 * option. An option is a `-`, its short name, one character, and its argument right behind the
 * short name. The entry of the short name in the parameter table says the type of the argument,
 * and with it how the argument is read:
 * - A number is read in base 10 by StringTools::StringTo(). It ends at the first character that
 *   is not part of it; what follows it up to the next `-` is skipped.
 * - A string runs up to the next `-` or the end of the parameter string, so it cannot hold a `-`.
 *   It keeps the spaces before and behind it, the one before the next option included.
 *
 * Whatever stands outside an option is skipped without an answer. Which argument types are read
 * at all is decided when the firmware is built, by the MSG_PARAMETER_PARSER_ARGUMENT_TYPE_
 * switches: as delivered, ArgumentTypeType::kUint8, kUint16 and kString. An option of another
 * type is skipped without an answer, as one of ArgumentTypeType::kNone is.
 *
 * An option that cannot be read is answered right away, in the middle of the answer line, with
 * the code of the error and the short name of the option, as in `Error=4:A`:
 * - `Error=3`, ErrorMessage::ErrorType::kParameterUnknown: the table has no entry for the short
 *   name.
 * - `Error=5`, ErrorMessage::ErrorType::kNoValueGiven: a number without a digit.
 * - `Error=4`, ErrorMessage::ErrorType::kValueOutOfBounds: a number outside the range of its type,
 *   such as `-A256` or `-A-1` for a `uint8_t`.
 *
 * Such an option is not handed on, so the setting it was to change is left as it was; the options
 * behind it are still read.
 *
 * Each command parser derives from this class with itself as `Derived`, makes this class its
 * friend, hands its table and the parameter string to the constructor, and provides two hooks,
 * which are not virtual. The overlay parsers share one `Derived`, MsgCmdOverlayParser, which
 * provides the hooks for them.
 * - `HandleParameter(char, T)` takes the short name and the value of each number that was read
 *   without error.
 * - `HandleParameter(char, const char*, LengthType)` takes the short name of each string, a
 *   pointer to its first character and its length; see Parse().
 *
 * The calls of both are compiled for every argument type that is switched on, whether the table
 * uses it or not, so every parser declares both, an empty one where it has no option of that kind.
 *
 * A parser also offers a public constructor that takes the parameter string, `Process()`, which
 * carries out what the options asked for, and `SendAnswer()`, which sends the fields of the
 * answer, mostly through SendAnswerParameter(). MsgCmdParser makes the parser and calls Parse(),
 * `Process()` and `SendAnswer()`, in this order.
 *
 * \tparam Derived The command parser that derives from this class.
 * \tparam ParameterTableSize The number of options in the parameter table.
 * \warning A number goes to the `HandleParameter()` that overload resolution picks. A parser with
 *          no overload for `uint16_t` gets a `uint16_t` value through its `byte` overload, cut to
 *          the low byte and without an error. A parser with an option of ArgumentTypeType::kUint16
 *          needs an overload of its own for it, as MsgCmdDateParser has.
 */
template <typename Derived, size_t ParameterTableSize> class MsgParameterParser
{
  public:
    /*! \brief An entry of the parameter table: one option, with the type of its argument. */
    using ParameterTableElementType = MsgParameter;
    /*! \brief The parameter table: one entry for each option of the command. */
    using ParameterTableType = std::array<ParameterTableElementType, ParameterTableSize>;
    /*! \brief A position in the parameter string, StringTools::PositionType. */
    using PositionType = StringTools::PositionType;
    /*! \brief The length handed on with a string argument, StringTools::LengthType. */
    using LengthType = StringTools::LengthType;

  protected:
    /*! \brief The character between the short name and the value of an answer field, `=`. */
    static constexpr char kOptionArgumentDelimiter{'='};
    /*! \brief Sends the `Error=` fields, of the options as Parse() reads them and of the parser. */
    ErrorMessage error_;

    /*!
     * \brief Makes a parser that reads `parameter` by `parameter_table`.
     *
     * The parser keeps a pointer to the parameter string and a reference to the table, not
     * copies; both have to outlive it. On the AVR it reads each entry of the table out of flash
     * with memcpy_P(), so the table has to be declared PROGMEM there.
     *
     * \param[in] parameter_table The parameter table of the command.
     * \param[in] parameter The parameter string, terminated by a null character.
     */
    // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
    constexpr MsgParameterParser(const ParameterTableType& parameter_table, const char* parameter)
    : error_(), parameter_buffer_(parameter), parameter_table_(parameter_table) {
        /*
         * Protected rather than private with the derived class as a friend, which is what
         * clang-tidy asks for: a friend would open this base's whole private side to it, where
         * protected opens only what is meant for it. What the private constructor would
         * prevent - somebody inheriting this template with another class's Derived - the
         * assertion catches instead, at the first point where Derived is a complete type.
         */
        static_assert(std::is_base_of<MsgParameterParser, Derived>::value,
                      "MsgParameterParser is a CRTP base: Derived has to be the class that inherits it");
    }

    /*! \brief Destroys the parser. */
    ~MsgParameterParser() = default;

  private:
    enum class StateType {
        kParse,
        kOptionStart,
        kOptionChar,
        kOptionArgument
    };

    static constexpr char kOptionStartChar{'-'};
    static constexpr byte kArgumentNumberBase{10u};

    const char* parameter_buffer_{nullptr};
    const ParameterTableType& parameter_table_;

    Derived& Underlying() { return static_cast<Derived&>(*this); }
    Derived const& Underlying() const { return static_cast<Derived const&>(*this); }

    static bool IsEndOfStringReached(char character) {
        return (character == STD_NULL_CHARACTER) ||
               (character == Communication::GetEndOfMessageChar() ||
               (character == kOptionStartChar));
    }

    ParameterTableElementType GetParameterTableElement(byte index) const {
        ParameterTableElementType parameter_table_element;
        memcpy_P(&parameter_table_element, &parameter_table_[index], sizeof(ParameterTableElementType));
        return parameter_table_element;
    }

    StdReturnType GetMsgParameterByOptionShortName(char option_short_name, ParameterTableElementType& parameter) const {
        for(byte index = 0u; index < parameter_table_.size(); index++) {
            ParameterTableElementType parameter_table_element = GetParameterTableElement(index);
            if(parameter_table_element.GetOptionShortName() == option_short_name) {
                parameter = parameter_table_element;
                return kEOk;
            }
        }
        return kENotOk;
    }

    /*
     * Reads the argument of `option_short_name` at `position` and hands it on, and says how many
     * characters it read. None for an unknown short name, so that the character behind it is read
     * again as anything else is: it used to be skipped, even when it began the next option.
     */
    PositionType ParseOption(char option_short_name, PositionType position) {
        ParameterTableElementType parameter;

        if(GetMsgParameterByOptionShortName(option_short_name, parameter) == kEOk) {
            return ParseArgument(parameter, &parameter_buffer_[position]);
        }
        error_.Send(option_short_name, ErrorMessage::ErrorType::kParameterUnknown);
        return 0u;
    }

    PositionType ParseArgument(MsgParameter parameter, const char* argument) {
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_CHAR == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kChar) { return ConvertArgument<char, MsgParameter::ArgumentTypeType::kChar>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT8 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kUint8) { return ConvertArgument<uint8_t, MsgParameter::ArgumentTypeType::kUint8>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT16 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kUint16) { return ConvertArgument<uint16_t, MsgParameter::ArgumentTypeType::kUint16>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT32 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kUint32) { return ConvertArgument<uint32_t, MsgParameter::ArgumentTypeType::kUint32>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_UINT64 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kUint64) { return ConvertArgument<uint64_t, MsgParameter::ArgumentTypeType::kUint64>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT8 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kInt8) { return ConvertArgument<int8_t, MsgParameter::ArgumentTypeType::kInt8>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT16 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kInt16) { return ConvertArgument<int16_t, MsgParameter::ArgumentTypeType::kInt16>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT32 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kInt32) { return ConvertArgument<int32_t, MsgParameter::ArgumentTypeType::kInt32>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_INT64 == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kInt64) { return ConvertArgument<int64_t, MsgParameter::ArgumentTypeType::kInt64>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_FLOAT == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kFloat) { return ConvertArgument<float, MsgParameter::ArgumentTypeType::kFloat>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_DOUBLE == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kDouble) { return ConvertArgument<double, MsgParameter::ArgumentTypeType::kDouble>(parameter, argument); }
# endif
# if (MSG_PARAMETER_PARSER_ARGUMENT_TYPE_STRING == STD_ON)
        if(parameter.GetArgumentType() == MsgParameter::ArgumentTypeType::kString) { return ConvertArgumentToString(parameter, argument); }
# endif
        return 0u;
    }

    template <typename T, MsgParameter::ArgumentTypeType ArgumentType>
    PositionType ConvertArgument(MsgParameter parameter, const char* argument) {
        T value;
        PositionType position;
        StringTools::ResultType result = StringTools::StringTo(argument, position, value, kArgumentNumberBase);
        HandleConvertResult(parameter, result);
        /*
         * Handed on only by a parser whose table has an option of this type. Every parser is
         * compiled for every type switched on above, and the call for a type its table never
         * names went to whatever HandleParameter() it has - a uint16_t into one taking a byte -
         * converted without a word. Unreachable, as no option of that type can arrive; but it was
         * also what would have cut -I300 to 44 the day the pixel index became a uint16_t without
         * a handler for one. Left out, the calls that remain are the ones a table asks for, and a
         * narrowing in one of them is a -Wconversion error rather than a quiet truncation.
         */
        if constexpr(IsArgumentTypeDeclared(ArgumentType)) {
            if(result == StringTools::ResultType::kOk) { Underlying().HandleParameter(parameter.GetOptionShortName(), value); }
        }
        return position;
    }

    static constexpr bool IsArgumentTypeDeclared(MsgParameter::ArgumentTypeType argument_type) {
        for(const ParameterTableElementType& element : Derived::kParameterTable) {
            if(element.GetArgumentType() == argument_type) { return true; }
        }
        return false;
    }

    PositionType ConvertArgumentToString(MsgParameter parameter, const char* argument) {
        PositionType position = 0u;
        while(!IsEndOfStringReached(argument[position])) { position++; }
        LengthType length = position + 1u;
        Underlying().HandleParameter(parameter.GetOptionShortName(), argument, length);
        return position;
    }

    void HandleConvertResult(MsgParameter parameter, StringTools::ResultType result) const {
        if(result == StringTools::ResultType::kNoValue) {
            error_.Send(parameter, ErrorMessage::ErrorType::kNoValueGiven);
        }
        if(result == StringTools::ResultType::kOverflow) {
            error_.Send(parameter, ErrorMessage::ErrorType::kValueOutOfBounds);
        }
    }

  public:

    /*! \brief Returns the parameter string the parser reads. */
    const char* GetParameter() const { return parameter_buffer_; }
    /*! \brief Returns the parameter table the parser reads the options by. */
    const ParameterTableType& GetParameterTable() const { return parameter_table_; }

    /*!
     * \brief Reads the options of the parameter string, answers those it cannot read, and hands the
     *        others to the derived parser.
     *
     * It reads the string from its start to its null character, as described at
     * MsgParameterParser. A number that was read is handed to `HandleParameter(char, T)`, with the
     * type of its table entry as `T`. A string is handed to
     * `HandleParameter(char, const char*, LengthType)` with a pointer to its first character in
     * the parameter string, where it is not terminated, and its number of characters plus one:
     * the size of a buffer that holds it with a null character, as StringTools::StringCopy() takes
     * it. An empty string, as `-T` has in `-T-A1`, is handed on with the length 1.
     *
     * After a number without a digit or an empty string, the reading goes on at the first
     * character of the argument: `-A -M1` answers `Error=5:A` and still reads `-M1`. A string is
     * handed on even with a leading or trailing space; it is for the derived parser to trim it.
     *
     * An option at the very end of the parameter string has an empty argument and is read as
     * any other: `9 -A` answers `Error=5:A`, and `13 -S -P` hands on two empty strings. An
     * unknown short name is answered with `Error=3`, and the reading goes on at the character
     * behind it: `9 -X-A3` answers `Error=3:X` and reads `-A3`.
     */
    void Parse()
    {
        StateType state = StateType::kParse;
        char option_short_name{STD_NULL_CHARACTER};
        PositionType position = 0u;

        while(true) {
            const char current_char = parameter_buffer_[position];

            /*
             * The argument starts here, at the end of the string as well, where it is empty. The
             * loop used to stop at the null character before it got here, and an option that
             * ended the line was dropped: 13 -S -P forgot no network and 16 -P cleared no
             * password.
             */
            if(state == StateType::kOptionArgument) {
                position += ParseOption(option_short_name, position);
                state = StateType::kParse;
                continue;
            }
            if(current_char == STD_NULL_CHARACTER) { break; }

            if(state == StateType::kParse) {
                if(current_char == kOptionStartChar) { state = StateType::kOptionChar; }
            } else {
                option_short_name = current_char;
                state = StateType::kOptionArgument;
            }
            position++;
        }
    }

    /*!
     * \brief Sends one field of the answer, such as `A=3`, with a space behind it.
     *
     * It writes the short name, `=` and `value` to `Serial`, and prints `value` as Serial.print()
     * prints its type.
     *
     * \tparam T The type of the value.
     * \param[in] option_short_name The short name of the option the field answers.
     * \param[in] value The value.
     * \param[in] append_space False to leave out the space, for the last field before the line
     *                         ends.
     */
    template <typename T> static void SendAnswerParameter(char option_short_name, T value, bool append_space = true)
    {
        Serial.print(option_short_name);
        Serial.print(kOptionArgumentDelimiter);
        Serial.print(value);
        if(append_space) { Serial.print(' '); }
    }
};

#endif
