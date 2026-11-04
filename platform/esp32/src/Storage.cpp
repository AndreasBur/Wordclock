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
/* Ahead of Storage.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <Preferences.h>

#include "Storage.h"

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  Read()
******************************************************************************************************************************************************/
/*! \brief          Reads the stored blob
 *  \details        A blob of a different length is refused rather than read as far as it
 *                  goes: it belongs to another version of the settings, and Persistence
 *                  would have to reject it anyway.
 *
 *  \return         kEOk if exactly Size bytes were read
******************************************************************************************************************************************************/
StdReturnType Storage::Read(byte* buffer, size_t size) const
{
    if((buffer == nullptr) || (size == 0u) || (size > kCapacity)) { return kENotOk; }

    Preferences store;
    if(!store.begin(STORAGE_NAMESPACE, true)) { return kENotOk; }

    const size_t stored_size = store.getBytesLength(STORAGE_KEY);
    if(stored_size != size) {
        store.end();
        return kENotOk;
    }

    const size_t read = store.getBytes(STORAGE_KEY, buffer, size);
    store.end();

    if(read != size) { return kENotOk; }

    return kEOk;
} /* Read */


/******************************************************************************************************************************************************
  Write()
******************************************************************************************************************************************************/
/*! \brief          Replaces the stored blob
 *
 *  \return         kEOk if the whole blob was stored
******************************************************************************************************************************************************/
StdReturnType Storage::Write(const byte* buffer, size_t size)
{
    if((buffer == nullptr) || (size == 0u) || (size > kCapacity)) { return kENotOk; }

    Preferences store;
    if(!store.begin(STORAGE_NAMESPACE, false)) { return kENotOk; }

    const size_t written = store.putBytes(STORAGE_KEY, buffer, size);
    store.end();

    if(written != size) { return kENotOk; }

    return kEOk;
} /* Write */


/******************************************************************************************************************************************************
  Clear()
******************************************************************************************************************************************************/
/*! \brief          Forgets the stored blob
 *  \details        A store that held nothing counts as cleared, so calling this twice is
 *                  not an error.
 *
 *  \return         kEOk if nothing is stored afterwards
******************************************************************************************************************************************************/
StdReturnType Storage::Clear()
{
    Preferences store;
    if(!store.begin(STORAGE_NAMESPACE, false)) { return kENotOk; }

    const bool was_removed = store.remove(STORAGE_KEY);
    const bool was_empty = !store.isKey(STORAGE_KEY);
    store.end();

    if(!was_removed && !was_empty) { return kENotOk; }

    return kEOk;
} /* Clear */
// NOLINTEND(readability-convert-member-functions-to-static)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
