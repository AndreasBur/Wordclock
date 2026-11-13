// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Simulator implementation of the settings store, backed by a file.
 */

#include "sim/Storage.h"

#include <cstdio>

/*
 * Not static, though nothing in them reads a member: these stand in for the backends the
 * clock really runs on, and there the same call reaches a peripheral through instance
 * state. A stand-in that answered the check would have a different API from the thing it
 * stands in for, which is the one property it exists to keep.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
StdReturnType Storage::Read(byte* buffer, size_t size) const {
  if ((buffer == nullptr) || (size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
  if (file == nullptr) {
    return kENotOk;
  }

  size_t const read = std::fread(buffer, 1u, size, file);
  /*
   * One byte more would mean the file outgrew what is being asked for, which is the
   * same mismatch as one byte less. Only asked when the read delivered, because after a
   * failed one the standard stops saying where the position is.
   */
  bool const is_at_end = (read == size) && (std::fgetc(file) == EOF);
  /* Nothing to report from closing a stream that was only read. */
  (void)std::fclose(file);

  if ((read != size) || !is_at_end) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Write(byte const* buffer, size_t size) {
  if ((buffer == nullptr) || (size == 0u) || (size > kCapacity)) {
    return kENotOk;
  }

  std::FILE* file = std::fopen(STORAGE_FILE_NAME, "wb");
  if (file == nullptr) {
    return kENotOk;
  }

  size_t const written = std::fwrite(buffer, 1u, size, file);
  bool const is_flushed = (std::fclose(file) == 0);

  if ((written != size) || !is_flushed) {
    return kENotOk;
  }

  return kEOk;
}

StdReturnType Storage::Clear() {
  std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
  if (file == nullptr) {
    return kEOk;
  }
  (void)std::fclose(file);

  if (std::remove(STORAGE_FILE_NAME) != 0) {
    return kENotOk;
  }

  return kEOk;
}
// NOLINTEND(readability-convert-member-functions-to-static)
