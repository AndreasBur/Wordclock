// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Serves the clock's two pages from the simulator, on localhost.
 *
 * What the ESP32's httpd and the Pico's ESPAsyncWebServer are to those backends. There is no
 * framework here to ask for one, so the socket, the HTTP and the web socket are written out - which
 * is worth it because it is the difference between looking at the pages on a board and looking at
 * them from the binary that already draws the window.
 *
 * **Polled, not threaded.** Every backend on hardware runs its server in a task of its own and pays
 * for that with a lock-free buffer between the two. Here Task() is called from the same wxTimer
 * that ticks the firmware, so a browser's command is injected on the thread that reads it and a
 * frame goes out on the thread that gathered it. No second thread, and therefore nothing to make
 * thread-safe - which is what lets the port stay a wxString.
 *
 * What that costs is honest to state: a write blocks the tick. Everything written here goes to
 * loopback, where the largest of them - a page, gzip-compressed - fits in the socket buffer many
 * times over, so it returns at once. A client that stops reading entirely could still stall the
 * window, which is a price a development tool may pay and a clock on a wall may not.
 */

#ifndef _WEB_HOST_H_
#define _WEB_HOST_H_

#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

/*!
 * \brief The TCP port the web host listens on, on loopback.
 *
 * Not 80: a desktop needs root for that, and the point of this is to be started like any other
 * program. The console prints the address on start-up.
 *
 * \note The address WebHost::Begin() prints and the message for a port that is taken spell out 8080
 *       as text, so they do not follow a change here.
 */
#define WEB_HOST_PORT                                   8080u

/*!
 * \brief How many browsers may watch at once, over a web socket each.
 *
 * A slot is a socket and nothing more. While every slot is taken, a new connection is closed
 * unread, a request for a page included.
 */
#define WEB_HOST_MAX_CLIENTS                            4u

/*!
 * \brief The web host: a small HTTP and web socket server on loopback, which serves the clock's two
 *        pages from WebFrontend and keeps the browsers that watch over a web socket.
 *
 * It answers these requests, one per connection:
 * - `/ws` with the web socket handshake; the connection then stays, as a client.
 * - `/commands` and `/display` with the JSON documents of WebFrontend::WriteCommands() and
 *   WebFrontend::WriteDisplay().
 * - The path of each file of WebFrontend::GetAsset() with that file.
 * - `POST /update` with a body: the body is read and thrown away, and the answer says whether
 *   it was large enough to be a firmware image. Nothing is installed.
 * - Every other path with a 404.
 *
 * The simulator's half of WebTransport.h, WebTransport and WebResponseBody, is implemented in
 * terms of IsListening(), HasClients(), SendToEveryClient() and SendResponse().
 *
 * It runs on the thread of the wx event loop, polled by Task() from the timer that ticks the
 * firmware, and so does everything it calls; nothing here is safe to call from another thread.
 */
class WebHost
{
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
    void ReadClient(size_t slot);
    size_t FindFreeSlot() const;
    void DropClient(size_t slot);
    static bool ServeConnection(SocketType socket);
    static bool AnswerRequest(SocketType socket, const std::string& request);
    static void AnswerUpdate(SocketType socket, size_t received);
    static StdReturnType SendAsset(SocketType socket, const std::string& path);
    static SocketType OpenListener();
    static std::string ReadRequest(SocketType socket);
    static StdReturnType GetPath(const std::string& request, std::string& path);
    static bool IsUpdateRequest(const std::string& request);
    static size_t GetAnnouncedLength(const std::string& request);
    static size_t GetArrivedBodyLength(const std::string& request);
    static size_t DrainBody(SocketType socket, size_t received, size_t announced);
    static void SetNonBlocking(SocketType socket);
    static void SetBlocking(SocketType socket);
    static void SetReceiveTimeout(SocketType socket, unsigned milliseconds);
    static bool SendAll(SocketType socket, const char* data, size_t length);
    static void SendLineToClients(const char* line);

  public:
    /*!
     * \brief Returns the one web host.
     *
     * It listens nowhere until Begin(), and has no clients until then either - which HasClients()
     * may be asked before Begin() ever ran.
     *
     * \return The web host.
     */
    static WebHost& GetInstance();

    /*!
     * \brief Opens the listening socket on loopback and takes over the output lines.
     *
     * Once the port is listening, the serial port's line sink is set, so that every finished
     * output line goes to the browsers too, and the address is printed on the port.
     *
     * A port that is taken is not fatal and must not be: somebody already has a simulator open,
     * and the second one is still a window worth having. It says so on the port and runs on, which
     * is the same shape the ESP32 backend's server failure has - fall back to what works rather
     * than refuse to start.
     *
     * Loopback only. This serves a firmware console with no password on it, and the one on a
     * clock is at least on somebody's own network; a desktop is not the place to open that to a
     * coffee shop.
     *
     * \return kEOk once the port is listening, or kENotOk if winsock does not start, on Windows, or
     *         the socket cannot be opened, bound or made to listen. Nothing is then listening and
     *         the line sink is left as it was, which leaves the simulator running exactly as it did
     *         before - a window and no browsers.
     */
    StdReturnType Begin();

    /*!
     * \brief Accepts what has arrived and answers it, and reads what the web sockets sent.
     *
     * Called from the firmware's tick, which is what makes every buffer here a plain one. A
     * request is read, answered and finished within the call unless it was a handshake, and a
     * handshake becomes a client that stays.
     *
     * One request per accepted connection: Connection: close goes out with every answer, so a
     * browser opens a socket per file. That is a page load's worth of connections on loopback and
     * buys the whole of the keep-alive bookkeeping not existing.
     *
     * A text frame from a client goes to WebFrontend::InjectCommand(). A close frame, or a
     * connection the browser has closed, drops the client; every other frame is read and ignored.
     * Nothing happens while the host is not listening.
     *
     * \note A new connection is read while Task() waits: until its headers have arrived, or until
     *       no byte has arrived for a receive timeout, which is longer for the body of an update. A
     *       request holds the tick for as long as its bytes keep arriving.
     */
    void Task();

    /*!
     * \brief Closes every socket, for the way out.
     *
     * Every client is dropped and the listener closed. The serial port's line sink stays set;
     * WebFrontend::BroadcastLine() sends nothing once the host no longer listens.
     */
    void End();

    /*! \brief Returns whether the listening socket is open. */
    bool IsListening() const;
    /*! \brief Returns whether a web socket client is connected. */
    bool HasClients() const;

    /*!
     * \brief Sends one web socket frame to every client.
     *
     * The frame is made once with WebSocket::EncodeFrame() and written to each client in turn. A
     * client whose send fails is not dropped here; Task() drops a client once it reads that the
     * connection was closed.
     *
     * \param[in] is_binary True for a binary frame, false for a text one.
     * \param[in] payload The bytes to send.
     * \param[in] length The number of bytes of `payload`.
     * \warning A client whose socket buffer is full holds the tick until it has room. On Windows
     *          the send fails there instead, and the client is left with part of the frame.
     */
    void SendToEveryClient(bool is_binary, const char* payload, size_t length);

    /*!
     * \brief Sends an HTTP response that closes the connection.
     *
     * Connection: close, because every connection but a web socket's is closed after its answer -
     * see Task(). The header carries `status`, `content_type`, a Content-Length of `length` and,
     * if `is_gzipped`, a Content-Encoding of gzip. If the header cannot be sent, the body is not
     * sent either. The connection is left open for the caller to close.
     *
     * What WebResponseBody is implemented in terms of.
     *
     * \param[in] socket The connection.
     * \param[in] status The status line's code and text, such as "200 OK".
     * \param[in] content_type The content type of the body.
     * \param[in] body The body.
     * \param[in] length The number of bytes of `body`.
     * \param[in] is_gzipped True if the body is compressed with gzip, which the header then says.
     * \pre `status` and `content_type` are short: the header is formatted into a buffer of 256
     *      characters, and a header that does not fit is not checked for.
     */
    static void SendResponse(SocketType socket, const char* status, const char* content_type,
                             const char* body, size_t length, bool is_gzipped);
};

#endif // _WEB_HOST_H_
