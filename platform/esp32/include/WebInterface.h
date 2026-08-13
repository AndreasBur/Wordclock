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
    static constexpr int NoClient{-1};

    /* Written only by the HTTP server's task - on a handshake and on a close - and read
       only by the firmware's task when it broadcasts. One writer is what makes the plain
       atomics enough here, the same reasoning as for WordclockSerial's ring buffer. */
    std::atomic<int> Clients[MaxClients];

    // functions
    void addClient(int);
    void removeClient(int);

    WebInterface() {
        for(size_t Slot = 0u; Slot < MaxClients; Slot++) { Clients[Slot].store(NoClient); }
    }
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

    /* For the socket handler, which runs in the server's task. */
    void onClientOpened(int Descriptor) { addClient(Descriptor); }
    void onClientClosed(int Descriptor) { removeClient(Descriptor); }
};

#endif // _WEB_INTERFACE_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
