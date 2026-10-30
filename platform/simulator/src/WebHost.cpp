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
/**     \file       WebHost.cpp
 *      \brief      Serves the clock's two pages from the simulator, on localhost
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
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
# define closeSocket closesocket
#else
# include <arpa/inet.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <poll.h>
# include <sys/socket.h>
# include <unistd.h>
# define closeSocket close
#endif

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  CONSTRUCTOR OF WebHost
******************************************************************************************************************************************************/
/*! \brief          WebHost constructor
 *  \details        A host that listens nowhere until begin(), and has no clients until then
 *                  either - which hasClients() may be asked before begin() ever ran.
 *
 *  \return         -
******************************************************************************************************************************************************/
WebHost::WebHost()
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) { Clients[Slot] = InvalidSocket; }
} /* WebHost */


WebHost& WebHost::getInstance()
{
    static WebHost SingletonInstance;
    return SingletonInstance;
}


/******************************************************************************************************************************************************
  begin()
******************************************************************************************************************************************************/
/*! \brief          Opens the listening socket and takes over the output lines
 *
 *  \return         E_OK once the port is listening
 *
 *  \details        A port that is taken is not fatal and must not be: somebody already has a
 *                  simulator open, and the second one is still a window worth having. It says
 *                  so and runs on, which is the same shape the ESP32 backend's server failure
 *                  has - fall back to what works rather than refuse to start.
******************************************************************************************************************************************************/
StdReturnType WebHost::begin()
{
#ifdef _WIN32
    WSADATA WinsockData;
    if(WSAStartup(MAKEWORD(2, 2), &WinsockData) != 0) { return E_NOT_OK; }
#endif

    Listener = openListener();
    if(Listener == InvalidSocket) { return E_NOT_OK; }

    WordclockSerial::getInstance().setLineSink(sendLineToClients);

    Serial.println("Web: http://localhost:8080/");

    return E_OK;
} /* begin */


/******************************************************************************************************************************************************
  task()
******************************************************************************************************************************************************/
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
******************************************************************************************************************************************************/
void WebHost::task()
{
    if(Listener == InvalidSocket) { return; }

    acceptClients();
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot] != InvalidSocket) { readClient(Slot); }
    }
} /* task */


/******************************************************************************************************************************************************
  end()
******************************************************************************************************************************************************/
/*! \brief          Closes every socket, for the way out
 *
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::end()
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) { dropClient(Slot); }

    if(Listener != InvalidSocket) {
        closeSocket(Listener);
        Listener = InvalidSocket;
    }
} /* end */


bool WebHost::isListening() const
{
    return Listener != InvalidSocket;
}


bool WebHost::hasClients() const
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot] != InvalidSocket) { return true; }
    }
    return false;
}


/******************************************************************************************************************************************************
  sendToEveryClient()
******************************************************************************************************************************************************/
/*! \brief          Sends one frame to every open web socket
 *
 *  \param[in]      IsBinary        a binary frame rather than a text one
 *  \param[in]      Payload         the bytes to send
 *  \param[in]      Length          the number of bytes
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::sendToEveryClient(bool IsBinary, const char* Payload, size_t Length)
{
    const std::string Frame = WebSocket::encodeFrame(IsBinary, Payload, Length);

    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot] != InvalidSocket) { sendAll(Clients[Slot], Frame.data(), Frame.size()); }
    }
} /* sendToEveryClient */


/******************************************************************************************************************************************************
  sendResponse()
******************************************************************************************************************************************************/
/*! \brief          Sends an HTTP response that closes the connection
 *  \details        Connection: close, because every connection but a web socket's is closed
 *                  after its answer - see task().
 *
 *  \param[in]      Socket          the connection
 *  \param[in]      Status          the status line's code and text, e.g. "200 OK"
 *  \param[in]      ContentType     the type of the body
 *  \param[in]      Body            the body
 *  \param[in]      Length          the length of the body
 *  \param[in]      IsGzipped       the body is gzip-compressed and says so
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::sendResponse(SocketType Socket, const char* Status, const char* ContentType,
                           const char* Body, size_t Length, bool IsGzipped)
{
    char Header[256];
    const int Written = snprintf(Header, sizeof(Header),
                                 "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n%sConnection: close\r\n\r\n",
                                 Status, ContentType, Length,
                                 IsGzipped ? "Content-Encoding: gzip\r\n" : "");

    if(!sendAll(Socket, Header, static_cast<size_t>(Written))) { return; }
    sendAll(Socket, Body, Length);
} /* sendResponse */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  acceptClients()
******************************************************************************************************************************************************/
/*! \brief          Answers every new connection
 *  \details        Keeps a connection that became a web socket, as long as there is a free
 *                  slot for it, and closes every other one. A connection that arrives with
 *                  every slot taken is closed unread.
 *
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::acceptClients()
{
    for(;;) {
        const SocketType Accepted = accept(Listener, nullptr, nullptr);
        if(Accepted == InvalidSocket) { return; }

        const size_t Slot = findFreeSlot();
        if((Slot < WEB_HOST_MAX_CLIENTS) && serveConnection(Accepted)) {
            setNonBlocking(Accepted);
            Clients[Slot] = Accepted;
            Pending[Slot].clear();
        } else {
            closeSocket(Accepted);
        }
    }
} /* acceptClients */


/******************************************************************************************************************************************************
  readClient()
******************************************************************************************************************************************************/
/*! \brief          Reads what a web socket has to say - commands, and the close that ends one
 *  \details        Collects the bytes until a frame is complete; a closed connection or a
 *                  close frame drops the client.
 *
 *  \param[in]      Slot            the client's slot
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::readClient(size_t Slot)
{
    char Chunk[FrameChunkLength];
    const int Read = static_cast<int>(recv(Clients[Slot], Chunk, sizeof(Chunk), 0));
    uint8_t Opcode{0u};
    std::string Payload;

    if(Read == 0) { dropClient(Slot); return; }
    if(Read > 0) { Pending[Slot].append(Chunk, static_cast<size_t>(Read)); }

    while(WebSocket::decodeFrame(Pending[Slot], Opcode, Payload) == E_OK) {
        if(Opcode == WebSocket::CloseOpcode) { dropClient(Slot); return; }
        if(Opcode == WebSocket::TextOpcode)  { WebFrontend::injectCommand(Payload.data(), Payload.size()); }
    }
} /* readClient */


/******************************************************************************************************************************************************
  findFreeSlot()
******************************************************************************************************************************************************/
/*! \brief          Finds a slot for a new web socket
 *
 *  \return         the first slot without a client, or WEB_HOST_MAX_CLIENTS when all are taken
******************************************************************************************************************************************************/
size_t WebHost::findFreeSlot() const
{
    size_t Slot{0u};

    while((Slot < WEB_HOST_MAX_CLIENTS) && (Clients[Slot] != InvalidSocket)) { Slot++; }
    return Slot;
} /* findFreeSlot */


/******************************************************************************************************************************************************
  dropClient()
******************************************************************************************************************************************************/
/*! \brief          Closes a web socket
 *  \details        Frees the slot and forgets what the client sent that was not yet a whole
 *                  frame. An empty slot is left alone, so that end() may call this on every one.
 *
 *  \param[in]      Slot            the client's slot
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::dropClient(size_t Slot)
{
    if(Clients[Slot] == InvalidSocket) { return; }

    closeSocket(Clients[Slot]);
    Clients[Slot] = InvalidSocket;
    Pending[Slot].clear();
} /* dropClient */


/******************************************************************************************************************************************************
  serveConnection()
******************************************************************************************************************************************************/
/*! \brief          Answers one accepted connection
 *  \details        An image is the one body that arrives here, and it comes in many packets
 *                  rather than the one every other request fits in, so an update is drained
 *                  before it is answered.
 *
 *  \param[in]      Socket          the new connection
 *  \return         true            the connection stays, which only a web socket does
 *                  false           the connection is done
******************************************************************************************************************************************************/
bool WebHost::serveConnection(SocketType Socket)
{
    const std::string Request = readRequest(Socket);
    if(Request.empty()) { return false; }

    const size_t Announced = isUpdateRequest(Request) ? getAnnouncedLength(Request) : 0u;
    if(Announced > 0u) {
        answerUpdate(Socket, drainBody(Socket, getArrivedBodyLength(Request), Announced));
        return false;
    }
    return answerRequest(Socket, Request);
} /* serveConnection */


/******************************************************************************************************************************************************
  answerRequest()
******************************************************************************************************************************************************/
/*! \brief          Answers everything that is not an update
 *  \details        Opens a web socket on its path, writes the two JSON documents, serves the
 *                  assets, and gives every other path a 404.
 *
 *  \param[in]      Socket          the connection
 *  \param[in]      Request         the whole request, headers included
 *  \return         true            the connection stays, which only a web socket does
 *                  false           the connection is done
******************************************************************************************************************************************************/
bool WebHost::answerRequest(SocketType Socket, const std::string& Request)
{
    std::string Path;
    std::string Answer;

    if(getPath(Request, Path) != E_OK) { return false; }

    if(Path == WebSocketPath) {
        if(WebSocket::getHandshakeAnswer(Request, Answer) != E_OK) { return false; }
        if(!sendAll(Socket, Answer.data(), Answer.size())) { return false; }
        WebFrontend::getInstance().onClientOpened();
        return true;
    }

    if((Path == "/commands") || (Path == "/display")) {
        WebResponseBody Body(Socket, "application/json");

        if(Path == "/commands") { WebFrontend::writeCommands(Body); }
        else                    { WebFrontend::writeDisplay(Body); }
        return false;
    }

    if(sendAsset(Socket, Path) == E_OK) { return false; }

    static constexpr char NotFound[]{"no such route on the simulator\n"};
    sendResponse(Socket, "404 Not Found", "text/plain", NotFound, sizeof(NotFound) - 1u, false);
    return false;
} /* answerRequest */


/******************************************************************************************************************************************************
  answerUpdate()
******************************************************************************************************************************************************/
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
******************************************************************************************************************************************************/
void WebHost::answerUpdate(SocketType Socket, size_t Received)
{
    static constexpr size_t SmallestPlausibleImage{size_t{64u} * 1024u};

    if(Received >= SmallestPlausibleImage) {
        static constexpr char Accepted[]{"{\"ok\":true}"};

        Serial.print("Web: update of ");
        Serial.print(static_cast<int>(Received / 1024u));
        Serial.println(" kB accepted - nothing was written, this is the simulator");
        sendResponse(Socket, "200 OK", "application/json", Accepted, sizeof(Accepted) - 1u, false);
        return;
    }

    char Refused[160];
    const int Written = snprintf(Refused, sizeof(Refused),
                                 "{\"ok\":false,\"error\":\"%zu bytes is too small to be a firmware image\"}",
                                 Received);

    sendResponse(Socket, "400 Bad Request", "application/json", Refused, static_cast<size_t>(Written), false);
} /* answerUpdate */


/******************************************************************************************************************************************************
  sendAsset()
******************************************************************************************************************************************************/
/*! \brief          Sends the embedded file served on a path
 *
 *  \param[in]      Socket          the connection
 *  \param[in]      Path            the path the browser asked for
 *  \return         E_OK
 *                  E_NOT_OK        no asset is served on that path, and nothing was sent
******************************************************************************************************************************************************/
StdReturnType WebHost::sendAsset(SocketType Socket, const std::string& Path)
{
    for(byte Index = 0u; Index < WebFrontend::getNumberOfAssets(); Index++) {
        const WebFrontend::AssetType& Asset = WebFrontend::getAsset(Index);

        if(Path == Asset.Path) {
            sendResponse(Socket, "200 OK", Asset.ContentType,
                         reinterpret_cast<const char*>(Asset.Bytes), Asset.Size, Asset.IsGzipped);
            return E_OK;
        }
    }
    return E_NOT_OK;
} /* sendAsset */


/******************************************************************************************************************************************************
  openListener()
******************************************************************************************************************************************************/
/*! \brief          Opens a listening socket on loopback
 *  \details        Loopback only. This serves a firmware console with no password on it, and
 *                  the one on a clock is at least on somebody's own network; a desktop is not
 *                  the place to open that to a coffee shop.
 *
 *  \return         the socket, or InvalidSocket when it could not be opened
******************************************************************************************************************************************************/
WebHost::SocketType WebHost::openListener()
{
    const SocketType Socket = socket(AF_INET, SOCK_STREAM, 0);
    if(Socket == InvalidSocket) { return InvalidSocket; }

    /* So a restart does not have to wait out the last connection's TIME_WAIT, which is a
       minute of "port already in use" for a program somebody restarts constantly. */
    int Reuse{1};
    setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&Reuse), sizeof(Reuse));

    sockaddr_in Address{};
    Address.sin_family = AF_INET;
    Address.sin_port = htons(WEB_HOST_PORT);
    Address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(Socket, reinterpret_cast<sockaddr*>(&Address), sizeof(Address)) != 0) {
        closeSocket(Socket);
        Serial.println("Web: port 8080 is taken, the window runs without a browser");
        return InvalidSocket;
    }
    if(listen(Socket, Backlog) != 0) {
        closeSocket(Socket);
        return InvalidSocket;
    }
    setNonBlocking(Socket);
    return Socket;
} /* openListener */


/******************************************************************************************************************************************************
  readRequest()
******************************************************************************************************************************************************/
/*! \brief          Reads an HTTP request up to the blank line after its headers
 *  \details        Read here and now. It is one packet on loopback, and a browser that sends
 *                  half a request and stops is a case this tool need not survive gracefully.
 *
 *  \param[in]      Socket          the new connection
 *  \return         the request, or what arrived of it before the connection closed
******************************************************************************************************************************************************/
std::string WebHost::readRequest(SocketType Socket)
{
    std::string Request;
    char Chunk[RequestChunkLength];

    while(Request.find("\r\n\r\n") == std::string::npos) {
        const int Read = static_cast<int>(recv(Socket, Chunk, sizeof(Chunk), 0));

        if(Read <= 0) { break; }
        Request.append(Chunk, static_cast<size_t>(Read));
    }
    return Request;
} /* readRequest */


/******************************************************************************************************************************************************
  getPath()
******************************************************************************************************************************************************/
/*! \brief          Finds the path in an HTTP request
 *
 *  \param[in]      Request         the whole request
 *  \param[out]     Path            what stands between the first two spaces of the request line
 *  \return         E_OK
 *                  E_NOT_OK        the request line has no path
******************************************************************************************************************************************************/
StdReturnType WebHost::getPath(const std::string& Request, std::string& Path)
{
    const size_t Start = Request.find(' ');
    const size_t End = (Start == std::string::npos) ? std::string::npos : Request.find(' ', Start + 1u);

    if(End == std::string::npos) { return E_NOT_OK; }

    Path = Request.substr(Start + 1u, End - Start - 1u);
    return E_OK;
} /* getPath */


/******************************************************************************************************************************************************
  isUpdateRequest()
******************************************************************************************************************************************************/
/*! \brief          Tells an update from every other request
 *  \details        Asked of the route and not only of the header, so that a body on any other
 *                  request is not mistaken for an image.
 *
 *  \param[in]      Request         the whole request
 *  \return         true            it is POST /update
******************************************************************************************************************************************************/
bool WebHost::isUpdateRequest(const std::string& Request)
{
    return Request.compare(0u, sizeof(UpdateRequestLine) - 1u, UpdateRequestLine) == 0;
} /* isUpdateRequest */


/******************************************************************************************************************************************************
  getAnnouncedLength()
******************************************************************************************************************************************************/
/*! \brief          What the request announced it would send
 *  \details        So a body can be read to its end rather than to whatever arrived in the
 *                  first packet.
 *
 *  \param[in]      Request         the whole request, headers included
 *  \return         the Content-Length, or 0 without one: absent means nothing follows the
 *                  headers
******************************************************************************************************************************************************/
size_t WebHost::getAnnouncedLength(const std::string& Request)
{
    std::string Lowered(Request);
    for(char& Character : Lowered) { Character = static_cast<char>(tolower(static_cast<unsigned char>(Character))); }

    const size_t Found = Lowered.find(LengthHeader);
    if(Found == std::string::npos) { return 0u; }

    return static_cast<size_t>(strtoul(&Request[Found + sizeof(LengthHeader) - 1u], nullptr, 10));
} /* getAnnouncedLength */


/******************************************************************************************************************************************************
  getArrivedBodyLength()
******************************************************************************************************************************************************/
/*! \brief          How much of the body came along in the packets that carried the headers
 *
 *  \param[in]      Request         what readRequest() read
 *  \return         the bytes past the blank line, or 0 when the headers never ended
******************************************************************************************************************************************************/
size_t WebHost::getArrivedBodyLength(const std::string& Request)
{
    const size_t HeaderEnd = Request.find("\r\n\r\n");
    return (HeaderEnd == std::string::npos) ? 0u : (Request.size() - HeaderEnd - 4u);
} /* getArrivedBodyLength */


/******************************************************************************************************************************************************
  drainBody()
******************************************************************************************************************************************************/
/*! \brief          Reads the rest of a body and throws it away
 *  \details        Drained rather than parsed: what the panel needs answered is how much of it
 *                  turned up.
 *
 *  \param[in]      Socket          the connection
 *  \param[in]      Received        what has arrived of the body so far
 *  \param[in]      Announced       what the request said would arrive
 *  \return         what arrived in all, short of Announced when the connection closed early
******************************************************************************************************************************************************/
size_t WebHost::drainBody(SocketType Socket, size_t Received, size_t Announced)
{
    char Chunk[RequestChunkLength];

    while(Received < Announced) {
        const int Read = static_cast<int>(recv(Socket, Chunk, sizeof(Chunk), 0));

        if(Read <= 0) { break; }
        Received += static_cast<size_t>(Read);
    }
    return Received;
} /* drainBody */


/******************************************************************************************************************************************************
  setNonBlocking()
******************************************************************************************************************************************************/
/*! \brief          Lets a socket return at once
 *  \details        So that polling it from the tick never stops the firmware.
 *
 *  \param[in]      Socket          the socket
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::setNonBlocking(SocketType Socket)
{
#ifdef _WIN32
    u_long Mode{1u};
    ioctlsocket(Socket, FIONBIO, &Mode);
#else
    fcntl(Socket, F_SETFL, fcntl(Socket, F_GETFL, 0) | O_NONBLOCK);
#endif
} /* setNonBlocking */


/******************************************************************************************************************************************************
  sendAll()
******************************************************************************************************************************************************/
/*! \brief          Sends all of a buffer
 *  \details        Everything or nothing, because a half-written frame is a broken connection
 *                  rather than a short one. Loopback takes all of it in one call in every case
 *                  this serves; the loop is what makes that an observation rather than an
 *                  assumption.
 *
 *  \param[in]      Socket          the connection
 *  \param[in]      Data            the bytes to send
 *  \param[in]      Length          the number of bytes
 *  \return         true            all was sent
 *                  false           the connection is broken
******************************************************************************************************************************************************/
bool WebHost::sendAll(SocketType Socket, const char* Data, size_t Length)
{
    size_t Sent{0u};

    while(Sent < Length) {
        const int Written = static_cast<int>(send(Socket, &Data[Sent], static_cast<int>(Length - Sent), 0));

        if(Written > 0) { Sent += static_cast<size_t>(Written); continue; }
#ifndef _WIN32
        if((Written < 0) && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) { continue; }
#endif
        return false;
    }
    return true;
} /* sendAll */


/******************************************************************************************************************************************************
  sendLineToClients()
******************************************************************************************************************************************************/
/*! \brief          Hands a finished output line to the pages
 *  \details        Static because SerialShim takes a plain function pointer.
 *
 *  \param[in]      Line            the line, without its line end
 *  \return         -
******************************************************************************************************************************************************/
void WebHost::sendLineToClients(const char* Line)
{
    WebFrontend::getInstance().broadcastLine(Line);
} /* sendLineToClients */


/******************************************************************************************************************************************************
 *  W E B   T R A N S P O R T
******************************************************************************************************************************************************/
/* The simulator's half of the contract WebFrontend.h states, in terms of the host above. */

WebTransport& WebTransport::getInstance()
{
    static WebTransport SingletonInstance;
    return SingletonInstance;
}

/* Not static, though nothing in them reads a member: these stand in for the backends the
   clock really runs on, and there the same call reaches a peripheral through instance
   state. A stand-in that answered the check would have a different API from the thing it
   stands in for, which is the one property it exists to keep. */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
bool WebTransport::isListening() const { return WebHost::getInstance().isListening(); }
bool WebTransport::hasClients() const { return WebHost::getInstance().hasClients(); }

void WebTransport::sendText(const char* Text, size_t Length)
{
    WebHost::getInstance().sendToEveryClient(false, Text, Length);
}

void WebTransport::sendBinary(const byte* Bytes, size_t Length)
{
    WebHost::getInstance().sendToEveryClient(true, reinterpret_cast<const char*>(Bytes), Length);
}
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
 *  W E B   R E S P O N S E   B O D Y
******************************************************************************************************************************************************/
/* Collected and sent whole. The two documents this carries are a few kilobytes at most and
   go to loopback, so a Content-Length up front is simpler than a chunked encoding - and it is
   what lets the browser show a progress that ends. */

WebResponseBody::WebResponseBody(int sSocket, const char* sContentType)
    : Socket(sSocket), ContentType(sContentType)
{
}

void WebResponseBody::write(const char* Data, size_t Length)
{
    Collected.append(Data, Length);
}

void WebResponseBody::finish()
{
    WebHost::sendResponse(Socket, "200 OK", ContentType, Collected.data(), Collected.size(), false);
}

#endif // (WEB_FRONTEND_SUPPORT == STD_ON)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
