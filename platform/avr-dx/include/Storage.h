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

#ifndef WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_STORAGE_H_
#define WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*!
 * \brief The most bytes one Storage::Write() may hold, and one Storage::Read() may ask for.
 *
 * Well above what the core writes today, so its settings can grow without the platforms having to
 * change together.
 */
#define STORAGE_CAPACITY                                128u

/*! \brief Where the stored data starts in the EEPROM: the length byte, and the blob behind it. */
#define STORAGE_EEPROM_ADDRESS                          0u

/*!
 * \brief The store the settings survive a restart in: one blob of bytes, which Persistence writes
 *        and reads whole.
 *
 * What is inside the blob is the core's, magic and checksum included, so a stale or foreign blob
 * is rejected there. The store only has to keep what it is given, and hand it back only to a
 * reader that asks for exactly its length.
 *
 * On the AVR the store is the on-chip EEPROM: a length byte at STORAGE_EEPROM_ADDRESS, and the
 * blob behind it. A write changes only the bytes that differ.
 */
class Storage
{
  public:
    /*! \brief The most bytes one Write() may hold, STORAGE_CAPACITY. */
    static constexpr size_t kCapacity{STORAGE_CAPACITY};

  private:
    // A length no blob can have, so an EEPROM that was never written - which reads 0xFF
    // throughout on a fresh part - says so by itself and needs no separate magic.
    static constexpr byte kLengthEmpty{0xFFu};

    Storage() { }
    ~Storage() = default;

  public:
    /*! \brief Returns the one store. */
    static Storage& GetInstance() {
        static Storage singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Reads the stored blob into `data`, if it is `size` bytes long.
     *
     * A stored length that differs from the one asked for is a blob from an older layout. Refused
     * rather than handed over short: what is inside is the core's structure, and giving it fewer
     * bytes than it reads would let it interpret whatever follows in memory. An EEPROM that was
     * never written or was cleared holds a length no blob can have.
     *
     * The AVR leaves `data` untouched on every refusal.
     *
     * \param[out] data A buffer of `size` bytes.
     * \param[in] size The length of the blob, from 1 to kCapacity.
     * \return kEOk, or kENotOk if `data` is null, `size` is 0 or above kCapacity, or no blob of
     *         `size` bytes could be read; `data` may then have been written in part.
     */
    StdReturnType Read(byte* data, size_t size) const;

    /*!
     * \brief Replaces the stored blob with the `size` bytes at `data`.
     *
     * The blob goes in first and the length last. Until the length is in, a restart part way
     * through finds the previous length in front of a half-written blob - which the core's
     * checksum rejects. Writing the length first would instead leave a blob that announces itself
     * as complete while it is not.
     *
     * The AVR has no refusal beyond a null `data` and such a `size`: it does not check what the
     * EEPROM took.
     *
     * \param[in] data The blob.
     * \param[in] size The length of the blob, from 1 to kCapacity.
     * \return kEOk, or kENotOk if `data` is null, `size` is 0 or above kCapacity, or the blob did
     *         not reach the store whole. For a null `data` or such a `size`, the store is left as
     *         it was; otherwise it may hold part of the blob.
     */
    StdReturnType Write(const byte* data, size_t size);

    /*!
     * \brief Forgets the stored blob.
     *
     * A store that was empty to begin with counts as cleared, so calling this twice is not an
     * error.
     *
     * Only the length byte is reset, not the blob behind it: the length is what Read() consults,
     * so the blob is unreachable either way, and erasing it would spend a write cycle on every
     * byte of it to no end. The AVR never refuses.
     *
     * \return kEOk if nothing is stored afterwards, or kENotOk if the blob could not be removed;
     *         the store then still holds it.
     */
    StdReturnType Clear();
};

#endif  // WORDCLOCK_PLATFORM_AVR_DX_INCLUDE_STORAGE_H_
