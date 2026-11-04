// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Where the settings survive a restart, in the on-chip EEPROM.
 *
 * Holds one blob and its length, and hands it back only to a reader asking for exactly that length.
 * What is inside it belongs to the core, which carries its own magic and checksum - so a foreign or
 * stale blob is rejected there rather than here, and the length is all this level needs to tell one
 * layout from another.
 *
 * The EEPROM is written rather than the flash, and only where the bytes differ: the settings are
 * saved whenever they change, and a part that wears out after a hundred thousand writes should not
 * be spent on bytes that already hold the right value.
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* Storage configuration parameter */
// What one Write() may hold. Well above what the core writes today, so its settings can
// grow without the platforms having to change together.
#define STORAGE_CAPACITY                                128u

/* Where the blob starts in the EEPROM. Behind the length byte that precedes it. */
#define STORAGE_EEPROM_ADDRESS                          0u

class Storage
{
  public:
    static constexpr size_t kCapacity{STORAGE_CAPACITY};

  private:
    // A length no blob can have, so an EEPROM that was never written - which reads 0xFF
    // throughout on a fresh part - says so by itself and needs no separate magic.
    static constexpr byte kLengthEmpty{0xFFu};

    Storage() { }
    ~Storage() { }

  public:
    static Storage& GetInstance() {
        static Storage singleton_instance;
        return singleton_instance;
    }

    // methods
    StdReturnType Read(byte*, size_t) const;
    StdReturnType Write(const byte*, size_t);
    StdReturnType Clear();
};

#endif // _STORAGE_H_
