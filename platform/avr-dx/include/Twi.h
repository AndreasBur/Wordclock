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
/**     \file       Twi.h
 *      \brief      The I2C bus the clock chip and the light sensor share
 *
 *      \details    A host-only master, polled rather than interrupt driven: both devices on
 *                  it are read at most once a second, and a transfer of a few bytes at
 *                  100 kHz is far shorter than the tick it happens in.
 *
 *                  Every wait is bounded. A sensor that is not fitted, or a bus held low by
 *                  one that has locked up, must cost a return code and not the clock - the
 *                  display and the console have to keep working on a board where neither
 *                  chip is populated.
 *
 *                  TWI1 rather than TWI0, and that is not free choice: TWI0 can only reach
 *                  PA2 and PA3, which carry the LED strip's clock and its data line. TWI1's
 *                  default pins are untouched by anything else here.
 *
******************************************************************************************************************************************************/
#ifndef _TWI_H_
#define _TWI_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Twi configuration parameter */
#define TWI_FREQUENCY_HZ                                100000uL

/* How long one byte may take before the bus is given up on, expressed in bit times so that
   it means a duration rather than a number of spins: twenty of them is twice what a byte
   needs, which leaves room for a device stretching the clock and still keeps a dead bus far
   inside a single tick.
   The conversion needs the cost of one iteration - a volatile register read, a mask and a
   branch, call it eight cycles. Guessing low is the safe direction: it yields more
   iterations, so an inaccurate estimate lengthens the timeout rather than cutting a healthy
   transfer short. */
#define TWI_TIMEOUT_BIT_TIMES                           20u
#define TWI_TIMEOUT_LOOP_CYCLES                         8u
#define TWI_TIMEOUT_LOOPS                               static_cast<uint16_t>(((F_CPU / TWI_FREQUENCY_HZ) * TWI_TIMEOUT_BIT_TIMES) / TWI_TIMEOUT_LOOP_CYCLES)

/******************************************************************************************************************************************************
 *  C L A S S   T W I
******************************************************************************************************************************************************/
class Twi
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    bool Initialised{false};

    Twi() { }
    ~Twi() { }

    static StdReturnType waitForWrite();
    static StdReturnType waitForRead();
    static void stop();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Twi& getInstance() {
        static Twi SingletonInstance;
        return SingletonInstance;
    }

    void init();

    /* Both address the device in seven-bit form, as it is printed in the data sheets; the
       read/write bit is added here. */
    StdReturnType write(byte Address, const byte* Data, byte Length);
    StdReturnType read(byte Address, byte* Data, byte Length);

    /* One register write, the shape every configuration command on this bus takes. */
    StdReturnType writeRegister(byte Address, byte Register, byte Value);

    /* Points the device at a register and reads from there, which is how both chips are
       read. */
    StdReturnType readRegister(byte Address, byte Register, byte* Data, byte Length);
};

#endif // _TWI_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
