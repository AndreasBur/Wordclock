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
/**     \file       GammaCorrection.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _GAMMA_CORRECTION_H_
#define _GAMMA_CORRECTION_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* GammaCorrection configuration parameter */


/* GammaCorrection parameter */
#define GAMMA_CORRECTION_GAMMA7_TABLE_NUMBER_OF_VALUES        16u


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class GammaCorrection
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using Gamma7TableElementType = byte;

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    static constexpr Gamma7TableElementType Gamma7Table[] PROGMEM
    {
        133u, 139u, 145u, 151u, 158u, 165u, 172u, 180u, 188u, 196u, 205u, 214u, 224u, 234u, 244u, 255u
    };

    // functions
    Gamma7TableElementType getGamma7TableElement(byte Index) const {
        Gamma7TableElementType Gamma7TableElement;
        memcpy_P(&Gamma7TableElement, &Gamma7Table[Index], sizeof(Gamma7TableElementType));
        return Gamma7TableElement;
    }

    /**************************************************************************************************************************************************
      calcGammaCorrectionValue()
    **************************************************************************************************************************************************/
    /*! \brief
     *  \details        calculates the gamma correction value with the help of a small table
     *                  and very efficient shift calculation
     *  param[in]       ValueLinear    7 bit unsigned (0..127)
     *  \return         exponential value (approx. 1.0443^x) (1..255)
     *************************************************************************************************************************************************/
    byte calcGamma7CorrectionValue(byte ValueLinear) const
    {
        Gamma7TableElementType exponent{getGamma7TableElement(ValueLinear % GAMMA_CORRECTION_GAMMA7_TABLE_NUMBER_OF_VALUES)};
        byte Log2OfResolution = numberOfBits<byte>();
        return exponent >> ((Log2OfResolution - 1u) - (ValueLinear / GAMMA_CORRECTION_GAMMA7_TABLE_NUMBER_OF_VALUES));
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr GammaCorrection() { }


    // get methods
    byte getCorrectedValue(byte ValueLinear) const { return calcGamma7CorrectionValue(ValueLinear / 2u); }

    // set methods

    // methods

};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
