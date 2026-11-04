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

#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* Storage configuration parameter */
/* Both are limited to 15 characters by NVS. */
#define STORAGE_NAMESPACE                               "wordclock"
#define STORAGE_KEY                                     "settings"

/* Storage parameter */
/*
 * What one Write() may hold. The overlay settings this was held open for have landed and
 * take the blob to 110 bytes, which 128 would still have covered - but only just, and the
 * next setting would have moved both platforms again. Raised once, with room, rather than
 * by the byte: NVS stores what it is given, so the unused part costs nothing here.
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
