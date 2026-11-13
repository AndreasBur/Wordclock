/*
 * How this backend opens the bus the BH1750 and the DS3231 share: the pins and the clock reach
 * Wire, and the clock does so after begin(), which would otherwise put its default back over it.
 * A clock other than that default is asked for, or the order could not be seen.
 */
#include <Wire.h>

#include "I2cBus.h"
#include "check.h"

namespace {
constexpr byte kPinSda{4u};
constexpr byte kPinScl{5u};
constexpr uint32_t kFastClockHz{400000u};

void TestOpenSetsThePinsAndTheClock() {
  Wire = TwoWire{};
  Check(I2cBus::Open(kPinSda, kPinScl, kFastClockHz) == kEOk, "the bus counts as open");
  Check((Wire.Sda == kPinSda) && (Wire.Scl == kPinScl), "on the pins asked for");
  Check(Wire.Clock == kFastClockHz, "at the clock asked for, set after begin()");
}
}  // namespace

int main() {
  TestOpenSetsThePinsAndTheClock();
  return Report();
}
