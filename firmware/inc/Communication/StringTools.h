/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       StringTools.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _STRING_TOOLS_H_
#define _STRING_TOOLS_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "errno.h"
#include <type_traits>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* StringTools configuration parameter */

/* StringTools parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   S T R I N G   T O O L S
******************************************************************************************************************************************************/
class StringTools
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum class ResultType {
        kOverflow,
        kNoValue,
        kOk
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

     static constexpr size_t npos = -1;
     static constexpr char kNullCharacter{'\0'};
     using PositionType = uint8_t;
     using LengthType = uint8_t;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr StringTools() { }
    ~StringTools() { }

    // get methods

    // set methods

    // methods
    static void StringCopy(char* destination, const char* source, int length) {
        strncpy(destination, source, length - 1u);
        destination[length - 1u] = kNullCharacter;
    }

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

    static void StringReverse(char* string) {
        unsigned int i, j;
        char c;

        for (i = 0, j = strlen(string)-1; i<j; i++, j--) {
            c = string[i];
            string[i] = string[j];
            string[j] = c;
        }
    }

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

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
