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

#include "sim/WebTransport.h"
#include "sim/WordclockSerial.h"

#include "WebFrontend.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
using SocketType = SOCKET;
static constexpr SocketType InvalidSocket{INVALID_SOCKET};
# define closeSocket closesocket
#else
# include <arpa/inet.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <poll.h>
# include <sys/socket.h>
# include <unistd.h>
using SocketType = int;
static constexpr SocketType InvalidSocket{-1};
# define closeSocket close
#endif

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

SocketType Listener{InvalidSocket};

/* A connection that has finished its handshake and is being broadcast to. Anything before
   that is a request being read and is gone by the end of the tick that read it. */
struct ClientType {
    SocketType Socket{InvalidSocket};
    std::string Pending;
};

ClientType Clients[WEB_HOST_MAX_CLIENTS];

/******************************************************************************************************************************************************
 *  S H A - 1   A N D   B A S E 6 4
******************************************************************************************************************************************************/
/* Both are here for one reason only: RFC 6455 makes the handshake answer the base64 of the
   SHA-1 of the client's key and a fixed string. There is nothing to choose about either, so
   they are written out rather than pulled in - a dependency for eighty lines of arithmetic
   would be a worse trade in a build that otherwise needs only wxWidgets.

   No other use, and in particular not a security one: this is a checksum a browser demands,
   not a secret. */
void sha1(const uint8_t* Message, size_t Length, uint8_t Digest[20])
{
    uint32_t State[5]{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};

    /* The message, its terminator bit and its length in bits, which is what SHA-1 hashes. */
    std::string Padded(reinterpret_cast<const char*>(Message), Length);
    Padded.push_back(static_cast<char>(0x80));
    while((Padded.size() % 64u) != 56u) { Padded.push_back('\0'); }

    const uint64_t Bits = static_cast<uint64_t>(Length) * 8u;
    for(int Shift = 56; Shift >= 0; Shift -= 8) {
        Padded.push_back(static_cast<char>((Bits >> Shift) & 0xFFu));
    }

    for(size_t Offset = 0u; Offset < Padded.size(); Offset += 64u) {
        uint32_t Word[80];

        for(size_t Index = 0u; Index < 16u; Index++) {
            const uint8_t* Chunk = reinterpret_cast<const uint8_t*>(&Padded[Offset + (Index * 4u)]);
            Word[Index] = (static_cast<uint32_t>(Chunk[0]) << 24) | (static_cast<uint32_t>(Chunk[1]) << 16)
                        | (static_cast<uint32_t>(Chunk[2]) << 8) | static_cast<uint32_t>(Chunk[3]);
        }
        for(size_t Index = 16u; Index < 80u; Index++) {
            const uint32_t Mixed = Word[Index - 3u] ^ Word[Index - 8u] ^ Word[Index - 14u] ^ Word[Index - 16u];
            Word[Index] = (Mixed << 1) | (Mixed >> 31);
        }

        uint32_t A{State[0]}, B{State[1]}, C{State[2]}, D{State[3]}, E{State[4]};

        for(size_t Index = 0u; Index < 80u; Index++) {
            uint32_t Mixed{0u};
            uint32_t Constant{0u};

            if(Index < 20u)      { Mixed = (B & C) | ((~B) & D);          Constant = 0x5A827999u; }
            else if(Index < 40u) { Mixed = B ^ C ^ D;                     Constant = 0x6ED9EBA1u; }
            else if(Index < 60u) { Mixed = (B & C) | (B & D) | (C & D);   Constant = 0x8F1BBCDCu; }
            else                 { Mixed = B ^ C ^ D;                     Constant = 0xCA62C1D6u; }

            const uint32_t Next = ((A << 5) | (A >> 27)) + Mixed + E + Constant + Word[Index];
            E = D;
            D = C;
            C = (B << 30) | (B >> 2);
            B = A;
            A = Next;
        }

        State[0] += A;
        State[1] += B;
        State[2] += C;
        State[3] += D;
        State[4] += E;
    }

    for(size_t Index = 0u; Index < 5u; Index++) {
        Digest[Index * 4u]        = static_cast<uint8_t>(State[Index] >> 24);
        Digest[(Index * 4u) + 1u] = static_cast<uint8_t>(State[Index] >> 16);
        Digest[(Index * 4u) + 2u] = static_cast<uint8_t>(State[Index] >> 8);
        Digest[(Index * 4u) + 3u] = static_cast<uint8_t>(State[Index]);
    }
}

std::string toBase64(const uint8_t* Bytes, size_t Length)
{
    static constexpr char Alphabet[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    std::string Encoded;

    for(size_t Offset = 0u; Offset < Length; Offset += 3u) {
        const size_t Remaining = Length - Offset;
        uint32_t Group = static_cast<uint32_t>(Bytes[Offset]) << 16;

        if(Remaining > 1u) { Group |= static_cast<uint32_t>(Bytes[Offset + 1u]) << 8; }
        if(Remaining > 2u) { Group |= static_cast<uint32_t>(Bytes[Offset + 2u]); }

        Encoded.push_back(Alphabet[(Group >> 18) & 0x3Fu]);
        Encoded.push_back(Alphabet[(Group >> 12) & 0x3Fu]);
        Encoded.push_back((Remaining > 1u) ? Alphabet[(Group >> 6) & 0x3Fu] : '=');
        Encoded.push_back((Remaining > 2u) ? Alphabet[Group & 0x3Fu] : '=');
    }
    return Encoded;
}

/******************************************************************************************************************************************************
 *  S O C K E T   H E L P E R S
******************************************************************************************************************************************************/
void setNonBlocking(SocketType Socket)
{
#ifdef _WIN32
    u_long Mode{1u};
    ioctlsocket(Socket, FIONBIO, &Mode);
#else
    fcntl(Socket, F_SETFL, fcntl(Socket, F_GETFL, 0) | O_NONBLOCK);
#endif
}

/* Everything or nothing, because a half-written frame is a broken connection rather than a
   short one. Loopback takes all of it in one call in every case this serves; the loop is
   what makes that an observation rather than an assumption. */
bool sendAll(SocketType Socket, const char* Data, size_t Length)
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
}

bool hasRoomForClient()
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot].Socket == InvalidSocket) { return true; }
    }
    return false;
}

void dropClient(ClientType& Client)
{
    if(Client.Socket == InvalidSocket) { return; }

    closeSocket(Client.Socket);
    Client.Socket = InvalidSocket;
    Client.Pending.clear();
}

/******************************************************************************************************************************************************
 *  W E B   S O C K E T   F R A M E S
******************************************************************************************************************************************************/
/* One frame out. Only the two lengths a clock ever sends are encoded: a line is well under
   126 bytes and a display frame is 330, so the 64-bit form is unreachable and writing it
   would be code no run ever takes. */
void sendFrame(SocketType Socket, bool IsBinary, const char* Payload, size_t Length)
{
    std::string Header;

    Header.push_back(static_cast<char>(IsBinary ? 0x82 : 0x81));
    if(Length < 126u) {
        Header.push_back(static_cast<char>(Length));
    } else {
        Header.push_back(static_cast<char>(126));
        Header.push_back(static_cast<char>((Length >> 8) & 0xFFu));
        Header.push_back(static_cast<char>(Length & 0xFFu));
    }

    if(!sendAll(Socket, Header.data(), Header.size())) { return; }
    sendAll(Socket, Payload, Length);
}

/* One frame in, or false while it is still arriving. What a browser sends is always masked,
   which is the one part of the format this has to undo. */
bool takeFrame(std::string& Buffer, uint8_t& Opcode, std::string& Payload)
{
    if(Buffer.size() < 2u) { return false; }

    const uint8_t First = static_cast<uint8_t>(Buffer[0]);
    const uint8_t Second = static_cast<uint8_t>(Buffer[1]);
    const bool IsMasked = (Second & 0x80u) != 0u;
    size_t Length = Second & 0x7Fu;
    size_t Offset = 2u;

    if(Length == 126u) {
        if(Buffer.size() < 4u) { return false; }
        Length = (static_cast<size_t>(static_cast<uint8_t>(Buffer[2])) << 8) | static_cast<uint8_t>(Buffer[3]);
        Offset = 4u;
    } else if(Length == 127u) {
        /* A command is never this long, and a browser that says so is not one this serves.
           Refusing is better than trying to read it. */
        return false;
    }

    const size_t MaskLength = IsMasked ? 4u : 0u;
    if(Buffer.size() < (Offset + MaskLength + Length)) { return false; }

    Payload.assign(&Buffer[Offset + MaskLength], Length);
    if(IsMasked) {
        for(size_t Index = 0u; Index < Length; Index++) {
            Payload[Index] = static_cast<char>(Payload[Index] ^ Buffer[Offset + (Index % 4u)]);
        }
    }

    Opcode = First & 0x0Fu;
    Buffer.erase(0u, Offset + MaskLength + Length);
    return true;
}

/******************************************************************************************************************************************************
 *  H T T P
******************************************************************************************************************************************************/
void sendSimpleResponse(SocketType Socket, const char* Status, const char* ContentType,
                        const char* Body, size_t Length, bool IsGzipped)
{
    char Header[256];
    const int Written = snprintf(Header, sizeof(Header),
                                 "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n%sConnection: close\r\n\r\n",
                                 Status, ContentType, Length,
                                 IsGzipped ? "Content-Encoding: gzip\r\n" : "");

    if(!sendAll(Socket, Header, static_cast<size_t>(Written))) { return; }
    sendAll(Socket, Body, Length);
}

/* The handshake, which is the whole of what makes a socket out of a request. */
bool completeHandshake(SocketType Socket, const std::string& Request)
{
    static constexpr char KeyHeader[]{"sec-websocket-key:"};
    static constexpr char Guid[]{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};

    std::string Lowered(Request);
    for(char& Character : Lowered) { Character = static_cast<char>(tolower(Character)); }

    const size_t Found = Lowered.find(KeyHeader);
    if(Found == std::string::npos) { return false; }

    size_t Start = Found + sizeof(KeyHeader) - 1u;
    while((Start < Request.size()) && (Request[Start] == ' ')) { Start++; }
    const size_t End = Request.find("\r\n", Start);
    if(End == std::string::npos) { return false; }

    const std::string Accept = Request.substr(Start, End - Start) + Guid;
    uint8_t Digest[20];
    sha1(reinterpret_cast<const uint8_t*>(Accept.data()), Accept.size(), Digest);

    char Header[256];
    const int Written = snprintf(Header, sizeof(Header),
                                 "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
                                 "Connection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n",
                                 toBase64(Digest, sizeof(Digest)).c_str());

    return sendAll(Socket, Header, static_cast<size_t>(Written));
}

/* Answers everything that is not the socket, and says whether the connection was kept. */
bool answerRequest(SocketType Socket, const std::string& Request)
{
    const size_t PathStart = Request.find(' ');
    if(PathStart == std::string::npos) { return false; }

    const size_t PathEnd = Request.find(' ', PathStart + 1u);
    if(PathEnd == std::string::npos) { return false; }

    const std::string Path = Request.substr(PathStart + 1u, PathEnd - PathStart - 1u);

    if(Path == "/ws") {
        if(!completeHandshake(Socket, Request)) { return false; }
        WebFrontend::getInstance().onClientOpened();
        return true;
    }

    if((Path == "/commands") || (Path == "/display")) {
        WebResponseBody Body(Socket, "application/json");

        if(Path == "/commands") { WebFrontend::writeCommands(Body); }
        else                    { WebFrontend::writeDisplay(Body); }
        return false;
    }

    for(byte Index = 0u; Index < WebFrontend::getNumberOfAssets(); Index++) {
        const WebFrontend::AssetType& Asset = WebFrontend::getAsset(Index);

        if(Path == Asset.Path) {
            sendSimpleResponse(Socket, "200 OK", Asset.ContentType,
                               reinterpret_cast<const char*>(Asset.Bytes), Asset.Size, Asset.IsGzipped);
            return false;
        }
    }

    static constexpr char NotFound[]{"no such route on the simulator\n"};
    sendSimpleResponse(Socket, "404 Not Found", "text/plain", NotFound, sizeof(NotFound) - 1u, false);
    return false;
}


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
void answerUpdate(SocketType Socket, size_t Received)
{
    static constexpr size_t SmallestPlausibleImage{size_t{64u} * 1024u};

    if(Received >= SmallestPlausibleImage) {
        static constexpr char Accepted[]{"{\"ok\":true}"};

        Serial.print("Web: update of ");
        Serial.print(static_cast<int>(Received / 1024u));
        Serial.println(" kB accepted - nothing was written, this is the simulator");
        sendSimpleResponse(Socket, "200 OK", "application/json", Accepted, sizeof(Accepted) - 1u, false);
        return;
    }

    char Refused[160];
    const int Written = snprintf(Refused, sizeof(Refused),
                                 "{\"ok\":false,\"error\":\"%zu bytes is too small to be a firmware image\"}",
                                 Received);

    sendSimpleResponse(Socket, "400 Bad Request", "application/json", Refused, static_cast<size_t>(Written), false);
}


/* What the request announced it would send, so a body can be read to its end rather than to
   whatever arrived in the first packet. Absent means nothing follows the headers. */
size_t announcedLength(const std::string& Request)
{
    static constexpr char Header[]{"content-length:"};

    std::string Lowered(Request);
    for(char& Character : Lowered) { Character = static_cast<char>(tolower(Character)); }

    const size_t Found = Lowered.find(Header);
    if(Found == std::string::npos) { return 0u; }

    return static_cast<size_t>(strtoul(&Request[Found + sizeof(Header) - 1u], nullptr, 10));
}

/* Handed to SerialShim, which takes a plain function pointer. */
void sendLineToClients(const char* Line)
{
    WebFrontend::getInstance().broadcastLine(Line);
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

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

    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) { Clients[Slot].Socket = InvalidSocket; }

    Listener = socket(AF_INET, SOCK_STREAM, 0);
    if(Listener == InvalidSocket) { return E_NOT_OK; }

    /* So a restart does not have to wait out the last connection's TIME_WAIT, which is a
       minute of "port already in use" for a program somebody restarts constantly. */
    int Reuse{1};
    setsockopt(Listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&Reuse), sizeof(Reuse));

    sockaddr_in Address{};
    Address.sin_family = AF_INET;
    Address.sin_port = htons(WEB_HOST_PORT);
    /* Loopback only. This serves a firmware console with no password on it, and the one on a
       clock is at least on somebody's own network; a desktop is not the place to open that to
       a coffee shop. */
    Address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(Listener, reinterpret_cast<sockaddr*>(&Address), sizeof(Address)) != 0) {
        closeSocket(Listener);
        Listener = InvalidSocket;
        Serial.println("Web: port 8080 is taken, the window runs without a browser");
        return E_NOT_OK;
    }
    if(listen(Listener, 4) != 0) {
        closeSocket(Listener);
        Listener = InvalidSocket;
        return E_NOT_OK;
    }
    setNonBlocking(Listener);

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

    for(;;) {
        const SocketType Accepted = accept(Listener, nullptr, nullptr);

        if(Accepted == InvalidSocket) { break; }
        if(!hasRoomForClient()) { closeSocket(Accepted); continue; }

        /* Read the request here and now. It is one packet on loopback, and a browser that
           sends half a request and stops is a case this tool need not survive gracefully. */
        std::string Request;
        char Chunk[4096];
        size_t HeaderEnd{std::string::npos};

        for(;;) {
            const int Read = static_cast<int>(recv(Accepted, Chunk, sizeof(Chunk), 0));

            if(Read <= 0) { break; }
            Request.append(Chunk, static_cast<size_t>(Read));

            HeaderEnd = Request.find("\r\n\r\n");
            if(HeaderEnd != std::string::npos) { break; }
        }

        if(Request.empty()) { closeSocket(Accepted); continue; }

        /* An image is the one body that arrives here, and it comes in many packets rather
           than the one every other request fits in. Drained rather than parsed: what the
           panel needs answered is how much of it turned up. Asked of the route and not only
           of the header, so that a body on any other request is not mistaken for one. */
        const size_t Announced = (Request.compare(0, 13, "POST /update ") == 0) ? announcedLength(Request) : 0u;

        if(Announced > 0u) {
            size_t Received = (HeaderEnd == std::string::npos) ? 0u : (Request.size() - HeaderEnd - 4u);

            while(Received < Announced) {
                const int Read = static_cast<int>(recv(Accepted, Chunk, sizeof(Chunk), 0));

                if(Read <= 0) { break; }
                Received += static_cast<size_t>(Read);
            }
            answerUpdate(Accepted, Received);
            closeSocket(Accepted);
            continue;
        }

        if(!answerRequest(Accepted, Request)) {
            closeSocket(Accepted);
            continue;
        }

        /* It was a handshake and it stood: this one stays and is broadcast to. */
        setNonBlocking(Accepted);
        for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
            if(Clients[Slot].Socket == InvalidSocket) {
                Clients[Slot].Socket = Accepted;
                Clients[Slot].Pending.clear();
                break;
            }
        }
    }

    /* What the open sockets have to say - commands, and the close that ends one. */
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        ClientType& Client = Clients[Slot];

        if(Client.Socket == InvalidSocket) { continue; }

        char Chunk[2048];
        const int Read = static_cast<int>(recv(Client.Socket, Chunk, sizeof(Chunk), 0));

        if(Read == 0) { dropClient(Client); continue; }
        if(Read > 0) { Client.Pending.append(Chunk, static_cast<size_t>(Read)); }

        uint8_t Opcode{0u};
        std::string Payload;

        while(takeFrame(Client.Pending, Opcode, Payload)) {
            if(Opcode == 0x8u) { dropClient(Client); break; }
            if(Opcode != 0x1u) { continue; }

            WebFrontend::injectCommand(Payload.data(), Payload.size());
        }
    }
} /* task */


void WebHost::end()
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) { dropClient(Clients[Slot]); }

    if(Listener != InvalidSocket) {
        closeSocket(Listener);
        Listener = InvalidSocket;
    }
}


bool WebHost::isListening() const
{
    return Listener != InvalidSocket;
}


bool WebHost::hasClients() const
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot].Socket != InvalidSocket) { return true; }
    }
    return false;
}


void WebHost::sendToEveryClient(bool IsBinary, const char* Payload, size_t Length)
{
    for(size_t Slot = 0u; Slot < WEB_HOST_MAX_CLIENTS; Slot++) {
        if(Clients[Slot].Socket != InvalidSocket) {
            sendFrame(Clients[Slot].Socket, IsBinary, Payload, Length);
        }
    }
}


/******************************************************************************************************************************************************
 *  W E B   T R A N S P O R T
******************************************************************************************************************************************************/
/* The simulator's half of the contract WebFrontend.h states, in terms of the host above. */

WebTransport& WebTransport::getInstance()
{
    static WebTransport SingletonInstance;
    return SingletonInstance;
}

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
    sendSimpleResponse(Socket, "200 OK", ContentType, Collected.data(), Collected.size(), false);
}

#endif // (WEB_FRONTEND_SUPPORT == STD_ON)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
