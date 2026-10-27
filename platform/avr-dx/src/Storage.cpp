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
 *      \brief      Where the settings survive a restart, in the on-chip EEPROM, see Storage.h
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <avr/eeprom.h>
#include <avr/io.h>

#include "Arduino.h"
#include "Storage.h"

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

/* The length byte, and the blob behind it. */
constexpr uint16_t LengthAddress{STORAGE_EEPROM_ADDRESS};
constexpr uint16_t DataAddress{STORAGE_EEPROM_ADDRESS + 1u};

/* The part's EEPROM has to hold both, or a write would run off its end and silently do
   nothing. */
static_assert((STORAGE_EEPROM_ADDRESS + 1u + STORAGE_CAPACITY) <= EEPROM_SIZE,
              "Storage: the capacity does not fit in this device's EEPROM");

/* avr-libc's EEPROM functions take the offset into the EEPROM in a pointer's clothes; it
   never points at RAM and is never dereferenced as such, so the cast is the interface. */
// NOLINTNEXTLINE(performance-no-int-to-ptr)
byte* toEepromAddress(uint16_t Offset) { return reinterpret_cast<byte*>(Offset); }

} // namespace

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/* Instance methods, though some of them read no member: they are the interface the
   firmware core calls through getInstance(), and every backend - the simulator's
   stand-ins included - offers it that way. Whether this backend's implementation
   happens to need a member is its own detail, and answering the check would carry
   that detail into the interface. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          Hands back the stored blob, if it is the one the caller expects
 *  \details        A stored length that differs from the one asked for is a blob from an
 *                  older layout. Refused rather than handed over short: what is inside is
 *                  the core's structure, and giving it fewer bytes than it reads would let
 *                  it interpret whatever follows in memory.
******************************************************************************************************************************************************/
StdReturnType Storage::read(byte* Data, size_t Size) const
{
    if(Data == nullptr) { return E_NOT_OK; }
    if((Size == 0u) || (Size > Capacity)) { return E_NOT_OK; }

    const byte StoredLength = eeprom_read_byte(toEepromAddress(LengthAddress));

    if(StoredLength != static_cast<byte>(Size)) { return E_NOT_OK; }

    eeprom_read_block(Data, toEepromAddress(DataAddress), Size);

    return E_OK;
} /* read */


/******************************************************************************************************************************************************
  write()
******************************************************************************************************************************************************/
/*! \brief          Stores one blob
 *  \details        The length goes in last. Until it does, a restart part way through finds
 *                  the previous length in front of a half-written blob - which the core's
 *                  checksum rejects. Writing the length first would instead leave a blob
 *                  that announces itself as complete while it is not.
******************************************************************************************************************************************************/
StdReturnType Storage::write(const byte* Data, size_t Size)
{
    if(Data == nullptr) { return E_NOT_OK; }
    if((Size == 0u) || (Size > Capacity)) { return E_NOT_OK; }

    /* update rather than write: it leaves bytes that already match untouched, and the
       settings are saved on every change. */
    eeprom_update_block(Data, toEepromAddress(DataAddress), Size);
    eeprom_update_byte(toEepromAddress(LengthAddress), static_cast<byte>(Size));

    return E_OK;
} /* write */


/******************************************************************************************************************************************************
  clear()
******************************************************************************************************************************************************/
/*! \brief          Forgets what was stored
 *  \details        Only the length is reset, not the blob behind it: the length is what
 *                  read() consults, so the blob is unreachable either way, and erasing it
 *                  would spend a hundred and twenty-eight write cycles to no end.
******************************************************************************************************************************************************/
StdReturnType Storage::clear()
{
    eeprom_update_byte(toEepromAddress(LengthAddress), LengthEmpty);

    return E_OK;
} /* clear */
// NOLINTEND(readability-convert-member-functions-to-static)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
