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
  private:
	Display* pDisplay;

  public:
	Transformation(Display*);
	~Transformation();

	// get methods


	// set methods

	stdReturnType shiftLeft(boolean = 0);
	stdReturnType shiftRight(boolean);
	stdReturnType shiftUp(boolean);
	stdReturnType shiftDown(boolean);

	// methods
	void shiftLeftFast(boolean = 0);
	void shiftRightFast(boolean);
	void shiftUpFast(boolean);
	void shiftDownFast(boolean);
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
