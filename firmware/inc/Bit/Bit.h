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
 *      \brief      Single bits and groups of bits in an unsigned integer
 *
 *      \details    Two ways in, for two kinds of value. A value that is kept - a row of pixels,
 *                  the set of favourite animations - is held in a Bit<T> and indexed like an
 *                  array: bits[column] = true. A value that is only passed through - a register,
 *                  a byte read off the bus, a field of the stored settings whose layout is a
 *                  file format - goes through the static functions, Bit<uint8_t>::ReadBit(value, 3).
 *
 *                  The masks are as wide as T. They used to be built from UINT64_C(1), which on
 *                  the AVR made every variable shift a 64-bit one: a library call, for a byte.
 *
 *                  A group is given by its mask as it stands in the value - a register's _gm -
 *                  and is read back in place; ReadBitGroup with a position is for a mask written
 *                  down unshifted, as the sensor drivers' _GM and _GP pairs are.
 *
 *                  A register is changed in place, by SetBits, ClearBits and WriteBitGroup on a
 *                  volatile reference. Not through bits[position]: that would hide a read-modify-
 *                  write behind an assignment, see the note above those three.
 *
******************************************************************************************************************************************************/
#ifndef _BIT_H_
#define _BIT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <stdint.h>


/******************************************************************************************************************************************************
 *  C L A S S   B I T
******************************************************************************************************************************************************/
template <typename T>
class Bit
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* What T is shifted in: T itself when it is at least an unsigned int, unsigned int
       otherwise. Shifting the uint8_t or uint16_t directly would promote it to int, which on
       the AVR is 16 bits wide and signed. */
    using ShiftType = decltype(T{} | 0u);

    T value_;

    /* What bits[position] hands out for writing: a bool that writes through. */
    class Reference
    {
      public:
        constexpr Reference(T& value, uint8_t position) : value_(value), position_(position) {}

        operator bool() const { return ReadBit(value_, position_); }
        Reference& operator=(bool bit) { value_ = WriteBit(value_, position_, bit); return *this; }

      private:
        T& value_;
        uint8_t position_;
    };

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Bit() : value_{0u} {}
    constexpr explicit Bit(T value) : value_{value} {}

    // get methods
    constexpr T GetValue() const { return value_; }

    // set methods
    void SetValue(T value) { value_ = value; }

    // operators
    constexpr bool operator[](uint8_t position) const { return ReadBit(value_, position); }
    Reference operator[](uint8_t position) { return Reference(value_, position); }

    // single bits
    static constexpr T ToMask(uint8_t position) { return static_cast<T>(static_cast<ShiftType>(1u) << position); }
    static constexpr T SetBit(T value, uint8_t position) { return static_cast<T>(value | ToMask(position)); }
    static constexpr T ClearBit(T value, uint8_t position) { return static_cast<T>(value & static_cast<T>(~ToMask(position))); }
    static constexpr T ToggleBit(T value, uint8_t position) { return static_cast<T>(value ^ ToMask(position)); }
    static constexpr T WriteBit(T value, uint8_t position, bool bit) {
        return bit ? SetBit(value, position) : ClearBit(value, position);
    }
    static constexpr bool ReadBit(T value, uint8_t position) { return (value & ToMask(position)) != 0u; }

    // groups of bits
    static constexpr T ReadBitGroup(T value, T mask) { return static_cast<T>(value & mask); }
    static constexpr T ReadBitGroup(T value, T mask, uint8_t position) {
        return static_cast<T>((static_cast<ShiftType>(value) >> position) & mask);
    }

    // in place
    /* Each of these reads the target, changes it and writes it back - which is what a
       register's |= did, and named so that it says so. That rules out two kinds of AVR
       register, which share a type with all the others, so nothing but this note keeps them
       away: INTFLAGS, where writing back a 1 that was read clears a flag nobody asked to
       clear, and the write-only halves OUTSET, OUTCLR, DIRSET, CTRLESET and their like, which
       take the mask alone in a single write. Nor is any of it atomic: a target an interrupt
       writes as well needs the caller's ATOMIC_BLOCK, which a platform-independent header
       cannot provide. */
    static void SetBits(volatile T& target, T mask) { target = static_cast<T>(target | mask); }
    static void ClearBits(volatile T& target, T mask) { target = static_cast<T>(target & static_cast<T>(~mask)); }
    static void WriteBitGroup(volatile T& target, T mask, T group) {
        target = static_cast<T>((target & static_cast<T>(~mask)) | (group & mask));
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
