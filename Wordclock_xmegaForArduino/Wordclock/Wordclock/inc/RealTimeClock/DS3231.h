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
#include "ClockDateTime.h"
#include "Wire.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* DS3231 configuration parameter */


/* DS3231 parameter */


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   T E M P L A T E
******************************************************************************************************************************************************/
class DS3231
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  
  
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte DeviceAddress{0x68u};
    static constexpr byte AddrSecond{0x00u};
    static constexpr byte AddrMinute{0x01u};
    static constexpr byte AddrHour{0x02u};
    static constexpr byte AddrWeekday{0x03u};
    static constexpr byte AddrDay{0x04u};
    static constexpr byte AddrMonthCentury{0x05u};
    static constexpr byte AddrYear{0x06u};
    static constexpr byte AddrControl{0x0Eu};
    static constexpr byte AddrStatus{0x0Fu};
    static constexpr byte AddrAgingOffset{0x10u};
    static constexpr byte AddrTmpMSB{0x11u};
    static constexpr byte AddrTmpLSB{0x12u};
        
    static constexpr byte StartAddrTime{0x00u};
    static constexpr byte StartAddrDate{0x03u};
    static constexpr byte TimeNumberOfBytes{0x03u};
    static constexpr byte DateNumberOfBytes{0x04u};
    static constexpr byte TimeDateNumberOfBytes{TimeNumberOfBytes + DateNumberOfBytes};
    
    // functions
	uint8_t convBinToBcd(uint8_t Value) { return Value + 6u * (Value / 10u); }
	uint8_t convBcdToBin(uint8_t Value) { return Value - 6u * (Value >> 4u); }
	
    ClockDateTime readDateTime() const {
        ClockDateTime dateTime;
        dateTime.setTime(readTime());
        Wire.read(); // dummy read for weekdays
        dateTime.setDate(readDate());
        return dateTime;
    }
    
    ClockDate readDate() const {
        ClockDate date;
        date.setDay(Wire.read());
        date.setMonth(static_cast<ClockDate::MonthType>(Wire.read()));
        date.setYear(Wire.read() + 2000u);
        return date;
    }
    
    ClockTime readTime() const {
        ClockTime time;
        time.setSecond(Wire.read());
        time.setMinute(Wire.read());
        time.setHour(Wire.read());
        return time;
    }
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr DS3231() { }
    ~DS3231() { }

	// get methods

	// set methods

	// methods
	ClockDateTime getDateTime() const
	{
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(StartAddrTime);
    	Wire.endTransmission();
        
        Wire.requestFrom(DeviceAddress, TimeDateNumberOfBytes);
    	return readDateTime();
	}
	
	ClockTime getTime() const
	{
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(StartAddrTime);
    	Wire.endTransmission();
    	
    	Wire.requestFrom(DeviceAddress, TimeNumberOfBytes);
        return readTime();
	}
	
	ClockDate getDate() const
	{
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(StartAddrDate);
    	Wire.endTransmission();
    	
        Wire.requestFrom(DeviceAddress, DateNumberOfBytes);
        return readDate();
	}
    
    void setDateTime(ClockDateTime DateTime) const {  }
    void setTime(ClockTime Time) const {  }
    void setDate(ClockDate Date) const {  }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
