// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Serves the console page and bridges its web socket to Serial.
 */

/* The framework's headers first, ahead of the Arduino.h that binds Serial to a macro. */
#include <LEAmDNS.h>
#include <ESPAsyncWebServer.h>
// The two halves of an update on this core: the image goes into the filesystem, and PicoOTA
// writes the command page that the loader at the front of the image reads on the next boot.
#include <LittleFS.h>
#include <PicoOTA.h>

#include "Arduino.h"

#include "System.h"
#include "WebFrontend.h"
#include "WebInterface.h"
#include "WebTransport.h"
#include "WordclockSerial.h"

#include <string.h>

namespace {
// Named http_server rather than Server: the Arduino core has a class Server in the global
// namespace, and an unqualified Server here is then ambiguous.
AsyncWebServer http_server(WEB_INTERFACE_PORT);
AsyncWebSocket socket("/ws");

// Whether Begin() got as far as listening. The server object exists from start-up either
// way, so unlike the ESP32's handle it cannot itself stand for that.
bool is_listening{false};

// Handed to WordclockSerial, which takes a plain function pointer - so the singleton is
// reached from here rather than carried along.
void SendLineToClients(const char* line)
{
    WebInterface::GetInstance().BroadcastLine(line);
}

// Fixed, and named in the realm so nobody has to guess it. Not a secret: what is secret is
// the password beside it.
constexpr char kConsoleUserName[]{"wordclock"};
constexpr char kConsoleRealm[]{"Wordclock, user wordclock"};

/*
 * Where the image waits for the loader. At the root of the filesystem and not beside the
 * settings, because it is not settings: this file is written by one update and gone from the
 * next one's point of view, and the loader is given the path as text.
 */
constexpr char kImagePath[]{"/firmware.bin"};
/*
 * Kept free on top of the image itself: the loader's command page is a file of its own, and
 * LittleFS spends blocks on metadata before it spends them on content. Two blocks would
 * probably do; this is a filesystem sized for an image plus room, so the slack is cheaper
 * than finding out on a clock.
 */
constexpr uint32_t kFilesystemReserve{8u * 1024u};

/*
 * An upload in progress. Static because the body arrives in chunks across several calls and
 * the request object is not the place to hang a file on - what the answer needs afterwards is
 * whether it got that far, which is what Failure and is_committed say.
 */
struct UploadType {
    File image;
    uint32_t written{0u};
    bool is_open{false};
    bool is_committed{false};
    // A literal, so the answer can name what went wrong without carrying a buffer for it.
    // nullptr means nothing has gone wrong *yet*, which is not the same as success.
    const char* failure{nullptr};
};
UploadType upload;

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
 */
bool IsRequestAuthorised(AsyncWebServerRequest* request)
{
    System& system = System::GetInstance();

    if(!system.IsConsoleProtected()) { return true; }

    char password[SYSTEM_PASSWORD_STRING_LENGTH]{};
    if(system.GetConsolePassword(password, sizeof(password)) == kENotOk) { return true; }

    // Basic and not digest: what a browser sends for digest is a hash of a nonce this server
    // would have to keep, and over plain http the two are worth the same anyway.
    if(request->authenticate(kConsoleUserName, password)) { return true; }

    request->requestAuthentication(AsyncAuthType::AUTH_BASIC, kConsoleRealm);
    return false;
}

/*! \brief          Gives up on the image being written, and takes it off the filesystem
 *
 *  \details        Removed rather than left behind: what is on the filesystem after a failed
 *                  upload is a file the loader will not read, and the next attempt needs the
 *                  space more than anyone needs the wreckage.
 */
void AbortImage(const char* reason)
{
    if(upload.is_open) { upload.image.close(); }
    upload.is_open = false;
    LittleFS.remove(kImagePath);
    upload.failure = reason;
}

/*! \brief          Opens the file the image is written into, or says why it cannot be
 *
 *  \details        Announced rather than measured, and it has to be: the space is checked
 *                  before the first byte is written, and the only size available then is the
 *                  one the request declares. A body that then turns out shorter is caught at
 *                  the end, where the length is compared again.
 *
 *                  The previous image is removed *before* the free space is measured, or a
 *                  second update would be refused for the space the first one still holds.
 */
void BeginImage(AsyncWebServerRequest* request, size_t announced)
{
    upload = UploadType{};

    if(!IsRequestAuthorised(request)) { upload.failure = "not authorised"; return; }
    if(announced == 0u) { upload.failure = "no image in the request body"; return; }
    if(!LittleFS.begin()) { upload.failure = "the filesystem is not there to write into"; return; }

    LittleFS.remove(kImagePath);

    FSInfo info{};
    if(LittleFS.info(info)) {
        const uint64_t free = info.totalBytes - info.usedBytes;

        if(free < static_cast<uint64_t>(announced) + kFilesystemReserve) {
            // Named as the filesystem's size and not as "too large": the image is the size it
            // is, and what is too small is the region this board keeps for it.
            upload.failure = "the image does not fit in the filesystem";
            return;
        }
    }

    upload.image = LittleFS.open(kImagePath, "w");
    if(!upload.image) { upload.failure = "the image file could not be opened"; return; }
    upload.is_open = true;
}

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
 */
void CommitImage(size_t announced)
{
    upload.image.close();
    upload.is_open = false;

    if(upload.written != announced) {
        LittleFS.remove(kImagePath);
        upload.failure = "the upload stopped before the image was complete";
        return;
    }

    picoOTA.begin();
    if(!picoOTA.addFile(kImagePath)) {
        LittleFS.remove(kImagePath);
        upload.failure = "the image could not be handed to the loader";
        return;
    }
    if(!picoOTA.commit()) {
        LittleFS.remove(kImagePath);
        upload.failure = "the loader's command could not be written";
        return;
    }
    upload.is_committed = true;
}

/*! \brief          Serves one of the files the clock hands out unchanged
 *
 *  \details        One handler for all five - the two pages, the manifest and the two icons -
 *                  because what separates them is data and the data is WebFrontend's table.
 *                  Where the IDF's server carries a user context per route, this library takes
 *                  a std::function, so the entry is captured instead.
 *
 *                  A gzipped asset is sent as it is stored and the browser unpacks it; that
 *                  is the whole of what the header does here, and the table says which.
 */
void HandleAsset(AsyncWebServerRequest* request, const WebFrontend::AssetType& asset)
{
    if(!IsRequestAuthorised(request)) { return; }

    AsyncWebServerResponse* response = request->beginResponse(200, asset.content_type, asset.bytes, asset.size);

    if(asset.is_gzipped) { response->addHeader("Content-Encoding", "gzip"); }
    request->send(response);
}

/*! \brief          Answers the update route, in the one shape the page reads
 *
 *  \details        The same two-field JSON the ESP32 backend answers with, because the panel
 *                  reading it is one page serving both boards.
 *
 *                  Through beginResponse like every other route here, rather than the
 *                  library's send(code, type, text): one way of answering per file is worth
 *                  more than the shorter call, and it is the form the backend's stub already
 *                  stands in for.
 */
void SendUpdateResult(AsyncWebServerRequest* request, bool ok, const char* reason)
{
    char answer[128];

    if(ok) {
        strncpy(answer, "{\"ok\":true}", sizeof(answer) - 1u);
    } else {
        /*
         * %.100s, as on the ESP32: an unbounded reason would be cut off together with the
         * closing brace, and the page would get an answer that is not JSON at all. 100 is
         * what the buffer holds beside the frame.
         */
        (void)snprintf(answer, sizeof(answer), "{\"ok\":false,\"error\":\"%.100s\"}", reason);
        Serial.print(F("Web: update refused - "));
        Serial.println(reason);
    }
    answer[sizeof(answer) - 1u] = '\0';

    request->send(request->beginResponse(ok ? 200 : 400, "application/json",
                                        reinterpret_cast<const uint8_t*>(answer), strlen(answer)));
}

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
 */
void HandleUpdateBody(AsyncWebServerRequest* request, uint8_t* data, size_t length, size_t index, size_t total)
{
    if(index == 0u) { BeginImage(request, total); }
    if(!upload.is_open) { return; }

    if(upload.image.write(data, length) != length) {
        AbortImage("the image could not be written to the filesystem");
        return;
    }
    upload.written += length;

    if(upload.written >= total) { CommitImage(total); }
}

/*! \brief          Answers once the body has been taken, and asks for the restart
 *
 *  \details        The restart is asked for rather than carried out, the same deferral RPC 31
 *                  uses: a controller restarting inside the handler sends the browser nothing,
 *                  which reads as a failed update. The application's tick does it, once this
 *                  answer has left.
 *
 *                  A POST with no body never reaches the body handler at all, so "nothing was
 *                  committed and nothing failed" is its own case and says so.
 */
void HandleUpdate(AsyncWebServerRequest* request)
{
    if(!IsRequestAuthorised(request)) { return; }

    /*
     * Still open here means the body ended before the length it announced - a client that
     * declared more than it sent. The image goes with it: an incomplete file is one no loader
     * will read, and the next attempt needs the room more than anybody needs the wreckage.
     * (A connection that simply dies never reaches this handler at all; what cleans up after
     * that one is the next upload, which removes the old image before it writes.)
     */
    if(upload.is_open) { AbortImage("the upload stopped before the image was complete"); }

    if(upload.failure != nullptr) { SendUpdateResult(request, false, upload.failure); return; }
    if(!upload.is_committed) { SendUpdateResult(request, false, "no image in the request body"); return; }

    SendUpdateResult(request, true, "");
    System::GetInstance().Restart();
}

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
 */
void HandleCommands(AsyncWebServerRequest* request)
{
    if(!IsRequestAuthorised(request)) { return; }

    WebResponseBody body(request, "application/json");
    WebFrontend::WriteCommands(body);
}

void HandleDisplay(AsyncWebServerRequest* request)
{
    if(!IsRequestAuthorised(request)) { return; }

    WebResponseBody body(request, "application/json");
    WebFrontend::WriteDisplay(body);
}

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
 */
void OnSocketEvent(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType type,
                   void* argument, uint8_t* payload, size_t length)
{
    if(type == WS_EVT_CONNECT) {
        WebInterface::GetInstance().OnClientOpened();
        return;
    }
    if(type != WS_EVT_DATA) { return; }

    const AwsFrameInfo* frame = static_cast<const AwsFrameInfo*>(argument);

    if((frame == nullptr) || (!frame->final) || (frame->index != 0u) || (frame->len != length)) { return; }
    if(frame->opcode != WS_TEXT) { return; }

    /*
     * The length bound, the terminator and the empty-frame case are WebFrontend's, so that a
     * command coming off a socket reaches the parser the same way on either backend. What
     * stays here is the framing, which is this library's shape and not the other's.
     */
    WebFrontend::InjectCommand(reinterpret_cast<const char*>(payload), length);
}

} // namespace

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/*! \brief          Starts the server and takes over the output lines
 *
 *  \return         kEOk if the server is listening
 */
StdReturnType WebInterface::Begin()
{
    socket.onEvent(OnSocketEvent);
    http_server.addHandler(&socket);

    /*
     * The files served unchanged, one route per entry of WebFrontend's table, so a file added
     * to web/ needs no edit here. The entry is captured by reference and outlives the lambda:
     * the table has static storage duration.
     */
    for(byte index = 0u; index < WebFrontend::GetNumberOfAssets(); index++) {
        const WebFrontend::AssetType& asset = WebFrontend::GetAsset(index);

        http_server.on(asset.path, HTTP_GET,
                      [&asset](AsyncWebServerRequest* request) { HandleAsset(request, asset); });
    }

    http_server.on("/commands", HTTP_GET, HandleCommands);
    http_server.on("/display", HTTP_GET, HandleDisplay);
    /*
     * Four arguments, and the fourth is where the image arrives: the library hands a POST
     * body to that callback in chunks and calls the request handler afterwards. The third,
     * the upload handler, is for multipart forms and stays empty - the panel sends the file
     * as the body itself, so nothing here has to parse what a <form> would have wrapped it
     * in.
     */
    http_server.on("/update", HTTP_POST, HandleUpdate, nullptr, HandleUpdateBody);

    /*
     * No failure to report: this library's begin() returns nothing and starts listening on
     * whatever address arrives later. Where the ESP32 backend can find its server refusing
     * to start and fall back to the UART, here the equivalent shows up as a page that never
     * answers - which is why the address is printed below either way.
     */
    http_server.begin();
    is_listening = true;

    /* Only now, so a line printed during startup cannot reach a half-built server. */
    WordclockSerial::GetInstance().SetLineSink(SendLineToClients);

    if(MDNS.begin(WEB_INTERFACE_HOSTNAME)) {
        MDNS.addService("http", "tcp", WEB_INTERFACE_PORT);
        Serial.print(F("Web: http://"));
        Serial.print(WEB_INTERFACE_HOSTNAME);
        Serial.println(F(".local"));
    } else {
        /* Not fatal: the address still works, it just has to be looked up. */
        Serial.println(F("Web: mDNS unavailable, reach the clock by its address"));
    }

    return kEOk;
}

/*! \brief          Sends one finished line to every open socket
 *  \details        The widening to UTF-8 and the bound on it are WebFrontend's, shared with
 *                  the other backend; what is this file's is the one call in SendText()
 *                  below, where the IDF needs a walk over descriptors.
 */
void WebInterface::BroadcastLine(const char* line)
{
    WebFrontend::GetInstance().BroadcastLine(line);
}

/*! \brief          Sends the pixel buffer to every open socket, when it changed
 *  \details        The rate limit, the gathering and the comparison against the last frame
 *                  are WebFrontend's, shared with the other backend - none of the three has
 *                  anything to do with which server is underneath. What is this file's is
 *                  SendBinary() below.
 */
void WebInterface::BroadcastFrame()
{
    WebFrontend::GetInstance().BroadcastFrame();
}

/*
 * This backend's half of the contract WebFrontend.h states. Defined here rather than in a
 * source of its own because what it reaches - the server and its web socket - is this file's
 * and stays this file's.
 */

WebTransport& WebTransport::GetInstance()
{
    static WebTransport singleton_instance;
    return singleton_instance;
}

bool WebTransport::IsListening() const
{
    return is_listening;
}

bool WebTransport::HasClients() const
{
    // The socket's own count, not a table of ours: it owns the list, so there is nothing here
    // that could fall out of step with it.
    return socket.count() > 0u;
}

void WebTransport::SendText(const char* text, size_t length)
{
    socket.textAll(text, length);
}

void WebTransport::SendBinary(const byte* bytes, size_t length)
{
    socket.binaryAll(reinterpret_cast<const char*>(bytes), length);
}
// NOLINTEND(readability-convert-member-functions-to-static)

// A response this server is pulled from rather than pushed to: the stream collects what is
// written and Finish() hands it to the request, which is when the library sends it.

WebResponseBody::WebResponseBody(AsyncWebServerRequest* request, const char* content_type)
    : request_(request), stream_(request->beginResponseStream(content_type))
{
}

void WebResponseBody::Write(const char* data, size_t length)
{
    stream_->write(reinterpret_cast<const uint8_t*>(data), length);
}

void WebResponseBody::Finish()
{
    request_->send(stream_);
}
