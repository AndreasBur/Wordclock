/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       Twi.cpp
 *      \brief      The I2C bus the clock chip and the light sensor share, see Twi.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <avr/io.h>

#include "Arduino.h"
#include "Twi.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* The bus the strip left free, see Twi.h. */
#define TWI_PERIPHERAL                  TWI1

/* The data sheet's baud formula without the rise-time term, which is negligible against
   the pull-ups this bus is built with. The five is the controller's own overhead per bit. */
#define TWI_BAUD_OVERHEAD_CYCLES        5uL
#define TWI_BAUD_REGISTER               static_cast<uint8_t>((F_CPU / (2uL * TWI_FREQUENCY_HZ)) - TWI_BAUD_OVERHEAD_CYCLES)

/* Both figures below are derived from F_CPU, and both fail quietly rather than loudly if it
   is changed far enough: a baud register computed from too low a clock underflows to a huge
   value and drops the bus to a crawl, and a timeout rounded to zero makes every transfer
   report failure without ever waiting. Neither shows up as a build error on its own. */
static_assert((F_CPU / (2uL * TWI_FREQUENCY_HZ)) > TWI_BAUD_OVERHEAD_CYCLES,
              "Twi: F_CPU is too low for this bus frequency, the baud register would underflow");
static_assert(TWI_BAUD_REGISTER <= 0xFFu, "Twi: F_CPU is too high for this bus frequency, the baud register does not fit");
static_assert(TWI_TIMEOUT_LOOPS > 0u, "Twi: the timeout rounded down to zero, every transfer would fail immediately");

/******************************************************************************************************************************************************
 *  P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  waitForWrite()
******************************************************************************************************************************************************/
/*! \brief          Waits for a written byte to have been acknowledged
 *  \return         E_NOT_OK when the device did not answer, or when the bus never reported
 *                  the byte as sent at all
******************************************************************************************************************************************************/
StdReturnType Twi::waitForWrite()
{
    for(uint16_t Loops = TWI_TIMEOUT_LOOPS; Loops > 0u; Loops--) {
        const uint8_t Status = TWI_PERIPHERAL.MSTATUS;

        if((Status & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) != 0u) { return E_NOT_OK; }

        if((Status & TWI_WIF_bm) != 0u) {
            /* Written, but not necessarily to anybody: an unfitted device leaves the line
               high, which reads back as a missing acknowledge. */
            return ((Status & TWI_RXACK_bm) != 0u) ? E_NOT_OK : E_OK;
        }
    }

    return E_NOT_OK;
} /* waitForWrite */


/******************************************************************************************************************************************************
  waitForRead()
******************************************************************************************************************************************************/
StdReturnType Twi::waitForRead()
{
    for(uint16_t Loops = TWI_TIMEOUT_LOOPS; Loops > 0u; Loops--) {
        const uint8_t Status = TWI_PERIPHERAL.MSTATUS;

        if((Status & (TWI_ARBLOST_bm | TWI_BUSERR_bm)) != 0u) { return E_NOT_OK; }
        if((Status & TWI_RIF_bm) != 0u) { return E_OK; }
    }

    return E_NOT_OK;
} /* waitForRead */


/******************************************************************************************************************************************************
  stop()
******************************************************************************************************************************************************/
/*! \brief          Releases the bus
 *  \details        Sent on every path out of a transfer, the failing ones included: a
 *                  transfer abandoned without a stop leaves the bus owned, and the next
 *                  reader - the other chip on it - would find it busy for good.
******************************************************************************************************************************************************/
void Twi::stop()
{
    TWI_PERIPHERAL.MCTRLB = TWI_MCMD_STOP_gc;
} /* stop */


/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  init()
******************************************************************************************************************************************************/
void Twi::init()
{
    if(Initialised) { return; }

    TWI_PERIPHERAL.MBAUD = TWI_BAUD_REGISTER;
    TWI_PERIPHERAL.MCTRLA = TWI_ENABLE_bm;
    /* The controller comes up believing the bus state unknown, and refuses to start a
       transfer until it is told otherwise. */
    TWI_PERIPHERAL.MSTATUS = TWI_BUSSTATE_IDLE_gc;

    Initialised = true;
} /* init */


/******************************************************************************************************************************************************
  write()
******************************************************************************************************************************************************/
StdReturnType Twi::write(byte Address, const byte* Data, byte Length)
{
    if(!Initialised) { return E_NOT_OK; }

    TWI_PERIPHERAL.MADDR = static_cast<uint8_t>(Address << 1u);

    if(waitForWrite() != E_OK) {
        stop();
        return E_NOT_OK;
    }

    for(byte Index = 0u; Index < Length; Index++) {
        TWI_PERIPHERAL.MDATA = Data[Index];

        if(waitForWrite() != E_OK) {
            stop();
            return E_NOT_OK;
        }
    }

    stop();
    return E_OK;
} /* write */


/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          Reads a number of bytes from a device
 *  \details        Every byte but the last is acknowledged, the last one is not: that is how
 *                  the device is told to stop driving the bus, and acknowledging it too
 *                  would leave it sending into the stop condition.
******************************************************************************************************************************************************/
StdReturnType Twi::read(byte Address, byte* Data, byte Length)
{
    if(!Initialised) { return E_NOT_OK; }
    if(Length == 0u) { return E_NOT_OK; }

    TWI_PERIPHERAL.MADDR = static_cast<uint8_t>((Address << 1u) | 1u);

    for(byte Index = 0u; Index < Length; Index++) {
        if(waitForRead() != E_OK) {
            stop();
            return E_NOT_OK;
        }

        Data[Index] = TWI_PERIPHERAL.MDATA;

        if(Index < (Length - 1u)) {
            TWI_PERIPHERAL.MCTRLB = TWI_MCMD_RECVTRANS_gc;
        }
    }

    TWI_PERIPHERAL.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
    return E_OK;
} /* read */


/******************************************************************************************************************************************************
  writeRegister()
******************************************************************************************************************************************************/
StdReturnType Twi::writeRegister(byte Address, byte Register, byte Value)
{
    const byte Buffer[]{Register, Value};

    return write(Address, Buffer, sizeof(Buffer));
} /* writeRegister */


/******************************************************************************************************************************************************
  readRegister()
******************************************************************************************************************************************************/
/*! \brief          Reads from a register of a device
 *  \details        Two transfers rather than a repeated start: the pointer write is stopped
 *                  before the read begins. Both chips here keep their register pointer across
 *                  the stop, and separating the transfers keeps this simple enough to have no
 *                  second waiting path to get wrong.
******************************************************************************************************************************************************/
StdReturnType Twi::readRegister(byte Address, byte Register, byte* Data, byte Length)
{
    if(write(Address, &Register, 1u) != E_OK) { return E_NOT_OK; }

    return read(Address, Data, Length);
} /* readRegister */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
