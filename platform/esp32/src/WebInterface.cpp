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
#include <esp_http_server.h>

#include "Arduino.h"

#include "Communication.h"
#include "DisplayCharacters.h"
#include "MessageCatalog.h"
#include "WebInterface.h"
#include "WebPage.h"

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
/******************************************************************************************************************************************************
  handleRoot()
******************************************************************************************************************************************************/
/*! \brief          Serves the console page straight out of flash
 *  \details        Sent compressed, which is how it is stored: the page was gzipped at
 *                  build time, so this only pushes the bytes and the browser unpacks them.
******************************************************************************************************************************************************/
esp_err_t handleRoot(httpd_req_t* Request)
{
    httpd_resp_set_type(Request, "text/html");
    httpd_resp_set_hdr(Request, "Content-Encoding", "gzip");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(WebPageGzip), WebPageGzipSize);
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
    httpd_resp_set_type(Request, "application/json");

    const DisplayCharacters Letters;
    ChunkWriter Writer(Request);

    Writer.put("{\"columns\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS);
    Writer.put(",\"rows\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    Writer.put(",\"letters\":\"");

    for(byte Index = 0u; Index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; Index++) {
        const byte Letter = static_cast<byte>(Letters.getCharacterFast(Index));

        if(Letter < 0x80u) {
            Writer.put(static_cast<char>(Letter));
        } else {
            /* The whole of Latin-1 widens by this rule, so the two umlauts need no table. */
            Writer.put(static_cast<char>(0xC0u | (Letter >> 6u)));
            Writer.put(static_cast<char>(0x80u | (Letter & 0x3Fu)));
        }
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
    Config.close_fn = onSocketClosed;
    /* Its own task, so nothing here runs inside the firmware's tick. */
    Config.core_id = 0;

    if(httpd_start(&HttpServer, &Config) != ESP_OK) {
        HttpServer = nullptr;
        Serial.println(F("Web: server failed to start, console stays on the UART"));
        return E_NOT_OK;
    }

    static const httpd_uri_t RootUri{"/", HTTP_GET, handleRoot, nullptr, false, false, nullptr};
    static const httpd_uri_t CommandsUri{"/commands", HTTP_GET, handleCommands, nullptr, false, false, nullptr};
    static const httpd_uri_t DisplayUri{"/display", HTTP_GET, handleDisplay, nullptr, false, false, nullptr};
    static const httpd_uri_t SocketUri{"/ws", HTTP_GET, handleSocket, nullptr, true, false, nullptr};

    httpd_register_uri_handler(HttpServer, &RootUri);
    httpd_register_uri_handler(HttpServer, &CommandsUri);
    httpd_register_uri_handler(HttpServer, &DisplayUri);
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

    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = reinterpret_cast<uint8_t*>(const_cast<char*>(Line));
    Frame.len = strlen(Line);

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
 *                  guessing it. Unlike the wx window, a browser can show the real colour -
 *                  that window renders brightness only.
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
        const Pixel Colour = Strip.getPixelFast(Index);

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
