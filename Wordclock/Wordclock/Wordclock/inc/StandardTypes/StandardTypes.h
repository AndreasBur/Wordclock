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
/**     \file       StandardTypes.h
 *      \brief      Main header file of standard types library
 *
 *      \details    Library with standard types
 *                  
 *
 *****************************************************************************************************************************************************/
#ifndef _STANDARD_TYPES_H_
#define _STANDARD_TYPES_H_

/******************************************************************************************************************************************************
 * INCLUDES
 *****************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
 *****************************************************************************************************************************************************/
 /* standard type for configuration */
#define STD_ON                  1u
#define STD_OFF                 0u

#define STD_NULL_CHARACTER      '\0'

/* assembler NOPs */
#define STD_NOP_1               "nop      \n\t"
#define STD_NOP_2               "rjmp .+0 \n\t"
#define STD_NOP_4               STD_NOP_2 STD_NOP_2
#define STD_NOP_8               STD_NOP_4 STD_NOP_4
#define STD_NOP_16              STD_NOP_8 STD_NOP_8


/******************************************************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
 *****************************************************************************************************************************************************/
/* set bit */
#define setBit(Var, Bit) \
    ((Var) |= (1 << (Bit)))

/* clear bit */
#define clearBit(Var, Bit) \
    ((Var) &= (unsigned)~(1 << (Bit)))

/* toggle bit */
#define toggleBit(Var, Bit) \
    ((Var) ^= (1 << (Bit)))
    
/* read bit */
#define readBit(Var, Bit) \
    (((Var) & (1 << (Bit))) >> Bit)

/* write bit */
#define writeBit(Var, Bit, Value) \
    ((Var) = ((Var & (unsigned)~(1 << Bit)) | (Value << Bit)))

/* is bit set */
#define isBitSet(Var, Bit) ((Var) & (1 << (Bit)))

/* is bit cleared */
#define isBitCleared(Var, Bit) !isBitSet(Var, Bit)

/* read Bit Group */
#define readBitGroup(Var, BitGroupMask, BitGroupPosition) \
((Var & (BitGroupMask << BitGroupPosition)) >> BitGroupPosition)

/* write Bit Group */
#define writeBitGroup(Var, BitGroupMask, BitGroupPosition, Value) \
(Var = ((Var & ~(BitGroupMask << BitGroupPosition)) | ((Value & BitGroupMask) << BitGroupPosition)))

/* binary to decimal */
#define B(x) ( \
  (0##x >>  0 & 0001) | \
  (0##x >>  2 & 0002) | \
  (0##x >>  4 & 0004) | \
  (0##x >>  6 & 0010) | \
  (0##x >>  8 & 0020) | \
  (0##x >> 10 & 0040) | \
  (0##x >> 12 & 0100) | \
  (0##x >> 14 & 0200) )


/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
 *****************************************************************************************************************************************************/
  /* standard return type for functions */
enum stdReturnType {
    E_OK = 0,
    E_NOT_OK = 1
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
 *****************************************************************************************************************************************************/