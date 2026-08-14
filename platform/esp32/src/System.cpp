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
/**     \file       System.cpp
 *      \brief      What the clock knows about the machine it runs on
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* Ahead of System.h, which pulls in the Arduino.h that binds Serial to a macro. */
#include <WiFi.h>

#include "System.h"
#include "WordclockConfiguration.h"

#include <string.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

constexpr uint32_t MillisecondsPerMinute{60u * 1000u};
constexpr uint32_t BytesPerKibibyte{1024u};

bool isNetworkJoined()
{
    return WiFi.status() == WL_CONNECTED;
}

/* Copies into the caller's buffer only if the whole string fits, so a value that grew
   past what the field was sized for is reported as no value rather than as half of one. */
StdReturnType copyIfItFits(const char* Value, char* String, size_t Capacity)
{
    String[0u] = STD_NULL_CHARACTER;

    if(strlen(Value) >= Capacity) { return E_NOT_OK; }

    strcpy(String, Value);
    return E_OK;
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getUptimeInMinutes()
******************************************************************************************************************************************************/
/*! \brief          Minutes since the last start
 *  \details        millis() wraps after forty-nine days, and the minutes after
 *                  forty-five - so the clock that has been up longest reports the least.
 *                  Left as it is rather than carried in a wider counter: a word clock that
 *                  has run for a month and a half has answered the question this field is
 *                  asked for.
 *
 *  \return         E_OK
******************************************************************************************************************************************************/
StdReturnType System::getUptimeInMinutes(uint16_t& Uptime) const
{
    Uptime = static_cast<uint16_t>(millis() / MillisecondsPerMinute);
    return E_OK;
} /* getUptimeInMinutes */


/******************************************************************************************************************************************************
  getFreeMemoryInKibibytes()
******************************************************************************************************************************************************/
StdReturnType System::getFreeMemoryInKibibytes(uint16_t& FreeMemory) const
{
    FreeMemory = static_cast<uint16_t>(ESP.getFreeHeap() / BytesPerKibibyte);
    return E_OK;
} /* getFreeMemoryInKibibytes */


/******************************************************************************************************************************************************
  getNetworkAddress()
******************************************************************************************************************************************************/
/*! \brief          The address the clock answers on
 *
 *  \return         E_OK if the clock has joined a network and the address fits the buffer
******************************************************************************************************************************************************/
StdReturnType System::getNetworkAddress(char* String) const
{
    String[0u] = STD_NULL_CHARACTER;

    if(!isNetworkJoined()) { return E_NOT_OK; }

    return copyIfItFits(WiFi.localIP().toString().c_str(), String, AddressStringLength);
} /* getNetworkAddress */


/******************************************************************************************************************************************************
  getLinkQuality()
******************************************************************************************************************************************************/
/*! \brief          The received signal strength in dBm
 *
 *  \return         E_OK if the clock has joined a network
******************************************************************************************************************************************************/
StdReturnType System::getLinkQuality(char* String) const
{
    String[0u] = STD_NULL_CHARACTER;

    if(!isNetworkJoined()) { return E_NOT_OK; }

    itoa(static_cast<int>(WiFi.RSSI()), String, 10);
    return E_OK;
} /* getLinkQuality */


/******************************************************************************************************************************************************
  performPendingRestart()
******************************************************************************************************************************************************/
/*! \brief          Carries out a restart that was asked for
 *  \details        From the application's tick, which is after the command's answer has
 *                  been printed and after the web interface has sent it on. Serial is
 *                  flushed first: the UART writes in the background, and a controller that
 *                  restarts with bytes still in that buffer cuts its own answer in half.
******************************************************************************************************************************************************/
void System::performPendingRestart()
{
    if(!RestartPending) { return; }

    Serial.flush();
    ESP.restart();
} /* performPendingRestart */


/******************************************************************************************************************************************************
  resynchroniseTime()
******************************************************************************************************************************************************/
/*! \brief          Asks the network for the time again
 *  \details        The same call the application makes at start-up rather than a reach
 *                  into SNTP: installing the servers again is what restarts the query, and
 *                  it keeps the zone rule in one place - a second installation of the
 *                  servers alone would leave the rule to be repeated here.
 *
 *  \return         E_OK if there is a network to ask over
******************************************************************************************************************************************************/
StdReturnType System::resynchroniseTime()
{
    if(!isNetworkJoined()) { return E_NOT_OK; }

    configTzTime(WORDCLOCK_TIMEZONE, WORDCLOCK_NTP_SERVER_PRIMARY, WORDCLOCK_NTP_SERVER_SECONDARY);
    return E_OK;
} /* resynchroniseTime */


/******************************************************************************************************************************************************
  reconnectNetwork()
******************************************************************************************************************************************************/
/*! \brief          Joins the network again
 *  \details        Refused where no network was ever configured, which is a clock running
 *                  on the serial line alone - there is nothing to reconnect to, and
 *                  answering E_OK would say there was.
 *
 *  \return         E_OK if the attempt was started
******************************************************************************************************************************************************/
StdReturnType System::reconnectNetwork()
{
    /* An unset SSID is a configuration that was never filled in, the same test the
       application makes before it joins at all. */
    if(sizeof(WORDCLOCK_WIFI_SSID) <= 1u) { return E_NOT_OK; }

    if(!WiFi.reconnect()) { return E_NOT_OK; }

    return E_OK;
} /* reconnectNetwork */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
