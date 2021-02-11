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
/**     \file       Bit.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _BIT_H_
#define _BIT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Bit configuration parameter */


/* Bit parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   B I T
******************************************************************************************************************************************************/
template <typename T>
class Bit
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I N O N S
******************************************************************************************************************************************************/
  private:
    T Value;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Bit(T sValue) { Value = sValue; }
    constexpr Bit() { Value = 0u; }
    ~Bit() {}

	// get methods
    T getValue() const { return Value; }

	// set methods
    Bit& setValue(T sValue) { Value = sValue; }
    Bit& setBit(uint8_t BitPos) { Value = setBit(Value, BitPos); return *this; }
    Bit& clearBit(uint8_t BitPos) { Value = clearBit(Value, BitPos); return *this; }
    Bit& toggleBit(uint8_t BitPos) { Value = toggleBit(Value, BitPos); return *this; }
    Bit& writeBit(uint8_t BitPos, bool sValue) { Value = writeBit(Value, BitPos); return *this; }
    Bit& shiftRight(uint8_t BitPos) { Value = shiftRight(Value, BitPos); return *this; }
    Bit& shiftLeft(uint8_t BitPos) { Value = shiftLeft(Value, BitPos); return *this; }
    Bit& setBitMask(T BitMask) { Value = setBitMask(Value, BitMask); return *this; }
    Bit& clearBitMask(T BitMask) { Value = clearBitMask(Value, BitMask); return *this; }
    Bit& writeBits(T BitMask, T BitsValue) { Value = writeBits(Value, BitMask, BitsValue); return *this; }

	// methods  
    bool readBit(uint8_t BitPos) { return readBit(Value, BitPos); }
    T readBitsValue(T BitMask, uint8_t BitPos) const { return readBitsValue(Value, BitPos); }
    T readBits(T BitMask) const { return readBits(Value, BitMask); }

    bool isBitSet(uint8_t BitPos) const { return isBitSet(Value, BitPos); }
    bool isBitCleared(uint8_t BitPos) const { return !isBitSet(Value, BitPos); }
    bool areBitsSet(T BitMask) const { return areBitsSet(Value, BitMask); }
    bool areBitsCleared(T BitMask) const { return readBits(~BitMask); }

    // static set methods
    template <typename ValueType> static ValueType setBit(ValueType sValue, uint8_t BitPos) { return sValue | (UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType clearBit(ValueType sValue, uint8_t BitPos) { return sValue & ~(UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType toggleBit(ValueType sValue, uint8_t BitPos) { return sValue ^ ~(UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType writeBit(ValueType sValue, uint8_t BitPos) { return clearBit(sValue, BitPos) | (sValue << BitPos); }
    template <typename ValueType> static ValueType shiftRight(ValueType sValue, uint8_t BitPos) { return sValue >> BitPos; }
    template <typename ValueType> static ValueType shiftLeft(ValueType sValue, uint8_t BitPos) { return sValue << BitPos; }
    template <typename ValueType> static ValueType setBitMask(ValueType sValue, T BitMask) { return sValue | BitMask; }
    template <typename ValueType> static ValueType clearBitMask(ValueType sValue, T BitMask) { return sValue & ~BitMask; }
    template <typename ValueType> static ValueType writeBits(ValueType sValue, T BitMask, T BitsValue) { return setBitMask(clearBitMask(sValue, BitMask), BitsValue); }

    // static get methods
    template <typename ValueType> static ValueType readBit(ValueType sValue, uint8_t BitPos) { return readBits(sValue, bitValue(BitPos)) >> BitPos; }
    template <typename ValueType> static ValueType readBitsValue(ValueType sValue, T BitMask, uint8_t BitPos) { return readBits(sValue, BitMask) >> BitPos; }
    template <typename ValueType> static ValueType readBits(ValueType sValue, T BitMask) { return sValue & BitMask; }

    template <typename ValueType> static ValueType isBitSet(ValueType sValue, uint8_t BitPos) { return sValue & (UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType isBitCleared(ValueType sValue, uint8_t BitPos) { return !isBitSet(sValue, BitPos); }        
    template <typename ValueType> static ValueType areBitsSet(ValueType sValue, T BitMask) { return readBits(sValue, BitMask) == BitMask; }
    template <typename ValueType> static ValueType areBitsCleared(ValueType sValue, T BitMask) { return readBits(sValue, ~BitMask); }

    // static methods
    static uint64_t bitValue(uint8_t BitPos) { return UINT64_C(1) << BitPos; }
    static uint64_t bitMask(uint8_t Length) { return bitValue(Length) - 1u; }
    template <typename Type> size_t numberOfBits() { return sizeof(Type) * (CHAR_BIT); }

    template <typename NumberType>
    static uint8_t digitsOfNumber(NumberType Number, unsigned Base)
    {
        T numberOfDigits = 0u;

        do { Number /= Base; numberOfDigits++; }
        while (Number != 0u);

        return numberOfDigits;
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
