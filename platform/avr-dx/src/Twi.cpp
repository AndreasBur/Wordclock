// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The I2C bus the clock chip and the light sensor share, see Twi.h.
 */

#include <avr/io.h>

#include "Arduino.h"
#include "Twi.h"

// Local to this file, so Doxygen leaves it out.
/*! \cond */
/* The bus the strip left free, see Twi.h. */
#define TWI_PERIPHERAL TWI1

// The data sheet's baud formula without the rise-time term, which is negligible against
// the pull-ups this bus is built with. The five is the controller's own overhead per bit.
#define TWI_BAUD_OVERHEAD_CYCLES 5uL
#define TWI_BAUD_REGISTER \
  static_cast<uint8_t>((F_CPU / (2uL * TWI_FREQUENCY_HZ)) - TWI_BAUD_OVERHEAD_CYCLES)
/*! \endcond */

/*
 * Both figures below are derived from F_CPU, and both fail quietly rather than loudly if it
 * is changed far enough: a baud register computed from too low a clock underflows to a huge
 * value and drops the bus to a crawl, and a timeout rounded to zero makes every transfer
 * report failure without ever waiting. Neither shows up as a build error on its own.
 */
static_assert((F_CPU / (2uL * TWI_FREQUENCY_HZ)) > TWI_BAUD_OVERHEAD_CYCLES,
              "Twi: F_CPU is too low for this bus frequency, the baud register would underflow");
static_assert(TWI_BAUD_REGISTER <= 0xFFu,
              "Twi: F_CPU is too high for this bus frequency, the baud register does not fit");
static_assert(TWI_TIMEOUT_LOOPS > 0u,
              "Twi: the timeout rounded down to zero, every transfer would fail immediately");

/*
 * Waits for a written byte to have been acknowledged. kENotOk when the device did not answer, or
 * when the bus never reported the byte as sent at all.
 */
StdReturnType Twi::WaitForWrite() {
  for (uint16_t loops = TWI_TIMEOUT_LOOPS; loops > 0u; loops--) {
    uint8_t const status = TWI_PERIPHERAL.MSTATUS;

    if ((status & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) != 0u) {
      return kENotOk;
    }

    if ((status & TWI_WIF_bm) != 0u) {
      // Written, but not necessarily to anybody: an unfitted device leaves the line
      // high, which reads back as a missing acknowledge.
      return ((status & TWI_RXACK_bm) != 0u) ? kENotOk : kEOk;
    }
  }

  return kENotOk;
}

StdReturnType Twi::WaitForRead() {
  for (uint16_t loops = TWI_TIMEOUT_LOOPS; loops > 0u; loops--) {
    uint8_t const status = TWI_PERIPHERAL.MSTATUS;

    if ((status & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) != 0u) {
      return kENotOk;
    }
    if ((status & TWI_RIF_bm) != 0u) {
      return kEOk;
    }
  }

  return kENotOk;
}

/*
 * Releases the bus. Sent on every path out of a transfer, the failing ones included: a transfer
 * abandoned without a stop leaves the bus owned, and the next reader - the other chip on it -
 * would find it busy for good.
 */
void Twi::Stop() { TWI_PERIPHERAL.MCTRLB = TWI_MCMD_STOP_gc; }

void Twi::Init() {
  if (initialised_) {
    return;
  }

  TWI_PERIPHERAL.MBAUD = TWI_BAUD_REGISTER;
  TWI_PERIPHERAL.MCTRLA = TWI_ENABLE_bm;
  // The controller comes up believing the bus state unknown, and refuses to start a
  // transfer until it is told otherwise.
  TWI_PERIPHERAL.MSTATUS = TWI_BUSSTATE_IDLE_gc;

  initialised_ = true;
}

// Not const, though nothing here writes a member: these drive the hardware, and a
// const would say they change nothing.
// NOLINTBEGIN(readability-make-member-function-const)
StdReturnType Twi::Write(byte address, byte const* data, byte length) {
  if (!initialised_) {
    return kENotOk;
  }

  TWI_PERIPHERAL.MADDR = static_cast<uint8_t>(address << 1u);

  if (WaitForWrite() != kEOk) {
    Stop();
    return kENotOk;
  }

  for (byte index = 0u; index < length; index++) {
    TWI_PERIPHERAL.MDATA = data[index];

    if (WaitForWrite() != kEOk) {
      Stop();
      return kENotOk;
    }
  }

  Stop();
  return kEOk;
}

StdReturnType Twi::Read(byte address, byte* data, byte length) {
  if (!initialised_) {
    return kENotOk;
  }
  if (length == 0u) {
    return kENotOk;
  }

  TWI_PERIPHERAL.MADDR = static_cast<uint8_t>((address << 1u) | 1u);

  for (byte index = 0u; index < length; index++) {
    if (WaitForRead() != kEOk) {
      Stop();
      return kENotOk;
    }

    data[index] = TWI_PERIPHERAL.MDATA;

    if (index < (length - 1u)) {
      TWI_PERIPHERAL.MCTRLB = TWI_MCMD_RECVTRANS_gc;
    }
  }

  TWI_PERIPHERAL.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
  return kEOk;
}
// NOLINTEND(readability-make-member-function-const)

StdReturnType Twi::WriteRegister(byte address, byte register_address, byte value) {
  byte const buffer[]{register_address, value};

  return Write(address, buffer, sizeof(buffer));
}

StdReturnType Twi::ReadRegister(byte address, byte register_address, byte* data, byte length) {
  if (Write(address, &register_address, 1u) != kEOk) {
    return kENotOk;
  }

  return Read(address, data, length);
}
