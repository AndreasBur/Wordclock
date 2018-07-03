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
 *  CLASS  Transformation
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
    Display* pDisplay;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    Transformation(Display*);
    ~Transformation();

    // get methods


    // set methods
    void setDisplay(Display* Display) { pDisplay = Display; }

    // methods
    stdReturnType shiftLeft(boolean = false);
    stdReturnType shiftRight(boolean = false);
    stdReturnType shiftUp(boolean = false);
    stdReturnType shiftDown(boolean = false);

    stdReturnType shiftRowLeft(byte, boolean = false);
    stdReturnType shiftRowRight(byte, boolean = false);
    stdReturnType shiftColumnUp(byte, boolean = false);
    stdReturnType shiftColumnDown(byte, boolean = false);

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
