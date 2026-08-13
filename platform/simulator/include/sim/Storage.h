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
 *      \brief      Simulator implementation of the settings store, backed by a file
 *
 *      \details    Mirrors the public API of the hardware store so Persistence keeps
 *                  either one without knowing which.
 *
 *                  A real file rather than a buffer in memory, so the simulator behaves
 *                  like the device: settings made over the console are still there after
 *                  a restart. That is what makes the whole persistence path testable
 *                  without hardware - a store that forgot on exit would leave the half
 *                  that matters unexercised.
 *
 *                  The file lands in the working directory. It carries no format of its
 *                  own: what is in it is whatever Persistence handed over, magic and
 *                  checksum included, so a stale or foreign file is rejected there rather
 *                  than here.
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
#define STORAGE_FILE_NAME                               "wordclock-settings.bin"

/* Storage parameter */
/* What one store() may hold. Well above what Persistence writes today, so the overlay
   settings can follow without the platforms having to change together. */
#define STORAGE_CAPACITY                                128u

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
