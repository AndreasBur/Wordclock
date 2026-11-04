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

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* Storage configuration parameter */
#define STORAGE_FILE_NAME                               "wordclock-settings.bin"

/* Storage parameter */
/*
 * What one store() may hold. The overlay settings this was held open for have landed and
 * take the blob to 110 bytes, which 128 would still have covered - but only just, and the
 * next setting would have moved both platforms again. Raised once, with room, rather than
 * by the byte: the store is a file here, so the unused part costs nothing.
 *
 * This bounds what store() and load() accept, and nothing else - it does not shape the
 * blob, so platforms may disagree about it and still read each other's. What each one has
 * to clear on its own is Persistence's static_assert that the settings fit, and the
 * platform with the smallest capacity is the one that stops compiling first when they
 * grow.
 */
#define STORAGE_CAPACITY                                256u

class Storage
{
  public:
    static constexpr size_t kCapacity{STORAGE_CAPACITY};

  private:
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
