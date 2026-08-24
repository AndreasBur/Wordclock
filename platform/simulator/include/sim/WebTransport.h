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
/**     \file       WebTransport.h
 *      \brief      What WebFrontend needs from this backend's server
 *
 *      \details    The simulator's half of the contract in
 *                  firmware/inc/Communication/WebFrontend/WebFrontend.h - the third one, after
 *                  the ESP32's and the RP2350's, and the one that needs no board.
 *
 *                  What sits behind it is WebHost, some four hundred lines of socket, HTTP and
 *                  web socket written out here because a desktop build has no framework to ask.
 *                  That is the price of this direction, and what it buys is that the pages a
 *                  clock serves can be opened from the same binary that draws the window.
 *
******************************************************************************************************************************************************/
#ifndef _WEB_TRANSPORT_H_
#define _WEB_TRANSPORT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
/* Where this backend's `byte` comes from - the two on hardware get it from the Arduino core,
   and this header is reached before that one on the way in from WebHost.cpp. */
#include "arduino/Types.h"

#include <stddef.h>
#include <string>

/******************************************************************************************************************************************************
 *  C L A S S   W E B   R E S P O N S E   B O D Y
******************************************************************************************************************************************************/
/* One route's answer, being written. Collected whole rather than pushed out in chunks: this
   server answers on loopback and the largest thing it writes is a page out of flash, so a
   length up front is simpler than a chunked encoding and is what finish() sends. */
class WebResponseBody
{
  private:
    int Socket;
    const char* ContentType;
    std::string Collected;

  public:
    WebResponseBody(int sSocket, const char* sContentType);

    void write(const char* Data, size_t Length);
    void finish();
};

/******************************************************************************************************************************************************
 *  C L A S S   W E B   T R A N S P O R T
******************************************************************************************************************************************************/
/* Where a finished line or a display frame leaves for whoever is watching. The client list is
   WebHost's, since nothing here comes with one. */
class WebTransport
{
  public:
    static WebTransport& getInstance();

    bool isListening() const;
    bool hasClients() const;
    void sendText(const char* Text, size_t Length);
    void sendBinary(const byte* Bytes, size_t Length);
};

#endif // _WEB_TRANSPORT_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
