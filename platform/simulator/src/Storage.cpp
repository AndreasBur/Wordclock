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
 *      \brief      Simulator implementation of the settings store, backed by a file
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/Storage.h"

#include <cstdio>

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Read()
******************************************************************************************************************************************************/
/*! \brief          Reads the stored blob
 *  \details        A file of a different length is refused rather than read as far as it
 *                  goes: it belongs to another build of the settings, and half of one is
 *                  worse than none.
 *
 *  \return         kEOk if exactly Size bytes were read
******************************************************************************************************************************************************/
/* Not static, though nothing in them reads a member: these stand in for the backends the
   clock really runs on, and there the same call reaches a peripheral through instance
   state. A stand-in that answered the check would have a different API from the thing it
   stands in for, which is the one property it exists to keep. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
StdReturnType Storage::Read(byte* buffer, size_t size) const
{
    if((buffer == nullptr) || (size == 0u) || (size > kCapacity)) { return kENotOk; }

    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
    if(file == nullptr) { return kENotOk; }

    const size_t read = std::fread(buffer, 1u, size, file);
    /* One byte more would mean the file outgrew what is being asked for, which is the
       same mismatch as one byte less. Only asked when the read delivered, because after a
       failed one the standard stops saying where the position is. */
    const bool is_at_end = (read == size) && (std::fgetc(file) == EOF);
    /* Nothing to report from closing a stream that was only read. */
    (void)std::fclose(file);

    if((read != size) || !is_at_end) { return kENotOk; }

    return kEOk;
} /* Read */


/******************************************************************************************************************************************************
  Write()
******************************************************************************************************************************************************/
/*! \brief          Replaces the stored blob
 *
 *  \return         kEOk if the whole blob reached the file
******************************************************************************************************************************************************/
StdReturnType Storage::Write(const byte* buffer, size_t size)
{
    if((buffer == nullptr) || (size == 0u) || (size > kCapacity)) { return kENotOk; }

    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "wb");
    if(file == nullptr) { return kENotOk; }

    const size_t written = std::fwrite(buffer, 1u, size, file);
    const bool is_flushed = (std::fclose(file) == 0);

    if((written != size) || !is_flushed) { return kENotOk; }

    return kEOk;
} /* Write */


/******************************************************************************************************************************************************
  Clear()
******************************************************************************************************************************************************/
/*! \brief          Forgets the stored blob
 *  \details        A store that was empty to begin with counts as cleared, so calling this
 *                  twice is not an error.
 *
 *  \return         kEOk if nothing is stored afterwards
******************************************************************************************************************************************************/
StdReturnType Storage::Clear()
{
    std::FILE* file = std::fopen(STORAGE_FILE_NAME, "rb");
    if(file == nullptr) { return kEOk; }
    (void)std::fclose(file);

    if(std::remove(STORAGE_FILE_NAME) != 0) { return kENotOk; }

    return kEOk;
} /* Clear */
// NOLINTEND(readability-convert-member-functions-to-static)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
