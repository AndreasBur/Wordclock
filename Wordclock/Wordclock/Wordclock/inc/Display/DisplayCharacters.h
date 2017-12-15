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
/**     \file       DisplayCharacters.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DISPLAY_CHARACTERS_H_
#define _DISPLAY_CHARACTERS_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
/* DisplayCharacters configuration parameter */


/* DisplayCharacters parameter */



/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  CLASS  DisplayCharacters
******************************************************************************************************************************************************/
class DisplayCharacters
{
  public:
/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
    enum CharacterType {
        CHARACTER_E_1,
        CHARACTER_S_1,
        CHARACTER_K_1,
        CHARACTER_I_1,
        CHARACTER_S_2,
        CHARACTER_T_1,
        CHARACTER_L_1,
        CHARACTER_F_1,
        CHARACTER_UE_1,
        CHARACTER_N_1,
        CHARACTER_F_2,
        CHARACTER_Z_1,
        CHARACTER_E_2,
        CHARACTER_H_1,
        CHARACTER_N_2,
        CHARACTER_Z_2,
        CHARACTER_W_1,
        CHARACTER_A_1,
        CHARACTER_N_3,
        CHARACTER_Z_3,
        CHARACTER_I_2,
        CHARACTER_G_1,
        CHARACTER_D_1,
        CHARACTER_R_1,
        CHARACTER_E_3,
        CHARACTER_I_3,
        CHARACTER_V_1,
        CHARACTER_I_4,
        CHARACTER_E_4,
        CHARACTER_R_2,
        CHARACTER_T_2,
        CHARACTER_E_5,
        CHARACTER_L_2,
        CHARACTER_T_3,
        CHARACTER_G_2,
        CHARACTER_N_4,
        CHARACTER_A_2,
        CHARACTER_C_1,
        CHARACTER_H_2,
        CHARACTER_V_2,
        CHARACTER_O_1,
        CHARACTER_R_3,
        CHARACTER_J_1,
        CHARACTER_M_1,
        CHARACTER_H_3,
        CHARACTER_A_3,
        CHARACTER_L_3,
        CHARACTER_B_1,
        CHARACTER_Q_1,
        CHARACTER_Z_4,
        CHARACTER_W_2,
        CHARACTER_OE_1,
        CHARACTER_L_4,
        CHARACTER_F_3,
        CHARACTER_P_1,
        CHARACTER_Z_5,
        CHARACTER_W_3,
        CHARACTER_E_6,
        CHARACTER_I_5,
        CHARACTER_N_5,
        CHARACTER_S_3,
        CHARACTER_I_6,
        CHARACTER_E_7,
        CHARACTER_B_2,
        CHARACTER_E_8,
        CHARACTER_N_6,
        CHARACTER_K_2,
        CHARACTER_D_2,
        CHARACTER_R_4,
        CHARACTER_E_9,
        CHARACTER_I_7,
        CHARACTER_R_5,
        CHARACTER_H_4,
        CHARACTER_F_4,
        CHARACTER_UE_2,
        CHARACTER_N_7,
        CHARACTER_F_5,
        CHARACTER_E_10,
        CHARACTER_L_5,
        CHARACTER_F_6,
        CHARACTER_N_8,
        CHARACTER_E_11,
        CHARACTER_U_3,
        CHARACTER_N_9,
        CHARACTER_V_3,
        CHARACTER_I_8,
        CHARACTER_E_12,
        CHARACTER_R_6,
        CHARACTER_W_4,
        CHARACTER_A_4,
        CHARACTER_C_2,
        CHARACTER_H_5,
        CHARACTER_T_5,
        CHARACTER_Z_6,
        CHARACTER_E_13,
        CHARACTER_H_6,
        CHARACTER_N_10,
        CHARACTER_R_7,
        CHARACTER_S_4,
        CHARACTER_B_3,
        CHARACTER_S_5,
        CHARACTER_E_14,
        CHARACTER_C_3,
        CHARACTER_H_7,
        CHARACTER_S_6,
        CHARACTER_F_7,
        CHARACTER_M_2,
        CHARACTER_U_4,
        CHARACTER_H_8,
        CHARACTER_R_8,
        CHARACTER_NUMBER_OF_CHARACTERS
    };

  private:


  public:
    DisplayCharacters();
    ~DisplayCharacters();

	// get methods


	// set methods

	// methods
    void init();
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
