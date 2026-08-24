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
 *      \details    The backend half of the contract in
 *                  firmware/inc/Communication/WebFrontend/WebFrontend.h: where a response
 *                  body goes, and where a line or a frame goes. Everything above it - the
 *                  catalog, the letter grid, the rate limit, the frame comparison - is the
 *                  firmware's and is not repeated here.
 *
 *                  Deliberately free of framework headers, and it has to be: the firmware
 *                  includes this, and this backend's Arduino.h binds Serial to a macro that
 *                  must not reach the framework's own headers - which is why every platform
 *                  source includes them first. A header pulled into firmware sources cannot
 *                  make that promise, so the library's two request classes are named by
 *                  forward declaration instead.
 *
 *                  Both classes are *defined* in WebInterface.cpp rather than in a source of
 *                  their own, because what they reach - the server and its web socket - is
 *                  that file's and stays that file's.
 *
******************************************************************************************************************************************************/
#ifndef _WEB_TRANSPORT_H_
#define _WEB_TRANSPORT_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

#include <stddef.h>

class AsyncWebServerRequest;
class AsyncResponseStream;

/******************************************************************************************************************************************************
 *  C L A S S   W E B   R E S P O N S E   B O D Y
******************************************************************************************************************************************************/
/* One route's answer, being written. This server is pulled from rather than pushed to: the
   stream collects what is written and finish() is what hands it to the request, after which
   the library sends it. */
class WebResponseBody
{
  private:
    AsyncWebServerRequest* Request;
    AsyncResponseStream* Stream;

  public:
    WebResponseBody(AsyncWebServerRequest* sRequest, const char* ContentType);

    void write(const char* Data, size_t Length);
    void finish();
};

/******************************************************************************************************************************************************
 *  C L A S S   W E B   T R A N S P O R T
******************************************************************************************************************************************************/
/* Where a finished line or a display frame leaves for whoever is watching. AsyncWebSocket
   owns its client list, counts it and broadcasts to it, so hasClients() is that count and
   there is no table here that could disagree with it. */
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
