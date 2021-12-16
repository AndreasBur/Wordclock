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
/**     \file       Overlay.h
 *      \brief      
 *
 *      \details    
 *                  
******************************************************************************************************************************************************/
#ifndef _OVERLAY_H_
#define _OVERLAY_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"


/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Overlay configuration parameter */


/* Overlay parameter */



/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   O V E R L A Y
******************************************************************************************************************************************************/
class Overlay
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
  
/******************************************************************************************************************************************************
 *  P R O T E C T E D   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  protected:
    byte PeriodInMinutes{1u};
    byte EnduranceInSeconds{1u};
    byte Month{1u};
    byte Day{1u};
    byte ValidInDays{1u};
    bool IsActive{false};
  
    // functions
    constexpr Overlay() { }
    ~Overlay() { }

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
  
/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:

	// get methods
    byte getPeriodInMinutes() const { return PeriodInMinutes; }
    byte getEnduranceInSeconds() const { return EnduranceInSeconds; }
    byte getMonth() const { return Month; }
    byte getDay() const { return Day; }
    byte getValidInDays() const { return ValidInDays; }
    bool getIsActive() const { return IsActive; }

	// set methods
    void setPeriodInMinutes(byte sPeriodInMinutes) { PeriodInMinutes = sPeriodInMinutes; }
    void setEnduranceInSeconds(byte sEnduranceInSeconds) { EnduranceInSeconds = sEnduranceInSeconds; }
    void setMonth(byte sMonth) { Month = sMonth; }
    void setDay(byte sDay) { Day = sDay; }
    void setValidInDays(byte sValidInDays) { ValidInDays = sValidInDays; }
    void setIsActive(bool sIsActive) { IsActive = sIsActive; }

	// methods
    void enableIsActive() { IsActive = true; }
    void disableIsActive() { IsActive = false; }
};

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/