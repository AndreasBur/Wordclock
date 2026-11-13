// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Where the settings survive a restart, in the on-chip EEPROM, see Storage.h.
 */

#include "Storage.h"

#include <avr/eeprom.h>
#include <avr/io.h>

#include "Arduino.h"

namespace {
/* The length byte, and the blob behind it. */
constexpr uint16_t kLengthAddress{STORAGE_EEPROM_ADDRESS};
constexpr uint16_t kDataAddress{STORAGE_EEPROM_ADDRESS + 1u};

// The part's EEPROM has to hold both, or a write would run off its end and silently do
// nothing.
static_assert((STORAGE_EEPROM_ADDRESS + 1u + STORAGE_CAPACITY) <= EEPROM_SIZE,
              "Storage: the capacity does not fit in this device's EEPROM");

// avr-libc's EEPROM functions take the offset into the EEPROM in a pointer's clothes; it
// never points at RAM and is never dereferenced as such, so the cast is the interface.
// NOLINTNEXTLINE(performance-no-int-to-ptr)
byte* ToEepromAddress(uint16_t offset) { return reinterpret_cast<byte*>(offset); }

}  // namespace

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
StdReturnType Storage::Read(byte* data, size_t size) const {
  if (data == nullptr) {
    return kENotOk;
  }
  if ((size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  byte const stored_length = eeprom_read_byte(ToEepromAddress(kLengthAddress));

  if (stored_length != static_cast<byte>(size)) {
    return kENotOk;
  }

  eeprom_read_block(data, ToEepromAddress(kDataAddress), size);

  return kEOk;
}

StdReturnType Storage::Write(byte const* data, size_t size) {
  if (data == nullptr) {
    return kENotOk;
  }
  if ((size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  // update rather than write: it leaves bytes that already match untouched, and the
  // settings are saved on every change.
  eeprom_update_block(data, ToEepromAddress(kDataAddress), size);
  eeprom_update_byte(ToEepromAddress(kLengthAddress), static_cast<byte>(size));

  return kEOk;
}

StdReturnType Storage::Clear() {
  eeprom_update_byte(ToEepromAddress(kLengthAddress), kLengthEmpty);

  return kEOk;
}
// NOLINTEND(readability-convert-member-functions-to-static)
