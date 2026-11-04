// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The conversions between strings and numbers that the command parsers use.
 */

#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "errno.h"
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
    ~StringTools() { }

    /*!
     * \brief Copies at most `length` - 1 characters of `source` into `destination`, and terminates
     *        it.
     *
     * The copy is done by strncpy(), so a shorter `source` is followed by terminators up to
     * `length` - 1. The terminator at `length` - 1 is written whatever `source` holds.
     *
     * \param[out] destination The buffer, with room for `length` characters.
     * \param[in] source The string to copy, terminated.
     * \param[in] length The size of `destination`, its terminator included.
     * \pre `length` is at least 1. Below that, the copy runs far past `destination`.
     */
    static void StringCopy(char* destination, const char* source, int length) {
        strncpy(destination, source, length - 1u);
        destination[length - 1u] = kNullCharacter;
    }

    /*!
     * \brief Writes `number` into `string` as decimal digits, behind a minus sign if it is
     *        negative, and terminates it.
     *
     * \param[in] number The number.
     * \param[out] string The buffer, with room for the digits, the sign and the terminator: up to
     *                    11 characters and the terminator.
     * \return A pointer to the terminator in `string`.
     * \warning `number` must not be `INT32_MIN`: it is made positive by negating it, which
     *          overflows an `int32_t` for that one value.
     */
    static char* Itoa(int32_t number, char* string)
    {
        int32_t i;
        const int32_t sign = number;   /* record sign */

        if (sign < 0) {
            number = -number;          /* make n positive */
        }
        i = 0;
        do {       /* generate digits in reverse order */
            string[i++] = number % 10 + '0';   /* get next digit */
        } while ((number /= 10) > 0);     /* delete it */
        if (sign < 0)
            string[i++] = '-';
        string[i] = '\0';
        StringReverse(string);
        return &string[i];
    }

    /*!
     * \brief Reverses the characters of `string` in place, up to its terminator.
     *
     * \param[in,out] string The string, terminated.
     * \warning `string` must not be empty: one less than its length then wraps to the largest
     *          `unsigned int`, and the characters swapped lie far past the string.
     */
    static void StringReverse(char* string) {
        unsigned int i, j;
        char c;

        for (i = 0, j = strlen(string)-1; i<j; i++, j--) {
            c = string[i];
            string[i] = string[j];
            string[j] = c;
        }
    }

    /*!
     * \brief Converts the number at the start of `string` into `value`, in the base `base`.
     *
     * The number is read by strtoul() for an unsigned type of `value` and by strtol() for a
     * signed one. Both skip leading white space, take a plus or a minus sign, and stop at the
     * first character that is not a digit of `base`.
     *
     * An unsigned `value` takes a number up to the largest its type holds. A larger one comes
     * back as ResultType::kOverflow, with `value` set to that largest. A minus sign negates the
     * number as an `unsigned long`, so `-1` is out of range for a `uint8_t` or a `uint16_t`.
     *
     * A signed `value` comes back as ResultType::kOverflow only for a number beyond the range of
     * `long`. `value` is then set to the limit of its type on that side if the type is narrower
     * than `long`, and left as it was otherwise.
     *
     * \param[in] string The string, with the number at its start.
     * \param[out] position The number of characters read, white space and sign included, or 0 if
     *                      there is no number.
     * \param[out] value The number, of an unsigned or a signed integer type.
     * \param[in] base The base, as strtoul() and strtol() take it.
     * \return ResultType::kOk if the number was stored in `value`, ResultType::kOverflow as above,
     *         or ResultType::kNoValue if `string` does not start with a number; `value` is then
     *         left as it was.
     * \warning For a signed `value`, a number within the range of `long` but beyond that of the
     *          type is cast to the type and comes back as ResultType::kOk: 200 into an `int8_t` is
     *          -56.
     */
    template<typename Unsigned,
             std::enable_if_t<std::is_unsigned<Unsigned>::value>* = nullptr,
             std::enable_if_t<std::is_integral<Unsigned>::value>* = nullptr>
    static ResultType StringTo(const char* string, PositionType& position, Unsigned& value, uint8_t base = 10u) {
        char* end = nullptr;
        errno = 0;
        uint64_t value_big = strtoul(string, &end, base);
        position = end - string;

        if(string == end) {
            return ResultType::kNoValue;
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
        int64_t value_big = strtol(string, &end, base);
        position = end - string;

        if(string == end) {
            return ResultType::kNoValue;
        } else if(errno == ERANGE) {
            if(value_big > std::numeric_limits<Signed>::max()) { value = std::numeric_limits<Signed>::max(); }
            if(value_big < std::numeric_limits<Signed>::min()) { value = std::numeric_limits<Signed>::min(); }
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
     * If strtod() reports the number out of range, ResultType::kOverflow comes back. `value` is
     * then set to the largest value of its type if the number is above it, and otherwise to
     * `std::numeric_limits<Float>::min()`, the smallest positive normalised value: for a negative
     * number out of range and for one too close to zero.
     *
     * \param[in] string The string, with the number at its start.
     * \param[out] position The number of characters read, white space and sign included, or 0 if
     *                      there is no number.
     * \param[out] value The number, of a floating-point type.
     * \return ResultType::kOk if the number was stored in `value`, ResultType::kOverflow as above,
     *         or ResultType::kNoValue if `string` does not start with a number; `value` is then
     *         left as it was.
     * \warning A negative number out of range, such as `-1e999`, sets `value` to the smallest
     *          positive value rather than the most negative one. And where `double` is wider than
     *          the type of `value`, a number beyond the range of that type but within that of
     *          `double` is cast to the type and comes back as ResultType::kOk.
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
        } else if(errno == ERANGE) {
            if(value_big > std::numeric_limits<Float>::max()) { value = std::numeric_limits<Float>::max(); }
            if(value_big < std::numeric_limits<Float>::min()) { value = std::numeric_limits<Float>::min(); }
            return ResultType::kOverflow;
        } else {
            value = static_cast<Float>(value_big);
            return ResultType::kOk;
        }
    }

};

#endif
