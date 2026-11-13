// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * How this backend opens the bus the BH1750 and the DS3231 share: one Wire.begin() with the pins
 * and the clock, whose answer is the answer - a refused bus is not open, so the drivers try again.
 */
#include <Wire.h>

#include "I2cBus.h"
#include "platform/test/check.h"

namespace {
constexpr byte kPinSda{8u};
constexpr byte kPinScl{9u};
constexpr uint32_t kFastClockHz{400000u};

void TestOpenHandsOverThePinsAndTheClock() {
  Wire = TwoWire{};
  Check(I2cBus::Open(kPinSda, kPinScl, kFastClockHz) == kEOk, "the bus opens");
  Check((Wire.Sda == kPinSda) && (Wire.Scl == kPinScl) && (Wire.Clock == kFastClockHz),
        "with the pins and the clock asked for");
}

void TestARefusedBusIsNotOpen() {
  Wire = TwoWire{};
  Wire.BeginAnswer = false;
  Check(I2cBus::Open(kPinSda, kPinScl, kFastClockHz) == kENotOk,
        "a bus Wire.begin() refused is not open");
}
}  // namespace

int main() {
  TestOpenHandsOverThePinsAndTheClock();
  TestARefusedBusIsNotOpen();
  return Report();
}
