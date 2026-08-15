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
/**     \file       Storage.h
 *      \brief      Where the settings survive a restart, in the on-chip EEPROM
 *
 *      \details    Holds one blob and its length, and hands it back only to a reader asking
 *                  for exactly that length. What is inside it belongs to the core, which
 *                  carries its own magic and checksum - so a foreign or stale blob is
 *                  rejected there rather than here, and the length is all this level needs
 *                  to tell one layout from another.
 *
 *                  The EEPROM is written rather than the flash, and only where the bytes
 *                  differ: the settings are saved whenever they change, and a part that
 *                  wears out after a hundred thousand writes should not be spent on bytes
 *                  that already hold the right value.
 *
******************************************************************************************************************************************************/
#ifndef _STORAGE_H_
#define _STORAGE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Storage configuration parameter */
/* What one write() may hold. Well above what the core writes today, so its settings can
   grow without the platforms having to change together. */
#define STORAGE_CAPACITY                                128u

/* Where the blob starts in the EEPROM. Behind the length byte that precedes it. */
#define STORAGE_EEPROM_ADDRESS                          0u

/******************************************************************************************************************************************************
 *  C L A S S   S T O R A G E
******************************************************************************************************************************************************/
class Storage
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    static constexpr size_t Capacity{STORAGE_CAPACITY};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* A length no blob can have, so an EEPROM that was never written - which reads 0xFF
       throughout on a fresh part - says so by itself and needs no separate magic. */
    static constexpr byte LengthEmpty{0xFFu};

    Storage() { }
    ~Storage() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Storage& getInstance() {
        static Storage SingletonInstance;
        return SingletonInstance;
    }

    // methods
    StdReturnType read(byte*, size_t) const;
    StdReturnType write(const byte*, size_t);
    StdReturnType clear();
};

#endif // _STORAGE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
