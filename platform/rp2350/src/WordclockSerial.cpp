// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The port Serial is bound to: the core's serial port plus an injected line source.
 */

#include "Arduino.h"

#ifndef WORDCLOCK_ARDUINO_SHIM
#error \
    "WordclockSerial: the core's Arduino.h was included instead of this platform's. Put platform/rp2350/include ahead of the framework on the include path, or the web console will be silently mute."  // NOLINT(whitespace/line_length)
#endif

#include <stdio.h>
#include <string.h>

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
void WordclockSerial::begin(unsigned long baud_rate) {  // NOLINT(runtime/int)
  wordclock_platform::HardwarePortBegin(baud_rate);
}

// Written by the producer only, which is what keeps this free of a lock - see the note on
// the indices in the header.
StdReturnType WordclockSerial::Inject(char const* characters, size_t length) {
  if ((characters == nullptr) || (length == 0u)) {
    return kENotOk;
  }

  size_t head = inject_head_.load(std::memory_order_relaxed);

  for (size_t index = 0u; index < length; index++) {
    size_t const next = NextIndex(head);

    /* One slot stays empty, which is what tells a full buffer from an empty one. */
    if (next == inject_tail_.load(std::memory_order_acquire)) {
      return kENotOk;
    }

    inject_buffer_[head] = characters[index];
    head = next;
    // Published after the character, so the reader cannot see the index move ahead of
    // the data it points at.
    inject_head_.store(head, std::memory_order_release);
  }

  return kEOk;
}

int WordclockSerial::available() const {
  int const from_port = wordclock_platform::HardwarePort().available();

  if (from_port > 0) {
    return from_port;
  }

  return IsInjectEmpty() ? 0 : 1;
}

int WordclockSerial::read() {
  Stream& port = wordclock_platform::HardwarePort();

  if (port.available() > 0) {
    return port.read();
  }
  if (!IsInjectEmpty()) {
    return static_cast<unsigned char>(ReadInjected());
  }

  return -1;
}

void WordclockSerial::print(char const* text) {
  if (text == nullptr) {
    return;
  }

  for (char const* character = text; *character != '\0'; character++) {
    print(*character);
  }
}

void WordclockSerial::print(__FlashStringHelper const* text) {
  // On this target flash is read through the same address space, so what F() wrapped can
  // be read as an ordinary string.
  print(reinterpret_cast<char const*>(text));
}

void WordclockSerial::print(int number) {
  char digits[12]{};

  /* Ten digits and a sign are the most an int can print to; nothing to truncate. */
  (void)snprintf(digits, sizeof(digits), "%d", number);
  print(digits);
}

void WordclockSerial::print(char character) {
  if (character == '\n') {
    println();
    return;
  }

  wordclock_platform::HardwarePort().write(static_cast<uint8_t>(character));
  AppendToLine(character);
}

void WordclockSerial::println() {
  wordclock_platform::HardwarePort().write(static_cast<uint8_t>('\r'));
  wordclock_platform::HardwarePort().write(static_cast<uint8_t>('\n'));

  FlushLine();
}

void WordclockSerial::println(char const* text) {
  print(text);
  println();
}

void WordclockSerial::println(int number) {
  print(number);
  println();
}

void WordclockSerial::println(__FlashStringHelper const* text) {
  print(text);
  println();
}

void WordclockSerial::flush() { wordclock_platform::HardwarePort().flush(); }
// NOLINTEND(readability-convert-member-functions-to-static)

char WordclockSerial::ReadInjected() {
  size_t const tail = inject_tail_.load(std::memory_order_relaxed);
  char const character = inject_buffer_[tail];

  // Published after the character was taken, so the producer cannot refill the slot
  // while it is still being read.
  inject_tail_.store(NextIndex(tail), std::memory_order_release);

  return character;
}

/*
 * Collects the line the sink will be handed. A line that outgrows the buffer is flushed early
 * rather than cut: the client then sees the answer split over two frames, which is readable,
 * where a truncated line would silently lose its end.
 */
void WordclockSerial::AppendToLine(char character) {
  if (line_used_ >= (kLineLength - 1u)) {
    FlushLine();
  }

  line_buffer_[line_used_++] = character;
}

/*
 * Hands the collected line to whoever registered for it. An empty line is dropped, because
 * println() is also used to terminate output that print() already ended with a newline -
 * passing that on would send a blank frame after every answer.
 */
void WordclockSerial::FlushLine() {
  if (line_used_ == 0u) {
    return;
  }

  line_buffer_[line_used_] = '\0';
  line_used_ = 0u;

  if (line_sink_ != nullptr) {
    line_sink_(line_buffer_);
  }
}
