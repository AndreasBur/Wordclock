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
 *      \brief      The settings blob, as one file in LittleFS
 *
 *      \details    Where the ESP32 backend has the IDF's key-value store, this core offers
 *                  a filesystem and an EEPROM emulation. The filesystem wins on one point
 *                  that Storage's contract turns on: a file carries its own length, and
 *                  read() has to refuse a blob of the wrong size because it belongs to
 *                  another version of the settings. The emulation would need that length
 *                  stored alongside and kept in step by hand.
 *
 *                  The namespace becomes a directory and the key a file inside it, so the
 *                  two names the contract already carries still say what they said.
 *
 *                  The filesystem is mounted on the first call rather than in an init():
 *                  the contract has no init(), and Persistence reads before it ever
 *                  writes.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of Storage.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <LittleFS.h>

#include "Storage.h"

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

constexpr char BlobPath[]{"/" STORAGE_NAMESPACE "/" STORAGE_KEY};

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* Mounted once and remembered, including the failure: a filesystem that would not come up
   will not come up on the next call either, and retrying on every read would turn a fault
   into a stall. Formatting on a failed mount is what makes a first boot work on a chip
   whose flash has never held one. */
bool isFilesystemReady()
{
    static bool WasTried{false};
    static bool IsMounted{false};

    if(!WasTried) {
        WasTried = true;
        IsMounted = LittleFS.begin();
    }

    return IsMounted;
}

/* The directory the blob lives in. mkdir() on an existing one is not an error worth
   reporting - the open that follows is what decides. */
void ensureDirectory()
{
    LittleFS.mkdir("/" STORAGE_NAMESPACE);
}

} // namespace

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
    if(!isFilesystemReady()) { return E_NOT_OK; }

    File Blob = LittleFS.open(BlobPath, "r");
    if(!Blob) { return E_NOT_OK; }

    if(Blob.size() != Size) {
        Blob.close();
        return E_NOT_OK;
    }

    const size_t Read = Blob.read(Buffer, Size);
    Blob.close();

    if(Read != Size) { return E_NOT_OK; }

    return E_OK;
} /* read */


/******************************************************************************************************************************************************
  write()
******************************************************************************************************************************************************/
/*! \brief          Replaces the stored blob
 *  \details        Opened for writing rather than appended to, so a shorter blob cannot
 *                  leave the tail of a longer one behind it - which read() would then
 *                  refuse for a length it never wrote.
 *
 *  \return         E_OK if the whole blob was stored
******************************************************************************************************************************************************/
StdReturnType Storage::write(const byte* Buffer, size_t Size)
{
    if((Buffer == nullptr) || (Size == 0u) || (Size > Capacity)) { return E_NOT_OK; }
    if(!isFilesystemReady()) { return E_NOT_OK; }

    ensureDirectory();

    File Blob = LittleFS.open(BlobPath, "w");
    if(!Blob) { return E_NOT_OK; }

    const size_t Written = Blob.write(Buffer, Size);
    Blob.close();

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
    if(!isFilesystemReady()) { return E_NOT_OK; }

    if(!LittleFS.exists(BlobPath)) { return E_OK; }

    if(!LittleFS.remove(BlobPath)) { return E_NOT_OK; }

    return E_OK;
} /* clear */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
