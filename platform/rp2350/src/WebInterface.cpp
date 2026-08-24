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
/* The two halves of an update on this core: the image goes into the filesystem, and PicoOTA
   writes the command page that the loader at the front of the image reads on the next boot. */
#include <LittleFS.h>
#include <PicoOTA.h>

#include "Arduino.h"

#include "System.h"
#include "WebFrontend.h"
#include "WebInterface.h"
#include "WebTransport.h"
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

/* Fixed, and named in the realm so nobody has to guess it. Not a secret: what is secret is
   the password beside it. */
constexpr char ConsoleUserName[]{"wordclock"};
constexpr char ConsoleRealm[]{"Wordclock, user wordclock"};

/* Where the image waits for the loader. At the root of the filesystem and not beside the
   settings, because it is not settings: this file is written by one update and gone from the
   next one's point of view, and the loader is given the path as text. */
constexpr char ImagePath[]{"/firmware.bin"};
/* Kept free on top of the image itself: the loader's command page is a file of its own, and
   LittleFS spends blocks on metadata before it spends them on content. Two blocks would
   probably do; this is a filesystem sized for an image plus room, so the slack is cheaper
   than finding out on a clock. */
constexpr uint32_t FilesystemReserve{8u * 1024u};

/* An upload in progress. Static because the body arrives in chunks across several calls and
   the request object is not the place to hang a file on - what the answer needs afterwards is
   whether it got that far, which is what Failure and IsCommitted say. */
struct UploadType {
    File Image;
    uint32_t Written{0u};
    bool IsOpen{false};
    bool IsCommitted{false};
    /* A literal, so the answer can name what went wrong without carrying a buffer for it.
       nullptr means nothing has gone wrong *yet*, which is not the same as success. */
    const char* Failure{nullptr};
};
UploadType Upload;

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
 *                  keeps out is a guest who opens the page and starts sending commands. It
 *                  keeps out nobody who can watch the traffic, and that would need https,
 *                  which a clock cannot offer credibly.
 *
 *                  Off unless a password was stored, so an update locks nobody out of a clock
 *                  that is already on a wall.
 *
 *                  The library compares for us here, which is why System hands over the
 *                  password rather than checking a blob the way the ESP32's does - that core
 *                  can encode base64 and not decode it, and this one has a server that needs
 *                  neither.
******************************************************************************************************************************************************/
bool isRequestAuthorised(AsyncWebServerRequest* Request)
{
    System& system = System::getInstance();

    if(!system.isConsoleProtected()) { return true; }

    char Password[SYSTEM_PASSWORD_STRING_LENGTH]{};
    if(system.getConsolePassword(Password, sizeof(Password)) == E_NOT_OK) { return true; }

    /* Basic and not digest: what a browser sends for digest is a hash of a nonce this server
       would have to keep, and over plain http the two are worth the same anyway. */
    if(Request->authenticate(ConsoleUserName, Password)) { return true; }

    Request->requestAuthentication(AsyncAuthType::AUTH_BASIC, ConsoleRealm);
    return false;
}


/******************************************************************************************************************************************************
  abortImage()
******************************************************************************************************************************************************/
/*! \brief          Gives up on the image being written, and takes it off the filesystem
 *
 *  \details        Removed rather than left behind: what is on the filesystem after a failed
 *                  upload is a file the loader will not read, and the next attempt needs the
 *                  space more than anyone needs the wreckage.
******************************************************************************************************************************************************/
void abortImage(const char* Reason)
{
    if(Upload.IsOpen) { Upload.Image.close(); }
    Upload.IsOpen = false;
    LittleFS.remove(ImagePath);
    Upload.Failure = Reason;
}


/******************************************************************************************************************************************************
  beginImage()
******************************************************************************************************************************************************/
/*! \brief          Opens the file the image is written into, or says why it cannot be
 *
 *  \details        Announced rather than measured, and it has to be: the space is checked
 *                  before the first byte is written, and the only size available then is the
 *                  one the request declares. A body that then turns out shorter is caught at
 *                  the end, where the length is compared again.
 *
 *                  The previous image is removed *before* the free space is measured, or a
 *                  second update would be refused for the space the first one still holds.
******************************************************************************************************************************************************/
void beginImage(AsyncWebServerRequest* Request, size_t Announced)
{
    Upload = UploadType{};

    if(!isRequestAuthorised(Request)) { Upload.Failure = "not authorised"; return; }
    if(Announced == 0u) { Upload.Failure = "no image in the request body"; return; }
    if(!LittleFS.begin()) { Upload.Failure = "the filesystem is not there to write into"; return; }

    LittleFS.remove(ImagePath);

    FSInfo Info{};
    if(LittleFS.info(Info)) {
        const uint64_t Free = Info.totalBytes - Info.usedBytes;

        if(Free < static_cast<uint64_t>(Announced) + FilesystemReserve) {
            /* Named as the filesystem's size and not as "too large": the image is the size it
               is, and what is too small is the region this board keeps for it. */
            Upload.Failure = "the image does not fit in the filesystem";
            return;
        }
    }

    Upload.Image = LittleFS.open(ImagePath, "w");
    if(!Upload.Image) { Upload.Failure = "the image file could not be opened"; return; }
    Upload.IsOpen = true;
}


/******************************************************************************************************************************************************
  commitImage()
******************************************************************************************************************************************************/
/*! \brief          Closes the image and writes the loader's command page beside it
 *
 *  \details        The one irreversible step of an update, and the last: until this page is
 *                  written the clock is running what it was running and would go on doing so
 *                  after any number of interrupted uploads. Its own writing is guarded by a
 *                  checksum the loader verifies, so a page that is itself half-written is a
 *                  page the loader ignores.
 *
 *                  The length is compared once more against what was announced. A connection
 *                  that stops mid-image ends without this call at all; one that stops between
 *                  the last chunk and here leaves a file that is short, and a short image
 *                  copied over the application is the one outcome this must not produce.
******************************************************************************************************************************************************/
void commitImage(size_t Announced)
{
    Upload.Image.close();
    Upload.IsOpen = false;

    if(Upload.Written != Announced) {
        LittleFS.remove(ImagePath);
        Upload.Failure = "the upload stopped before the image was complete";
        return;
    }

    picoOTA.begin();
    if(!picoOTA.addFile(ImagePath)) {
        LittleFS.remove(ImagePath);
        Upload.Failure = "the image could not be handed to the loader";
        return;
    }
    if(!picoOTA.commit()) {
        LittleFS.remove(ImagePath);
        Upload.Failure = "the loader's command could not be written";
        return;
    }
    Upload.IsCommitted = true;
}


/******************************************************************************************************************************************************
  handleAsset()
******************************************************************************************************************************************************/
/*! \brief          Serves one of the files the clock hands out unchanged
 *
 *  \details        One handler for all five - the two pages, the manifest and the two icons -
 *                  because what separates them is data and the data is WebFrontend's table.
 *                  Where the IDF's server carries a user context per route, this library takes
 *                  a std::function, so the entry is captured instead.
 *
 *                  A gzipped asset is sent as it is stored and the browser unpacks it; that
 *                  is the whole of what the header does here, and the table says which.
******************************************************************************************************************************************************/
void handleAsset(AsyncWebServerRequest* Request, const WebFrontend::AssetType& Asset)
{
    if(!isRequestAuthorised(Request)) { return; }

    AsyncWebServerResponse* Response = Request->beginResponse(200, Asset.ContentType, Asset.Bytes, Asset.Size);

    if(Asset.IsGzipped) { Response->addHeader("Content-Encoding", "gzip"); }
    Request->send(Response);
}


/******************************************************************************************************************************************************
  sendUpdateResult()
******************************************************************************************************************************************************/
/*! \brief          Answers the update route, in the one shape the page reads
 *
 *  \details        The same two-field JSON the ESP32 backend answers with, because the panel
 *                  reading it is one page serving both boards.
 *
 *                  Through beginResponse like every other route here, rather than the
 *                  library's send(code, type, text): one way of answering per file is worth
 *                  more than the shorter call, and it is the form the backend's stub already
 *                  stands in for.
******************************************************************************************************************************************************/
void sendUpdateResult(AsyncWebServerRequest* Request, bool Ok, const char* Reason)
{
    char Answer[128];

    if(Ok) {
        strncpy(Answer, "{\"ok\":true}", sizeof(Answer) - 1u);
    } else {
        snprintf(Answer, sizeof(Answer), "{\"ok\":false,\"error\":\"%s\"}", Reason);
        Serial.print(F("Web: update refused - "));
        Serial.println(Reason);
    }
    Answer[sizeof(Answer) - 1u] = '\0';

    Request->send(Request->beginResponse(Ok ? 200 : 400, "application/json",
                                        reinterpret_cast<const uint8_t*>(Answer), strlen(Answer)));
}


/******************************************************************************************************************************************************
  handleUpdateBody()
******************************************************************************************************************************************************/
/*! \brief          Takes the image out of the request body and into the filesystem
 *
 *  \details        This core has no second app partition, so an update here is not a write
 *                  into a spare slot but a *file*: the image goes into LittleFS, a command
 *                  page beside it names that file, and the OTA loader every image on this
 *                  board already carries - ten kilobytes at the front of flash, ahead of the
 *                  application - copies it into place on the next boot and clears the command
 *                  afterwards, so it is done once and not on every boot.
 *
 *                  What that arrangement buys is the property the ESP32's second slot gives
 *                  for free: the firmware being written is never the one running. Which is why
 *                  the command page is written last, after the final chunk has arrived and the
 *                  file measures what was announced - no command page, no update, and a
 *                  half-written image is a file nobody reads.
 *
 *                  The body arrives *before* the request handler runs, so authorisation is
 *                  asked here as well and not only there: a check that happened after the
 *                  writing would have let an unauthorised upload fill the filesystem first.
******************************************************************************************************************************************************/
void handleUpdateBody(AsyncWebServerRequest* Request, uint8_t* Data, size_t Length, size_t Index, size_t Total)
{
    if(Index == 0u) { beginImage(Request, Total); }
    if(!Upload.IsOpen) { return; }

    if(Upload.Image.write(Data, Length) != Length) {
        abortImage("the image could not be written to the filesystem");
        return;
    }
    Upload.Written += Length;

    if(Upload.Written >= Total) { commitImage(Total); }
}


/******************************************************************************************************************************************************
  handleUpdate()
******************************************************************************************************************************************************/
/*! \brief          Answers once the body has been taken, and asks for the restart
 *
 *  \details        The restart is asked for rather than carried out, the same deferral RPC 31
 *                  uses: a controller restarting inside the handler sends the browser nothing,
 *                  which reads as a failed update. The application's tick does it, once this
 *                  answer has left.
 *
 *                  A POST with no body never reaches the body handler at all, so "nothing was
 *                  committed and nothing failed" is its own case and says so.
******************************************************************************************************************************************************/
void handleUpdate(AsyncWebServerRequest* Request)
{
    if(!isRequestAuthorised(Request)) { return; }

    /* Still open here means the body ended before the length it announced - a client that
       declared more than it sent. The image goes with it: an incomplete file is one no loader
       will read, and the next attempt needs the room more than anybody needs the wreckage.
       (A connection that simply dies never reaches this handler at all; what cleans up after
       that one is the next upload, which removes the old image before it writes.) */
    if(Upload.IsOpen) { abortImage("the upload stopped before the image was complete"); }

    if(Upload.Failure != nullptr) { sendUpdateResult(Request, false, Upload.Failure); return; }
    if(!Upload.IsCommitted) { sendUpdateResult(Request, false, "no image in the request body"); return; }

    sendUpdateResult(Request, true, "");
    System::getInstance().restart();
}


/******************************************************************************************************************************************************
  handleCommands() / handleDisplay()
******************************************************************************************************************************************************/
/*! \brief          Serves the command catalog and the panel's letters
 *
 *  \details        Both documents are written by WebFrontend, shared with the ESP32 backend:
 *                  what a page is sent is a property of the catalog and the letter table, not
 *                  of a controller. They were generated here once, and identically in the
 *                  other backend - two functions that had to be kept the same by hand and
 *                  whose divergence would have surfaced in a browser rather than in a test.
 *
 *                  What is left here is what is genuinely this server's: the password, the
 *                  content type, and a body to write into.
******************************************************************************************************************************************************/
void handleCommands(AsyncWebServerRequest* Request)
{
    if(!isRequestAuthorised(Request)) { return; }

    WebResponseBody Body(Request, "application/json");
    WebFrontend::writeCommands(Body);
}

void handleDisplay(AsyncWebServerRequest* Request)
{
    if(!isRequestAuthorised(Request)) { return; }

    WebResponseBody Body(Request, "application/json");
    WebFrontend::writeDisplay(Body);
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
    if(Frame->opcode != WS_TEXT) { return; }

    /* The length bound, the terminator and the empty-frame case are WebFrontend's, so that a
       command coming off a socket reaches the parser the same way on either backend. What
       stays here is the framing, which is this library's shape and not the other's. */
    WebFrontend::injectCommand(reinterpret_cast<const char*>(Payload), Length);
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

    /* The files served unchanged, one route per entry of WebFrontend's table, so a file added
       to web/ needs no edit here. The entry is captured by reference and outlives the lambda:
       the table has static storage duration. */
    for(byte Index = 0u; Index < WebFrontend::getNumberOfAssets(); Index++) {
        const WebFrontend::AssetType& Asset = WebFrontend::getAsset(Index);

        HttpServer.on(Asset.Path, HTTP_GET,
                      [&Asset](AsyncWebServerRequest* Request) { handleAsset(Request, Asset); });
    }

    HttpServer.on("/commands", HTTP_GET, handleCommands);
    HttpServer.on("/display", HTTP_GET, handleDisplay);
    /* Four arguments, and the fourth is where the image arrives: the library hands a POST
       body to that callback in chunks and calls the request handler afterwards. The third,
       the upload handler, is for multipart forms and stays empty - the panel sends the file
       as the body itself, so nothing here has to parse what a <form> would have wrapped it
       in. */
    HttpServer.on("/update", HTTP_POST, handleUpdate, nullptr, handleUpdateBody);

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
 *  \details        The widening to UTF-8 and the bound on it are WebFrontend's, shared with
 *                  the other backend; what is this file's is the one call in sendText()
 *                  below, where the IDF needs a walk over descriptors.
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
   source of its own because what it reaches - the server and its web socket - is this file's
   and stays this file's. */

WebTransport& WebTransport::getInstance()
{
    static WebTransport SingletonInstance;
    return SingletonInstance;
}

bool WebTransport::isListening() const
{
    return IsListening;
}

bool WebTransport::hasClients() const
{
    /* The socket's own count, not a table of ours: it owns the list, so there is nothing here
       that could fall out of step with it. */
    return Socket.count() > 0u;
}

void WebTransport::sendText(const char* Text, size_t Length)
{
    Socket.textAll(Text, Length);
}

void WebTransport::sendBinary(const byte* Bytes, size_t Length)
{
    Socket.binaryAll(reinterpret_cast<const char*>(Bytes), Length);
}


/******************************************************************************************************************************************************
 *  W E B   R E S P O N S E   B O D Y
******************************************************************************************************************************************************/
/* A response this server is pulled from rather than pushed to: the stream collects what is
   written and finish() hands it to the request, which is when the library sends it. */

WebResponseBody::WebResponseBody(AsyncWebServerRequest* sRequest, const char* ContentType)
    : Request(sRequest), Stream(sRequest->beginResponseStream(ContentType))
{
}

void WebResponseBody::write(const char* Data, size_t Length)
{
    Stream->write(reinterpret_cast<const uint8_t*>(Data), Length);
}

void WebResponseBody::finish()
{
    Request->send(Stream);
}


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
