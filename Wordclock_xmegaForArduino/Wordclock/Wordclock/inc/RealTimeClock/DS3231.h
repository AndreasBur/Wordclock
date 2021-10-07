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
#include "DS3231Regs.h"
#include "Bit.h"
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
	uint8_t convBinToBcd(uint8_t Value) const { return Value + 6u * (Value / 10u); }
	uint8_t convBcdToBin(uint8_t Value) const { return Value - 6u * (Value >> 4u); }
    uint8_t convWeekday(WeekdayType Weekday) const {
        return Weekday == 0u ? 7 : Weekday;
    }
        
    byte readRegister(byte RegAddress) const {
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(RegAddress);
    	Wire.endTransmission();
        
        Wire.requestFrom(RegAddress, 1u);
        return Wire.read();
    }
    
    void writeRegister(byte RegAddress, byte Value) const {
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(RegAddress);
        Wire.write(Value);
    	Wire.endTransmission();
    }
	
    ClockDateTime readDateTime() const {
        ClockDateTime dateTime;
        dateTime.setTime(readTime());
        Wire.read(); // dummy read for weekdays
        dateTime.setDate(readDate());
        return dateTime;
    }
    
    ClockDate readDate() const {
        ClockDate date;
        date.setDay(convBcdToBin(Wire.read()));
        date.setMonth(readMonth());
        date.setYear(convBcdToBin(Wire.read()) + 2000u);
        return date;
    }
    
    MonthType readMonth() const {
        uint8_t monthRaw = Bit(Wire.read()).readBits(DS3231Regs::MonthBM);
        return static_cast<MonthType>(convBcdToBin(monthRaw));
    }
    
    ClockTime readTime() const {
        ClockTime time;
        time.setSecond(convBcdToBin(Wire.read()));
        time.setMinute(convBcdToBin(Wire.read()));
        time.setHour(convBcdToBin(Wire.read()));
        return time;
    }
    
    void writeDateTime(ClockDateTime DateTime) const {
        writeTime(DateTime.getTime());
        writeDate(DateTime.getDate());
    }
    
    void writeTime(ClockTime Time) const {
        Wire.write(convBinToBcd(Time.getSecond()));
        Wire.write(convBinToBcd(Time.getMinute()));
        Wire.write(convBinToBcd(Time.getHour()));
    }
    
    void writeDate(ClockDate Date) const {
        Wire.write(convWeekday(Date.getWeekday()));
        Wire.write(convBinToBcd(Date.getDay()));
        Wire.write(convBinToBcd(Date.getMonth()));
        Wire.write(convBinToBcd(Date.getYear() - 2000u));
    }
    
    void enableOscillator() const {
        byte status = readRegister(DS3231Regs::AddrStatus);
        Bit(status).writeBit(DS3231Regs::StatusOscillatorStopFlagBP, 
            DS3231Regs::StatusOscillatorStopFlagRunning);
        writeRegister(DS3231Regs::AddrStatus, status);
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
    	Wire.write(DS3231Regs::StartAddrTime);
    	Wire.endTransmission();
        
        Wire.requestFrom(DeviceAddress, TimeDateNumberOfBytes);
    	return readDateTime();
	}
	
	ClockTime getTime() const
	{
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(DS3231Regs::StartAddrTime);
    	Wire.endTransmission();
    	
    	Wire.requestFrom(DeviceAddress, TimeNumberOfBytes);
        return readTime();
	}
	
	ClockDate getDate() const
	{
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(DS3231Regs::StartAddrDate);
    	Wire.endTransmission();
    	
        Wire.requestFrom(DeviceAddress, DateNumberOfBytes);
        return readDate();
	}
    
    void setDateTime(ClockDateTime DateTime) const
    { 
    	Wire.beginTransmission(DeviceAddress);
    	Wire.write(DS3231Regs::StartAddrTime);
        writeDateTime(DateTime);
    	Wire.endTransmission();
        enableOscillator();
    }
    
    void setTime(ClockTime Time) const
    {
        Wire.beginTransmission(DeviceAddress);
        Wire.write(DS3231Regs::StartAddrTime);
        writeTime(Time);
        Wire.endTransmission();
        enableOscillator();
    }
    
    void setDate(ClockDate Date) const
    {
        Wire.beginTransmission(DeviceAddress);
        Wire.write(DS3231Regs::StartAddrDate);
        writeDate(Date);
        Wire.endTransmission();
        enableOscillator();
    }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
