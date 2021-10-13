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
/**     \file       DS3231.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _DS3231_H_
#define _DS3231_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Bit.h"
#include "ClockDateTime.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DS3231 configuration parameter */


/* DS3231 parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   D S 3 2 3 1
******************************************************************************************************************************************************/
class DS3231
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    using HourType = ClockDateTime::HourType;
    using minutetype = ClockDateTime::MinuteType;
    using SecondType = ClockDateTime::SecondType;

    using YearType = ClockDateTime::YearType;
    using MonthType = ClockDateTime::MonthType;
    using DayType = ClockDateTime::DayType;
    using WeekdayType = ClockDateTime::WeekdayType;
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte DeviceAddress{0x68u};       
    static constexpr byte TimeNumberOfBytes{0x03u};
    static constexpr byte DateNumberOfBytes{0x04u};
    static constexpr byte TimeDateNumberOfBytes{TimeNumberOfBytes + DateNumberOfBytes};
    
    // functions
	byte convBinToBcd(uint8_t Value) const { return Value + 6u * (Value / 10u); }
	byte convBcdToBin(uint8_t Value) const { return Value - 6u * (Value >> 4u); }
    byte convWeekday(WeekdayType Weekday) const { return static_cast<byte>(Weekday) == 0u ? 7u : static_cast<byte>(Weekday); }

    byte readRegister(byte RegAddress) const;
    void writeRegister(byte RegAddress, byte Value) const;
	
    ClockDateTime readDateTime() const;    
    ClockDate readDate() const;
    MonthType readMonth() const;
    ClockTime readTime() const;    
    
    void writeDateTime(ClockDateTime DateTime) const;    
    void writeTime(ClockTime Time) const;    
    void writeDate(ClockDate Date) const;    
    
    void enableOscillator() const;
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DS3231() { }
    ~DS3231() { }

	// get methods

	// set methods

	// methods
	ClockDateTime getDateTime() const;
	ClockTime getTime() const;
    ClockDate getDate() const;
        
    void setDateTime(ClockDateTime DateTime) const;    
    void setTime(ClockTime Time) const;
    void setDate(ClockDate Date) const;
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
