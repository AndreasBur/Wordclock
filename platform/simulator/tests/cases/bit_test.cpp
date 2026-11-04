// Single bits and bit groups, through the static functions and through a held Bit<T>.

#include "check.h"
#include "cases.h"

#include "Bit.h"

#include <stdint.h>

/*
 * The masks have to work where a 64-bit one hid the problem: the top bit of a uint16_t, which
 * is where shifting a promoted int would turn negative on the AVR, and a bit above 16 in a
 * uint32_t, which an int-wide shift would lose there. Compile-time first, because a group
 * read sits in a constexpr function of the DS3231 driver.
 */
static_assert(Bit<uint16_t>::ToMask(15u) == 0x8000u, "the top bit of a uint16_t");
static_assert(Bit<uint32_t>::ToMask(20u) == 0x00100000uL, "a bit above 16 in a uint32_t");
static_assert(Bit<uint8_t>::ReadBitGroup(0b11000000u, 0b11u, 6u) == 0b11u, "a group given unshifted, at its position");

void TestSingleBits()
{
    // Toggling used to xor with the inverted mask, which flips every bit but the one asked
    // for. Nothing called it, which is the only reason it never showed.
    Expect(Bit<uint8_t>::ToggleBit(0b00000000u, 3u) == 0b00001000u, "toggling a clear bit sets that bit and no other");
    Expect(Bit<uint8_t>::ToggleBit(0b00001000u, 3u) == 0b00000000u, "and toggling it again clears it");

    Expect(Bit<uint16_t>::SetBit(0u, 15u) == 0x8000u, "the top bit of a uint16_t can be set");
    Expect(Bit<uint16_t>::ClearBit(0xFFFFu, 15u) == 0x7FFFu, "and cleared without touching the rest");
    Expect(Bit<uint32_t>::ReadBit(0x00100000uL, 20u), "a bit above 16 in a uint32_t reads as set");
    Expect(!Bit<uint32_t>::ReadBit(0x00100000uL, 4u), "and the one 16 below it as clear");

    Expect(Bit<uint8_t>::WriteBit(0b0101u, 1u, true) == 0b0111u, "writing true sets the bit");
    Expect(Bit<uint8_t>::WriteBit(0b0101u, 2u, false) == 0b0001u, "writing false clears it");
}

void TestBitGroups()
{
    /* in place, the way a register's _gm and _gc are written */
    Expect(Bit<uint8_t>::ReadBitGroup(0b10110110u, 0b00001110u) == 0b00000110u, "a group read in place keeps its position");
    Expect(Bit<uint8_t>::ReadBitGroup(0b10100000u, 0b111u, 5u) == 0b101u, "a group given unshifted comes back shifted down");
}

// What the AVR's registers are changed through. A volatile byte stands in for one: the
// functions take it by reference, which is the whole of what they add over the value forms.
void TestBitsChangedInPlace()
{
    volatile uint8_t target{0b10110110u};

    Bit<uint8_t>::WriteBitGroup(target, 0b00001110u, 0b00001000u);
    Expect(target == 0b10111000u, "a group written in place leaves the bits outside the mask alone");
    Bit<uint8_t>::WriteBitGroup(target, 0b00001110u, 0b11111111u);
    Expect(target == 0b10111110u, "and a value wider than the mask does not spill over it");

    Bit<uint8_t>::ClearBits(target, 0b10000010u);
    Expect(target == 0b00111100u, "clearing takes exactly the bits of the mask");
    Bit<uint8_t>::SetBits(target, 0b10000001u);
    Expect(target == 0b10111101u, "and setting adds them without touching the rest");
}

void TestHeldBits()
{
    Bit<uint16_t> bits;
    Expect(bits.GetValue() == 0u, "a default Bit starts clear");

    bits[0] = true;
    bits[15] = true;
    Expect(bits.GetValue() == 0x8001u, "writing through the index sets the bit it names");
    Expect(bits[15] && !bits[14], "and reading through it answers that bit alone");

    bits[0] = false;
    Expect(bits.GetValue() == 0x8000u, "writing false clears it again");

    const Bit<uint16_t> held{0x0005u};
    Expect(held[0] && !held[1] && held[2], "a const Bit is read through the index too");

    bits.SetValue(0x00FFu);
    Expect(bits.GetValue() == 0x00FFu, "and a whole value can be put back at once");
}
