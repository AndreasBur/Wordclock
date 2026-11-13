// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Settings store backed by the ESP32's NVS partition.
 *
 * Through Preferences rather than the nvs_* functions: it is part of the Arduino core, and the
 * flash initialisation the core already does at startup is all it needs.
 *
 * Every call opens and closes the namespace. Holding it open would save little - these calls happen
 * when a setting changed, not per tick - and it would keep a handle alive across a reconfiguration.
 */

// clang-format off
/* Ahead of Storage.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Preferences.h>

#include "Storage.h"
// clang-format on

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
StdReturnType Storage::Read(byte* buffer, size_t size) const {
  if ((buffer == nullptr) || (size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  Preferences store;
  if (!store.begin(STORAGE_NAMESPACE, true)) {
    return kENotOk;
  }

  size_t const stored_size = store.getBytesLength(STORAGE_KEY);
  if (stored_size != size) {
    store.end();
    return kENotOk;
  }

  size_t const read = store.getBytes(STORAGE_KEY, buffer, size);
  store.end();

  if (read != size) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Write(byte const* buffer, size_t size) {
  if ((buffer == nullptr) || (size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  Preferences store;
  if (!store.begin(STORAGE_NAMESPACE, false)) {
    return kENotOk;
  }

  size_t const written = store.putBytes(STORAGE_KEY, buffer, size);
  store.end();

  if (written != size) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Clear() {
  Preferences store;
  if (!store.begin(STORAGE_NAMESPACE, false)) {
    return kENotOk;
  }

  bool const was_removed = store.remove(STORAGE_KEY);
  bool const was_empty = !store.isKey(STORAGE_KEY);
  store.end();

  if (!was_removed && !was_empty) {
    return kENotOk;
  }

  return kEOk;
}
// NOLINTEND(readability-convert-member-functions-to-static)
