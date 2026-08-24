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
/**     \file       WebInterface.cpp
 *      \brief      Serves the console page and bridges its web socket to Serial
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
/* The framework's headers first, ahead of the Arduino.h that binds Serial to a macro. */
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_http_server.h>

#include "Arduino.h"

#include "System.h"
#include "WebFrontend.h"
#include "WebInterface.h"
#include "WebPage.h"
#include "WebTransport.h"
#include "WordclockSerial.h"

#include <atomic>
#include <string.h>
#include <unistd.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* Named HttpServer rather than Server: the Arduino core has a class Server in the global
   namespace, and an unqualified Server here is then ambiguous. */
httpd_handle_t HttpServer{nullptr};

constexpr size_t MaxClients{WEB_INTERFACE_MAX_CLIENTS};
constexpr int NoClient{-1};

/* The descriptors this server hands out, which it expects to be kept: unlike a library that
   owns its client list, the IDF's server reports a handshake and a close and leaves the
   bookkeeping here.

   Written only by the HTTP server's task - on a handshake and on a close - and read only by
   the firmware's task when it broadcasts. One writer is what makes the plain atomics enough,
   the same reasoning as for WordclockSerial's ring buffer.

   Reached through a function rather than left as an object with static storage duration,
   because the slots start at NoClient and not at zero, and zero is a descriptor a socket can
   actually be given. Initialising on first use is also what the singleton it used to live on
   did. */
struct ClientTableType {
    std::atomic<int> Slots[MaxClients];

    ClientTableType() {
        for(size_t Slot = 0u; Slot < MaxClients; Slot++) { Slots[Slot].store(NoClient); }
    }
};

ClientTableType& clientTable()
{
    static ClientTableType Table;
    return Table;
}

/* One frame to everybody, which is what both of WebTransport's send calls come down to - the
   type and the payload are all that differ between a line and a display frame. A send that
   fails is not retried and the client is not dropped here: the server's close hook owns that,
   and dropping a client from this side would race with it. */
void sendToEveryClient(httpd_ws_type_t Type, uint8_t* Payload, size_t Length)
{
    if((HttpServer == nullptr) || (Length == 0u)) { return; }

    httpd_ws_frame_t Frame{};
    Frame.type = Type;
    Frame.payload = Payload;
    Frame.len = Length;

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        const int Descriptor = clientTable().Slots[Slot].load(std::memory_order_acquire);

        if(Descriptor != NoClient) { httpd_ws_send_frame_async(HttpServer, Descriptor, &Frame); }
    }
}

/* Handed to WordclockSerial, which takes a plain function pointer - so the singleton is
   reached from here rather than carried along. */
void sendLineToClients(const char* Line)
{
    WebInterface::getInstance().broadcastLine(Line);
}

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/******************************************************************************************************************************************************
  isRequestAuthorised()
******************************************************************************************************************************************************/
/*! \brief          Whether this request may be answered, and the 401 when it may not
 *
 *  \return         true when the handler may go on
 *
 *  \details        Basic authentication, and what it is worth saying plainly: the credential
 *                  travels base64-encoded over plain http, which is not encryption. What this
 *                  keeps out is a guest who opens the page and starts sending commands, or
 *                  installing firmware. It keeps out nobody who can watch the traffic, and
 *                  that would need https, which a clock cannot offer credibly.
 *
 *                  Off unless a password was stored, so an update locks nobody out of a clock
 *                  that is already on a wall, and the access point of a first setup still
 *                  answers.
 *
 *                  Every route goes through this, the web socket's handshake included - a
 *                  console that asked for a password and then took commands on an unchecked
 *                  socket would be a lock on the wrong door.
******************************************************************************************************************************************************/
bool isRequestAuthorised(httpd_req_t* Request)
{
    if(!System::getInstance().isConsoleProtected()) { return true; }

    /* "Basic " and the blob. Sized for the longest credential the store can hold, and a
        header longer than that cannot be one this clock would accept. */
    char Header[SYSTEM_PASSWORD_STRING_LENGTH + 128u]{};
    static constexpr char Scheme[]{"Basic "};

    if(httpd_req_get_hdr_value_str(Request, "Authorization", Header, sizeof(Header)) == ESP_OK) {
        if(strncmp(Header, Scheme, sizeof(Scheme) - 1u) == 0) {
            if(System::getInstance().isConsoleCredentialValid(&Header[sizeof(Scheme) - 1u])) { return true; }
        }
    }

    /* The realm carries the user name, because it is the one field a browser cannot guess and
       it is not a secret - the password beside it is. */
    httpd_resp_set_status(Request, "401 Unauthorized");
    httpd_resp_set_hdr(Request, "WWW-Authenticate", "Basic realm=\"Wordclock, user wordclock\"");
    httpd_resp_set_type(Request, "text/plain");
    httpd_resp_send(Request, "this clock asks for a password\n", HTTPD_RESP_USE_STRLEN);

    return false;
}


/******************************************************************************************************************************************************
  sendPage()
******************************************************************************************************************************************************/
/*! \brief          Serves one of the two pages straight out of flash
 *  \details        Sent compressed, which is how it is stored: the pages were gzipped at build
 *                  time, so this only pushes the bytes and the browser unpacks them.
******************************************************************************************************************************************************/
esp_err_t sendPage(httpd_req_t* Request, const uint8_t* Page, size_t Size)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "text/html");
    httpd_resp_set_hdr(Request, "Content-Encoding", "gzip");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(Page), Size);
}


/******************************************************************************************************************************************************
  handleRoot()
******************************************************************************************************************************************************/
/*! \brief          Serves the page a clock hands out at "/"
 *  \details        The one made for what somebody changes: a colour, a brightness, an
 *                  animation. Nobody types a path - they type the clock's address and take
 *                  what comes - so what comes is the page for the frequent things, and the
 *                  console is one link away.
******************************************************************************************************************************************************/
esp_err_t handleRoot(httpd_req_t* Request)
{
    return sendPage(Request, WebAppGzip, WebAppGzipSize);
}


/******************************************************************************************************************************************************
  handleConsole()
******************************************************************************************************************************************************/
/*! \brief          Serves the console at "/console"
 *  \details        Everything the page at "/" does not cover, which is most of the command
 *                  set: it draws a group per command out of the catalog, so a command added to
 *                  the firmware appears there without a page being touched. That is why it
 *                  moved rather than being replaced.
******************************************************************************************************************************************************/
esp_err_t handleConsole(httpd_req_t* Request)
{
    return sendPage(Request, WebPageGzip, WebPageGzipSize);
}


/******************************************************************************************************************************************************
  handleManifest()
******************************************************************************************************************************************************/
/*! \brief          Serves the web app manifest, which is what makes the console installable
 *  \details        Nothing on the clock reads it - a browser does, to put the console on a
 *                  home screen with an icon and start it without an address bar. Sent as it
 *                  is, unlike the page: it is 485 bytes, and compressing it would save
 *                  fewer than 250 of them for a header on the wire and an inflate in
 *                  anything that wants to read it - curl and this backend's own test
 *                  included.
******************************************************************************************************************************************************/
esp_err_t handleManifest(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "application/manifest+json");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(WebManifest), WebManifestSize);
}


/******************************************************************************************************************************************************
  sendUpdateResult()
******************************************************************************************************************************************************/
/*! \brief          Answers the update route, in the one shape the page reads
 *  \details        JSON like the other two routes that answer data, and the HTTP status set
 *                  with it - so the page can branch on the status and still have a sentence
 *                  to show. The reason travels as text on purpose: what goes wrong here is
 *                  a wrong file or a dropped connection, and a number for that would only
 *                  have to be translated back somewhere.
******************************************************************************************************************************************************/
esp_err_t sendUpdateResult(httpd_req_t* Request, bool Ok, const char* Reason)
{
    char Body[192u];

    httpd_resp_set_type(Request, "application/json");

    if(Ok) {
        httpd_resp_set_status(Request, "200 OK");
        snprintf(Body, sizeof(Body), "{\"ok\":true}");
    } else {
        httpd_resp_set_status(Request, "400 Bad Request");
        /* %.120s, because errorString() is the framework's text and the buffer is ours. */
        snprintf(Body, sizeof(Body), "{\"ok\":false,\"error\":\"%.120s\"}", Reason);
        Serial.print(F("Web: update refused - "));
        Serial.println(Reason);
    }

    return httpd_resp_send(Request, Body, HTTPD_RESP_USE_STRLEN);
}


/******************************************************************************************************************************************************
  handleUpdate()
******************************************************************************************************************************************************/
/*! \brief          Takes a new firmware image over HTTP and reboots into it
 *
 *  \return         ESP_OK once an answer has been sent, whichever answer that was
 *
 *  \details        The body is the image itself, not a form: a multipart parser in flash
 *                  would exist to undo something the browser only does because a <form>
 *                  asked it to, and the page sends the file as the request body instead.
 *                  That is also what makes `curl --data-binary` a first-class way in, which
 *                  is the one that works when the page is the thing that broke.
 *
 *                  The image goes to the partition the running one is not in, and the
 *                  bootloader is pointed at it only by end() - so an upload that stops
 *                  halfway leaves the clock running what it was running. That is the whole
 *                  safety argument for doing this at all, and it is the framework's rather
 *                  than ours.
 *
 *                  The reboot is asked for and not carried out: this handler's answer is
 *                  still in the server's buffer, and a controller that restarts here sends
 *                  the browser nothing at all - which reads as a failed update rather than
 *                  a finished one. System::restart() already defers to the application's
 *                  tick for exactly this reason, so it is what carries it out.
******************************************************************************************************************************************************/
esp_err_t handleUpdate(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    /* Announced rather than measured, and it has to be: Update needs the size before the
       first byte, to know which partition can hold it and to erase it. A body that then
       turns out shorter is caught by end() below, which refuses to finish an image that is
       not all there. */
    const size_t Announced = Request->content_len;

    if(Announced == 0u) { return sendUpdateResult(Request, false, "no image in the request body"); }

    if(!Update.begin(Announced)) {
        /* The usual one is an image larger than the partition, which is worth saying as
           itself rather than as "update failed": it means the wrong file was picked - the
           factory image instead of the plain one, most likely. */
        return sendUpdateResult(Request, false, Update.errorString());
    }

    /* One kilobyte at a time, on the server's own task. Larger buffers buy nothing here:
       the flash write is what takes the time, and this runs beside the clock rather than
       inside its tick, so the display keeps its frames throughout. */
    char Chunk[1024u];
    size_t Received = 0u;

    while(Received < Announced) {
        const int Read = httpd_req_recv(Request, Chunk, sizeof(Chunk));

        /* A dropped connection halfway through, which is the case the abort exists for:
           without it the next attempt would find the partition still claimed. */
        if(Read <= 0) {
            Update.abort();
            return sendUpdateResult(Request, false, "the upload stopped before the image was complete");
        }
        if(Update.write(reinterpret_cast<uint8_t*>(Chunk), static_cast<size_t>(Read)) != static_cast<size_t>(Read)) {
            const char* Reason = Update.errorString();
            Update.abort();
            return sendUpdateResult(Request, false, Reason);
        }
        Received += static_cast<size_t>(Read);
    }

    if(!Update.end(true)) { return sendUpdateResult(Request, false, Update.errorString()); }

    System::getInstance().restart();

    return sendUpdateResult(Request, true, "");
}


/******************************************************************************************************************************************************
  handleIcon192() / handleIcon512()
******************************************************************************************************************************************************/
/*! \brief          Serves the home screen icons
 *  \details        Two sizes because that is what a manifest is asked for: the small one is
 *                  the icon itself and what iOS takes, the large one is what Android draws
 *                  the splash screen from. Sent as they are - a PNG is already deflated, so
 *                  gzipping one would add a header and save nothing.
******************************************************************************************************************************************************/
esp_err_t handleIcon192(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "image/png");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(WebIcon192), WebIcon192Size);
}

esp_err_t handleIcon512(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "image/png");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(WebIcon512), WebIcon512Size);
}


/******************************************************************************************************************************************************
  handleCommands() / handleDisplay()
******************************************************************************************************************************************************/
/*! \brief          Serves the command catalog and the panel's letters
 *
 *  \details        Both documents are written by WebFrontend, shared with the RP2350 backend:
 *                  what a page is sent is a property of the catalog and the letter table, not
 *                  of a controller. They were generated here once, and identically in the
 *                  other backend - two functions that had to be kept the same by hand and
 *                  whose divergence would have surfaced in a browser rather than in a test.
 *
 *                  What is left here is what is genuinely this server's: the password, the
 *                  content type, and a body to write into.
 *
 *  \return         ESP_OK
******************************************************************************************************************************************************/
esp_err_t handleCommands(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    WebResponseBody Body(Request, "application/json");
    WebFrontend::writeCommands(Body);

    return ESP_OK;
}

esp_err_t handleDisplay(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    WebResponseBody Body(Request, "application/json");
    WebFrontend::writeDisplay(Body);

    return ESP_OK;
}


/******************************************************************************************************************************************************
  handleSocket()
******************************************************************************************************************************************************/
/*! \brief          Takes one web socket frame and injects it as typed characters
 *  \details        A GET is the handshake, and returning E_OK from it is what completes the
 *                  upgrade - the frame handling below only ever sees later requests.
 *
 *                  The command is injected followed by the end-of-message character, unless
 *                  the browser already sent one. That keeps the page free of protocol
 *                  bookkeeping: it sends the command text, the terminator is this side's
 *                  business.
 *
 *  \return         ESP_OK unless the frame could not be read
******************************************************************************************************************************************************/
esp_err_t handleSocket(httpd_req_t* Request)
{
    /* The handshake only. A frame arriving later has no headers to carry a credential, so
       the check belongs at the one moment the browser does send them - which is the GET the
       server calls this handler with before the socket exists. */
    if(Request->method == HTTP_GET) {
        if(!isRequestAuthorised(Request)) { return ESP_OK; }
    }

    if(Request->method == HTTP_GET) {
        WebInterface::getInstance().onClientOpened(httpd_req_to_sockfd(Request));
        return ESP_OK;
    }

    uint8_t Payload[WEB_FRONTEND_MAX_FRAME_LENGTH + 1u]{};
    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = Payload;

    /* Asking with the buffer's size rather than in two steps: a frame that does not fit is
       refused whole, so half a command can never reach the parser. */
    const esp_err_t Result = httpd_ws_recv_frame(Request, &Frame, WEB_FRONTEND_MAX_FRAME_LENGTH);
    if(Result != ESP_OK) { return Result; }

    if(Frame.type == HTTPD_WS_TYPE_CLOSE) {
        WebInterface::getInstance().onClientClosed(httpd_req_to_sockfd(Request));
        return ESP_OK;
    }
    if(Frame.type != HTTPD_WS_TYPE_TEXT) { return ESP_OK; }

    /* The terminator and the empty-frame case are WebFrontend's, so that a command coming off
       a socket reaches the parser the same way on either backend. */
    WebFrontend::injectCommand(reinterpret_cast<const char*>(Payload), Frame.len);

    return ESP_OK;
}


/******************************************************************************************************************************************************
  onSocketClosed()
******************************************************************************************************************************************************/
/*! \brief          Drops a client the server has finished with
 *  \details        Registered as the server's close hook rather than guessed from a failing
 *                  send: a browser that is simply gone never sends a close frame, and its
 *                  descriptor would otherwise be handed to send() for the rest of the run.
******************************************************************************************************************************************************/
void onSocketClosed(httpd_handle_t, int Descriptor)
{
    WebInterface::getInstance().onClientClosed(Descriptor);
    close(Descriptor);
}

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  begin()
******************************************************************************************************************************************************/
/*! \brief          Starts the server and takes over the output lines
 *
 *  \return         E_OK if the server is listening
******************************************************************************************************************************************************/
StdReturnType WebInterface::begin()
{
    httpd_config_t Config = HTTPD_DEFAULT_CONFIG();
    Config.server_port = WEB_INTERFACE_PORT;
    /* The sockets the server keeps have to cover the console clients plus the ones fetching
       the page, or a reload can push a console off. */
    Config.max_open_sockets = WEB_INTERFACE_MAX_CLIENTS + 2u;
    /* Said rather than left to the default, which is 8 and was exactly the count until the
       console moved to its own route. A route past the limit is refused by the registration
       below and nothing here reads that answer, so the failure would be one route quietly
       missing - and the test that counts them is what would find it, on a host, rather than a
       browser on somebody's wall. The next route to be added has to raise this number with it. */
    Config.max_uri_handlers = 9u;
    Config.close_fn = onSocketClosed;
    /* Its own task, so nothing here runs inside the firmware's tick. */
    Config.core_id = 0;

    if(httpd_start(&HttpServer, &Config) != ESP_OK) {
        HttpServer = nullptr;
        Serial.println(F("Web: server failed to start, console stays on the UART"));
        return E_NOT_OK;
    }

    static const httpd_uri_t RootUri{"/", HTTP_GET, handleRoot, nullptr, false, false, nullptr};
    static const httpd_uri_t ConsoleUri{"/console", HTTP_GET, handleConsole, nullptr, false, false, nullptr};
    static const httpd_uri_t CommandsUri{"/commands", HTTP_GET, handleCommands, nullptr, false, false, nullptr};
    static const httpd_uri_t DisplayUri{"/display", HTTP_GET, handleDisplay, nullptr, false, false, nullptr};
    static const httpd_uri_t ManifestUri{"/manifest.webmanifest", HTTP_GET, handleManifest, nullptr, false, false, nullptr};
    static const httpd_uri_t Icon192Uri{"/icon-192.png", HTTP_GET, handleIcon192, nullptr, false, false, nullptr};
    static const httpd_uri_t Icon512Uri{"/icon-512.png", HTTP_GET, handleIcon512, nullptr, false, false, nullptr};
    static const httpd_uri_t UpdateUri{"/update", HTTP_POST, handleUpdate, nullptr, false, false, nullptr};
    static const httpd_uri_t SocketUri{"/ws", HTTP_GET, handleSocket, nullptr, true, false, nullptr};

    httpd_register_uri_handler(HttpServer, &RootUri);
    httpd_register_uri_handler(HttpServer, &ConsoleUri);
    httpd_register_uri_handler(HttpServer, &CommandsUri);
    httpd_register_uri_handler(HttpServer, &DisplayUri);
    httpd_register_uri_handler(HttpServer, &ManifestUri);
    httpd_register_uri_handler(HttpServer, &Icon192Uri);
    httpd_register_uri_handler(HttpServer, &Icon512Uri);
    httpd_register_uri_handler(HttpServer, &UpdateUri);
    httpd_register_uri_handler(HttpServer, &SocketUri);

    /* Only now, so a line printed during startup cannot reach a half-built server. */
    WordclockSerial::getInstance().setLineSink(sendLineToClients);

    if(MDNS.begin(WEB_INTERFACE_HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_INTERFACE_PORT);
        Serial.print(F("Web: http://"));
        Serial.print(WEB_INTERFACE_HOSTNAME);
        Serial.println(F(".local"));
    } else {
        /* Not fatal: the address still works, it just has to be looked up. */
        Serial.println(F("Web: mDNS unavailable, reach the clock by its address"));
    }

    return E_OK;
} /* begin */


/******************************************************************************************************************************************************
  broadcastLine()
******************************************************************************************************************************************************/
/*! \brief          Sends one finished line to every open socket
 *  \details        The widening to UTF-8 and the bound on it are WebFrontend's, shared with
 *                  the other backend; what is this file's is the walk over the descriptors,
 *                  in sendText() below.
******************************************************************************************************************************************************/
void WebInterface::broadcastLine(const char* Line)
{
    WebFrontend::getInstance().broadcastLine(Line);
} /* broadcastLine */


/******************************************************************************************************************************************************
  broadcastFrame()
******************************************************************************************************************************************************/
/*! \brief          Sends the pixel buffer to every open socket, when it changed
 *  \details        The rate limit, the gathering and the comparison against the last frame
 *                  are WebFrontend's, shared with the other backend - none of the three has
 *                  anything to do with which server is underneath. What is this file's is
 *                  sendBinary() below.
 *
 *  \return         -
******************************************************************************************************************************************************/
void WebInterface::broadcastFrame()
{
    WebFrontend::getInstance().broadcastFrame();
} /* broadcastFrame */


/******************************************************************************************************************************************************
 *  W E B   T R A N S P O R T
******************************************************************************************************************************************************/
/* This backend's half of the contract WebFrontend.h states. Defined here rather than in a
   source of its own because what it reads is this file's: the server handle above and the
   descriptor table on WebInterface. */

WebTransport& WebTransport::getInstance()
{
    static WebTransport SingletonInstance;
    return SingletonInstance;
}

bool WebTransport::isListening() const
{
    return HttpServer != nullptr;
}

bool WebTransport::hasClients() const
{
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(clientTable().Slots[Slot].load(std::memory_order_acquire) != NoClient) { return true; }
    }
    return false;
}

void WebTransport::sendText(const char* Text, size_t Length)
{
    /* const_cast because httpd's frame struct is the same type used for receiving, where the
       payload is written into. Nothing on the sending path writes through it. */
    sendToEveryClient(HTTPD_WS_TYPE_TEXT, reinterpret_cast<uint8_t*>(const_cast<char*>(Text)), Length);
}

void WebTransport::sendBinary(const byte* Bytes, size_t Length)
{
    sendToEveryClient(HTTPD_WS_TYPE_BINARY, const_cast<uint8_t*>(Bytes), Length);
}


/******************************************************************************************************************************************************
 *  W E B   R E S P O N S E   B O D Y
******************************************************************************************************************************************************/
/* A chunked response, which is what this server offers: a chunk handed over is a chunk on the
   wire, and the empty one at the end is what closes it. */

WebResponseBody::WebResponseBody(struct httpd_req* sRequest, const char* ContentType)
    : Request(sRequest)
{
    httpd_resp_set_type(Request, ContentType);
}

void WebResponseBody::write(const char* Data, size_t Length)
{
    httpd_resp_send_chunk(Request, Data, static_cast<ssize_t>(Length));
}

void WebResponseBody::finish()
{
    httpd_resp_send_chunk(Request, nullptr, 0);
}


/******************************************************************************************************************************************************
  onClientOpened()
******************************************************************************************************************************************************/
/*! \brief          Remembers a socket to broadcast to
 *  \details        A full table drops the newcomer rather than an established console, and
 *                  says so on the UART - silently serving a page whose socket then never
 *                  answers is the more confusing failure.
******************************************************************************************************************************************************/
void WebInterface::onClientOpened(int Descriptor)
{
    /* Told before the table is even looked at: either way a client just arrived, and it has to
       be shown the display as it stands rather than as it next changes. That includes the
       descriptor already being in the table - the system reuses them, so the same number
       coming back is a new client rather than the old one. */
    WebFrontend::getInstance().onClientOpened();

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(clientTable().Slots[Slot].load(std::memory_order_relaxed) == Descriptor) { return; }
    }

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(clientTable().Slots[Slot].load(std::memory_order_relaxed) == NoClient) {
            clientTable().Slots[Slot].store(Descriptor, std::memory_order_release);
            return;
        }
    }

    Serial.println(F("Web: too many consoles, this one gets no answers"));
} /* onClientOpened */


/******************************************************************************************************************************************************
  onClientClosed()
******************************************************************************************************************************************************/
void WebInterface::onClientClosed(int Descriptor)
{
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(clientTable().Slots[Slot].load(std::memory_order_relaxed) == Descriptor) {
            clientTable().Slots[Slot].store(NoClient, std::memory_order_release);
            return;
        }
    }
} /* onClientClosed */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
