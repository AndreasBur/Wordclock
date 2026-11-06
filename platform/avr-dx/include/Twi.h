// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The I2C bus the clock chip and the light sensor share.
 *
 * A host-only master, polled rather than interrupt driven: both devices on it are read at most once
 * a second, and a transfer of a few bytes at 100 kHz is far shorter than the tick it happens in.
 *
 * Every wait is bounded. A sensor that is not fitted, or a bus held low by one that has locked up,
 * must cost a return code and not the clock - the display and the console have to keep working on a
 * board where neither chip is populated.
 *
 * TWI1 rather than TWI0, and that is not free choice: TWI0 can only reach PA2 and PA3, which carry
 * the LED strip's clock and its data line. TWI1's default pins are untouched by anything else here.
 */

#ifndef _TWI_H_
#define _TWI_H_

#include "StandardTypes.h"
#include "Arduino.h"

/*! \brief The clock of the bus, in hertz. */
#define TWI_FREQUENCY_HZ                                100000uL

/*!
 * \brief How long one byte may take before the bus is given up on, in bit times.
 *
 * Expressed in bit times so that it means a duration rather than a number of spins: twenty of them
 * is twice what a byte needs, which leaves room for a device stretching the clock and still keeps a
 * dead bus far inside a single tick.
 */
#define TWI_TIMEOUT_BIT_TIMES                           20u
/*!
 * \brief The processor cycles one poll of the status is taken to cost, which turns
 *        TWI_TIMEOUT_BIT_TIMES into TWI_TIMEOUT_LOOPS.
 *
 * The conversion needs the cost of one iteration - a volatile register read, a mask and a branch,
 * call it eight cycles. Guessing low is the safe direction: it yields more iterations, so an
 * inaccurate estimate lengthens the timeout rather than cutting a healthy transfer short.
 */
#define TWI_TIMEOUT_LOOP_CYCLES                         8u
/*! \brief The polls of the status a wait for one byte makes before it gives up. */
#define TWI_TIMEOUT_LOOPS                               static_cast<uint16_t>(((F_CPU / TWI_FREQUENCY_HZ) * TWI_TIMEOUT_BIT_TIMES) / TWI_TIMEOUT_LOOP_CYCLES)

/*!
 * \brief The host of the two-wire bus on TWI1, which the clock chip and the light sensor share.
 *
 * There is one instance, GetInstance(), and Init() brings it up. A transfer polls the status for
 * each byte, at most TWI_TIMEOUT_LOOPS times, and gives up on an arbitration lost, a bus error or
 * a written byte that is not acknowledged. Every transfer that has addressed a device ends with a
 * stop condition, the failing ones included, so the bus is free for the next one.
 */
class Twi
{
  private:
    bool initialised_{false};

    Twi() { }
    ~Twi() = default;

    static StdReturnType WaitForWrite();
    static StdReturnType WaitForRead();
    static void Stop();

  public:
    /*! \brief Returns the one bus. */
    static Twi& GetInstance() {
        static Twi singleton_instance;
        return singleton_instance;
    }

    /*!
     * \brief Brings the bus up: its baud rate for TWI_FREQUENCY_HZ, the host enabled and the bus
     *        state set to idle.
     *
     * Only the first call acts; the ones after it return at once.
     */
    void Init();

    /*!
     * \brief Writes `length` bytes to a device.
     *
     * With a `length` of 0, it only addresses the device and stops, which tells whether the device
     * answers.
     *
     * \param[in] address The address of the device in seven-bit form, as it is printed in the data
     *                    sheets; the read/write bit is added here.
     * \param[in] data The bytes to write.
     * \param[in] length The number of bytes.
     * \return kEOk, or kENotOk in two cases. Before Init(), the bus is not touched. When the device
     *         does not acknowledge its address or a byte, the arbitration is lost, the bus reports
     *         an error or a byte is not reported as sent in time, the transfer is stopped there;
     *         the bytes before that one have reached the device.
     */
    StdReturnType Write(byte address, const byte* data, byte length);

    /*!
     * \brief Reads `length` bytes from a device.
     *
     * Every byte but the last is acknowledged, the last one is not: that is how the device is told
     * to stop driving the bus, and acknowledging it too would leave it sending into the stop
     * condition.
     *
     * \param[in] address The address of the device in seven-bit form, as it is printed in the data
     *                    sheets; the read/write bit is added here.
     * \param[out] data Where the bytes go.
     * \param[in] length The number of bytes, at least 1.
     * \return kEOk, or kENotOk in two cases. Before Init() or with a `length` of 0, the bus is not
     *         touched and `data` is left as it was. When a byte does not arrive in time, which is
     *         also what a device that does not acknowledge its address leads to, the arbitration is
     *         lost or the bus reports an error, the transfer is stopped there; `data` then holds
     *         the bytes that arrived before, and the rest of it is left as it was.
     */
    StdReturnType Read(byte address, byte* data, byte length);

    /*!
     * \brief Writes `value` into the register `register_address` of a device.
     *
     * One register write, the shape every configuration command on this bus takes: the register
     * address and the value, in one Write().
     *
     * \param[in] address The address of the device in seven-bit form.
     * \param[in] register_address The register.
     * \param[in] value The value.
     * \return kEOk, or kENotOk as Write() returns it.
     */
    StdReturnType WriteRegister(byte address, byte register_address, byte value);

    /*!
     * \brief Points a device at a register and reads `length` bytes from there, which is how both
     *        chips are read.
     *
     * Two transfers rather than a repeated start: the pointer write is stopped before the read
     * begins. Both chips here keep their register pointer across the stop, and separating the
     * transfers keeps this simple enough to have no second waiting path to get wrong.
     *
     * \param[in] address The address of the device in seven-bit form.
     * \param[in] register_address The register to start at.
     * \param[out] data Where the bytes go.
     * \param[in] length The number of bytes, at least 1.
     * \return kEOk, or kENotOk if the pointer write fails as Write() does, which leaves `data` as
     *         it was, or if the read fails as Read() does.
     */
    StdReturnType ReadRegister(byte address, byte register_address, byte* data, byte length);
};

#endif // _TWI_H_
