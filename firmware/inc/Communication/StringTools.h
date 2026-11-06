// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The conversions between strings and numbers that the command parsers use.
 */

#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

#include "StandardTypes.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

/*!
 * \brief The conversions between strings and numbers that the command parsers use, and a copy of a
 *        string into a buffer of a given size.
 *
 * Every function is static; the object holds nothing.
 */
class StringTools
{
  public:
    /*! \brief What a conversion by StringTo() found. */
    enum class ResultType {
        kOverflow, /*!< A number out of range; what the value is then depends on its type. */
        kNoValue,  /*!< No number at the start of the string; the value is left as it was. */
        kOk        /*!< A number, stored in the value. */
    };
      /*
       * struct ResultType {
       *    PositionType Position;
       *    ConvertResultType ConvertResult;
       * }
       *
       * enum ConvertResultType {
       *    CONVERT_RESULT_OVERFLOW,
       *    CONVERT_RESULT_NO_VALUE,
       *    CONVERT_RESULT_OK
       * };
       */

     static constexpr size_t npos = -1; /*!< The largest `size_t`, for no position. */
     static constexpr char kNullCharacter{'\0'}; /*!< The character that ends a string. */
     using PositionType = uint8_t; /*!< A position in a string, counted from 0. */
     using LengthType = uint8_t; /*!< A length of a string, in characters. */

  private:

  public:
    /*! \brief Makes the string tools, which hold nothing. */
    constexpr StringTools() { }
    /*! \brief Destroys the string tools. */
    ~StringTools() = default;

    /*!
     * \brief Copies at most `length` - 1 characters of `source` into `destination`, and terminates
     *        it.
     *
     * The copy is done by strncpy(), so a shorter `source` is followed by terminators up to
     * `length` - 1. The terminator at `length` - 1 is written whatever `source` holds.
     *
     * \param[out] destination The buffer, with room for `length` characters.
     * \param[in] source The string to copy, terminated.
     * \param[in] length The size of `destination`, its terminator included. Below 1 there is no
     *                   room even for the terminator, and `destination` is left as it was.
     */
    static void StringCopy(char* destination, const char* source, int length) {
        if(length < 1) { return; }
        const size_t size{static_cast<size_t>(length)};
        strncpy(destination, source, size - 1u);
        destination[size - 1u] = kNullCharacter;
    }

    /*!
     * \brief Writes `number` into `string` as decimal digits, behind a minus sign if it is
     *        negative, and terminates it.
     *
     * \param[in] number The number.
     * \param[out] string The buffer, with room for the digits, the sign and the terminator: up to
     *                    11 characters and the terminator.
     * \return A pointer to the terminator in `string`.
     */
    static char* Itoa(int32_t number, char* string)
    {
        // The magnitude in an unsigned type, where negating INT32_MIN is defined; in an int32_t
        // it overflows.
        uint32_t magnitude{static_cast<uint32_t>(number)};
        if(number < 0) { magnitude = 0u - magnitude; }

        size_t length{0u};
        do {
            string[length++] = static_cast<char>('0' + (magnitude % 10u));
            magnitude /= 10u;
        } while(magnitude > 0u);
        if(number < 0) { string[length++] = '-'; }
        string[length] = kNullCharacter;

        // The digits came out last first.
        StringReverse(string);
        return &string[length];
    }

    /*!
     * \brief Reverses the characters of `string` in place, up to its terminator.
     *
     * \param[in,out] string The string, terminated; an empty one is left as it is.
     */
    static void StringReverse(char* string) {
        // `end` is one past the character swapped, so that an empty string needs no 0 - 1.
        for(size_t begin = 0u, end = strlen(string); begin + 1u < end; begin++, end--) {
            const char character{string[begin]};
            string[begin] = string[end - 1u];
            string[end - 1u] = character;
        }
    }

    /*!
     * \brief Converts the number at the start of `string` into `value`, in the base `base`.
     *
     * The number is read by strtoul() for an unsigned type of `value` and by strtol() for a
     * signed one. Both skip leading white space, take a plus or a minus sign, and stop at the
     * first character that is not a digit of `base`.
     *
     * A number beyond the range of the type of `value` comes back as ResultType::kOverflow, with
     * `value` set to the limit of the type on that side: 200 into an `int8_t` is 127, and a
     * negative number into an unsigned type is 0. Only `-0` is taken by an unsigned type. The
     * number is read as a `long` or an `unsigned long`, so for a type wider than that, as a
     * 64-bit type on the AVR, a number beyond the `long` is out of range too.
     *
     * \param[in] string The string, with the number at its start.
     * \param[out] position The number of characters read, white space and sign included, or 0 if
     *                      there is no number.
     * \param[out] value The number, of an unsigned or a signed integer type.
     * \param[in] base The base, as strtoul() and strtol() take it.
     * \return ResultType::kOk if the number was stored in `value`, ResultType::kOverflow as above,
     *         or ResultType::kNoValue if `string` does not start with a number; `value` is then
     *         left as it was.
     */
    template<typename Unsigned,
             std::enable_if_t<std::is_unsigned<Unsigned>::value>* = nullptr,
             std::enable_if_t<std::is_integral<Unsigned>::value>* = nullptr>
    static ResultType StringTo(const char* string, PositionType& position, Unsigned& value, uint8_t base = 10u) {
        char* end = nullptr;
        errno = 0;
        const unsigned long value_big{strtoul(string, &end, base)};
        position = end - string;

        if(string == end) {
            return ResultType::kNoValue;
        } else if((value_big != 0u) && (memchr(string, '-', static_cast<size_t>(end - string)) != nullptr)) {
            // strtoul() takes a minus sign and negates the number as an unsigned long, which
            // turns -1 into a number that fits a uint32_t on the AVR and none on the host. A minus
            // among the characters read can only be that sign: no digit of any base is one.
            value = 0u;
            return ResultType::kOverflow;
        } else if(errno == ERANGE || value_big > std::numeric_limits<Unsigned>::max()) {
            value = std::numeric_limits<Unsigned>::max();
            return ResultType::kOverflow;
        } else {
            value = static_cast<Unsigned>(value_big);
            return ResultType::kOk;
        }
    }

    /*! \copydoc StringTools::StringTo(const char*, PositionType&, Unsigned&, uint8_t) */
    template<typename Signed,
             std::enable_if_t<std::is_signed<Signed>::value>* = nullptr,
             std::enable_if_t<std::is_integral<Signed>::value>* = nullptr>
    static ResultType StringTo(const char* string, PositionType& position, Signed& value, uint8_t base = 10u) {
        char* end = nullptr;
        errno = 0;
        const long value_big{strtol(string, &end, base)};
        position = end - string;

        if(string == end) {
            return ResultType::kNoValue;
        } else if(errno == ERANGE || value_big > std::numeric_limits<Signed>::max() ||
                  value_big < std::numeric_limits<Signed>::min()) {
            // On ERANGE strtol() returns LONG_MAX or LONG_MIN, so the sign still tells the side.
            value = (value_big > 0) ? std::numeric_limits<Signed>::max() : std::numeric_limits<Signed>::min();
            return ResultType::kOverflow;
        } else {
            value = static_cast<Signed>(value_big);
            return ResultType::kOk;
        }
    }

    /*!
     * \brief Converts the number at the start of `string` into the floating-point `value`.
     *
     * The number is read by strtod(), which skips leading white space and takes a sign. The
     * fourth parameter is not used: it is there so that a call with a base compiles for every
     * type of `value`.
     *
     * A number beyond the range of the type of `value` comes back as ResultType::kOverflow, with
     * `value` set to the largest or the most negative finite value of the type, by its sign;
     * `inf` counts as such a number. One too close to zero for strtod() comes back as
     * ResultType::kOverflow as well, with `value` set to 0.
     *
     * \param[in] string The string, with the number at its start.
     * \param[out] position The number of characters read, white space and sign included, or 0 if
     *                      there is no number.
     * \param[out] value The number, of a floating-point type.
     * \return ResultType::kOk if the number was stored in `value`, ResultType::kOverflow as above,
     *         or ResultType::kNoValue if `string` does not start with a number; `value` is then
     *         left as it was.
     */
    template<typename Float,
             std::enable_if_t<std::is_floating_point<Float>::value>* = nullptr>
    static ResultType StringTo(const char* string, PositionType& position, Float& value, uint8_t = 10u) {
        char* end = nullptr;
        errno = 0;
        double value_big = strtod(string, &end);
        position = end - string;

        if(string == end) {
            return ResultType::kNoValue;
        } else if(value_big > static_cast<double>(std::numeric_limits<Float>::max())) {
            value = std::numeric_limits<Float>::max();
            return ResultType::kOverflow;
        } else if(value_big < static_cast<double>(std::numeric_limits<Float>::lowest())) {
            value = std::numeric_limits<Float>::lowest();
            return ResultType::kOverflow;
        } else if(errno == ERANGE) {
            // Within the range, so what strtod() reports is an underflow.
            value = 0;
            return ResultType::kOverflow;
        } else {
            value = static_cast<Float>(value_big);
            return ResultType::kOk;
        }
    }

};

#endif
