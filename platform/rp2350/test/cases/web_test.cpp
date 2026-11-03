/* Drives the registered handlers the way the server reaches them - through the registration
   call - so WebInterface's own wiring is covered rather than bypassed.

   One thing this can no longer ask, and the ESP32's version can: which client a frame went
   to. On the target that accounting has moved out of our code and into the socket, so what
   is left to check here is what the backend still decides - whether a frame is sent at all
   and when - rather than who received it. */
#include <ESPAsyncWebServer.h>
#include <LEAmDNS.h>
/* The two the update path leaves its evidence in: the image is a file, and the command page
   is what the loader would read on the next boot. */
#include <LittleFS.h>
#include <PicoOTA.h>

#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "System.h"
#include "WebInterface.h"
#include "check.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

/* Enough of a parse to say the document is well formed: brackets balanced and no string
   left open. A browser would refuse the whole thing otherwise, and that failure looks
   like a broken server rather than a broken label. */
static bool IsBalancedJson(const std::string& text)
{
    int depth = 0;
    bool in_string = false;

    for(size_t index = 0u; index < text.size(); index++) {
        const char character = text[index];

        if(in_string) {
            if(character == '\\')      { index++; }
            else if(character == '"')  { in_string = false; }
            continue;
        }
        if(character == '"')                                 { in_string = true; }
        else if((character == '[') || (character == '{'))     { depth++; }
        else if((character == ']') || (character == '}'))     { depth--; if(depth < 0) { return false; } }
    }
    return (depth == 0) && !in_string;
}

/* Runs one route and hands back what it produced. The request owns its response, so it
   outlives neither - hence the copy out. */
/* Whether the next Fetch() arrives with a credential the library would accept. The library
   does the comparing on the target, so what a case here decides is its answer. */
static bool request_authorised = true;
static bool authentication_was_requested = false;

static std::string Fetch(const char* route, std::string* content_encoding = nullptr)
{
    AsyncWebServerRequest request;
    request.Authorised = request_authorised;
    const auto found = webStubState().Routes.find(route);

    if(found == webStubState().Routes.end()) { return ""; }

    found->second(&request);

    authentication_was_requested = request.AuthenticationRequested;

    if(request.Response == nullptr) { return ""; }
    if(content_encoding != nullptr) {
        const auto header = request.Response->Headers.find("Content-Encoding");
        *content_encoding = (header == request.Response->Headers.end()) ? "" : header->second;
    }
    return request.Response->Body;
}

/* An upload, in the order the library performs one: the body arrives in chunks through the
   body handler, and only when it is through does the request handler answer. A test that
   called the request handler alone would be testing the answer and not the update.

   Chunked on purpose, and at a size that leaves a remainder, because the handler's arithmetic
   is what decides when an image is complete. */
struct UploadOutcome {
    int code{0};
    std::string body;
};

static UploadOutcome Post(const char* route, const std::string& content, size_t announced,
                          size_t chunk_size = 300u)
{
    AsyncWebServerRequest request;
    request.Authorised = request_authorised;

    const auto body = webStubState().BodyRoutes.find(route);
    const auto handler = webStubState().Routes.find(route);

    if((body == webStubState().BodyRoutes.end()) || (handler == webStubState().Routes.end())) { return {}; }

    for(size_t index = 0u; index < content.size(); index += chunk_size) {
        const size_t length = std::min(chunk_size, content.size() - index);
        std::vector<uint8_t> chunk(content.begin() + index, content.begin() + index + length);
        body->second(&request, chunk.data(), length, index, announced);
    }
    handler->second(&request);

    authentication_was_requested = request.AuthenticationRequested;
    if(request.Response == nullptr) { return {}; }
    return {request.Response->Code, request.Response->Body};
}

/* One text frame into the socket, the way the library reports one. */
static void SendFrame(const std::string& payload, bool final = true)
{
    AwsFrameInfo info{};
    info.final = final;
    info.index = 0u;
    info.len = payload.size();
    info.opcode = WS_TEXT;

    std::vector<uint8_t> bytes(payload.begin(), payload.end());
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_DATA, &info,
                                 bytes.empty() ? nullptr : bytes.data(), bytes.size());
}


int main()
{
    Check(WebInterface::GetInstance().Begin() == kEOk, "the server starts and registers its handlers");
    Check(webStubState().Listening, "and it is listening");
    /* "/" is the panel and the console has its own route, so both have to be there: a missing
       registration would answer 404 on the page everything the panel does not cover goes
       through. */
    Check(webStubState().Routes.count("/console") == 1u, "the console has a route of its own");
    Check((webStubState().SocketHandler != nullptr) && (webStubState().Routes.count("/") == 1u)
          && (webStubState().Routes.count("/commands") == 1u)
          && (webStubState().Routes.count("/display") == 1u),
          "all four handlers were registered");
    Check((webStubState().Routes.count("/manifest.webmanifest") == 1u)
          && (webStubState().Routes.count("/icon-192.png") == 1u)
          && (webStubState().Routes.count("/icon-512.png") == 1u),
          "and the three the home screen needs with them");
    Check(webStubState().Routes.count("/update") == 1u, "and the update route");
    /* The body handler is the half that matters: without it the image would arrive nowhere
       and the request handler would answer about an upload that never happened. */
    Check(webStubState().BodyRoutes.count("/update") == 1u, "with the body handler the image arrives through");

    /* both pages: gzipped, announced as such, and not the same bytes - a swapped pair of
       pointers would pass every other check here. */
    std::string encoding;
    const std::string page = Fetch("/", &encoding);
    Check(encoding == "gzip", "the page is announced as gzip");
    Check((page.size() > 100u) && (static_cast<unsigned char>(page[0]) == 0x1fu)
                               && (static_cast<unsigned char>(page[1]) == 0x8bu),
          "the page really is a gzip stream");

    const std::string console = Fetch("/console", &encoding);
    Check(encoding == "gzip", "the console is announced as gzip");
    Check(console.size() > 100u && console != page, "and is a different page from the panel");

    /* the manifest and the icons, which nothing on the clock reads and a browser does */
    const std::string manifest = Fetch("/manifest.webmanifest", &encoding);
    Check(encoding.empty(), "the manifest is sent as it is, not gzipped");
    Check(IsBalancedJson(manifest), "and is well formed JSON");
    Check(manifest.find("standalone") != std::string::npos, "it asks to start without an address bar");
    Check(manifest.find("icon-192.png") != std::string::npos
          && manifest.find("icon-512.png") != std::string::npos, "it names both icons");
    /* Relative, because the clock answers on whatever address it was given: an absolute
       start_url baked in here would send an installed console to a different clock. */
    Check(manifest.find("\"start_url\": \".\"") != std::string::npos, "and starts at a relative URL");

    const std::string small_icon = Fetch("/icon-192.png", &encoding);
    Check(encoding.empty(), "an icon is not gzipped either, a PNG being deflated already");
    Check(small_icon.size() > 500u && small_icon.compare(1u, 3u, "PNG") == 0, "and really is a PNG");
    const std::string large_icon = Fetch("/icon-512.png");
    Check(large_icon.size() > small_icon.size() && large_icon.compare(1u, 3u, "PNG") == 0,
          "the large icon is a PNG too, and the larger of the two");

    /* ---- the update ------------------------------------------------------------------
       This board has no second app partition, so an update is a file plus a command page that
       the loader at the front of the image reads on the next boot. Which makes the interesting
       question not "did it answer 200" but *what it left on the filesystem* - because that,
       and only that, is what the next boot acts on.
    */
    const std::string image(2000u, '\xa5');

    /* Only the image is swept away between cases, never the whole store: the console password
       is a file in this same filesystem, and a case that cleared it would unprotect the
       console and then pass for the wrong reason - which is what the first version of the
       authorisation case below did.

       The failing cases come first, and that order is not cosmetic either. A restart that has
       been asked for cannot be unasked - the flag is one-way, because on the target the call
       that reads it does not return - so "no restart was asked for" can only be checked
       before anything has asked for one. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    UploadOutcome update = Post("/update", image.substr(0u, 900u), image.size());
    Check(update.code == 400 && update.body.find("stopped before") != std::string::npos,
          "an upload that stops short is refused, and says that is what happened");
    Check(picoOTA.Committed.empty(), "and nothing was handed to the loader");
    Check(littleFsStore().count("/firmware.bin") == 0u, "and leaves no image behind either");
    System::GetInstance().PerformPendingRestart();
    Check(!rp2040.Restarted, "and the clock is left running what it has");

    /* An image that does not fit is refused before the first byte is written, so it cannot
       fill the filesystem the settings live in. 64 KB is what this board's filesystem was
       before it had to hold an image. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    LittleFS.Total = 64u * 1024u;
    update = Post("/update", image, 200u * 1024u);
    Check(update.code == 400 && update.body.find("does not fit") != std::string::npos,
          "an image larger than the filesystem is refused, and says so");
    Check(littleFsStore().count("/firmware.bin") == 0u, "and never reaches the filesystem");
    System::GetInstance().PerformPendingRestart();
    Check(!rp2040.Restarted, "and asks for no restart");
    LittleFS.Total = 1024u * 1024u;

    /* A command page that will not write is a failed update, and it takes the image with it:
       what is on the filesystem after a failure is a file nobody will read. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    picoOTA.CommitSucceeds = false;
    update = Post("/update", image, image.size());
    Check(update.code == 400, "a command page that will not write is a failed update");
    Check(littleFsStore().count("/firmware.bin") == 0u, "and the image is taken off the filesystem");
    picoOTA = PicoOtaStub{};

    /* A POST that carries nothing never reaches the body handler at all, which is its own
       case and not an empty success. */
    littleFsStore().erase("/firmware.bin");
    const std::string nothing = Fetch("/update");
    Check(nothing.find("\"ok\":false") != std::string::npos, "a POST with no body is refused");
    Check(IsBalancedJson(nothing), "and the answer is well formed JSON");

    /* And the one that works, last for the reason above. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    update = Post("/update", image, image.size());
    Check(update.code == 200 && update.body.find("\"ok\":true") != std::string::npos,
          "a complete image is accepted");
    Check(littleFsStore().count("/firmware.bin") == 1u
          && littleFsStore()["/firmware.bin"] == image,
          "and is on the filesystem, byte for byte");
    Check(picoOTA.Committed == "/firmware.bin", "the loader was told which file to install");
    /* Asked for and not taken: a controller that restarted inside the handler would send the
       browser nothing, which reads as a failed update. The application's tick carries it. */
    Check(!rp2040.Restarted, "the handler does not restart the controller itself");
    System::GetInstance().PerformPendingRestart();
    Check(rp2040.Restarted, "but the tick after it does");

    /* the catalog, generated from the real table */
    const std::string catalog = Fetch("/commands");
    Check(!catalog.empty() && (catalog.front() == '[') && (catalog.back() == ']'),
          "the catalog is a JSON array");
    Check(IsBalancedJson(catalog), "its brackets and strings are balanced");
    Check(catalog.find("\"number\":3") != std::string::npos, "it carries the display brightness command");
    Check(catalog.find("\"short\":\"B\"") != std::string::npos, "it carries an option short name");
    Check(catalog.find("\"max\":255") != std::string::npos, "it carries the ranges");
    Check(catalog.find("\"values\":[") != std::string::npos, "it carries named values for the enumerations");
    /* Without this the page would offer the status fields as inputs, and a value typed
       into one of them would come back as "parameter unknown". */
    Check(catalog.find("\"readonly\":true") != std::string::npos, "it marks the fields that only appear in answers");
    size_t commands = 0u;
    for(size_t at = catalog.find("\"number\":"); at != std::string::npos; at = catalog.find("\"number\":", at + 1u)) { commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", catalog.size(), commands);

    /* the panel description: shape and letters, the letters as UTF-8 */
    const std::string panel = Fetch("/display");
    Check(IsBalancedJson(panel), "the display description is well formed");
    Check(panel.find("\"columns\":11") != std::string::npos, "it carries the column count");
    Check(panel.find("\"rows\":10") != std::string::npos, "it carries the row count");
    Check(panel.find("ESKISTLF") != std::string::npos, "it carries the letters");
    /* U with an umlaut is 0xDC in the firmware's table and has to arrive as two UTF-8 bytes;
       the raw byte would make the whole document invalid. */
    Check(panel.find("\xc3\x9c") != std::string::npos, "the umlauts arrive as UTF-8");
    Check(panel.find("\xdc") == std::string::npos, "no raw Latin-1 byte escapes into the JSON");

    /* the handshake, which is an event here rather than a GET to answer */
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_CONNECT, nullptr, nullptr, 0u);
    webStubState().ClientCount = 1u;

    /* a frame without a terminator - the bridge has to add it */
    webStubState().Text.clear();
    SendFrame("3 -B200");

    Communication& communication = Communication::GetInstance();
    communication.Task();
    communication.Task();

    bool found = false;
    for(const auto& line : webStubState().Text) { if(line.find("B=200") != std::string::npos) { found = true; } }
    Check(found, "the answer came back over the socket");
    for(const auto& line : webStubState().Text) { printf("   sent: \"%s\"\n", line.c_str()); }

    /* An answer carrying a Latin-1 byte, which is what the overlay text command answers with
       once somebody sets a text with an umlaut in it - the thing the font tables carry
       umlauts for. A web socket text frame has to be valid UTF-8, so a raw 0xFC here does
       not draw one character wrong, it makes the browser close the connection.

       The strings are split around the escapes on purpose: "\xFCck" would swallow the c as a
       further hex digit and stop compiling. */
    webStubState().Text.clear();
    WebInterface::GetInstance().BroadcastLine("T=Gl\xFC" "ck");
    Check(!webStubState().Text.empty(), "an answer with an umlaut is sent at all");
    Check(!webStubState().Text.empty() && webStubState().Text.back() == "T=Gl\xC3\xBC" "ck",
          "a Latin-1 byte in an answer leaves as UTF-8");
    Check(!webStubState().Text.empty() && webStubState().Text.back().find('\xFC') == std::string::npos,
          "and no raw Latin-1 byte survives into the frame");

    /* the display frame */
    WebInterface& web = WebInterface::GetInstance();
    constexpr unsigned kInterval{WEB_FRONTEND_FRAME_INTERVAL_TICKS};
    auto run_interval = [&web](unsigned count) {
        for(unsigned tick = 0u; tick < (count * kInterval); tick++) { web.BroadcastFrame(); }
    };

    /* The handshake above counted as a client arriving, so one frame is already owed. Drain
       it before measuring anything, or every count below is one too high. */
    run_interval(1u);
    webStubState().Binary.clear();

    run_interval(3u);
    Check(webStubState().Binary.empty(), "a display that never changed sends no frame");

    Pixels::GetInstance().SetPixel(0u, 10u, 20u, 30u);
    run_interval(3u);
    Check(webStubState().Binary.size() == 1u, "a change is sent once and not repeated");

    if(!webStubState().Binary.empty()) {
        const std::string& frame = webStubState().Binary[0];
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(frame.data());

        Check(frame.size() == 110u * 3u, "the frame is 110 * 3 bytes");
        Check((bytes[0] == 20u) && (bytes[1] == 10u) && (bytes[2] == 30u),
              "the frame carries green, red, blue like the strip");
        printf("   frame: %zu bytes, one per %u ticks\n", frame.size(), kInterval);
    }

    webStubState().Binary.clear();
    Pixels::GetInstance().DisablePixels();
    run_interval(1u);
    const bool browser_dark = !webStubState().Binary.empty() &&
        std::all_of(webStubState().Binary[0].begin(), webStubState().Binary[0].end(),
                    [](char value) { return value == 0; });
    Check(browser_dark, "disabling the display also blanks the browser frame");

    webStubState().Binary.clear();
    Pixels::GetInstance().EnablePixels();
    run_interval(1u);
    Check(!webStubState().Binary.empty() &&
          static_cast<unsigned char>(webStubState().Binary[0][0]) == 20u &&
          static_cast<unsigned char>(webStubState().Binary[0][1]) == 10u &&
          static_cast<unsigned char>(webStubState().Binary[0][2]) == 30u,
          "enabling the display restores the browser frame");

    /* A client arriving between two changes still has to be shown the panel. Who it goes to
       is the socket's business here, so what is checked is that a frame goes out at all on
       an unchanged display - which is the decision that is still ours. */
    webStubState().Binary.clear();
    web.OnClientOpened();
    run_interval(1u);
    Check(webStubState().Binary.size() == 1u, "a client that arrives is shown the standing display");

    /* Nothing changed and nobody new arrived, so the next interval is silent again. */
    webStubState().Binary.clear();
    run_interval(2u);
    Check(webStubState().Binary.empty(), "and the intervals after it stay quiet");

    /* Changed on every tick, and still one frame per interval rather than one per tick. */
    webStubState().Binary.clear();
    for(unsigned tick = 0u; tick < (4u * kInterval); tick++) {
        Pixels::GetInstance().SetPixel(1u, static_cast<byte>(tick + 1u), 0u, 0u);
        web.BroadcastFrame();
    }
    Check(webStubState().Binary.size() == 4u, "a display changing every tick still sends one per interval");

    /* Nothing to send to, so nothing is gathered - the check the client count exists for. */
    webStubState().ClientCount = 0u;
    webStubState().Binary.clear();
    Pixels::GetInstance().SetPixel(2u, 5u, 5u, 5u);
    run_interval(2u);
    Check(webStubState().Binary.empty(), "with nobody watching, no frame is built at all");
    webStubState().ClientCount = 1u;

    /* An oversized frame must be refused whole rather than truncated into the parser, and a
       split one dropped rather than reassembled. Both would put half a command in front of
       it. */
    webStubState().Text.clear();
    SendFrame(std::string(WEB_FRONTEND_MAX_FRAME_LENGTH + 10u, 'x'));
    communication.Task();
    communication.Task();
    Check(webStubState().Text.empty(), "an oversized frame is refused");

    SendFrame("3 -B7", false);
    communication.Task();
    communication.Task();
    Check(webStubState().Text.empty(), "and so is a split one");


    /* ---- the console password --------------------------------------------------------
       The gate, and only the gate: the credential itself is compared by the server library on
       the target, so what a host can check is who is asked and what happens when the answer is
       no. Base64 over plain http is not encryption either way, and this backend's notes say so.
    */
    Check(!System::GetInstance().IsConsoleProtected(), "a clock with no password is not protected");
    request_authorised = false;
    Check(!Fetch("/display").empty(), "and answers a request that carries no credential");

    Check(System::GetInstance().SetConsolePassword("hunter2") == kEOk, "a password can be stored");
    Check(System::GetInstance().IsConsoleProtected(), "which is what protected means");

    for(const char* route : {"/", "/console", "/commands", "/display", "/manifest.webmanifest", "/icon-192.png", "/update"}) {
        authentication_was_requested = false;
        const std::string body = Fetch(route);
        Check(body.empty() && authentication_was_requested, route);
    }

    /* The update again, through the path an image really takes: the body arrives before the
       request handler runs, so a check that sat only in the handler would have let an
       unauthorised upload write the filesystem first and refuse afterwards. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    const UploadOutcome unauthorised = Post("/update", image, image.size());
    Check(unauthorised.body.find("\"ok\":false") != std::string::npos || unauthorised.body.empty(),
          "an unauthorised upload is not accepted");
    Check(littleFsStore().count("/firmware.bin") == 0u, "and never reaches the filesystem");
    Check(picoOTA.Committed.empty(), "and nothing was handed to the loader");
    /* No restart assertion here: by this point a successful upload above has asked for one,
       and that flag does not come back. What this case is about is the two lines above it -
       that the filesystem was never touched by a request that had no business writing it. */

    request_authorised = true;
    Check(!Fetch("/display").empty(), "the right credential is let through");

    /* And the way back for a clock nobody can log into any more. */
    Check(System::GetInstance().SetConsolePassword("") == kEOk, "the password can be cleared");
    Check(!System::GetInstance().IsConsoleProtected(), "which unprotects the console");
    request_authorised = false;
    Check(!Fetch("/display").empty(), "and answers everybody again");
    request_authorised = true;

    return Report();
}
