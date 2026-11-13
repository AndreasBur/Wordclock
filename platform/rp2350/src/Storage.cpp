// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The settings blob, as one file in LittleFS.
 *
 * Where the ESP32 backend has the IDF's key-value store, this core offers a filesystem and an
 * EEPROM emulation. The filesystem wins on one point that Storage's contract turns on: a file
 * carries its own length, and Read() has to refuse a blob of the wrong size because it belongs to
 * another version of the settings. The emulation would need that length stored alongside and kept
 * in step by hand.
 *
 * The namespace becomes a directory and the key a file inside it, so the two names the contract
 * already carries still say what they said.
 *
 * The filesystem is mounted on the first call rather than in an Init(): the contract has no Init(),
 * and Persistence reads before it ever writes.
 */

// clang-format off
/* Ahead of Storage.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <LittleFS.h>

#include "Storage.h"
// clang-format on

namespace {
constexpr char kBlobPath[]{"/" STORAGE_NAMESPACE "/" STORAGE_KEY};

/*
 * Mounted once and remembered, including the failure: a filesystem that would not come up
 * will not come up on the next call either, and retrying on every read would turn a fault
 * into a stall. Formatting on a failed mount is what makes a first boot work on a chip
 * whose flash has never held one.
 */
bool IsFilesystemReady() {
  static bool was_tried{false};
  static bool is_mounted{false};

  if (!was_tried) {
    was_tried = true;
    is_mounted = LittleFS.begin();
  }

  return is_mounted;
}

// The directory the blob lives in. mkdir() on an existing one is not an error worth
// reporting - the open that follows is what decides.
void EnsureDirectory() { LittleFS.mkdir("/" STORAGE_NAMESPACE); }

}  // namespace

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
  if (!IsFilesystemReady()) {
    return kENotOk;
  }

  File blob = LittleFS.open(kBlobPath, "r");
  if (!blob) {
    return kENotOk;
  }

  if (blob.size() != size) {
    blob.close();
    return kENotOk;
  }

  size_t const read = blob.read(buffer, size);
  blob.close();

  if (read != size) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Write(byte const* buffer, size_t size) {
  if ((buffer == nullptr) || (size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }
  if (!IsFilesystemReady()) {
    return kENotOk;
  }

  EnsureDirectory();

  File blob = LittleFS.open(kBlobPath, "w");
  if (!blob) {
    return kENotOk;
  }

  size_t const written = blob.write(buffer, size);
  blob.close();

  if (written != size) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Clear() {
  if (!IsFilesystemReady()) {
    return kENotOk;
  }

  if (!LittleFS.exists(kBlobPath)) {
    return kEOk;
  }

  if (!LittleFS.remove(kBlobPath)) {
    return kENotOk;
  }

  return kEOk;
}
// NOLINTEND(readability-convert-member-functions-to-static)
