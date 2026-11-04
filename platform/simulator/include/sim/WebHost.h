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
 *                  Here Task() is called from the same wxTimer that ticks the firmware, so a
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
// Not 80: a desktop needs root for that, and the point of this is to be started like any
// other program. The console prints the address on start-up.
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
    static constexpr SocketType kInvalidSocket{~SocketType{0u}};
#else
    using SocketType = int;
    static constexpr SocketType kInvalidSocket{-1};
#endif

    static constexpr int kBacklog{4};
    static constexpr size_t kRequestChunkLength{4096u};
    static constexpr size_t kFrameChunkLength{2048u};
    static constexpr unsigned kRequestTimeoutMs{100u};
    static constexpr unsigned kBodyTimeoutMs{2000u};
    static constexpr char kWebSocketPath[]{"/ws"};
    static constexpr char kUpdateRequestLine[]{"POST /update "};
    static constexpr char kLengthHeader[]{"content-length:"};

    /*
     * A slot holds a connection that has finished its handshake and is being broadcast to.
     * Anything before that is a request being read and is gone by the end of the tick that
     * read it.
     */
    SocketType listener_{kInvalidSocket};
    SocketType clients_[WEB_HOST_MAX_CLIENTS];
    std::string pending_[WEB_HOST_MAX_CLIENTS];

    WebHost();

    void AcceptClients();
    void ReadClient(size_t);
    size_t FindFreeSlot() const;
    void DropClient(size_t);
    static bool ServeConnection(SocketType);
    static bool AnswerRequest(SocketType, const std::string&);
    static void AnswerUpdate(SocketType, size_t);
    static StdReturnType SendAsset(SocketType, const std::string&);
    static SocketType OpenListener();
    static std::string ReadRequest(SocketType);
    static StdReturnType GetPath(const std::string&, std::string&);
    static bool IsUpdateRequest(const std::string&);
    static size_t GetAnnouncedLength(const std::string&);
    static size_t GetArrivedBodyLength(const std::string&);
    static size_t DrainBody(SocketType, size_t, size_t);
    static void SetNonBlocking(SocketType);
    static void SetBlocking(SocketType);
    static void SetReceiveTimeout(SocketType, unsigned);
    static bool SendAll(SocketType, const char*, size_t);
    static void SendLineToClients(const char*);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static WebHost& GetInstance();

    /*
     * Opens the listening socket and takes over the output lines. Answers kENotOk when the
     * port is taken, which leaves the simulator running exactly as it did before - a window
     * and no browsers, which is what it was until now.
     */
    StdReturnType Begin();

    /* Accepts what has arrived and answers it. Called from the firmware's tick. */
    void Task();

    /* Closes every socket, for the way out. */
    void End();

    /* What WebTransport is implemented in terms of. */
    bool IsListening() const;
    bool HasClients() const;
    void SendToEveryClient(bool is_binary, const char* payload, size_t length);

    /* What WebResponseBody is implemented in terms of. */
    static void SendResponse(SocketType socket, const char* status, const char* content_type,
                             const char* body, size_t length, bool is_gzipped);
};

#endif // _WEB_HOST_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
