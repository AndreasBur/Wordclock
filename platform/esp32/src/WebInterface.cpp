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

#include "Communication.h"
#include "DisplayCharacters.h"
#include "MessageCatalog.h"
#include "System.h"
#include "WebInterface.h"
#include "WebPage.h"
#include "WordclockSerial.h"

#include <string.h>
#include <unistd.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* Named HttpServer rather than Server: the Arduino core has a class Server in the global
   namespace, and an unqualified Server here is then ambiguous. */
httpd_handle_t HttpServer{nullptr};

/* Handed to WordclockSerial, which takes a plain function pointer - so the singleton is
   reached from here rather than carried along. */
void sendLineToClients(const char* Line)
{
    WebInterface::getInstance().broadcastLine(Line);
}

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/* One Latin-1 byte as UTF-8, into Target, answering how many bytes that took. The whole of
   Latin-1 widens by this one rule, so no table is needed: below 0x80 a byte is itself, and
   above it becomes two.

   Both ways out of here need it, for different reasons. The letters served by /display are
   Latin-1 because the table holds one byte per letter, and JSON is UTF-8 - a raw 0xDC there
   makes the whole document invalid rather than one letter wrong.

   An answer line needs it for a harder reason: it leaves as a web socket *text* frame, and
   RFC 6455 requires those to be valid UTF-8. A raw 0xF6 in one does not draw a wrong
   character, it makes the browser close the connection. That is reachable rather than
   theoretical - command 8 answers with the overlay text it was given, and an umlaut in that
   text is exactly what the font tables carry umlauts for. */
byte toUtf8(byte Latin1, char* Target)
{
    if(Latin1 < 0x80u) {
        Target[0u] = static_cast<char>(Latin1);
        return 1u;
    }
    Target[0u] = static_cast<char>(0xC0u | (Latin1 >> 6u));
    Target[1u] = static_cast<char>(0x80u | (Latin1 & 0x3Fu));
    return 2u;
}

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
  C H U N K   W R I T E R
******************************************************************************************************************************************************/
/* Collects the catalog's JSON in a small buffer and hands it over in chunks, so neither the
   whole document nor any allocation is needed to serve it. */
class ChunkWriter
{
  private:
    static constexpr size_t Capacity{256u};

    httpd_req_t* Request;
    char Buffer[Capacity]{};
    size_t Used{0u};

    void flush() {
        if(Used == 0u) { return; }

        httpd_resp_send_chunk(Request, Buffer, Used);
        Used = 0u;
    }

  public:
    explicit ChunkWriter(httpd_req_t* sRequest) : Request(sRequest) { }

    void put(char Character) {
        if(Used == Capacity) { flush(); }

        Buffer[Used++] = Character;
    }

    void put(const char* Text) {
        if(Text == nullptr) { return; }

        for(const char* Character = Text; *Character != '\0'; Character++) { put(*Character); }
    }

    void putNumber(uint16_t Number) {
        char Digits[8]{};

        snprintf(Digits, sizeof(Digits), "%u", static_cast<unsigned>(Number));
        put(Digits);
    }

    /* Escaped, because a label is data: a quote in one would otherwise produce a document
       the browser refuses whole, and that failure looks like a server fault. */
    void putString(const char* Text) {
        put('"');
        for(const char* Character = Text; (Text != nullptr) && (*Character != '\0'); Character++) {
            if((*Character == '"') || (*Character == '\\')) { put('\\'); }
            put(*Character);
        }
        put('"');
    }

    /* The empty chunk is what ends a chunked response. */
    void finish() {
        flush();
        httpd_resp_send_chunk(Request, nullptr, 0);
    }
};

/******************************************************************************************************************************************************
  handleCommands()
******************************************************************************************************************************************************/
/*! \brief          Serves the command catalog, so the page can build its own form
 *  \details        Generated from MessageCatalog rather than written out a second time.
 *                  That table is what the simulator's message builder derives its whole
 *                  dialog from, down to the input hints; serving it here makes the browser
 *                  a second renderer of the same description, so a command added to the
 *                  catalog shows up in both front ends and on the wire at once.
 *
 *  \return         ESP_OK
******************************************************************************************************************************************************/
esp_err_t handleCommands(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "application/json");

    ChunkWriter Writer(Request);
    Writer.put('[');

    for(byte Index = 0u; Index < MessageCatalog::getNumberOfCommands(); Index++) {
        const MessageCatalog::CommandType& Command = MessageCatalog::getCommand(Index);

        if(Index > 0u) { Writer.put(','); }
        Writer.put("{\"number\":");
        Writer.putNumber(Command.Number);
        Writer.put(",\"label\":");
        Writer.putString(Command.Label);
        Writer.put(",\"options\":[");

        for(byte OptionIndex = 0u; OptionIndex < Command.NumberOfOptions; OptionIndex++) {
            const MessageCatalog::OptionType& Option = Command.Options[OptionIndex];

            if(OptionIndex > 0u) { Writer.put(','); }
            Writer.put("{\"short\":\"");
            Writer.put(Option.ShortName);
            Writer.put("\",\"label\":");
            Writer.putString(Option.Label);
            Writer.put(",\"type\":");
            Writer.putNumber(static_cast<uint16_t>(Option.Argument));
            Writer.put(",\"min\":");
            Writer.putNumber(Option.Minimum);
            Writer.put(",\"max\":");
            Writer.putNumber(Option.Maximum);

            /* Only where it is set, so the page's form stays as it was for every option
               that can be sent, and the read-only fields it must not offer are the ones
               that say so. */
            if(Option.ReadOnly) { Writer.put(",\"readonly\":true"); }

            if((Option.ValueNames != nullptr) && (Option.NumberOfValueNames > 0u)) {
                Writer.put(",\"values\":[");
                for(byte NameIndex = 0u; NameIndex < Option.NumberOfValueNames; NameIndex++) {
                    if(NameIndex > 0u) { Writer.put(','); }
                    Writer.putString(Option.ValueNames[NameIndex]);
                }
                Writer.put(']');
            }
            Writer.put('}');
        }
        Writer.put("]}");
    }

    Writer.put(']');
    Writer.finish();

    return ESP_OK;
}


/******************************************************************************************************************************************************
  handleDisplay()
******************************************************************************************************************************************************/
/*! \brief          Serves the panel's shape and its letters
 *  \details        The page draws a grid of letters and has to know which, and there is
 *                  exactly one table of them - DisplayCharacters in the firmware. Serving
 *                  it keeps the browser from carrying a second copy, which is the
 *                  duplication the simulator used to have and no longer does.
 *
 *                  The table stores one byte per letter, so the umlauts are Latin-1 and are
 *                  widened to UTF-8 on the way out: JSON is UTF-8, and a raw 0xDC makes the
 *                  whole document invalid rather than one letter wrong.
 *
 *  \return         ESP_OK
******************************************************************************************************************************************************/
esp_err_t handleDisplay(httpd_req_t* Request)
{
    if(!isRequestAuthorised(Request)) { return ESP_OK; }

    httpd_resp_set_type(Request, "application/json");

    const DisplayCharacters Letters;
    ChunkWriter Writer(Request);

    Writer.put("{\"columns\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS);
    Writer.put(",\"rows\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    Writer.put(",\"letters\":\"");

    for(byte Index = 0u; Index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; Index++) {
        char Utf8[2u];
        const byte Length = toUtf8(static_cast<byte>(Letters.getCharacter(Index)), Utf8);

        for(byte Byte = 0u; Byte < Length; Byte++) { Writer.put(Utf8[Byte]); }
    }

    Writer.put("\"}");
    Writer.finish();

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

    uint8_t Payload[WEB_INTERFACE_MAX_FRAME_LENGTH + 1u]{};
    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = Payload;

    /* Asking with the buffer's size rather than in two steps: a frame that does not fit is
       refused whole, so half a command can never reach the parser. */
    const esp_err_t Result = httpd_ws_recv_frame(Request, &Frame, WEB_INTERFACE_MAX_FRAME_LENGTH);
    if(Result != ESP_OK) { return Result; }

    if(Frame.type == HTTPD_WS_TYPE_CLOSE) {
        WebInterface::getInstance().onClientClosed(httpd_req_to_sockfd(Request));
        return ESP_OK;
    }
    if((Frame.type != HTTPD_WS_TYPE_TEXT) || (Frame.len == 0u)) { return ESP_OK; }

    WordclockSerial& Port = WordclockSerial::getInstance();
    Port.inject(reinterpret_cast<const char*>(Payload), Frame.len);

    if(Payload[Frame.len - 1u] != Communication::getEndOfMessageChar()) {
        const char Terminator = Communication::getEndOfMessageChar();
        Port.inject(&Terminator, 1u);
    }

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
 *  \details        Runs in the firmware's task, which is allowed to send asynchronously into
 *                  the server. A send that fails is not retried and the client is not
 *                  dropped here - the server's close hook owns that, and dropping a client
 *                  from this side would race with it.
******************************************************************************************************************************************************/
void WebInterface::broadcastLine(const char* Line)
{
    if((HttpServer == nullptr) || (Line == nullptr)) { return; }

    /* Widened rather than sent as it stands - see toUtf8(). Two bytes per Latin-1 byte is
       the worst case, so a line that is nothing but umlauts still fits and there is no
       truncation to expect. Sized from the producer's own line length rather than from a
       number of its own, because that is the buffer the line was assembled in.

       The bound is still checked. Nothing longer can arrive today, but this takes a plain
       pointer and the sink it is installed as is a function pointer - the day a second
       producer appears, an assumption written only in a comment is the one that gives. */
    char Payload[2u * WORDCLOCK_SERIAL_LINE_LENGTH];
    size_t Used{0u};

    for(const char* Character = Line; (*Character != '\0') && ((Used + 2u) <= sizeof(Payload)); Character++) {
        Used += toUtf8(static_cast<byte>(*Character), &Payload[Used]);
    }

    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = reinterpret_cast<uint8_t*>(Payload);
    Frame.len = Used;

    if(Frame.len == 0u) { return; }

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        const int Descriptor = Clients[Slot].load(std::memory_order_acquire);

        if(Descriptor != NoClient) { httpd_ws_send_frame_async(HttpServer, Descriptor, &Frame); }
    }
} /* broadcastLine */


/******************************************************************************************************************************************************
  broadcastFrame()
******************************************************************************************************************************************************/
/*! \brief          Sends the pixel buffer to every open socket, when it changed
 *  \details        Rate limited first and compared second, so a display that changes on
 *                  every tick still costs one frame per interval, and one that stands still
 *                  costs a comparison.
 *
 *                  Sent in the strip's own byte order, green first: it is what the buffer
 *                  already holds, and the page is told the shape by /display rather than
 *                  guessing it. Taken from the output pixel, so what goes out is dimmed the
 *                  way the strip is - which is what lets the page blank itself when the
 *                  display is switched off, without a redraw to tell it so. The page reads
 *                  the bytes for that on/off state only and paints its own colour; the wx
 *                  window is the colour-accurate view.
 *
 *  \return         -
******************************************************************************************************************************************************/
void WebInterface::broadcastFrame()
{
    if(HttpServer == nullptr) { return; }

    const bool Forced = ForceFrame.exchange(false, std::memory_order_acq_rel);

    if(!Forced) {
        if(FrameCountdown > 0u) { FrameCountdown--; return; }
    }
    FrameCountdown = WEB_INTERFACE_FRAME_INTERVAL_TICKS - 1u;

    /* Nothing to send to, so nothing is even gathered. */
    bool HasClient = false;
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_acquire) != NoClient) { HasClient = true; }
    }
    if(!HasClient) { return; }

    byte Frame[FrameSize];
    byte* Target = Frame;
    const Pixels& Strip = Pixels::getInstance();

    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        const Pixel Colour = Strip.getOutputPixel(Index);

        *Target++ = Colour.getGreen();
        *Target++ = Colour.getRed();
        *Target++ = Colour.getBlue();
    }

    if(!Forced && (memcmp(Frame, LastFrame, FrameSize) == 0)) { return; }
    memcpy(LastFrame, Frame, FrameSize);

    httpd_ws_frame_t WsFrame{};
    WsFrame.type = HTTPD_WS_TYPE_BINARY;
    WsFrame.payload = Frame;
    WsFrame.len = FrameSize;

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        const int Descriptor = Clients[Slot].load(std::memory_order_acquire);

        if(Descriptor != NoClient) { httpd_ws_send_frame_async(HttpServer, Descriptor, &WsFrame); }
    }
} /* broadcastFrame */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  addClient()
******************************************************************************************************************************************************/
/*! \brief          Remembers a socket to broadcast to
 *  \details        A full table drops the newcomer rather than an established console, and
 *                  says so on the UART - silently serving a page whose socket then never
 *                  answers is the more confusing failure.
******************************************************************************************************************************************************/
void WebInterface::addClient(int Descriptor)
{
    /* Set before the table is even looked at: either way a client just arrived, and it has to
       be shown the display as it stands rather than as it next changes. That includes the
       descriptor already being in the table - the system reuses them, so the same number
       coming back is a new client rather than the old one. */
    ForceFrame.store(true, std::memory_order_release);

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == Descriptor) { return; }
    }

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == NoClient) {
            Clients[Slot].store(Descriptor, std::memory_order_release);
            return;
        }
    }

    Serial.println(F("Web: too many consoles, this one gets no answers"));
} /* addClient */


/******************************************************************************************************************************************************
  removeClient()
******************************************************************************************************************************************************/
void WebInterface::removeClient(int Descriptor)
{
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == Descriptor) {
            Clients[Slot].store(NoClient, std::memory_order_release);
            return;
        }
    }
} /* removeClient */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
