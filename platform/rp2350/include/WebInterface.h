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
/**     \file       WebInterface.h
 *      \brief      Serves the console page and bridges its web socket to Serial
 *
 *      \details    The page arrives as one gzipped blob out of flash, and its socket is
 *                  wired straight to WordclockSerial: what a browser sends is injected as
 *                  characters, and every line the firmware finishes is sent back as one
 *                  frame. There is no protocol here beyond that - the commands are the
 *                  ones in docs/serial-commands.md, parsed by the same parser as the wire.
 *
 *                  Nothing in this class ever touches the firmware. The HTTP server runs
 *                  in its own task, and the only thing that crosses over is the injected
 *                  character stream, which WordclockSerial takes from one producer without
 *                  a lock. Answers travel the other way, sent from the firmware's own task.
 *
******************************************************************************************************************************************************/
#ifndef _WEB_INTERFACE_H_
#define _WEB_INTERFACE_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

/* What a browser is *sent* lives there, together with the frame rate limit and the longest
   command a socket frame may carry - shared with the ESP32 backend rather than kept twice. */
#include "WebFrontend.h"

#include <stddef.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WebInterface configuration parameter */
/* There is no client limit here, and the one that used to stand in this place was never one:
   it sized a constant nothing read, because AsyncWebSocket owns the list and enforces its own
   ceiling. The ESP32 keeps its WEB_INTERFACE_MAX_CLIENTS because a table there really is that
   long and its server is told the same number. */
#define WEB_INTERFACE_PORT                              80u

/* What the clock answers to on the local network, so no address has to be looked up. */
#define WEB_INTERFACE_HOSTNAME                          "wordclock"

/******************************************************************************************************************************************************
 *  C L A S S   W E B   I N T E R F A C E
******************************************************************************************************************************************************/
class WebInterface
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Nothing is kept here at all any more. The list of clients never was - AsyncWebSocket
       owns it, counts it and broadcasts to it, so a second copy could only ever disagree with
       the first - and the frame bookkeeping that used to sit beside it is WebFrontend's now,
       a rate limit and a comparison being the same on any backend. */
    WebInterface() { }
    ~WebInterface() { }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WebInterface& getInstance() {
        static WebInterface SingletonInstance;
        return SingletonInstance;
    }

    // methods
    /* Starts the server and registers itself as the sink for finished output lines. Safe to
       call before the network is up: the server listens on whatever address arrives later.
       Returns E_NOT_OK when the server could not be started, which leaves the clock running
       on the UART alone rather than failing to boot. */
    StdReturnType begin();

    /* Sends one finished line to every open socket. Called from the firmware's task through
       WordclockSerial's line sink, not from the server's. Both of these forward into
       WebFrontend, which is where the widening, the rate limit and the comparison live; they
       stay on this class because the application and the line sink call them by this name. */
    void broadcastLine(const char*);

    /* Sends the pixel buffer as one binary frame, at most every
       WEB_FRONTEND_FRAME_INTERVAL_TICKS and only when it changed. Called from the
       application's tick, after the display has been handed to the strip. */
    void broadcastFrame();

    /* For the socket callback, which runs in the network stack's context rather than the
       firmware's tick. Only the force flag is touched from there; who is connected is the
       socket's own business. */
    void onClientOpened() { WebFrontend::getInstance().onClientOpened(); }
};

#endif // _WEB_INTERFACE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
