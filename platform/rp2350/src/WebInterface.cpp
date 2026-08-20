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
#include <LEAmDNS.h>
#include <ESPAsyncWebServer.h>

#include "Arduino.h"

#include "Communication.h"
#include "DisplayCharacters.h"
#include "MessageCatalog.h"
#include "WebInterface.h"
#include "WebPage.h"
#include "WordclockSerial.h"

#include <string.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

/* Named HttpServer rather than Server: the Arduino core has a class Server in the global
   namespace, and an unqualified Server here is then ambiguous. */
AsyncWebServer HttpServer(WEB_INTERFACE_PORT);
AsyncWebSocket Socket("/ws");

/* Whether begin() got as far as listening. The server object exists from start-up either
   way, so unlike the ESP32's handle it cannot itself stand for that. */
bool IsListening{false};

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
  handleRoot()
******************************************************************************************************************************************************/
/*! \brief          Serves the console page straight out of flash
 *  \details        Sent compressed, which is how it is stored: the page was gzipped at
 *                  build time, so this only pushes the bytes and the browser unpacks them.
******************************************************************************************************************************************************/
void handleRoot(AsyncWebServerRequest* Request)
{
    AsyncWebServerResponse* Response = Request->beginResponse(200, "text/html", WebPageGzip, WebPageGzipSize);

    Response->addHeader("Content-Encoding", "gzip");
    Request->send(Response);
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
void handleManifest(AsyncWebServerRequest* Request)
{
    Request->send(Request->beginResponse(200, "application/manifest+json", WebManifest, WebManifestSize));
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
void handleIcon192(AsyncWebServerRequest* Request)
{
    Request->send(Request->beginResponse(200, "image/png", WebIcon192, WebIcon192Size));
}

void handleIcon512(AsyncWebServerRequest* Request)
{
    Request->send(Request->beginResponse(200, "image/png", WebIcon512, WebIcon512Size));
}


/******************************************************************************************************************************************************
  C H U N K   W R I T E R
******************************************************************************************************************************************************/
/* Writes the catalog's JSON into the response stream the server hands out. The ESP32
   backend collects it in a buffer of its own and pushes chunks, because the IDF's server
   wants to be pushed to; this one is pulled from, and the stream is the pull side.

   It grows on the heap as it is written, which the ESP32 version deliberately avoids. Not
   worth avoiding here: this library builds every response, header and socket frame out of
   String and std::list, so a buffer of our own would save nothing measurable while making
   the two handlers below read differently from their counterparts. */
class ChunkWriter
{
  private:
    AsyncResponseStream* Stream;

  public:
    explicit ChunkWriter(AsyncResponseStream* sStream) : Stream(sStream) { }

    void put(char Character) { Stream->write(Character); }

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

    /* Nothing to terminate: the server sends the stream once the handler returns it. */
    void finish() { }
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
void handleCommands(AsyncWebServerRequest* Request)
{
    AsyncResponseStream* Stream = Request->beginResponseStream("application/json");
    ChunkWriter Writer(Stream);
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

    Request->send(Stream);
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
void handleDisplay(AsyncWebServerRequest* Request)
{
    AsyncResponseStream* Stream = Request->beginResponseStream("application/json");

    const DisplayCharacters Letters;
    ChunkWriter Writer(Stream);

    Writer.put("{\"columns\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS);
    Writer.put(",\"rows\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    Writer.put(",\"letters\":\"");

    for(byte Index = 0u; Index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; Index++) {
        char Utf8[2u];
        const byte Length = toUtf8(static_cast<byte>(Letters.getCharacterFast(Index)), Utf8);

        for(byte Byte = 0u; Byte < Length; Byte++) { Writer.put(Utf8[Byte]); }
    }

    Writer.put("\"}");
    Writer.finish();

    Request->send(Stream);
}


/******************************************************************************************************************************************************
  handleSocket()
******************************************************************************************************************************************************/
/*! \brief          Takes one web socket frame and injects it as typed characters
 *  \details        The library completes the handshake itself and reports the result as an
 *                  event, so unlike the ESP32 backend there is no GET to answer here.
 *
 *                  Only a whole single-frame text message is taken. A split one is dropped
 *                  rather than reassembled, which keeps half a command from reaching the
 *                  parser - the same guarantee the IDF version gets by asking for the frame
 *                  with the buffer's size.
 *
 *                  The command is injected followed by the end-of-message character, unless
 *                  the browser already sent one. That keeps the page free of protocol
 *                  bookkeeping: it sends the command text, the terminator is this side's
 *                  business.
 *
 *                  A close needs no handling. Where the IDF hands out socket descriptors
 *                  this side has to keep, the socket owns its client list, so a client that
 *                  simply vanished stops being broadcast to without anything here noticing.
******************************************************************************************************************************************************/
void onSocketEvent(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType Type,
                   void* Argument, uint8_t* Payload, size_t Length)
{
    if(Type == WS_EVT_CONNECT) {
        WebInterface::getInstance().onClientOpened();
        return;
    }
    if(Type != WS_EVT_DATA) { return; }

    const AwsFrameInfo* Frame = static_cast<const AwsFrameInfo*>(Argument);

    if((Frame == nullptr) || (!Frame->final) || (Frame->index != 0u) || (Frame->len != Length)) { return; }
    if((Frame->opcode != WS_TEXT) || (Length == 0u) || (Length > WEB_INTERFACE_MAX_FRAME_LENGTH)) { return; }

    WordclockSerial& Port = WordclockSerial::getInstance();
    Port.inject(reinterpret_cast<const char*>(Payload), Length);

    if(Payload[Length - 1u] != Communication::getEndOfMessageChar()) {
        const char Terminator = Communication::getEndOfMessageChar();
        Port.inject(&Terminator, 1u);
    }
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
    Socket.onEvent(onSocketEvent);
    HttpServer.addHandler(&Socket);

    HttpServer.on("/", HTTP_GET, handleRoot);
    HttpServer.on("/commands", HTTP_GET, handleCommands);
    HttpServer.on("/display", HTTP_GET, handleDisplay);
    HttpServer.on("/manifest.webmanifest", HTTP_GET, handleManifest);
    HttpServer.on("/icon-192.png", HTTP_GET, handleIcon192);
    HttpServer.on("/icon-512.png", HTTP_GET, handleIcon512);

    /* No failure to report: this library's begin() returns nothing and starts listening on
       whatever address arrives later. Where the ESP32 backend can find its server refusing
       to start and fall back to the UART, here the equivalent shows up as a page that never
       answers - which is why the address is printed below either way. */
    HttpServer.begin();
    IsListening = true;

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
    if(!IsListening || (Line == nullptr)) { return; }

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

    if(Used == 0u) { return; }

    /* One call where the IDF needs a walk over the descriptors: the socket knows who is
       connected, so there is no list here to fall out of step with it. */
    Socket.textAll(Payload, Used);
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
    if(!IsListening) { return; }

    const bool Forced = ForceFrame.exchange(false, std::memory_order_acq_rel);

    if(!Forced) {
        if(FrameCountdown > 0u) { FrameCountdown--; return; }
    }
    FrameCountdown = WEB_INTERFACE_FRAME_INTERVAL_TICKS - 1u;

    /* Nothing to send to, so nothing is even gathered. */
    if(Socket.count() == 0u) { return; }

    byte Frame[FrameSize];
    byte* Target = Frame;
    const Pixels& Strip = Pixels::getInstance();

    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        const Pixel Colour = Strip.getOutputPixelFast(Index);

        *Target++ = Colour.getGreen();
        *Target++ = Colour.getRed();
        *Target++ = Colour.getBlue();
    }

    if(!Forced && (memcmp(Frame, LastFrame, FrameSize) == 0)) { return; }
    memcpy(LastFrame, Frame, FrameSize);

    Socket.binaryAll(reinterpret_cast<const char*>(Frame), FrameSize);
} /* broadcastFrame */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
