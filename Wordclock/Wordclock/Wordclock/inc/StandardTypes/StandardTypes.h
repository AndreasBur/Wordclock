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
******************************************************************************************************************************************************/
#include <stdint.h>

/******************************************************************************************************************************************************
 *  GLOBAL CONSTANT MACROS
******************************************************************************************************************************************************/
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
******************************************************************************************************************************************************/
/* bit */
#define BIT_VALUE(Bit) \
    (((Bit) < 32) ? (((Bit) < 16) ? (UINT16_C(1) << (Bit)) : (UINT32_C(1) << (Bit))) : (UINT64_C(1) << (Bit)))

/* bit mask */
#define BIT_MASK(Len) \
    (BIT_VALUE(Len) - 1)

/* set bit */
#define SET_BIT(Var, Bit) \
    ((Var) |= (1UL << (Bit)))

/* clear bit */
#define CLEAR_BIT(Var, Bit) \
    ((Var) &= ~(1UL << (Bit)))

/* toggle bit */
#define TOGGLE_BIT(Var, Bit) \
    ((Var) ^= (1UL << (Bit)))
    
/* read bit */
#define READ_BIT(Var, Bit) \
    (((Var) & (1UL << (Bit))) >> (Bit))

/* write bit */
#define WRITE_BIT(Var, Bit, Value) \
    ((Var) = ((Var) & ~(1UL << (Bit))) | ((Value) << (Bit)))

/* is bit set */
#define IS_BIT_SET(Var, Bit) ((Var) & (1UL << (Bit)))

/* is bit cleared */
#define IS_BIT_CLEARED(Var, Bit) !IS_BIT_SET(Var, Bit)

/* read Bit Group */
#define READ_BIT_GROUP(Var, BitGroupMask, BitGroupPosition) \
    (((Var) & ((BitGroupMask) << (BitGroupPosition))) >> (BitGroupPosition))

/* write Bit Group */
#define WRITE_BIT_GROUP(Var, BitGroupMask, BitGroupPosition, Value) \
    ((Var) = (((Var) & ~((BitGroupMask) << (BitGroupPosition))) | (((Value) & (BitGroupMask)) << (BitGroupPosition))))

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
******************************************************************************************************************************************************/
/* standard return type for functions */
enum stdReturnType {
    E_OK = 0,
    E_NOT_OK = 1
};


/******************************************************************************************************************************************************
 *  GLOBAL INLINE FUNCTIONS
******************************************************************************************************************************************************/
/* bit */
template <typename ReturnValue, typename BitType>
inline ReturnValue bitValue(BitType Bit)
{
    return ((Bit < 32) ? ((Bit < 16) ? (UINT16_C(1) << Bit) : (UINT32_C(1) << Bit)) : (UINT64_C(1) << Bit));
}

/* bit mask */
template <typename ReturnValue, typename LengthType>
inline ReturnValue bitMask(LengthType Length)
{
    return (bitValue(Length) - 1);
}

/* read Bit Group */
template <typename VarType, typename MaskType>
inline VarType readBitGroup(VarType Var, MaskType BitGroupMask, MaskType BitGroupPosition)
{
    return ((Var & (static_cast<VarType>(BitGroupMask) << BitGroupPosition)) >> BitGroupPosition);
}

/* write Bit Group */
template <typename VarType, typename MaskType, typename ValType>
inline void writeBitGroup(VarType& Var, MaskType BitGroupMask, MaskType BitGroupPosition, ValType Value)
{
    Var = ((Var & ~(static_cast<VarType>(BitGroupMask) << BitGroupPosition)) | ((VarType)(Value & BitGroupMask) << BitGroupPosition));
}

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
