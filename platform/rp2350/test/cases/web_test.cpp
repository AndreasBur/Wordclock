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
static bool isBalancedJson(const std::string& Text)
{
    int Depth = 0;
    bool InString = false;

    for(size_t Index = 0u; Index < Text.size(); Index++) {
        const char Character = Text[Index];

        if(InString) {
            if(Character == '\\')      { Index++; }
            else if(Character == '"')  { InString = false; }
            continue;
        }
        if(Character == '"')                                 { InString = true; }
        else if((Character == '[') || (Character == '{'))     { Depth++; }
        else if((Character == ']') || (Character == '}'))     { Depth--; if(Depth < 0) { return false; } }
    }
    return (Depth == 0) && !InString;
}

/* Runs one route and hands back what it produced. The request owns its response, so it
   outlives neither - hence the copy out. */
/* Whether the next fetch() arrives with a credential the library would accept. The library
   does the comparing on the target, so what a case here decides is its answer. */
static bool RequestAuthorised = true;
static bool AuthenticationWasRequested = false;

static std::string fetch(const char* Route, std::string* ContentEncoding = nullptr)
{
    AsyncWebServerRequest Request;
    Request.Authorised = RequestAuthorised;
    const auto Found = webStubState().Routes.find(Route);

    if(Found == webStubState().Routes.end()) { return ""; }

    Found->second(&Request);

    AuthenticationWasRequested = Request.AuthenticationRequested;

    if(Request.Response == nullptr) { return ""; }
    if(ContentEncoding != nullptr) {
        const auto Header = Request.Response->Headers.find("Content-Encoding");
        *ContentEncoding = (Header == Request.Response->Headers.end()) ? "" : Header->second;
    }
    return Request.Response->Body;
}

/* An upload, in the order the library performs one: the body arrives in chunks through the
   body handler, and only when it is through does the request handler answer. A test that
   called the request handler alone would be testing the answer and not the update.

   Chunked on purpose, and at a size that leaves a remainder, because the handler's arithmetic
   is what decides when an image is complete. */
struct UploadOutcome {
    int Code{0};
    std::string Body;
};

static UploadOutcome post(const char* Route, const std::string& Content, size_t Announced,
                          size_t ChunkSize = 300u)
{
    AsyncWebServerRequest Request;
    Request.Authorised = RequestAuthorised;

    const auto Body = webStubState().BodyRoutes.find(Route);
    const auto Handler = webStubState().Routes.find(Route);

    if((Body == webStubState().BodyRoutes.end()) || (Handler == webStubState().Routes.end())) { return {}; }

    for(size_t Index = 0u; Index < Content.size(); Index += ChunkSize) {
        const size_t Length = std::min(ChunkSize, Content.size() - Index);
        std::vector<uint8_t> Chunk(Content.begin() + Index, Content.begin() + Index + Length);
        Body->second(&Request, Chunk.data(), Length, Index, Announced);
    }
    Handler->second(&Request);

    AuthenticationWasRequested = Request.AuthenticationRequested;
    if(Request.Response == nullptr) { return {}; }
    return {Request.Response->Code, Request.Response->Body};
}

/* One text frame into the socket, the way the library reports one. */
static void sendFrame(const std::string& Payload, bool Final = true)
{
    AwsFrameInfo Info{};
    Info.final = Final;
    Info.index = 0u;
    Info.len = Payload.size();
    Info.opcode = WS_TEXT;

    std::vector<uint8_t> Bytes(Payload.begin(), Payload.end());
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_DATA, &Info,
                                 Bytes.empty() ? nullptr : Bytes.data(), Bytes.size());
}


int main()
{
    check(WebInterface::getInstance().begin() == E_OK, "the server starts and registers its handlers");
    check(webStubState().Listening, "and it is listening");
    /* "/" is the panel and the console has its own route, so both have to be there: a missing
       registration would answer 404 on the page everything the panel does not cover goes
       through. */
    check(webStubState().Routes.count("/console") == 1u, "the console has a route of its own");
    check((webStubState().SocketHandler != nullptr) && (webStubState().Routes.count("/") == 1u)
          && (webStubState().Routes.count("/commands") == 1u)
          && (webStubState().Routes.count("/display") == 1u),
          "all four handlers were registered");
    check((webStubState().Routes.count("/manifest.webmanifest") == 1u)
          && (webStubState().Routes.count("/icon-192.png") == 1u)
          && (webStubState().Routes.count("/icon-512.png") == 1u),
          "and the three the home screen needs with them");
    check(webStubState().Routes.count("/update") == 1u, "and the update route");
    /* The body handler is the half that matters: without it the image would arrive nowhere
       and the request handler would answer about an upload that never happened. */
    check(webStubState().BodyRoutes.count("/update") == 1u, "with the body handler the image arrives through");

    /* both pages: gzipped, announced as such, and not the same bytes - a swapped pair of
       pointers would pass every other check here. */
    std::string Encoding;
    const std::string Page = fetch("/", &Encoding);
    check(Encoding == "gzip", "the page is announced as gzip");
    check((Page.size() > 100u) && (static_cast<unsigned char>(Page[0]) == 0x1fu)
                               && (static_cast<unsigned char>(Page[1]) == 0x8bu),
          "the page really is a gzip stream");

    const std::string Console = fetch("/console", &Encoding);
    check(Encoding == "gzip", "the console is announced as gzip");
    check(Console.size() > 100u && Console != Page, "and is a different page from the panel");

    /* the manifest and the icons, which nothing on the clock reads and a browser does */
    const std::string Manifest = fetch("/manifest.webmanifest", &Encoding);
    check(Encoding.empty(), "the manifest is sent as it is, not gzipped");
    check(isBalancedJson(Manifest), "and is well formed JSON");
    check(Manifest.find("standalone") != std::string::npos, "it asks to start without an address bar");
    check(Manifest.find("icon-192.png") != std::string::npos
          && Manifest.find("icon-512.png") != std::string::npos, "it names both icons");
    /* Relative, because the clock answers on whatever address it was given: an absolute
       start_url baked in here would send an installed console to a different clock. */
    check(Manifest.find("\"start_url\": \".\"") != std::string::npos, "and starts at a relative URL");

    const std::string SmallIcon = fetch("/icon-192.png", &Encoding);
    check(Encoding.empty(), "an icon is not gzipped either, a PNG being deflated already");
    check(SmallIcon.size() > 500u && SmallIcon.compare(1u, 3u, "PNG") == 0, "and really is a PNG");
    const std::string LargeIcon = fetch("/icon-512.png");
    check(LargeIcon.size() > SmallIcon.size() && LargeIcon.compare(1u, 3u, "PNG") == 0,
          "the large icon is a PNG too, and the larger of the two");

    /* ---- the update ------------------------------------------------------------------
       This board has no second app partition, so an update is a file plus a command page that
       the loader at the front of the image reads on the next boot. Which makes the interesting
       question not "did it answer 200" but *what it left on the filesystem* - because that,
       and only that, is what the next boot acts on.
    */
    const std::string Image(2000u, '\xa5');

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
    UploadOutcome Update = post("/update", Image.substr(0u, 900u), Image.size());
    check(Update.Code == 400 && Update.Body.find("stopped before") != std::string::npos,
          "an upload that stops short is refused, and says that is what happened");
    check(picoOTA.Committed.empty(), "and nothing was handed to the loader");
    check(littleFsStore().count("/firmware.bin") == 0u, "and leaves no image behind either");
    System::getInstance().performPendingRestart();
    check(!rp2040.Restarted, "and the clock is left running what it has");

    /* An image that does not fit is refused before the first byte is written, so it cannot
       fill the filesystem the settings live in. 64 KB is what this board's filesystem was
       before it had to hold an image. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    LittleFS.Total = 64u * 1024u;
    Update = post("/update", Image, 200u * 1024u);
    check(Update.Code == 400 && Update.Body.find("does not fit") != std::string::npos,
          "an image larger than the filesystem is refused, and says so");
    check(littleFsStore().count("/firmware.bin") == 0u, "and never reaches the filesystem");
    System::getInstance().performPendingRestart();
    check(!rp2040.Restarted, "and asks for no restart");
    LittleFS.Total = 1024u * 1024u;

    /* A command page that will not write is a failed update, and it takes the image with it:
       what is on the filesystem after a failure is a file nobody will read. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    picoOTA.CommitSucceeds = false;
    Update = post("/update", Image, Image.size());
    check(Update.Code == 400, "a command page that will not write is a failed update");
    check(littleFsStore().count("/firmware.bin") == 0u, "and the image is taken off the filesystem");
    picoOTA = PicoOtaStub{};

    /* A POST that carries nothing never reaches the body handler at all, which is its own
       case and not an empty success. */
    littleFsStore().erase("/firmware.bin");
    const std::string Nothing = fetch("/update");
    check(Nothing.find("\"ok\":false") != std::string::npos, "a POST with no body is refused");
    check(isBalancedJson(Nothing), "and the answer is well formed JSON");

    /* And the one that works, last for the reason above. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    Update = post("/update", Image, Image.size());
    check(Update.Code == 200 && Update.Body.find("\"ok\":true") != std::string::npos,
          "a complete image is accepted");
    check(littleFsStore().count("/firmware.bin") == 1u
          && littleFsStore()["/firmware.bin"] == Image,
          "and is on the filesystem, byte for byte");
    check(picoOTA.Committed == "/firmware.bin", "the loader was told which file to install");
    /* Asked for and not taken: a controller that restarted inside the handler would send the
       browser nothing, which reads as a failed update. The application's tick carries it. */
    check(!rp2040.Restarted, "the handler does not restart the controller itself");
    System::getInstance().performPendingRestart();
    check(rp2040.Restarted, "but the tick after it does");

    /* the catalog, generated from the real table */
    const std::string Catalog = fetch("/commands");
    check(!Catalog.empty() && (Catalog.front() == '[') && (Catalog.back() == ']'),
          "the catalog is a JSON array");
    check(isBalancedJson(Catalog), "its brackets and strings are balanced");
    check(Catalog.find("\"number\":3") != std::string::npos, "it carries the display brightness command");
    check(Catalog.find("\"short\":\"B\"") != std::string::npos, "it carries an option short name");
    check(Catalog.find("\"max\":255") != std::string::npos, "it carries the ranges");
    check(Catalog.find("\"values\":[") != std::string::npos, "it carries named values for the enumerations");
    /* Without this the page would offer the status fields as inputs, and a value typed
       into one of them would come back as "parameter unknown". */
    check(Catalog.find("\"readonly\":true") != std::string::npos, "it marks the fields that only appear in answers");
    size_t Commands = 0u;
    for(size_t At = Catalog.find("\"number\":"); At != std::string::npos; At = Catalog.find("\"number\":", At + 1u)) { Commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", Catalog.size(), Commands);

    /* the panel description: shape and letters, the letters as UTF-8 */
    const std::string Panel = fetch("/display");
    check(isBalancedJson(Panel), "the display description is well formed");
    check(Panel.find("\"columns\":11") != std::string::npos, "it carries the column count");
    check(Panel.find("\"rows\":10") != std::string::npos, "it carries the row count");
    check(Panel.find("ESKISTLF") != std::string::npos, "it carries the letters");
    /* U with an umlaut is 0xDC in the firmware's table and has to arrive as two UTF-8 bytes;
       the raw byte would make the whole document invalid. */
    check(Panel.find("\xc3\x9c") != std::string::npos, "the umlauts arrive as UTF-8");
    check(Panel.find("\xdc") == std::string::npos, "no raw Latin-1 byte escapes into the JSON");

    /* the handshake, which is an event here rather than a GET to answer */
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_CONNECT, nullptr, nullptr, 0u);
    webStubState().ClientCount = 1u;

    /* a frame without a terminator - the bridge has to add it */
    webStubState().Text.clear();
    sendFrame("3 -B200");

    Communication& communication = Communication::getInstance();
    communication.task();
    communication.task();

    bool Found = false;
    for(const auto& Line : webStubState().Text) { if(Line.find("B=200") != std::string::npos) { Found = true; } }
    check(Found, "the answer came back over the socket");
    for(const auto& Line : webStubState().Text) { printf("   sent: \"%s\"\n", Line.c_str()); }

    /* An answer carrying a Latin-1 byte, which is what the overlay text command answers with
       once somebody sets a text with an umlaut in it - the thing the font tables carry
       umlauts for. A web socket text frame has to be valid UTF-8, so a raw 0xFC here does
       not draw one character wrong, it makes the browser close the connection.

       The strings are split around the escapes on purpose: "\xFCck" would swallow the c as a
       further hex digit and stop compiling. */
    webStubState().Text.clear();
    WebInterface::getInstance().broadcastLine("T=Gl\xFC" "ck");
    check(!webStubState().Text.empty(), "an answer with an umlaut is sent at all");
    check(!webStubState().Text.empty() && webStubState().Text.back() == "T=Gl\xC3\xBC" "ck",
          "a Latin-1 byte in an answer leaves as UTF-8");
    check(!webStubState().Text.empty() && webStubState().Text.back().find('\xFC') == std::string::npos,
          "and no raw Latin-1 byte survives into the frame");

    /* the display frame */
    WebInterface& web = WebInterface::getInstance();
    const unsigned interval = WEB_FRONTEND_FRAME_INTERVAL_TICKS;
    auto runInterval = [&web, interval](unsigned count) {
        for(unsigned tick = 0u; tick < (count * interval); tick++) { web.broadcastFrame(); }
    };

    /* The handshake above counted as a client arriving, so one frame is already owed. Drain
       it before measuring anything, or every count below is one too high. */
    runInterval(1u);
    webStubState().Binary.clear();

    runInterval(3u);
    check(webStubState().Binary.empty(), "a display that never changed sends no frame");

    Pixels::getInstance().setPixel(0u, 10u, 20u, 30u);
    runInterval(3u);
    check(webStubState().Binary.size() == 1u, "a change is sent once and not repeated");

    if(!webStubState().Binary.empty()) {
        const std::string& Frame = webStubState().Binary[0];
        const unsigned char* Bytes = reinterpret_cast<const unsigned char*>(Frame.data());

        check(Frame.size() == 110u * 3u, "the frame is 110 * 3 bytes");
        check((Bytes[0] == 20u) && (Bytes[1] == 10u) && (Bytes[2] == 30u),
              "the frame carries green, red, blue like the strip");
        printf("   frame: %zu bytes, one per %u ticks\n", Frame.size(), interval);
    }

    webStubState().Binary.clear();
    Pixels::getInstance().disablePixels();
    runInterval(1u);
    const bool BrowserDark = !webStubState().Binary.empty() &&
        std::all_of(webStubState().Binary[0].begin(), webStubState().Binary[0].end(),
                    [](char Value) { return Value == 0; });
    check(BrowserDark, "disabling the display also blanks the browser frame");

    webStubState().Binary.clear();
    Pixels::getInstance().enablePixels();
    runInterval(1u);
    check(!webStubState().Binary.empty() &&
          static_cast<unsigned char>(webStubState().Binary[0][0]) == 20u &&
          static_cast<unsigned char>(webStubState().Binary[0][1]) == 10u &&
          static_cast<unsigned char>(webStubState().Binary[0][2]) == 30u,
          "enabling the display restores the browser frame");

    /* A client arriving between two changes still has to be shown the panel. Who it goes to
       is the socket's business here, so what is checked is that a frame goes out at all on
       an unchanged display - which is the decision that is still ours. */
    webStubState().Binary.clear();
    web.onClientOpened();
    runInterval(1u);
    check(webStubState().Binary.size() == 1u, "a client that arrives is shown the standing display");

    /* Nothing changed and nobody new arrived, so the next interval is silent again. */
    webStubState().Binary.clear();
    runInterval(2u);
    check(webStubState().Binary.empty(), "and the intervals after it stay quiet");

    /* Changed on every tick, and still one frame per interval rather than one per tick. */
    webStubState().Binary.clear();
    for(unsigned tick = 0u; tick < (4u * interval); tick++) {
        Pixels::getInstance().setPixel(1u, static_cast<byte>(tick + 1u), 0u, 0u);
        web.broadcastFrame();
    }
    check(webStubState().Binary.size() == 4u, "a display changing every tick still sends one per interval");

    /* Nothing to send to, so nothing is gathered - the check the client count exists for. */
    webStubState().ClientCount = 0u;
    webStubState().Binary.clear();
    Pixels::getInstance().setPixel(2u, 5u, 5u, 5u);
    runInterval(2u);
    check(webStubState().Binary.empty(), "with nobody watching, no frame is built at all");
    webStubState().ClientCount = 1u;

    /* An oversized frame must be refused whole rather than truncated into the parser, and a
       split one dropped rather than reassembled. Both would put half a command in front of
       it. */
    webStubState().Text.clear();
    sendFrame(std::string(WEB_FRONTEND_MAX_FRAME_LENGTH + 10u, 'x'));
    communication.task();
    communication.task();
    check(webStubState().Text.empty(), "an oversized frame is refused");

    sendFrame("3 -B7", false);
    communication.task();
    communication.task();
    check(webStubState().Text.empty(), "and so is a split one");


    /* ---- the console password --------------------------------------------------------
       The gate, and only the gate: the credential itself is compared by the server library on
       the target, so what a host can check is who is asked and what happens when the answer is
       no. Base64 over plain http is not encryption either way, and this backend's notes say so.
    */
    check(!System::getInstance().isConsoleProtected(), "a clock with no password is not protected");
    RequestAuthorised = false;
    check(!fetch("/display").empty(), "and answers a request that carries no credential");

    check(System::getInstance().setConsolePassword("hunter2") == E_OK, "a password can be stored");
    check(System::getInstance().isConsoleProtected(), "which is what protected means");

    for(const char* Route : {"/", "/console", "/commands", "/display", "/manifest.webmanifest", "/icon-192.png", "/update"}) {
        AuthenticationWasRequested = false;
        const std::string Body = fetch(Route);
        check(Body.empty() && AuthenticationWasRequested, Route);
    }

    /* The update again, through the path an image really takes: the body arrives before the
       request handler runs, so a check that sat only in the handler would have let an
       unauthorised upload write the filesystem first and refuse afterwards. */
    littleFsStore().erase("/firmware.bin");
    picoOTA = PicoOtaStub{};
    const UploadOutcome Unauthorised = post("/update", Image, Image.size());
    check(Unauthorised.Body.find("\"ok\":false") != std::string::npos || Unauthorised.Body.empty(),
          "an unauthorised upload is not accepted");
    check(littleFsStore().count("/firmware.bin") == 0u, "and never reaches the filesystem");
    check(picoOTA.Committed.empty(), "and nothing was handed to the loader");
    /* No restart assertion here: by this point a successful upload above has asked for one,
       and that flag does not come back. What this case is about is the two lines above it -
       that the filesystem was never touched by a request that had no business writing it. */

    RequestAuthorised = true;
    check(!fetch("/display").empty(), "the right credential is let through");

    /* And the way back for a clock nobody can log into any more. */
    check(System::getInstance().setConsolePassword("") == E_OK, "the password can be cleared");
    check(!System::getInstance().isConsoleProtected(), "which unprotects the console");
    RequestAuthorised = false;
    check(!fetch("/display").empty(), "and answers everybody again");
    RequestAuthorised = true;

    return report();
}
