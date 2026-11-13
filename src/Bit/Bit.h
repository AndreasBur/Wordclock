// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Single bits and groups of bits in an unsigned integer.
 *
 * Two ways in, for two kinds of value. A value that is kept - a row of pixels, the set of favourite
 * animations - is held in a Bit<T> and indexed like an array: bits[column] = true. A value that is
 * only passed through - a register, a byte read off the bus, a field of the stored settings whose
 * layout is a file format - goes through the static functions, Bit<uint8_t>::ReadBit(value, 3).
 *
 * The masks are as wide as T. They used to be built from UINT64_C(1), which on the AVR made every
 * variable shift a 64-bit one: a library call, for a byte.
 *
 * A group is given by its mask as it stands in the value - a register's _gm - and is read back in
 * place; ReadBitGroup with a position is for a mask written down unshifted, as the sensor drivers'
 * _GM and _GP pairs are.
 *
 * A register is changed in place, by SetBits, ClearBits and WriteBitGroup on a volatile reference.
 * Not through bits[position]: that would hide a read-modify-write behind an assignment, see the
 * warning at SetBits().
 */

#ifndef WORDCLOCK_SRC_BIT_BIT_H_
#define WORDCLOCK_SRC_BIT_BIT_H_

#include <stdint.h>

/*!
 * \brief An unsigned integer read and written bit by bit, kept as a value or passed through the
 *        static functions.
 *
 * A position counts from 0, the least significant bit, and lies below the width of `T`.
 *
 * \tparam T The unsigned integer type, such as `uint8_t` or `uint16_t`.
 */
template <typename T>
class Bit {
 private:
  /*
   * What T is shifted in: T itself when it is at least an unsigned int, unsigned int
   * otherwise. Shifting the uint8_t or uint16_t directly would promote it to int, which on
   * the AVR is 16 bits wide and signed.
   */
  using ShiftType = decltype(T{} | 0u);

  T value_;

  // What bits[position] hands out for writing: a bool that writes through.
  class Reference {
   public:
    constexpr Reference(T& value, uint8_t position) : value_(value), position_(position) {}

    operator bool() const { return ReadBit(value_, position_); }
    Reference& operator=(bool bit) {
      value_ = WriteBit(value_, position_, bit);
      return *this;
    }

   private:
    T& value_;
    uint8_t position_;
  };

 public:
  /*! \brief Makes a value with every bit cleared. */
  constexpr Bit() : value_{0u} {}

  /*!
   * \brief Makes a value with the bits of `value`.
   *
   * \param[in] value The bits to start with.
   */
  constexpr explicit Bit(T value) : value_{value} {}

  /*! \brief Returns all the bits as one value. */
  constexpr T GetValue() const { return value_; }

  /*!
   * \brief Sets all the bits at once.
   *
   * \param[in] value The new bits.
   */
  void SetValue(T value) { value_ = value; }

  /*!
   * \brief Returns whether the bit at `position` is set.
   *
   * \param[in] position The position of the bit.
   * \return True if the bit is set.
   */
  constexpr bool operator[](uint8_t position) const { return ReadBit(value_, position); }

  /*!
   * \brief Returns the bit at `position` for writing: assigning a bool to it sets or clears that
   *        bit.
   *
   * \param[in] position The position of the bit.
   * \return The bit, which reads as a bool and writes through to the value.
   */
  Reference operator[](uint8_t position) { return Reference(value_, position); }

  /*!
   * \brief Returns the mask of the bit at `position`.
   *
   * \param[in] position The position of the bit.
   * \return A value with that bit set and every other bit cleared.
   */
  static constexpr T ToMask(uint8_t position) {
    return static_cast<T>(static_cast<ShiftType>(1u) << position);
  }

  /*!
   * \brief Returns `value` with the bit at `position` set.
   *
   * \param[in] value The value to change.
   * \param[in] position The position of the bit.
   * \return The changed value.
   */
  static constexpr T SetBit(T value, uint8_t position) {
    return static_cast<T>(value | ToMask(position));
  }

  /*!
   * \brief Returns `value` with the bit at `position` cleared.
   *
   * \param[in] value The value to change.
   * \param[in] position The position of the bit.
   * \return The changed value.
   */
  static constexpr T ClearBit(T value, uint8_t position) {
    return static_cast<T>(value & static_cast<T>(~ToMask(position)));
  }

  /*!
   * \brief Returns `value` with the bit at `position` inverted.
   *
   * \param[in] value The value to change.
   * \param[in] position The position of the bit.
   * \return The changed value.
   */
  static constexpr T ToggleBit(T value, uint8_t position) {
    return static_cast<T>(value ^ ToMask(position));
  }

  /*!
   * \brief Returns `value` with the bit at `position` set to `bit`.
   *
   * \param[in] value The value to change.
   * \param[in] position The position of the bit.
   * \param[in] bit True to set the bit, false to clear it.
   * \return The changed value.
   */
  static constexpr T WriteBit(T value, uint8_t position, bool bit) {
    return bit ? SetBit(value, position) : ClearBit(value, position);
  }

  /*!
   * \brief Returns whether the bit at `position` of `value` is set.
   *
   * \param[in] value The value to read.
   * \param[in] position The position of the bit.
   * \return True if the bit is set.
   */
  static constexpr bool ReadBit(T value, uint8_t position) {
    return (value & ToMask(position)) != 0u;
  }

  /*!
   * \brief Returns the bits of `value` that `mask` selects, where they stand.
   *
   * \param[in] value The value to read.
   * \param[in] mask The group as it stands in the value, such as a register's `_gm`.
   * \return The bits of the group in place, every other bit cleared.
   */
  static constexpr T ReadBitGroup(T value, T mask) { return static_cast<T>(value & mask); }

  /*!
   * \brief Returns the group of bits of `value` that starts at `position`, shifted down to bit 0.
   *
   * \param[in] value The value to read.
   * \param[in] mask The group written down unshifted, such as a sensor driver's `_GM`.
   * \param[in] position The position of the group's lowest bit, such as a sensor driver's `_GP`.
   * \return The bits of the group from bit 0 on, every other bit cleared.
   */
  static constexpr T ReadBitGroup(T value, T mask, uint8_t position) {
    return static_cast<T>((static_cast<ShiftType>(value) >> position) & mask);
  }

  /*!
   * \brief Sets the bits of `mask` in `target`, the other bits left as they were.
   *
   * \param[in,out] target The register or variable to change.
   * \param[in] mask The bits to set.
   * \warning Each of SetBits(), ClearBits() and WriteBitGroup() reads the target, changes it and
   *          writes it back - which is what a register's |= did, and named so that it says so. That
   *          rules out two kinds of AVR register, which share a type with all the others, so
   *          nothing but this note keeps them away: INTFLAGS, where writing back a 1 that was read
   *          clears a flag nobody asked to clear, and the write-only halves OUTSET, OUTCLR, DIRSET,
   *          CTRLESET and their like, which take the mask alone in a single write. Nor is any of it
   *          atomic: a target an interrupt writes as well needs the caller's ATOMIC_BLOCK, which a
   *          platform-independent header cannot provide.
   */
  static void SetBits(T volatile& target, T mask) { target = static_cast<T>(target | mask); }

  /*!
   * \brief Clears the bits of `mask` in `target`, the other bits left as they were.
   *
   * \param[in,out] target The register or variable to change.
   * \param[in] mask The bits to clear.
   * \warning It reads the target, changes it and writes it back, and not atomically; see SetBits()
   *          for the registers that rules out.
   */
  static void ClearBits(T volatile& target, T mask) {
    target = static_cast<T>(target & static_cast<T>(~mask));
  }

  /*!
   * \brief Writes `group` into the bits of `target` that `mask` selects, the other bits left as
   *        they were.
   *
   * \param[in,out] target The register or variable to change.
   * \param[in] mask The group as it stands in the target, such as a register's `_gm`.
   * \param[in] group The new bits of the group, in place; bits outside `mask` are ignored.
   * \warning It reads the target, changes it and writes it back, and not atomically; see SetBits()
   *          for the registers that rules out.
   */
  static void WriteBitGroup(T volatile& target, T mask, T group) {
    target = static_cast<T>((target & static_cast<T>(~mask)) | (group & mask));
  }
};

#endif  // WORDCLOCK_SRC_BIT_BIT_H_
