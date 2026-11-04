// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Serves the clock's two pages from the simulator, on localhost.
 */

#include "sim/WebHost.h"

#if (WEB_FRONTEND_SUPPORT == STD_ON)

#include "sim/WebSocket.h"
#include "sim/WebTransport.h"
#include "sim/WordclockSerial.h"

#include "WebFrontend.h"

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# define CLOSE_SOCKET closesocket
#else
# include <arpa/inet.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <poll.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
# define CLOSE_SOCKET close
#endif

/*! \brief          WebHost constructor
 *  \details        A host that listens nowhere until Begin(), and has no clients until then
 *                  either - which HasClients() may be asked before Begin() ever ran.
 */
WebHost::WebHost()
{
    for(size_t slot = 0u; slot < WEB_HOST_MAX_CLIENTS; slot++) { clients_[slot] = kInvalidSocket; }
}

WebHost& WebHost::GetInstance()
{
    static WebHost singleton_instance;
    return singleton_instance;
}

/*! \brief          Opens the listening socket and takes over the output lines
 *
 *  \return         kEOk once the port is listening
 *
 *  \details        A port that is taken is not fatal and must not be: somebody already has a
 *                  simulator open, and the second one is still a window worth having. It says
 *                  so and runs on, which is the same shape the ESP32 backend's server failure
 *                  has - fall back to what works rather than refuse to start.
 */
StdReturnType WebHost::Begin()
{
#ifdef _WIN32
    WSADATA winsock_data;
    if(WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) { return kENotOk; }
#endif

    listener_ = OpenListener();
    if(listener_ == kInvalidSocket) { return kENotOk; }

    WordclockSerial::GetInstance().SetLineSink(SendLineToClients);

    Serial.println("Web: http://localhost:8080/");

    return kEOk;
}

/*! \brief          Accepts what has arrived and answers it
 *
 *  \details        Called from the firmware's tick, which is what makes every buffer here a
 *                  plain one. A request is read, answered and finished within the call unless
 *                  it was a handshake, and a handshake becomes a client that stays.
 *
 *                  One request per accepted connection: Connection: close goes out with every
 *                  answer, so a browser opens a socket per file. That is a page load's worth
 *                  of connections on loopback and buys the whole of the keep-alive
 *                  bookkeeping not existing.
 */
void WebHost::Task()
{
    if(listener_ == kInvalidSocket) { return; }

    AcceptClients();
    for(size_t slot = 0u; slot < WEB_HOST_MAX_CLIENTS; slot++) {
        if(clients_[slot] != kInvalidSocket) { ReadClient(slot); }
    }
}

/*! \brief          Closes every socket, for the way out
 */
void WebHost::End()
{
    for(size_t slot = 0u; slot < WEB_HOST_MAX_CLIENTS; slot++) { DropClient(slot); }

    if(listener_ != kInvalidSocket) {
        CLOSE_SOCKET(listener_);
        listener_ = kInvalidSocket;
    }
}

bool WebHost::IsListening() const
{
    return listener_ != kInvalidSocket;
}

bool WebHost::HasClients() const
{
    for(size_t slot = 0u; slot < WEB_HOST_MAX_CLIENTS; slot++) {
        if(clients_[slot] != kInvalidSocket) { return true; }
    }
    return false;
}

/*! \brief          Sends one frame to every open web socket
 *
 *  \param[in]      is_binary       a binary frame rather than a text one
 *  \param[in]      payload         the bytes to send
 *  \param[in]      length          the number of bytes
 */
void WebHost::SendToEveryClient(bool is_binary, const char* payload, size_t length)
{
    const std::string frame = WebSocket::EncodeFrame(is_binary, payload, length);

    for(size_t slot = 0u; slot < WEB_HOST_MAX_CLIENTS; slot++) {
        if(clients_[slot] != kInvalidSocket) { SendAll(clients_[slot], frame.data(), frame.size()); }
    }
}

/*! \brief          Sends an HTTP response that closes the connection
 *  \details        Connection: close, because every connection but a web socket's is closed
 *                  after its answer - see Task().
 *
 *  \param[in]      socket          the connection
 *  \param[in]      status          the status line's code and text, e.g. "200 OK"
 *  \param[in]      content_type    the type of the body
 *  \param[in]      body            the body
 *  \param[in]      length          the length of the body
 *  \param[in]      is_gzipped      the body is gzip-compressed and says so
 */
void WebHost::SendResponse(SocketType socket, const char* status, const char* content_type,
                           const char* body, size_t length, bool is_gzipped)
{
    char header[256];
    const int written = snprintf(header, sizeof(header),
                                 "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n%sConnection: close\r\n\r\n",
                                 status, content_type, length,
                                 is_gzipped ? "Content-Encoding: gzip\r\n" : "");

    if(!SendAll(socket, header, static_cast<size_t>(written))) { return; }
    SendAll(socket, body, length);
}

/*! \brief          Answers every new connection
 *  \details        Keeps a connection that became a web socket, as long as there is a free
 *                  slot for it, and closes every other one. A connection that arrives with
 *                  every slot taken is closed unread.
 */
void WebHost::AcceptClients()
{
    for(;;) {
        const SocketType accepted = accept(listener_, nullptr, nullptr);
        if(accepted == kInvalidSocket) { return; }

        const size_t slot = FindFreeSlot();
        if((slot < WEB_HOST_MAX_CLIENTS) && ServeConnection(accepted)) {
            SetNonBlocking(accepted);
            clients_[slot] = accepted;
            pending_[slot].clear();
        } else {
            CLOSE_SOCKET(accepted);
        }
    }
}

/*! \brief          Reads what a web socket has to say - commands, and the close that ends one
 *  \details        Collects the bytes until a frame is complete; a closed connection or a
 *                  close frame drops the client.
 *
 *  \param[in]      slot            the client's slot
 */
void WebHost::ReadClient(size_t slot)
{
    char chunk[kFrameChunkLength];
    const int read = static_cast<int>(recv(clients_[slot], chunk, sizeof(chunk), 0));
    uint8_t opcode{0u};
    std::string payload;

    if(read == 0) { DropClient(slot); return; }
    if(read > 0) { pending_[slot].append(chunk, static_cast<size_t>(read)); }

    while(WebSocket::DecodeFrame(pending_[slot], opcode, payload) == kEOk) {
        if(opcode == WebSocket::kCloseOpcode) { DropClient(slot); return; }
        if(opcode == WebSocket::kTextOpcode)  { WebFrontend::InjectCommand(payload.data(), payload.size()); }
    }
}

/*! \brief          Finds a slot for a new web socket
 *
 *  \return         the first slot without a client, or WEB_HOST_MAX_CLIENTS when all are taken
 */
size_t WebHost::FindFreeSlot() const
{
    size_t slot{0u};

    while((slot < WEB_HOST_MAX_CLIENTS) && (clients_[slot] != kInvalidSocket)) { slot++; }
    return slot;
}

/*! \brief          Closes a web socket
 *  \details        Frees the slot and forgets what the client sent that was not yet a whole
 *                  frame. An empty slot is left alone, so that End() may call this on every one.
 *
 *  \param[in]      slot            the client's slot
 */
void WebHost::DropClient(size_t slot)
{
    if(clients_[slot] == kInvalidSocket) { return; }

    CLOSE_SOCKET(clients_[slot]);
    clients_[slot] = kInvalidSocket;
    pending_[slot].clear();
}

/*! \brief          Answers one accepted connection
 *  \details        An image is the one body that arrives here, and it comes in many packets
 *                  rather than the one every other request fits in, so an update is drained
 *                  before it is answered.
 *
 *  \param[in]      socket          the new connection
 *  \return         true            the connection stays, which only a web socket does
 *                  false           the connection is done
 */
bool WebHost::ServeConnection(SocketType socket)
{
    SetBlocking(socket);
    SetReceiveTimeout(socket, kRequestTimeoutMs);
    const std::string request = ReadRequest(socket);
    if(request.empty()) { return false; }

    const size_t announced = IsUpdateRequest(request) ? GetAnnouncedLength(request) : 0u;
    if(announced > 0u) {
        SetReceiveTimeout(socket, kBodyTimeoutMs);
        AnswerUpdate(socket, DrainBody(socket, GetArrivedBodyLength(request), announced));
        return false;
    }
    return AnswerRequest(socket, request);
}

/*! \brief          Answers everything that is not an update
 *  \details        Opens a web socket on its path, writes the two JSON documents, serves the
 *                  assets, and gives every other path a 404.
 *
 *  \param[in]      socket          the connection
 *  \param[in]      request         the whole request, headers included
 *  \return         true            the connection stays, which only a web socket does
 *                  false           the connection is done
 */
bool WebHost::AnswerRequest(SocketType socket, const std::string& request)
{
    std::string path;
    std::string answer;

    if(GetPath(request, path) != kEOk) { return false; }

    if(path == kWebSocketPath) {
        if(WebSocket::GetHandshakeAnswer(request, answer) != kEOk) { return false; }
        if(!SendAll(socket, answer.data(), answer.size())) { return false; }
        WebFrontend::GetInstance().OnClientOpened();
        return true;
    }

    if((path == "/commands") || (path == "/display")) {
        WebResponseBody body(socket, "application/json");

        if(path == "/commands") { WebFrontend::WriteCommands(body); }
        else                    { WebFrontend::WriteDisplay(body); }
        return false;
    }

    if(SendAsset(socket, path) == kEOk) { return false; }

    static constexpr char kNotFound[]{"no such route on the simulator\n"};
    SendResponse(socket, "404 Not Found", "text/plain", kNotFound, sizeof(kNotFound) - 1u, false);
    return false;
}

/*! \brief          Answers POST /update, in the two shapes the panel reads
 *
 *  \details        Nothing is installed and nothing is written, and that is not a shortcut
 *                  being taken: there is no second partition here and no filesystem an image
 *                  belongs in, so an honest answer is the most this can be. What it stands in
 *                  for is the whole of what the panel does - the progress on the way up, the
 *                  two outcomes and the shapes they arrive in - which is the one part of that
 *                  card that cannot otherwise be tried without a board.
 *
 *                  Refused below a size no real image is, so both answers are reachable: pick
 *                  a small file to see the failing path, a release image to see the other.
 */
void WebHost::AnswerUpdate(SocketType socket, size_t received)
{
    static constexpr size_t kSmallestPlausibleImage{size_t{64u} * 1024u};

    if(received >= kSmallestPlausibleImage) {
        static constexpr char kAccepted[]{"{\"ok\":true}"};

        Serial.print("Web: update of ");
        Serial.print(static_cast<int>(received / 1024u));
        Serial.println(" kB accepted - nothing was written, this is the simulator");
        SendResponse(socket, "200 OK", "application/json", kAccepted, sizeof(kAccepted) - 1u, false);
        return;
    }

    char refused[160];
    const int written = snprintf(refused, sizeof(refused),
                                 "{\"ok\":false,\"error\":\"%zu bytes is too small to be a firmware image\"}",
                                 received);

    SendResponse(socket, "400 Bad Request", "application/json", refused, static_cast<size_t>(written), false);
}

/*! \brief          Sends the embedded file served on a path
 *
 *  \param[in]      socket          the connection
 *  \param[in]      path            the path the browser asked for
 *  \return         kEOk
 *                  kENotOk        no asset is served on that path, and nothing was sent
 */
StdReturnType WebHost::SendAsset(SocketType socket, const std::string& path)
{
    for(byte index = 0u; index < WebFrontend::GetNumberOfAssets(); index++) {
        const WebFrontend::AssetType& asset = WebFrontend::GetAsset(index);

        if(path == asset.path) {
            SendResponse(socket, "200 OK", asset.content_type,
                         reinterpret_cast<const char*>(asset.bytes), asset.size, asset.is_gzipped);
            return kEOk;
        }
    }
    return kENotOk;
}

/*! \brief          Opens a listening socket on loopback
 *  \details        Loopback only. This serves a firmware console with no password on it, and
 *                  the one on a clock is at least on somebody's own network; a desktop is not
 *                  the place to open that to a coffee shop.
 *
 *  \return         the socket, or kInvalidSocket when it could not be opened
 */
WebHost::SocketType WebHost::OpenListener()
{
    const SocketType socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if(socket == kInvalidSocket) { return kInvalidSocket; }

    // So a restart does not have to wait out the last connection's TIME_WAIT, which is a
    // minute of "port already in use" for a program somebody restarts constantly.
    int reuse{1};
    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(WEB_HOST_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        CLOSE_SOCKET(socket);
        Serial.println("Web: port 8080 is taken, the window runs without a browser");
        return kInvalidSocket;
    }
    if(listen(socket, kBacklog) != 0) {
        CLOSE_SOCKET(socket);
        return kInvalidSocket;
    }
    SetNonBlocking(socket);
    return socket;
}

/*! \brief          Reads an HTTP request up to the blank line after its headers
 *  \details        Read here and now, because it is one packet on loopback - but never waited
 *                  for without end. A connection that is opened and says nothing is not a
 *                  broken browser but an ordinary one opening a socket ahead of the request
 *                  it may never make, and every recv() here blocks the firmware's tick, so
 *                  such a socket used to stop the window and every other client until the
 *                  browser closed it. The receive timeout set before this call is what ends
 *                  the wait; what has arrived by then is all there is.
 *
 *  \param[in]      socket          the new connection
 *  \return         the request, or what arrived of it before the connection closed
 */
std::string WebHost::ReadRequest(SocketType socket)
{
    std::string request;
    char chunk[kRequestChunkLength];

    while(request.find("\r\n\r\n") == std::string::npos) {
        const int read = static_cast<int>(recv(socket, chunk, sizeof(chunk), 0));

        if(read <= 0) { break; }
        request.append(chunk, static_cast<size_t>(read));
    }
    return request;
}

/*! \brief          Finds the path in an HTTP request
 *
 *  \param[in]      request         the whole request
 *  \param[out]     path            what stands between the first two spaces of the request line
 *  \return         kEOk
 *                  kENotOk        the request line has no path
 */
StdReturnType WebHost::GetPath(const std::string& request, std::string& path)
{
    const size_t start = request.find(' ');
    const size_t end = (start == std::string::npos) ? std::string::npos : request.find(' ', start + 1u);

    if(end == std::string::npos) { return kENotOk; }

    path = request.substr(start + 1u, end - start - 1u);
    return kEOk;
}

/*! \brief          Tells an update from every other request
 *  \details        Asked of the route and not only of the header, so that a body on any other
 *                  request is not mistaken for an image.
 *
 *  \param[in]      request         the whole request
 *  \return         true            it is POST /update
 */
bool WebHost::IsUpdateRequest(const std::string& request)
{
    return request.compare(0u, sizeof(kUpdateRequestLine) - 1u, kUpdateRequestLine) == 0;
}

/*! \brief          What the request announced it would send
 *  \details        So a body can be read to its end rather than to whatever arrived in the
 *                  first packet.
 *
 *  \param[in]      request         the whole request, headers included
 *  \return         the Content-Length, or 0 without one: absent means nothing follows the
 *                  headers
 */
size_t WebHost::GetAnnouncedLength(const std::string& request)
{
    std::string lowered(request);
    for(char& character : lowered) { character = static_cast<char>(tolower(static_cast<unsigned char>(character))); }

    const size_t found = lowered.find(kLengthHeader);
    if(found == std::string::npos) { return 0u; }

    return static_cast<size_t>(strtoul(&request[found + sizeof(kLengthHeader) - 1u], nullptr, 10));
}

/*! \brief          How much of the body came along in the packets that carried the headers
 *
 *  \param[in]      request         what ReadRequest() read
 *  \return         the bytes past the blank line, or 0 when the headers never ended
 */
size_t WebHost::GetArrivedBodyLength(const std::string& request)
{
    const size_t header_end = request.find("\r\n\r\n");
    return (header_end == std::string::npos) ? 0u : (request.size() - header_end - 4u);
}

/*! \brief          Reads the rest of a body and throws it away
 *  \details        Drained rather than parsed: what the panel needs answered is how much of it
 *                  turned up. Under its own, longer receive timeout than the request: an
 *                  upload that has started may pause between packets on its way through a
 *                  forwarded port, and cutting it short answers "too small" for an image
 *                  that was not - while one that announced more than it sends must still
 *                  not hold the tick for good.
 *
 *  \param[in]      socket          the connection
 *  \param[in]      received        what has arrived of the body so far
 *  \param[in]      announced       what the request said would arrive
 *  \return         what arrived in all, short of Announced when the connection closed early
 */
size_t WebHost::DrainBody(SocketType socket, size_t received, size_t announced)
{
    char chunk[kRequestChunkLength];

    while(received < announced) {
        const int read = static_cast<int>(recv(socket, chunk, sizeof(chunk), 0));

        if(read <= 0) { break; }
        received += static_cast<size_t>(read);
    }
    return received;
}

/*! \brief          Lets a socket return at once
 *  \details        So that polling it from the tick never stops the firmware.
 *
 *  \param[in]      socket          the socket
 */
void WebHost::SetNonBlocking(SocketType socket)
{
#ifdef _WIN32
    u_long mode{1u};
    ioctlsocket(socket, FIONBIO, &mode);
#else
    fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);
#endif
}

/*! \brief          Lets a socket wait for its bytes
 *  \details        For a connection fresh from accept(), which on Windows has the listener's
 *                  properties - non-blocking among them - and on Linux has not. Left as it
 *                  came, it made the first recv() on Windows fail at once whenever the request
 *                  was still on its way, and the connection was closed unanswered; and the
 *                  receive timeout, which only a blocking socket heeds, would not have applied
 *                  there at all. Said on both platforms, so that both read the same way.
 *
 *  \param[in]      socket          the socket
 */
void WebHost::SetBlocking(SocketType socket)
{
#ifdef _WIN32
    u_long mode{0u};
    ioctlsocket(socket, FIONBIO, &mode);
#else
    fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) & ~O_NONBLOCK);
#endif
}

/*! \brief          Lets recv() on a blocking socket give up after a while
 *  \details        A timeout on the socket rather than a select() before every recv(): the
 *                  read loops already end on a recv() that returns nothing, so they need no
 *                  change to end on one that waited long enough.
 *
 *  \param[in]      socket          the connection
 *  \param[in]      milliseconds    how long one recv() may wait for the next bytes
 */
void WebHost::SetReceiveTimeout(SocketType socket, unsigned milliseconds)
{
#ifdef _WIN32
    const DWORD timeout{milliseconds};
#else
    timeval timeout{};
    timeout.tv_sec = static_cast<time_t>(milliseconds / 1000u);
    timeout.tv_usec = static_cast<suseconds_t>((milliseconds % 1000u) * 1000u);
#endif
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
}

/*! \brief          Sends all of a buffer
 *  \details        Everything or nothing, because a half-written frame is a broken connection
 *                  rather than a short one. Loopback takes all of it in one call in every case
 *                  this serves; the loop is what makes that an observation rather than an
 *                  assumption.
 *
 *  \param[in]      socket          the connection
 *  \param[in]      data            the bytes to send
 *  \param[in]      length          the number of bytes
 *  \return         true            all was sent
 *                  false           the connection is broken
 */
bool WebHost::SendAll(SocketType socket, const char* data, size_t length)
{
    size_t sent{0u};

    while(sent < length) {
        const int written = static_cast<int>(send(socket, &data[sent], static_cast<int>(length - sent), 0));

        if(written > 0) { sent += static_cast<size_t>(written); continue; }
#ifndef _WIN32
        if((written < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { continue; }
#endif
        return false;
    }
    return true;
}

/*! \brief          Hands a finished output line to the pages
 *  \details        Static because SerialShim takes a plain function pointer.
 *
 *  \param[in]      line            the line, without its line end
 */
void WebHost::SendLineToClients(const char* line)
{
    WebFrontend::GetInstance().BroadcastLine(line);
}

/* The simulator's half of the contract WebFrontend.h states, in terms of the host above. */

WebTransport& WebTransport::GetInstance()
{
    static WebTransport singleton_instance;
    return singleton_instance;
}

/*
 * Not static, though nothing in them reads a member: these stand in for the backends the
 * clock really runs on, and there the same call reaches a peripheral through instance
 * state. A stand-in that answered the check would have a different API from the thing it
 * stands in for, which is the one property it exists to keep.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
bool WebTransport::IsListening() const { return WebHost::GetInstance().IsListening(); }
bool WebTransport::HasClients() const { return WebHost::GetInstance().HasClients(); }

void WebTransport::SendText(const char* text, size_t length)
{
    WebHost::GetInstance().SendToEveryClient(false, text, length);
}

void WebTransport::SendBinary(const byte* bytes, size_t length)
{
    WebHost::GetInstance().SendToEveryClient(true, reinterpret_cast<const char*>(bytes), length);
}
// NOLINTEND(readability-convert-member-functions-to-static)

/*
 * Collected and sent whole. The two documents this carries are a few kilobytes at most and
 * go to loopback, so a Content-Length up front is simpler than a chunked encoding - and it is
 * what lets the browser show a progress that ends.
 */

WebResponseBody::WebResponseBody(int socket, const char* content_type)
    : socket_(socket), content_type_(content_type)
{
}

void WebResponseBody::Write(const char* data, size_t length)
{
    collected_.append(data, length);
}

void WebResponseBody::Finish()
{
    WebHost::SendResponse(socket_, "200 OK", content_type_, collected_.data(), collected_.size(), false);
}

#endif // (WEB_FRONTEND_SUPPORT == STD_ON)
