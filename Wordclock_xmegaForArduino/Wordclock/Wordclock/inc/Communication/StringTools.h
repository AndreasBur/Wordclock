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
	  
	 static const size_t npos = -1;
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
	static void stringCopy(char *Destination, const char *Source, int Length) {
		strncpy(Destination, Source, Length - 1);
		Destination[Length - 1] = '\0';
	}
	
	template<typename Unsigned, typename std::enable_if_t<std::is_unsigned<Unsigned>::value, int> = 0>
	static ResultType stringTo(const char* String, PositionType& Position, uint8_t Base, Unsigned& Value) {
		char* end = nullptr;
		errno = 0;
		uint64_t valueMax = strtoul(String, &end, Base);
		Position = end - String;
		
		if(String == end) { 
			return RESULT_NO_VALUE;
		} else if(errno == ERANGE || valueMax > std::numeric_limits<Unsigned>::max()) {
			Value = std::numeric_limits<Unsigned>::max();
			return RESULT_OVERFLOW;
		} else {
			Value = static_cast<Unsigned>(valueMax);	
			return RESULT_OK;
		}
	}
	
	template<typename Signed, typename std::enable_if_t<std::is_signed<Signed>::value, int> = 0> 
	static ResultType stringTo(const char* String, PositionType& Position, uint8_t Base, Signed& Value) {
		char* end = nullptr;
		errno = 0;
		int64_t valueMax = strtol(String, &end, Base);
		Position = end - String;
		
		if(String == end) {
			return RESULT_NO_VALUE;
		} else if(errno == ERANGE || valueMax > std::numeric_limits<Signed>::max()) {
			Value = std::numeric_limits<Signed>::max();
			return RESULT_OVERFLOW;
		} else {
			Value = static_cast<Signed>(valueMax);
			return RESULT_OK;
		}
	}
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
