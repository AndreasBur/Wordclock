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
#include "limits"
#include <stdint.h>
#include <limits.h>
#include <stddef.h>

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

/* Dummy statement to prevent compiler warnings unused parameter */
#define UNUSED(x) (void)(x)

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
    kEOk = 0u,
    kENotOk = 1u
};

/******************************************************************************************************************************************************
 *  GLOBAL INLINE FUNCTION PROTOTYPES
******************************************************************************************************************************************************/
template <typename ReturnType, typename BitType> static inline ReturnType BitValue(BitType);
template <typename ReturnType, typename LengthType> static inline ReturnType BitMask(LengthType);
template <typename VarType> static inline VarType SetBit(VarType, uint8_t);
template <typename VarType> static inline VarType ClearBit(VarType, uint8_t);
template <typename VarType> static inline VarType ToggleBit(VarType, uint8_t);
template <typename VarType> static inline bool ReadBit(VarType, uint8_t);
template <typename VarType> static inline VarType WriteBit(VarType, uint8_t, bool);
template <typename VarType> static inline bool IsBitSet(VarType, uint8_t);
template <typename VarType> static inline bool IsBitCleared(VarType, uint8_t);
template <typename VarType, typename PositionType> static inline VarType ShiftLeft(VarType, PositionType);
template <typename VarType, typename PositionType> static inline VarType ShiftRight(VarType, PositionType);
template <typename VarType, typename MaskType, typename GroupType> static inline VarType ReadBitGroup(VarType, MaskType, GroupType, bool = true);
template <typename VarType, typename MaskType, typename GroupType, typename ValType> static inline void WriteBitGroup(VarType&, MaskType, GroupType, ValType, bool = true);
template <typename VarType, typename MaskType> static inline VarType ReadBitGroup(VarType, MaskType);
template <typename VarType, typename MaskType, typename ValType> static inline void WriteBitGroup(VarType&, MaskType, ValType);
template <typename T> uint8_t DigitsOfNumber(T number, unsigned = 10);

/******************************************************************************************************************************************************
 *  GLOBAL INLINE FUNCTIONS
******************************************************************************************************************************************************/
/* bit */
template <typename ReturnType, typename BitType>
static inline ReturnType BitValue(BitType bit)
{
    return UINT64_C(1) << bit;
}

/* bit mask */
template <typename ReturnType, typename LengthType>
static inline ReturnType BitMask(LengthType length)
{
    return BitValue(length) - 1u;
}

/* set bit */
template <typename VarType>
static inline VarType SetBit(VarType var, uint8_t bit)
{
    return var | (UINT64_C(1) << bit);
}

/* clear bit */
template <typename VarType>
static inline VarType ClearBit(VarType var, uint8_t bit)
{
    return var & ~(UINT64_C(1) << bit);
}

/* toggle bit */
template <typename VarType>
static inline VarType ToggleBit(VarType var, uint8_t bit)
{
    return var ^ ~(UINT64_C(1) << bit);
}

/* read bit */
template <typename VarType>
static inline bool ReadBit(VarType var, uint8_t bit)
{
    return (var & (UINT64_C(1) << bit)) >> bit;
}

/* write bit */
template <typename VarType>
static inline VarType WriteBit(VarType var, uint8_t bit, bool value)
{
    return (var & ~(UINT64_C(1) << bit)) | (value << bit);
}

/* is bit set */
template <typename VarType>
static inline bool IsBitSet(VarType var, uint8_t bit)
{
    return var & (UINT64_C(1) << bit);
}

/* is bit cleared */
template <typename VarType>
static inline bool IsBitCleared(VarType var, uint8_t bit)
{
    return !IsBitSet(var, bit);
}

/* shift left */
template <typename VarType, typename PositionType>
static inline VarType ShiftLeft(VarType var, PositionType position)
{
    return var << position;
}

/* shift right */
template <typename VarType, typename PositionType>
static inline VarType ShiftRight(VarType var, PositionType position)
{
    return var >> position;
}

/* read bit group */
template <typename VarType, typename MaskType, typename GroupType>
static inline VarType ReadBitGroup(VarType var, MaskType bit_group_mask, GroupType bit_group_position, bool shift_bit_group_mask)
{
    if(shift_bit_group_mask) {
        return (var & (static_cast<VarType>(bit_group_mask) << bit_group_position)) >> bit_group_position;
    } else {
        return (var & static_cast<VarType>(bit_group_mask)) >> bit_group_position;
    }

}

/* write bit group */
template <typename VarType, typename MaskType, typename GroupType, typename ValType>
static inline void WriteBitGroup(VarType& var, MaskType bit_group_mask, GroupType bit_group_position, ValType value, bool shift_bit_group_mask)
{
    if(shift_bit_group_mask) {
        var = (var & ~(static_cast<VarType>(bit_group_mask) << bit_group_position)) | (static_cast<VarType>(value & bit_group_mask) << bit_group_position);
    } else {
        var = (var & ~(static_cast<VarType>(bit_group_mask))) | (static_cast<VarType>(value & bit_group_mask) << bit_group_position);
    }
}

/* read bit group */
template <typename VarType, typename MaskType>
static inline VarType ReadBitGroup(VarType var, MaskType bit_group_mask)
{
    return var & (static_cast<VarType>(bit_group_mask));
}

/* write bit group */
template <typename VarType, typename MaskType, typename ValType>
static inline void WriteBitGroup(VarType& var, MaskType bit_group_mask, ValType value)
{
    var = (var & ~(static_cast<VarType>(bit_group_mask))) | (static_cast<VarType>(value & bit_group_mask));
}

template <typename T>
size_t NumberOfBits()
{
    return sizeof (T) * (CHAR_BIT);
}

template <typename T>
uint8_t DigitsOfNumber(T number, unsigned base)
{
    T number_of_digits = 0u;

    do { number /= base; ++number_of_digits; }
    while (number != 0u);

    return number_of_digits;
}

#endif
/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
