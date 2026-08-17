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

#include "Pixels.h"

#include <atomic>
#include <stddef.h>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* WebInterface configuration parameter */
/* How many browsers may watch at once. Each costs a socket in the server and a slot here;
   a wall clock does not need more, and the server's own limit has to be at least this. */
#define WEB_INTERFACE_MAX_CLIENTS                       4u

/* The longest command a frame may carry. Longer frames are refused rather than split, so
   half a command can never reach the parser. Well above the longest command in the
   reference, whose text overlay is the only one that comes close. */
#define WEB_INTERFACE_MAX_FRAME_LENGTH                  192u

#define WEB_INTERFACE_PORT                              80u

/* In scheduler ticks, so 5 * 10 ms is twenty frames a second. The buffer is 330 bytes, so
   that is under 7 KB/s and only while somebody is watching - sending on every tick would be
   five times that for a display that changes far more slowly. */
#define WEB_INTERFACE_FRAME_INTERVAL_TICKS              5u

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
    static constexpr size_t MaxClients{WEB_INTERFACE_MAX_CLIENTS};

    static constexpr size_t FrameSize{PIXELS_NUMBER_OF_LEDS * Pixel::getNumberOfColors()};

    /* What was last sent, so an unchanged display costs a comparison rather than a frame.
       Same shape as Persistence, and for the same reason: nothing has to remember to report
       a change, so nothing can forget. Pixels' own dirty flag cannot serve here - render()
       clears it on its way to the strip, and a client that was not due on that tick would
       never learn of the change. */
    byte LastFrame[FrameSize]{};
    byte FrameCountdown{0u};
    /* Set when a client arrives, so the next frame goes out even though nothing changed. A
       browser that connects to a standing display would otherwise wait for the next change -
       on a word clock, up to five minutes of empty panel. Written by the server's callback
       and cleared by the firmware's tick; a byte either way, so a lost race costs one
       interval.

       The list of clients itself is not kept here, unlike on the ESP32: AsyncWebSocket owns
       it, counts it and broadcasts to it, so a second copy could only ever disagree with
       the first. */
    std::atomic<bool> ForceFrame{false};

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
       WordclockSerial's line sink, not from the server's. */
    void broadcastLine(const char*);

    /* Sends the pixel buffer as one binary frame, at most every
       WEB_INTERFACE_FRAME_INTERVAL_TICKS and only when it changed. Called from the
       application's tick, after the display has been handed to the strip. */
    void broadcastFrame();

    /* For the socket callback, which runs in the network stack's context rather than the
       firmware's tick. Only the force flag is touched from there; who is connected is the
       socket's own business. */
    void onClientOpened() { ForceFrame.store(true, std::memory_order_release); }
};

#endif // _WEB_INTERFACE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
