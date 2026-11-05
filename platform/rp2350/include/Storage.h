// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Settings store backed by one file in LittleFS, on the RP2350's flash.
 *
 * Mirrors the public API of the simulator's file-backed store, so Persistence keeps either one
 * without knowing which.
 *
 * One blob in one file rather than a file per setting. A blob keeps the format in one place -
 * Persistence's versioned struct - instead of spreading it over file names that then have to be
 * kept in step with the struct.
 *
 * Persistence's Task() only writes when a setting actually changed, so the write count follows
 * configuration changes rather than the clock.
 */

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The directory of LittleFS the blob is kept in, below the root. */
#define STORAGE_NAMESPACE                               "wordclock"
/*! \brief The name of the blob's file in STORAGE_NAMESPACE. */
#define STORAGE_KEY                                     "settings"

/*!
 * \brief The most bytes one Storage::Write() may hold, and one Storage::Read() may ask for.
 *
 * The overlay settings this was held open for have landed and left the blob little room below the
 * 128 bytes this stood at before, and the next setting would have moved both platforms again.
 * Raised once, with room, rather than by the byte: the file holds only the bytes it is
 * given, so the unused part costs nothing here.
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
 * On the RP2350 the store is one file of LittleFS, `/STORAGE_NAMESPACE/STORAGE_KEY`, reached
 * through the Arduino core's `LittleFS`. The first call of any of the three mounts the filesystem;
 * a mount that failed is not tried again, and every later call then answers kENotOk.
 */
class Storage
{
  public:
    /*! \brief The most bytes one Write() may hold, STORAGE_CAPACITY. */
    static constexpr size_t kCapacity{STORAGE_CAPACITY};

  private:
    Storage() { }
    ~Storage() { }

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
     * It opens the file and compares its length with `size` before it reads anything, so a
     * filesystem that is not mounted, a file that does not open and a length that differs all
     * leave `buffer` untouched. A read that comes back short leaves it written in part.
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
     * It creates the directory STORAGE_NAMESPACE where it is missing and opens the file anew for
     * writing, which empties it, rather than appending: a shorter blob then cannot leave the tail
     * of a longer one behind it, which Read() would refuse for a length it never wrote. The blob
     * did not reach the store whole if the filesystem is not mounted, the file does not open or
     * fewer than `size` bytes were written.
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
     * It removes the file where there is one, and answers kEOk where there is none. A filesystem
     * that is not mounted answers kENotOk, whether a blob is stored or not.
     *
     * \return kEOk if nothing is stored afterwards, or kENotOk if the blob could not be removed;
     *         the store then still holds it.
     */
    StdReturnType Clear();
};

#endif // _STORAGE_H_
