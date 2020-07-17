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
/* StringManipulation configuration parameter */

/* StringManipulation parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class StringTools
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum ResultType {
        RESULT_OVERFLOW,
        RESULT_NO_VALUE,
        RESULT_OK
    };
      /*
        struct ResultType {
            PositionType Position;
            ConvertResultType ConvertResult;
        }
        
        enum ConvertResultType {
            CONVERT_RESULT_OVERFLOW,
            CONVERT_RESULT_NO_VALUE,
            CONVERT_RESULT_OK
        };
      */
      
     static constexpr size_t npos = -1;
     using PositionType = uint8_t;
      
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
        
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    StringTools() {}
    ~StringTools() {}

    // get methods

    // set methods

    // methods
    static void stringCopy(char* Destination, const char* Source, int Length) {
        strncpy(Destination, Source, Length - 1u);
        Destination[Length - 1u] = '\0';
    }
    
    template<typename T,
             typename std::enable_if_t<std::is_unsigned<T>::value, int> = 0,
             typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
    static ResultType stringTo(const char* String, PositionType& Position, T& Value, uint8_t Base = 10u) {
        char* end = nullptr;
        errno = 0;
        uint64_t valueBig = strtoul(String, &end, Base);
        Position = end - String;
        
        if(String == end) { 
            return RESULT_NO_VALUE;
        } else if(errno == ERANGE || valueBig > std::numeric_limits<T>::max()) {
            Value = std::numeric_limits<T>::max();
            return RESULT_OVERFLOW;
        } else {
            Value = static_cast<T>(valueBig);   
            return RESULT_OK;
        }
    }

    template<typename Signed, 
             typename std::enable_if_t<std::is_signed<Signed>::value, int> = 0,
             typename std::enable_if_t<std::is_integral<Signed>::value, int> = 0> 
    static ResultType stringTo(const char* String, PositionType& Position, Signed& Value, uint8_t Base = 10u) {
        char* end = nullptr;
        errno = 0;
        int64_t valueBig = strtol(String, &end, Base);
        Position = end - String;
        
        if(String == end) {
            return RESULT_NO_VALUE;
        } else if(errno == ERANGE) {
            if(valueBig > std::numeric_limits<Signed>::max()) { Value = std::numeric_limits<Signed>::max(); }
            if(valueBig < std::numeric_limits<Signed>::min()) { Value = std::numeric_limits<Signed>::min(); }
            return RESULT_OVERFLOW;
        } else {
            Value = static_cast<Signed>(valueBig);
            return RESULT_OK;
        }
    }

    template<typename Float,
             typename std::enable_if_t<std::is_floating_point<Float>::value, int> = 0>
    static ResultType stringTo(const char* String, PositionType& Position, Float& Value, uint8_t Base = 10u) {
        char* end = nullptr;
        errno = 0;
        double valueBig = strtod(String, &end);
        Position = end - String;
        
        if(String == end) {
            return RESULT_NO_VALUE;
        } else if(errno == ERANGE) {
            if(valueBig > std::numeric_limits<Float>::max()) { Value = std::numeric_limits<Float>::max(); }
            if(valueBig < std::numeric_limits<Float>::min()) { Value = std::numeric_limits<Float>::min(); }
            return RESULT_OVERFLOW;
        } else {
            Value = static_cast<Float>(valueBig);
            return RESULT_OK;
        }
    }

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
