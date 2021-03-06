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
/**     \file       GammaCorrection.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _GAMMA_CORRECTION_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "GammaCorrection.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const GammaCorrection::Gamma7TableElementType Gamma7Table[] PROGMEM 
{
    133, 139, 145, 151, 158, 165, 172, 180, 188, 196, 205, 214, 224, 234, 244, 255
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  calcGammaCorrectionValue()
******************************************************************************************************************************************************/
/*! \brief          
 *  \details        calculates the gamma correction value with the help of a small table
 *                  and very efficient shift calculation
 *  param[in]       ValueLinear    7 bit unsigned (0..127)
 *  \return         exponential value (approx. 1.0443^x) (1..255)
 *****************************************************************************************************************************************************/
byte GammaCorrection::calcGamma7CorrectionValue(byte ValueLinear)
{
    Gamma7TableElementType Exponent = getGamma7TableElement(ValueLinear % GAMMA_CORRECTION_GAMMA7_TABLE_NUMBER_OF_VALUES);
    return Exponent >> (7 - (ValueLinear / GAMMA_CORRECTION_GAMMA7_TABLE_NUMBER_OF_VALUES));
} /* calcGamma7CorrectionValue */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 