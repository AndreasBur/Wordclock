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
******************************************************************************************************************************************************/
#ifndef _STANDARD_TYPES_H_
#define _STANDARD_TYPES_H_

/******************************************************************************************************************************************************
 * INCLUDES
******************************************************************************************************************************************************/
#include <stdint.h>
#include "Arduino.h"
#include "limits"

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
    ((UINT64_C(1) << (Bit))

/* bit mask */
#define BIT_MASK(Len) \
    (BIT_VALUE(Len) - 1)

/* set bit */
#define SET_BIT(Var, Bit) \
    ((Var) |= (UINT64_C(1) << (Bit)))

/* clear bit */
#define CLEAR_BIT(Var, Bit) \
    ((Var) &= ~(UINT64_C(1) << (Bit)))

/* toggle bit */
#define TOGGLE_BIT(Var, Bit) \
    ((Var) ^= (UINT64_C(1) << (Bit)))
    
/* read bit */
#define READ_BIT(Var, Bit) \
    (((Var) & (UINT64_C(1) << (Bit))) >> (Bit))

/* write bit */
#define WRITE_BIT(Var, Bit, Value) \
    ((Var) = ((Var) & ~(UINT64_C(1) << (Bit))) | ((Value) << (Bit)))

/* is bit set */
#define IS_BIT_SET(Var, Bit) (!!((Var) & (UINT64_C(1) << (Bit))))

/* is bit cleared */
#define IS_BIT_CLEARED(Var, Bit) !IS_BIT_SET(Var, Bit)

/* read bit group */
#define READ_BIT_GROUP(Var, BitGroupMask, BitGroupPosition) \
    (((Var) & ((BitGroupMask) << (BitGroupPosition))) >> (BitGroupPosition))

/* write bit group */
#define WRITE_BIT_GROUP(Var, BitGroupMask, BitGroupPosition, Value) \
    ((Var) = (((Var) & ~((BitGroupMask) << (BitGroupPosition))) | (((Value) & (BitGroupMask)) << (BitGroupPosition))))


/* binary to decimal */
/*
#define B(x) ( \
(0##x >>  0 & 0001) | \
(0##x >>  2 & 0002) | \
(0##x >>  4 & 0004) | \
(0##x >>  6 & 0010) | \
(0##x >>  8 & 0020) | \
(0##x >> 10 & 0040) | \
(0##x >> 12 & 0100) | \
(0##x >> 14 & 0200) )
*/

/******************************************************************************************************************************************************
 *  GLOBAL DATA TYPES AND STRUCTURES
******************************************************************************************************************************************************/
/* standard return type for functions */
enum StdReturnType {
    E_OK = 0,
    E_NOT_OK = 1
};


/******************************************************************************************************************************************************
 *  GLOBAL INLINE FUNCTION PROTOTYPES
******************************************************************************************************************************************************/
template <typename ReturnType, typename BitType> static inline ReturnType bitValue(BitType);
template <typename ReturnType, typename LengthType> static inline ReturnType bitMask(LengthType);
template <typename VarType> static inline VarType setBit(VarType, uint8_t);
template <typename VarType> static inline VarType clearBit(VarType, uint8_t);
template <typename VarType> static inline VarType toggleBit(VarType, uint8_t);
template <typename VarType> static inline boolean readBit(VarType, uint8_t);
template <typename VarType> static inline VarType writeBit(VarType, uint8_t, boolean);
template <typename VarType> static inline boolean isBitSet(VarType, uint8_t);
template <typename VarType> static inline boolean isBitCleared(VarType, uint8_t);
template <typename VarType, typename PositionType> static inline VarType shiftLeft(VarType, PositionType);
template <typename VarType, typename PositionType> static inline VarType shiftRight(VarType, PositionType);
template <typename VarType, typename MaskType, typename GroupType> static inline VarType readBitGroup(VarType, MaskType, GroupType, bool = true);
template <typename VarType, typename MaskType, typename GroupType, typename ValType> static inline void writeBitGroup(VarType&, MaskType, GroupType, ValType, bool = true);
template <typename VarType, typename MaskType> static inline VarType readBitGroup(VarType, MaskType);
template <typename VarType, typename MaskType, typename ValType> static inline void writeBitGroup(VarType&, MaskType, ValType);
template <typename T> uint8_t digitsOfNumber(T Number, unsigned = 10);

/******************************************************************************************************************************************************
 *  GLOBAL INLINE FUNCTIONS
******************************************************************************************************************************************************/
/* bit */
template <typename ReturnType, typename BitType>
static inline ReturnType bitValue(BitType Bit)
{
    return UINT64_C(1) << Bit;
}

/* bit mask */
template <typename ReturnType, typename LengthType>
static inline ReturnType bitMask(LengthType Length)
{
    return bitValue(Length) - 1;
}

/* set bit */
template <typename VarType>
static inline VarType setBit(VarType Var, uint8_t Bit)
{
    return Var & ~(UINT64_C(1) << Bit);
}

/* clear bit */
template <typename VarType>
static inline VarType clearBit(VarType Var, uint8_t Bit)
{
    return Var & ~(UINT64_C(1) << Bit);
}

/* toggle bit */
template <typename VarType>
static inline VarType toggleBit(VarType Var, uint8_t Bit)
{
    return Var ^ ~(UINT64_C(1) << Bit);
}

/* read bit */
template <typename VarType>
static inline boolean readBit(VarType Var, uint8_t Bit)
{
    return (Var & (UINT64_C(1) << Bit)) >> Bit;
}

/* write bit */
template <typename VarType>
static inline VarType writeBit(VarType Var, uint8_t Bit, boolean Value)
{
    return Var & ~(UINT64_C(1) << Bit) | (Value << Bit);
}

/* is bit set */
template <typename VarType>
static inline boolean isBitSet(VarType Var, uint8_t Bit)
{
    return Var & (UINT64_C(1) << Bit);
}

/* is bit cleared */
template <typename VarType>
static inline boolean isBitCleared(VarType Var, uint8_t Bit)
{
    return !isBitSet(Var, Bit);
}

/* shift left */
template <typename VarType, typename PositionType>
static inline VarType shiftLeft(VarType Var, PositionType Position)
{
    return Var << Position;
}

/* shift right */
template <typename VarType, typename PositionType>
static inline VarType shiftRight(VarType Var, PositionType Position)
{
    return Var >> Position;
}

/* read bit group */
template <typename VarType, typename MaskType, typename GroupType>
static inline VarType readBitGroup(VarType Var, MaskType BitGroupMask, GroupType BitGroupPosition, bool ShiftBitGroupMask)
{
    if(ShiftBitGroupMask) {
        return ((Var & (static_cast<VarType>(BitGroupMask) << BitGroupPosition)) >> BitGroupPosition);
    } else {
        return ((Var & static_cast<VarType>(BitGroupMask)) >> BitGroupPosition);
    }

}

/* write bit group */
template <typename VarType, typename MaskType, typename GroupType, typename ValType>
static inline void writeBitGroup(VarType& Var, MaskType BitGroupMask, GroupType BitGroupPosition, ValType Value, bool ShiftBitGroupMask)
{
    if(ShiftBitGroupMask) {
        Var = (Var & ~(static_cast<VarType>(BitGroupMask) << BitGroupPosition)) | (static_cast<VarType>(Value & BitGroupMask) << BitGroupPosition);
    } else {
        Var = (Var & ~(static_cast<VarType>(BitGroupMask))) | (static_cast<VarType>(Value & BitGroupMask) << BitGroupPosition);
    }
}

/* read bit group */
template <typename VarType, typename MaskType>
static inline VarType readBitGroup(VarType Var, MaskType BitGroupMask)
{
    return (Var & (static_cast<VarType>(BitGroupMask)));
}

/* write bit group */
template <typename VarType, typename MaskType, typename ValType>
static inline void writeBitGroup(VarType& Var, MaskType BitGroupMask, ValType Value)
{
    Var = (Var & ~(static_cast<VarType>(BitGroupMask))) | (static_cast<VarType>(Value & BitGroupMask));
}

template <typename T>
uint8_t digitsOfNumber(T Number, unsigned Base)
{
	T NumberOfDigits = 0;

	do { Number /= Base; NumberOfDigits++; }
	while (Number != 0);

	return NumberOfDigits;
}

#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
