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
    
    class BitReference
    {
        public:
            constexpr BitReference(T& sValue, size_t sBitPos) : Value(sValue), BitPos(sBitPos) {}

            operator bool() const { return readBit(Value, BitPos); }
            void operator = (bool Bit) { writeBit(Value, BitPos); }

        private:
            T& Value;
            size_t BitPos;
    };
  
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
    Bit& setValue(T sValue) { Value = sValue; return *this; }
    Bit& setBit(size_t BitPos) { Value = setBit(Value, BitPos); return *this; }
    Bit& clearBit(size_t BitPos) { Value = clearBit(Value, BitPos); return *this; }
    Bit& toggleBit(size_t BitPos) { Value = toggleBit(Value, BitPos); return *this; }
    Bit& writeBit(size_t BitPos, bool BitValue) { Value = writeBit(Value, BitValue, BitPos); return *this; }
    Bit& shiftRight(size_t BitPos) { Value = shiftRight(Value, BitPos); return *this; }
    Bit& shiftLeft(size_t BitPos) { Value = shiftLeft(Value, BitPos); return *this; }
    Bit& setBitMask(T BitMask) { Value = setBitMask(Value, BitMask); return *this; }
    Bit& clearBitMask(T BitMask) { Value = clearBitMask(Value, BitMask); return *this; }
    Bit& writeBits(T BitMask, T BitsValue) { Value = writeBits(Value, BitMask, BitsValue); return *this; }

	// methods
    size_t numberOfDigits(unsigned Base) { return numberOfDigits(Value, Base); }
    bool readBit(size_t BitPos) const { return readBit(Value, BitPos); }
    T readBitsValue(T BitMask, size_t BitPos) const { return readBitsValue(Value, BitPos); }
    T readBits(T BitMask) const { return readBits(Value, BitMask); }

    bool isBitSet(size_t BitPos) const { return isBitSet(Value, BitPos); }
    bool isBitCleared(size_t BitPos) const { return !isBitSet(Value, BitPos); }
    bool areBitsSet(T BitMask) const { return areBitsSet(Value, BitMask); }
    bool areBitsCleared(T BitMask) const { return readBits(~BitMask); }
		
	// operators
	// read [] operator
    bool operator [] (size_t BitPos) const { return readBit(BitPos); }
    // write [] operator
    BitReference operator [] (size_t BitPos) { return BitReference(Value, BitPos); }

    // static set methods
    template <typename ValueType> static ValueType setBit(ValueType sValue, size_t BitPos) { return sValue | (UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType clearBit(ValueType sValue, size_t BitPos) { return sValue & ~(UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType toggleBit(ValueType sValue, size_t BitPos) { return sValue ^ ~(UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType writeBit(ValueType sValue, bool BitValue, size_t BitPos) { return clearBit(sValue, BitPos) | (static_cast<ValueType>(BitValue) << BitPos); }
    template <typename ValueType> static ValueType shiftRight(ValueType sValue, size_t BitPos) { return sValue >> BitPos; }
    template <typename ValueType> static ValueType shiftLeft(ValueType sValue, size_t BitPos) { return sValue << BitPos; }
    template <typename ValueType> static ValueType setBitMask(ValueType sValue, T BitMask) { return sValue | BitMask; }
    template <typename ValueType> static ValueType clearBitMask(ValueType sValue, T BitMask) { return sValue & ~BitMask; }
    template <typename ValueType> static ValueType writeBits(ValueType sValue, T BitMask, T BitsValue) { return setBitMask(clearBitMask(sValue, BitMask), BitsValue); }

    // static get methods
    template <typename ValueType> static ValueType readBit(ValueType sValue, size_t BitPos) { return readBits(sValue, bitValue(BitPos)) >> BitPos; }
    template <typename ValueType> static ValueType readBitsValue(ValueType sValue, T BitMask, size_t BitPos) { return readBits(sValue, BitMask) >> BitPos; }
    template <typename ValueType> static ValueType readBits(ValueType sValue, T BitMask) { return sValue & BitMask; }

    template <typename ValueType> static ValueType isBitSet(ValueType sValue, size_t BitPos) { return sValue & (UINT64_C(1) << BitPos); }
    template <typename ValueType> static ValueType isBitCleared(ValueType sValue, size_t BitPos) { return !isBitSet(sValue, BitPos); }        
    template <typename ValueType> static ValueType areBitsSet(ValueType sValue, T BitMask) { return readBits(sValue, BitMask) == BitMask; }
    template <typename ValueType> static ValueType areBitsCleared(ValueType sValue, T BitMask) { return readBits(sValue, ~BitMask); }

    // static methods
    static uint64_t bitValue(size_t BitPos) { return UINT64_C(1) << BitPos; }
    static uint64_t bitMask(size_t Length) { return bitValue(Length) - 1u; }
    template <typename Type> size_t numberOfBits() { return sizeof(Type) * CHAR_BIT; }

    template <typename NumberType> static size_t numberOfDigits(NumberType Number, unsigned Base)
    {
        size_t numberOfDigits = 0u;
        do { Number /= Base; numberOfDigits++; }
        while (Number != 0u);
        return numberOfDigits;
    }
	
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/