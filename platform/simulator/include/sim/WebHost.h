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
/**     \file       WebHost.h
 *      \brief      Serves the clock's two pages from the simulator, on localhost
 *
 *      \details    What the ESP32's httpd and the Pico's ESPAsyncWebServer are to those
 *                  backends. There is no framework here to ask for one, so the socket, the
 *                  HTTP and the web socket are written out - which is worth it because it is
 *                  the difference between looking at the pages on a board and looking at them
 *                  from the binary that already draws the window.
 *
 *                  **Polled, not threaded.** Every backend on hardware runs its server in a
 *                  task of its own and pays for that with a lock-free buffer between the two.
 *                  Here task() is called from the same wxTimer that ticks the firmware, so a
 *                  browser's command is injected on the thread that reads it and a frame goes
 *                  out on the thread that gathered it. No second thread, and therefore nothing
 *                  to make thread-safe - which is what lets the port stay a wxString.
 *
 *                  What that costs is honest to state: a write blocks the tick. Everything
 *                  written here goes to loopback, where the largest of them - a 70 KB page -
 *                  fits in the socket buffer many times over, so it returns at once. A client
 *                  that stops reading entirely could still stall the window, which is a price
 *                  a development tool may pay and a clock on a wall may not.
 *
******************************************************************************************************************************************************/
#ifndef _WEB_HOST_H_
#define _WEB_HOST_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Not 80: a desktop needs root for that, and the point of this is to be started like any
   other program. The console prints the address on start-up. */
#define WEB_HOST_PORT                                   8080u

/* How many browsers may watch at once. A slot is a socket and nothing more. */
#define WEB_HOST_MAX_CLIENTS                            4u

/******************************************************************************************************************************************************
 *  C L A S S   W E B   H O S T
******************************************************************************************************************************************************/
class WebHost
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
#ifdef _WIN32
    /* What winsock2.h calls SOCKET, spelled out so that this header need not include it. */
    using SocketType = uintptr_t;
    static constexpr SocketType InvalidSocket{~SocketType{0u}};
#else
    using SocketType = int;
    static constexpr SocketType InvalidSocket{-1};
#endif

    static constexpr int Backlog{4};
    static constexpr size_t RequestChunkLength{4096u};
    static constexpr size_t FrameChunkLength{2048u};
    static constexpr unsigned RequestTimeoutMs{100u};
    static constexpr unsigned BodyTimeoutMs{2000u};
    static constexpr char WebSocketPath[]{"/ws"};
    static constexpr char UpdateRequestLine[]{"POST /update "};
    static constexpr char LengthHeader[]{"content-length:"};

    /* A slot holds a connection that has finished its handshake and is being broadcast to.
       Anything before that is a request being read and is gone by the end of the tick that
       read it. */
    SocketType Listener{InvalidSocket};
    SocketType Clients[WEB_HOST_MAX_CLIENTS];
    std::string Pending[WEB_HOST_MAX_CLIENTS];

    WebHost();

    void acceptClients();
    void readClient(size_t);
    size_t findFreeSlot() const;
    void dropClient(size_t);
    static bool serveConnection(SocketType);
    static bool answerRequest(SocketType, const std::string&);
    static void answerUpdate(SocketType, size_t);
    static StdReturnType sendAsset(SocketType, const std::string&);
    static SocketType openListener();
    static std::string readRequest(SocketType);
    static StdReturnType getPath(const std::string&, std::string&);
    static bool isUpdateRequest(const std::string&);
    static size_t getAnnouncedLength(const std::string&);
    static size_t getArrivedBodyLength(const std::string&);
    static size_t drainBody(SocketType, size_t, size_t);
    static void setNonBlocking(SocketType);
    static void setReceiveTimeout(SocketType, unsigned);
    static bool sendAll(SocketType, const char*, size_t);
    static void sendLineToClients(const char*);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WebHost& getInstance();

    /* Opens the listening socket and takes over the output lines. Answers E_NOT_OK when the
       port is taken, which leaves the simulator running exactly as it did before - a window
       and no browsers, which is what it was until now. */
    StdReturnType begin();

    /* Accepts what has arrived and answers it. Called from the firmware's tick. */
    void task();

    /* Closes every socket, for the way out. */
    void end();

    /* What WebTransport is implemented in terms of. */
    bool isListening() const;
    bool hasClients() const;
    void sendToEveryClient(bool IsBinary, const char* Payload, size_t Length);

    /* What WebResponseBody is implemented in terms of. */
    static void sendResponse(SocketType Socket, const char* Status, const char* ContentType,
                             const char* Body, size_t Length, bool IsGzipped);
};

#endif // _WEB_HOST_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
