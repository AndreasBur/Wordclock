// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Settings store backed by the ESP32's NVS partition.
 *
 * Mirrors the public API of the simulator's file-backed store, so Persistence keeps either one
 * without knowing which.
 *
 * One blob under one key rather than a key per setting. NVS would manage either, but a blob keeps
 * the format in one place - Persistence's versioned struct - instead of spreading it over key names
 * that then have to be kept in step with the struct.
 *
 * NVS wear-levels its own sectors, and Persistence only writes when a setting actually changed, so
 * the write count follows configuration changes rather than the clock.
 */

#ifndef WORDCLOCK_PLATFORM_ESP32_INCLUDE_STORAGE_H_
#define WORDCLOCK_PLATFORM_ESP32_INCLUDE_STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* Both are limited to 15 characters by NVS. */
/*! \brief The NVS namespace the blob is kept in. */
#define STORAGE_NAMESPACE "wordclock"
/*! \brief The key of the blob in STORAGE_NAMESPACE. */
#define STORAGE_KEY "settings"

/*!
 * \brief The most bytes one Storage::Write() may hold, and one Storage::Read() may ask for.
 *
 * The overlay settings this was held open for have landed and take the blob to 110 bytes, which 128
 * would still have covered - but only just, and the next setting would have moved both platforms
 * again. Raised once, with room, rather than by the byte: NVS stores what it is given, so the
 * unused part costs nothing here.
 *
 * This bounds what Write() and Read() accept, and nothing else - it does not shape the blob, so
 * platforms may disagree about it and still read each other's. What each one has to clear on its
 * own is Persistence's static_assert that the settings fit, and the platform with the smallest
 * capacity is the one that stops compiling first when they grow.
 */
#define STORAGE_CAPACITY 256u

/*!
 * \brief The store the settings survive a restart in: one blob of bytes, which Persistence writes
 *        and reads whole.
 *
 * What is inside the blob is the core's, magic and checksum included, so a stale or foreign blob
 * is rejected there. The store only has to keep what it is given, and hand it back only to a
 * reader that asks for exactly its length.
 *
 * On the ESP32 the store is one key of NVS, STORAGE_KEY in the namespace STORAGE_NAMESPACE,
 * reached through the Arduino core's `Preferences`. Every call opens the namespace and closes it
 * again.
 */
class Storage {
 public:
  /*! \brief The most bytes one Write() may hold, STORAGE_CAPACITY. */
  static constexpr size_t kCapacity{STORAGE_CAPACITY};

 private:
  Storage() {}
  ~Storage() = default;

 public:
  /*! \brief Returns the one store. */
  static Storage& GetInstance() {
    static Storage singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Reads the stored blob into `buffer`, if it is `size` bytes long.
   *
   * A blob of a different length is refused rather than read as far as it goes: it belongs to
   * another version of the settings, and Persistence would have to reject it anyway.
   *
   * It opens the namespace read-only and compares the stored length with `size` before it reads
   * anything, so a namespace that does not open and a stored length that differs both leave
   * `buffer` untouched.
   *
   * \param[out] buffer A buffer of `size` bytes.
   * \param[in] size The length of the blob, from 1 to kCapacity.
   * \return kEOk, or kENotOk if `buffer` is null, `size` is 0 or above kCapacity, or no blob of
   *         `size` bytes could be read; `buffer` may then have been written in part.
   */
  StdReturnType Read(byte* buffer, size_t size) const;

  /*!
   * \brief Replaces the stored blob with the `size` bytes at `buffer`.
   *
   * It opens the namespace for writing and stores the blob with `Preferences::putBytes()`. The
   * blob did not reach the store whole if the namespace does not open or `putBytes()` reports
   * fewer bytes than `size`.
   *
   * \param[in] buffer The blob.
   * \param[in] size The length of the blob, from 1 to kCapacity.
   * \return kEOk, or kENotOk if `buffer` is null, `size` is 0 or above kCapacity, or the blob did
   *         not reach the store whole. For a null `buffer` or such a `size`, the store is left as
   *         it was; otherwise it may hold part of the blob.
   */
  StdReturnType Write(byte const* buffer, size_t size);

  /*!
   * \brief Forgets the stored blob.
   *
   * A store that was empty to begin with counts as cleared, so calling this twice is not an
   * error.
   *
   * It removes STORAGE_KEY and then asks whether the key is still there, which is how a store
   * that held nothing counts as cleared. A namespace that does not open answers kENotOk, whether
   * a blob is stored or not.
   *
   * \return kEOk if nothing is stored afterwards, or kENotOk if the blob could not be removed; the
   *         store then still holds it.
   */
  StdReturnType Clear();
};

#endif  // WORDCLOCK_PLATFORM_ESP32_INCLUDE_STORAGE_H_
