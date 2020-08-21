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
/**     \file       Message.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Message configuration parameter */
#define MESSAGE_LENGTH    20u

/* Message parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class Message
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
      static constexpr size_t npos = -1;
      static constexpr char NullCharacter{STD_NULL_CHARACTER};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    char Buffer[MESSAGE_LENGTH];

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Message() { clear(); }
    ~Message() { }

    // get methods
    const char* getMessage() const { return Buffer; }

    // set methods

    // methods
    void clear() { Buffer[0u] = NullCharacter; }

    size_t length() const
    {
        return strlen(Buffer);
    }

    size_t find(const char* String, size_t position = 0u) const
    {
        const char* finding = strstr(&Buffer[position], String);
        if(finding == nullptr) return npos;
        else return finding - Buffer;
    }

    size_t find(char Char, size_t position = 0u) const
    {
        const char* finding = strchr(&Buffer[position], Char);
        if(finding == nullptr) return npos;
        else return finding - Buffer;
    }

    StdReturnType addChar(char Char)
    {
        byte messageLength = strlen(Buffer);
        // check for free space in Message buffer
        if(messageLength < (MESSAGE_LENGTH - 1u))
        {
            // save new char
            Buffer[messageLength] = Char;
            Buffer[messageLength + 1] = NullCharacter;
            return E_OK;
            } else {
            return E_NOT_OK;
        }
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/