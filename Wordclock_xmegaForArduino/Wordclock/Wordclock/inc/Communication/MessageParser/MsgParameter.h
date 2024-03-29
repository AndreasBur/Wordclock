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
/**     \file       MsgOption.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _MSG_PARAMETER_H_
#define _MSG_PARAMETER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* FontChar configuration parameter */


/* FontChar parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   F O N T C H A R
******************************************************************************************************************************************************/
class MsgParameter
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum ArgumentTypeType {
        ARGUMENT_TYPE_NONE,
        ARGUMENT_TYPE_STRING,
        ARGUMENT_TYPE_CHAR,
        ARGUMENT_TYPE_UINT8,
        ARGUMENT_TYPE_UINT16,
        ARGUMENT_TYPE_UINT32,
        ARGUMENT_TYPE_UINT64,
        ARGUMENT_TYPE_INT8,
        ARGUMENT_TYPE_INT16,
        ARGUMENT_TYPE_INT32,
        ARGUMENT_TYPE_INT64,
        ARGUMENT_TYPE_FLOAT,
        ARGUMENT_TYPE_DOUBLE,
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    char OptionShortName;
    ArgumentTypeType ArgumentType;

/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr MsgParameter(char sOptionShortName, ArgumentTypeType sArgumentType)
    : OptionShortName(sOptionShortName), ArgumentType(sArgumentType) { }

    constexpr MsgParameter() : OptionShortName(' '), ArgumentType(ARGUMENT_TYPE_NONE) { }

    // get methods
    byte getOptionShortName() const { return OptionShortName; }
    ArgumentTypeType getArgumentType() const { return ArgumentType; }

    // set methods
    void setOptionShortName(byte sOptionShortName) { OptionShortName = sOptionShortName; }
    void setArgumentType(ArgumentTypeType sArgumentType) { ArgumentType = sArgumentType; }

    // methods
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
