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
/**     \file       Storage.cpp
 *      \brief      Settings store backed by the ESP32's NVS partition
 *
 *      \details    Through Preferences rather than the nvs_* functions: it is part of the
 *                  Arduino core, and the flash initialisation the core already does at
 *                  startup is all it needs.
 *
 *                  Every call opens and closes the namespace. Holding it open would save
 *                  little - these calls happen when a setting changed, not per tick - and
 *                  it would keep a handle alive across a reconfiguration.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Storage.h"

#include <Preferences.h>

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          Reads the stored blob
 *  \details        A blob of a different length is refused rather than read as far as it
 *                  goes: it belongs to another version of the settings, and Persistence
 *                  would have to reject it anyway.
 *
 *  \return         E_OK if exactly Size bytes were read
******************************************************************************************************************************************************/
StdReturnType Storage::read(byte* Buffer, size_t Size) const
{
    if((Buffer == nullptr) || (Size == 0u) || (Size > Capacity)) { return E_NOT_OK; }

    Preferences Store;
    if(!Store.begin(STORAGE_NAMESPACE, true)) { return E_NOT_OK; }

    const size_t StoredSize = Store.getBytesLength(STORAGE_KEY);
    if(StoredSize != Size) {
        Store.end();
        return E_NOT_OK;
    }

    const size_t Read = Store.getBytes(STORAGE_KEY, Buffer, Size);
    Store.end();

    if(Read != Size) { return E_NOT_OK; }

    return E_OK;
} /* read */


/******************************************************************************************************************************************************
  write()
******************************************************************************************************************************************************/
/*! \brief          Replaces the stored blob
 *
 *  \return         E_OK if the whole blob was stored
******************************************************************************************************************************************************/
StdReturnType Storage::write(const byte* Buffer, size_t Size)
{
    if((Buffer == nullptr) || (Size == 0u) || (Size > Capacity)) { return E_NOT_OK; }

    Preferences Store;
    if(!Store.begin(STORAGE_NAMESPACE, false)) { return E_NOT_OK; }

    const size_t Written = Store.putBytes(STORAGE_KEY, Buffer, Size);
    Store.end();

    if(Written != Size) { return E_NOT_OK; }

    return E_OK;
} /* write */


/******************************************************************************************************************************************************
  clear()
******************************************************************************************************************************************************/
/*! \brief          Forgets the stored blob
 *  \details        A store that held nothing counts as cleared, so calling this twice is
 *                  not an error.
 *
 *  \return         E_OK if nothing is stored afterwards
******************************************************************************************************************************************************/
StdReturnType Storage::clear()
{
    Preferences Store;
    if(!Store.begin(STORAGE_NAMESPACE, false)) { return E_NOT_OK; }

    const bool WasRemoved = Store.remove(STORAGE_KEY);
    const bool WasEmpty = !Store.isKey(STORAGE_KEY);
    Store.end();

    if(!WasRemoved && !WasEmpty) { return E_NOT_OK; }

    return E_OK;
} /* clear */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
