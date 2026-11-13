// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator implementation of the settings store, backed by a file.
 *
 * Mirrors the public API of the hardware store so Persistence keeps either one without knowing
 * which.
 *
 * A real file rather than a buffer in memory, so the simulator behaves like the device: settings
 * made over the console are still there after a restart. That is what makes the whole persistence
 * path testable without hardware - a store that forgot on exit would leave the half that matters
 * unexercised.
 *
 * The file lands in the working directory. It carries no format of its own: what is in it is
 * whatever Persistence handed over, magic and checksum included, so a stale or foreign file is
 * rejected there rather than here.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_STORAGE_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The file the blob is kept in, in the working directory. */
#define STORAGE_FILE_NAME                               "wordclock-settings.bin"

/*!
 * \brief The most bytes one Storage::Write() may hold, and one Storage::Read() may ask for.
 *
 * The overlay settings this was held open for have landed and take the blob to 110 bytes, which 128
 * would still have covered - but only just, and the next setting would have moved both platforms
 * again. Raised once, with room, rather than by the byte: the store is a file here, so the unused
 * part costs nothing.
 *
 * This bounds what Write() and Read() accept, and nothing else - it does not shape the blob, so
 * platforms may disagree about it and still read each other's. What each one has to clear on its
 * own is Persistence's static_assert that the settings fit, and the platform with the smallest
 * capacity is the one that stops compiling first when they grow.
 */
#define STORAGE_CAPACITY                                256u

/*!
 * \brief The store the settings survive a restart in: one blob of bytes, which Persistence writes
 *        and reads whole.
 *
 * What is inside the blob is the core's, magic and checksum included, so a stale or foreign blob
 * is rejected there. The store only has to keep what it is given, and hand it back only to a
 * reader that asks for exactly its length.
 *
 * The simulator keeps the blob in a file, STORAGE_FILE_NAME in the working directory, so settings
 * made over the console are still there after the next start.
 */
class Storage
{
  public:
    /*! \brief The most bytes one Write() may hold, STORAGE_CAPACITY. */
    static constexpr size_t kCapacity{STORAGE_CAPACITY};

  private:
    Storage() { }
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
     * A file of a different length is refused rather than read as far as it goes: it belongs to
     * another build of the settings, and half of one is worse than none.
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
     * In the simulator, the file is written anew.
     *
     * \param[in] buffer The blob.
     * \param[in] size The length of the blob, from 1 to kCapacity.
     * \return kEOk, or kENotOk if `buffer` is null, `size` is 0 or above kCapacity, or the blob did
     *         not reach the store whole. For a null `buffer` or such a `size`, the store is left as
     *         it was; otherwise it may hold part of the blob.
     */
    StdReturnType Write(const byte* buffer, size_t size);

    /*!
     * \brief Forgets the stored blob.
     *
     * A store that was empty to begin with counts as cleared, so calling this twice is not an
     * error.
     *
     * \return kEOk if nothing is stored afterwards, or kENotOk if the blob could not be removed;
     *         the store then still holds it.
     */
    StdReturnType Clear();
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_STORAGE_H_
