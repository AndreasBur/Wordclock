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
/**     \file       Transformation.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* Transformation configuration parameter */


/* Transformation parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T R A N S F O R M A T I O N
******************************************************************************************************************************************************/
class Transformation
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Transformation() {
        
    }
    ~Transformation() {
        
    }

    // get methods


    // set methods

    // methods
    StdReturnType shiftLeft(boolean = false);
    StdReturnType shiftRight(boolean = false);
    StdReturnType shiftUp(boolean = false);
    StdReturnType shiftDown(boolean = false);

    StdReturnType shiftRowLeft(byte, boolean = false);
    StdReturnType shiftRowRight(byte, boolean = false);
    StdReturnType shiftColumnUp(byte, boolean = false);
    StdReturnType shiftColumnDown(byte, boolean = false);

    void shiftLeftFast(boolean = false);
    void shiftRightFast(boolean = false);
    void shiftUpFast(boolean = false);
    void shiftDownFast(boolean = false);

    void shiftRowLeftFast(byte, boolean = false);
    void shiftRowRightFast(byte, boolean = false);
    void shiftColumnUpFast(byte, boolean = false);
    void shiftColumnDownFast(byte, boolean = false);
};


#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
