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
 *      \brief      Settings store backed by the ESP32's NVS partition
 *
 *      \details    Mirrors the public API of the simulator's file-backed store, so
 *                  Persistence keeps either one without knowing which.
 *
 *                  One blob under one key rather than a key per setting. NVS would manage
 *                  either, but a blob keeps the format in one place - Persistence's
 *                  versioned struct - instead of spreading it over key names that then
 *                  have to be kept in step with the struct.
 *
 *                  NVS wear-levels its own sectors, and Persistence only writes when a
 *                  setting actually changed, so the write count follows configuration
 *                  changes rather than the clock.
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
/* Both are limited to 15 characters by NVS. */
#define STORAGE_NAMESPACE                               "wordclock"
#define STORAGE_KEY                                     "settings"

/* Storage parameter */
/* What one write() may hold. The overlay settings this was held open for have landed and
   take the blob to 110 bytes, which 128 would still have covered - but only just, and the
   next setting would have moved both platforms again. Raised once, with room, rather than
   by the byte: NVS stores what it is given, so the unused part costs nothing here. Keep
   this the same on every platform, or a blob written by one is refused by the other. */
#define STORAGE_CAPACITY                                256u

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
