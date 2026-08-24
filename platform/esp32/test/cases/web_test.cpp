/* Drives the registered handlers the way the server reaches them - through the
   registration call - so WebInterface's own wiring is covered rather than bypassed. */
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "System.h"
#include "Update.h"
#include "WebInterface.h"
#include "check.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>


static httpd_handler_t SocketHandler = nullptr;
static httpd_handler_t RootHandler = nullptr;
static httpd_handler_t CommandsHandler = nullptr;
static httpd_handler_t DisplayHandler = nullptr;
static httpd_handler_t ManifestHandler = nullptr;
static httpd_handler_t Icon192Handler = nullptr;
static httpd_handler_t Icon512Handler = nullptr;
static httpd_handler_t UpdateHandler = nullptr;
static httpd_handler_t ConsoleHandler = nullptr;
static std::string SentType;
static std::string SentStatus;
/* Every header a handler set, by name - the gate's WWW-Authenticate is read out of here. */
static std::map<std::string, std::string> SentHeaders;
/* The body the next upload is to be fed, and how much of it the stub is willing to hand
   over - short of the announced length is a connection that went away mid-upload. */
static std::string Body;
static size_t BodyOffset = 0u;
static size_t BodyLimit = 0u;
static std::string PendingFrame;
static std::vector<std::string> Sent;
static std::string RootBody;
static std::string RootEncoding;
static std::string Chunked;

esp_err_t httpd_start(httpd_handle_t* h, const httpd_config_t*) { *h = (httpd_handle_t)1; return ESP_OK; }

esp_err_t httpd_register_uri_handler(httpd_handle_t, const httpd_uri_t* u)
{
    if(strcmp(u->uri, "/ws") == 0)            { SocketHandler = u->handler; }
    else if(strcmp(u->uri, "/commands") == 0) { CommandsHandler = u->handler; }
    else if(strcmp(u->uri, "/display") == 0)  { DisplayHandler = u->handler; }
    else if(strcmp(u->uri, "/manifest.webmanifest") == 0) { ManifestHandler = u->handler; }
    else if(strcmp(u->uri, "/icon-192.png") == 0) { Icon192Handler = u->handler; }
    else if(strcmp(u->uri, "/icon-512.png") == 0) { Icon512Handler = u->handler; }
    else if(strcmp(u->uri, "/update") == 0)   { UpdateHandler = u->handler; }
    else if(strcmp(u->uri, "/console") == 0)  { ConsoleHandler = u->handler; }
    else                                      { RootHandler = u->handler; }
    return ESP_OK;
}

esp_err_t httpd_resp_set_type(httpd_req_t*, const char* t) { SentType = t; return ESP_OK; }

esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char* k, const char* v)
{
    if(strcmp(k, "Content-Encoding") == 0) { RootEncoding = v; }
    SentHeaders[k] = v;
    return ESP_OK;
}

esp_err_t httpd_resp_set_status(httpd_req_t*, const char* s) { SentStatus = s; return ESP_OK; }

/* The Authorization header the next request is to carry, empty for a request without one. */
static std::string Authorization;

esp_err_t httpd_req_get_hdr_value_str(httpd_req_t*, const char* Field, char* Target, size_t Capacity)
{
    if((strcmp(Field, "Authorization") != 0) || Authorization.empty()) { return ESP_FAIL; }
    if(Authorization.size() >= Capacity) { return ESP_FAIL; }

    memcpy(Target, Authorization.c_str(), Authorization.size() + 1u);
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t*, const char* b, ssize_t n)
{
    /* The update route answers with the framework's "until the terminator" sentinel, the
       others with a length - and a body of bytes may hold a zero, so the two cannot be
       collapsed into strlen(). */
    RootBody.assign(b, (n == HTTPD_RESP_USE_STRLEN) ? strlen(b) : static_cast<size_t>(n));
    return ESP_OK;
}

/* Hands over the prepared body a chunk at a time, and stops at BodyLimit rather than at its
   end - which is how a dropped connection is reached without a socket to drop. */
int httpd_req_recv(httpd_req_t*, char* Target, size_t Max)
{
    if(BodyOffset >= BodyLimit) { return 0; }

    const size_t Take = std::min(Max, BodyLimit - BodyOffset);
    memcpy(Target, Body.data() + BodyOffset, Take);
    BodyOffset += Take;

    return static_cast<int>(Take);
}

esp_err_t httpd_resp_send_chunk(httpd_req_t*, const char* b, ssize_t n)
{
    if(b != nullptr) { Chunked.append(b, static_cast<size_t>(n)); }
    return ESP_OK;
}

int httpd_req_to_sockfd(httpd_req_t*) { return 7; }

esp_err_t httpd_ws_recv_frame(httpd_req_t*, httpd_ws_frame_t* f, size_t max)
{
    if(PendingFrame.size() > max) { return -1; }
    memcpy(f->payload, PendingFrame.data(), PendingFrame.size());
    f->len = PendingFrame.size();
    f->type = HTTPD_WS_TYPE_TEXT;
    return ESP_OK;
}

static std::vector<std::string> SentBinary;
/* The descriptor each frame went to, so a test can ask who was served rather than only how
   many frames left - the answer differs once more than one client is registered. */
static std::vector<int> SentBinaryTo;

esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int descriptor, httpd_ws_frame_t* f)
{
    std::string payload(reinterpret_cast<const char*>(f->payload), f->len);

    if(f->type == HTTPD_WS_TYPE_BINARY) { SentBinary.push_back(payload); SentBinaryTo.push_back(descriptor); }
    else                                { Sent.push_back(payload); }
    return ESP_OK;
}

static size_t framesSentTo(int descriptor)
{
    size_t count = 0u;
    for(const int to : SentBinaryTo) { if(to == descriptor) { count++; } }
    return count;
}

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

int main()
{
    check(WebInterface::getInstance().begin() == E_OK, "the server starts and registers its handlers");
    check((SocketHandler != nullptr) && (RootHandler != nullptr) && (CommandsHandler != nullptr)
          && (DisplayHandler != nullptr), "all four handlers were registered");
    /* Separately, because the server's own handler limit is what would drop these: it is a
       number in the configuration, and one route past it is refused rather than reported. */
    check((ManifestHandler != nullptr) && (Icon192Handler != nullptr) && (Icon512Handler != nullptr)
          && (UpdateHandler != nullptr),
          "and the four added since fit under the handler limit as well");
    /* The ninth, which is the one that pushed the limit past its default: "/" is the panel and
       the console moved here, so a missing registration would answer 404 on the page that
       everything the panel does not cover goes through. */
    check(ConsoleHandler != nullptr, "and the console's own route fits too");

    httpd_req_t request{};
    request.method = HTTP_GET;

    /* the two pages: both gzipped, both announced as such, and not the same bytes - "/" is
       the panel and "/console" the generated console. Told apart by their titles, because
       that is what a browser would show and what a swapped pair of pointers would get
       wrong while every other check here still passed. */
    ConsoleHandler(&request);
    const std::string Console = RootBody;
    check(RootEncoding == "gzip", "the console is announced as gzip");
    check(Console.size() > 100u, "and is a page");

    RootHandler(&request);
    check(RootEncoding == "gzip", "the page is announced as gzip");
    check((RootBody.size() > 100u) && (static_cast<unsigned char>(RootBody[0]) == 0x1fu)
                                   && (static_cast<unsigned char>(RootBody[1]) == 0x8bu),
          "the page really is a gzip stream");
    check(RootBody != Console, "and the two routes are two different pages");

    /* the manifest: a browser reads it, so the type is part of the answer and not decoration */
    RootEncoding.clear();
    ManifestHandler(&request);
    check(SentType == "application/manifest+json", "the manifest is announced as a manifest");
    check(RootEncoding.empty(), "and sent as it is, not gzipped");
    const std::string Manifest = RootBody;
    check(isBalancedJson(Manifest), "the manifest is well formed JSON");
    check(Manifest.find("\"display\"") != std::string::npos
          && Manifest.find("standalone") != std::string::npos, "it asks to start without an address bar");
    check(Manifest.find("icon-192.png") != std::string::npos
          && Manifest.find("icon-512.png") != std::string::npos, "it names both icons");
    /* Relative, because the clock answers on whatever address it was given: an absolute
       start_url baked in here would send an installed console to a different clock. */
    check(Manifest.find("\"start_url\": \".\"") != std::string::npos, "and starts at a relative URL");

    /* the icons: sent as they are, which is what a PNG wants */
    RootEncoding.clear();
    Icon192Handler(&request);
    check(SentType == "image/png", "the small icon is announced as a PNG");
    check(RootEncoding.empty(), "and not as gzip, a PNG being deflated already");
    check(RootBody.size() > 500u && RootBody.compare(1u, 3u, "PNG") == 0, "and really is one");

    Icon512Handler(&request);
    check(RootBody.size() > 5000u && RootBody.compare(1u, 3u, "PNG") == 0,
          "the large icon is a PNG too, and the larger of the two");

    /* the catalog, generated from the real table */
    CommandsHandler(&request);
    check(!Chunked.empty() && (Chunked.front() == '[') && (Chunked.back() == ']'),
          "the catalog is a JSON array");
    check(isBalancedJson(Chunked), "its brackets and strings are balanced");
    check(Chunked.find("\"number\":3") != std::string::npos, "it carries the display brightness command");
    check(Chunked.find("\"short\":\"B\"") != std::string::npos, "it carries an option short name");
    check(Chunked.find("\"max\":255") != std::string::npos, "it carries the ranges");
    check(Chunked.find("\"values\":[") != std::string::npos, "it carries named values for the enumerations");
    /* Without this the page would offer the status fields as inputs, and a value typed
       into one of them would come back as "parameter unknown". */
    check(Chunked.find("\"readonly\":true") != std::string::npos, "it marks the fields that only appear in answers");
    size_t Commands = 0u;
    for(size_t At = Chunked.find("\"number\":"); At != std::string::npos; At = Chunked.find("\"number\":", At + 1u)) { Commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", Chunked.size(), Commands);

    /* the panel description: shape and letters, the letters as UTF-8 */
    Chunked.clear();
    DisplayHandler(&request);
    check(isBalancedJson(Chunked), "the display description is well formed");
    check(Chunked.find("\"columns\":11") != std::string::npos, "it carries the column count");
    check(Chunked.find("\"rows\":10") != std::string::npos, "it carries the row count");
    check(Chunked.find("ESKISTLF") != std::string::npos, "it carries the letters");
    /* U with an umlaut is 0xDC in the firmware's table and has to arrive as two UTF-8 bytes;
       the raw byte would make the whole document invalid. */
    check(Chunked.find("\xc3\x9c") != std::string::npos, "the umlauts arrive as UTF-8");
    check(Chunked.find("\xdc") == std::string::npos, "no raw Latin-1 byte escapes into the JSON");

    /* the handshake registers the client, so answers have somewhere to go */
    SocketHandler(&request);

    /* a frame without a terminator - the bridge has to add it */
    Sent.clear();
    PendingFrame = "3 -B200";
    request.method = HTTP_POST;
    check(SocketHandler(&request) == ESP_OK, "a command frame is accepted");

    Communication& communication = Communication::getInstance();
    communication.task();
    communication.task();

    bool Found = false;
    for(const auto& Line : Sent) { if(Line.find("B=200") != std::string::npos) { Found = true; } }
    check(Found, "the answer came back over the socket");
    for(const auto& Line : Sent) { printf("   sent: \"%s\"\n", Line.c_str()); }

    /* An answer carrying a Latin-1 byte, which is what the overlay text command answers with
       once somebody sets a text with an umlaut in it - the thing the font tables carry
       umlauts for. A web socket text frame has to be valid UTF-8, so a raw 0xFC here does
       not draw one character wrong, it makes the browser close the connection.

       The strings are split around the escapes on purpose: "\xFCck" would swallow the c as a
       further hex digit and stop compiling. */
    Sent.clear();
    WebInterface::getInstance().broadcastLine("T=Gl\xFC" "ck");
    check(!Sent.empty(), "an answer with an umlaut is sent at all");
    check(!Sent.empty() && Sent.back() == "T=Gl\xC3\xBC" "ck",
          "a Latin-1 byte in an answer leaves as UTF-8");
    check(!Sent.empty() && Sent.back().find('\xFC') == std::string::npos,
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
    SentBinary.clear();
    SentBinaryTo.clear();

    runInterval(3u);
    check(SentBinary.empty(), "a display that never changed sends no frame");

    Pixels::getInstance().setPixel(0u, 10u, 20u, 30u);
    runInterval(3u);
    check(SentBinary.size() == 1u, "a change is sent once and not repeated");

    if(!SentBinary.empty()) {
        const unsigned char* frame = reinterpret_cast<const unsigned char*>(SentBinary[0].data());

        check(SentBinary[0].size() == 110u * 3u, "the frame is 110 * 3 bytes");
        check((frame[0] == 20u) && (frame[1] == 10u) && (frame[2] == 30u),
              "the frame carries green, red, blue like the strip");
        printf("   frame: %zu bytes, one per %u ticks\n", SentBinary[0].size(), interval);
    }

    SentBinary.clear();
    Pixels::getInstance().disablePixels();
    runInterval(1u);
    const bool BrowserDark = !SentBinary.empty() &&
        std::all_of(SentBinary[0].begin(), SentBinary[0].end(), [](char Value) { return Value == 0; });
    check(BrowserDark, "disabling the display also blanks the browser frame");

    SentBinary.clear();
    Pixels::getInstance().enablePixels();
    runInterval(1u);
    check(!SentBinary.empty() &&
          static_cast<unsigned char>(SentBinary[0][0]) == 20u &&
          static_cast<unsigned char>(SentBinary[0][1]) == 10u &&
          static_cast<unsigned char>(SentBinary[0][2]) == 30u,
          "enabling the display restores the browser frame");

    /* A client arriving between two changes still has to be shown the panel - and every
       client that is already watching gets the same frame, which is why this counts by
       descriptor rather than by frames. */
    SentBinary.clear();
    SentBinaryTo.clear();
    web.onClientOpened(9);
    runInterval(1u);
    check(framesSentTo(9) == 1u, "a client that arrives is shown the standing display");
    check(framesSentTo(7) == 1u, "and the one already watching gets it too");

    /* A descriptor the system handed out again is a new client, not the old one, so it also
       has to be shown the panel - the early return on the duplicate used to skip that. */
    SentBinary.clear();
    SentBinaryTo.clear();
    web.onClientOpened(9);
    runInterval(1u);
    check(framesSentTo(9) == 1u, "a reused descriptor counts as a client arriving");

    /* Changed on every tick, and still one frame per interval rather than one per tick. */
    SentBinary.clear();
    SentBinaryTo.clear();
    for(unsigned tick = 0u; tick < (4u * interval); tick++) {
        Pixels::getInstance().setPixel(1u, static_cast<byte>(tick + 1u), 0u, 0u);
        web.broadcastFrame();
    }
    check(framesSentTo(9) == 4u, "a display changing every tick still sends one per interval");

    /* an oversized frame must be refused whole rather than truncated into the parser */
    PendingFrame.assign(WEB_FRONTEND_MAX_FRAME_LENGTH + 10u, 'x');
    check(SocketHandler(&request) != ESP_OK, "an oversized frame is refused");

    /* ---- the firmware update ----------------------------------------------------------
       What a host can say about this is the sequence: begun with the size the request
       announced, written what arrived, ended only when all of it did, and the reboot asked
       for afterwards rather than taken inside the handler. Whether the bytes land in the
       other partition is the framework's business and no test here can speak to it. */
    auto upload = [](const std::string& Image, size_t Deliver) {
        Update.forget();
        Body = Image;
        BodyOffset = 0u;
        BodyLimit = Deliver;
        SentStatus.clear();

        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = Image.size();
        UpdateHandler(&post);
    };

    /* An image of no particular content: what is being checked is the byte count, and the
       header the real Update would insist on is the framework's check rather than this. */
    const std::string Image(4096u, '\xe9');

    upload(Image, Image.size());
    check(SentStatus == "200 OK", "a complete image is accepted");
    check(RootBody == "{\"ok\":true}", "and answered so the page can say it landed");
    check(Update.Announced == Image.size(), "the partition is claimed for the size the request announced");
    check(Update.Written == Image.size(), "every byte that arrived was written");
    check(Update.Ended && !Update.Aborted, "and the image was finished rather than abandoned");

    /* The reboot is the point of the deferral: asked for here, carried out by the tick after
       the answer has gone out. A handler that restarted on the spot would send nothing. */
    check(!ESP.Restarted, "the handler does not restart the controller itself");
    System::getInstance().performPendingRestart();
    check(ESP.Restarted, "the tick after the answer does");

    /* Half an image, which is what a connection that goes away mid-upload leaves. The clock
       has to keep running what it was running, so nothing may be finished and the partition
       has to be given back. */
    upload(Image, Image.size() / 2u);
    check(SentStatus == "400 Bad Request", "an upload that stops halfway is refused");
    check(!Update.Ended, "nothing is finished on a partial image");
    check(Update.Aborted, "and the partition is released rather than left claimed");
    check(RootBody.find("stopped before") != std::string::npos,
          "with a reason that says what happened rather than that something did");

    /* Refused before a byte is written, which is what an image too large for the partition
       does - the factory file instead of the OTA one, most likely. The reason is the
       framework's own text, because it is the one that names which limit was hit. */
    Update.forget();
    Update.RefuseBegin = true;
    Update.Error = "Not Enough Space";
    Body = Image;
    BodyOffset = 0u;
    BodyLimit = Image.size();
    SentStatus.clear();
    {
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = Image.size();
        UpdateHandler(&post);
    }
    check(SentStatus == "400 Bad Request", "an image the partition cannot hold is refused");
    check(Update.Written == 0u, "before anything is written");
    check(RootBody.find("Not Enough Space") != std::string::npos, "and the framework's reason is passed on");

    /* No body at all, which is what an empty file picker sends. Refused without claiming the
       partition, or a mis-click would take the running firmware's spare with it. */
    Update.forget();
    SentStatus.clear();
    {
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = 0u;
        UpdateHandler(&post);
    }
    check(SentStatus == "400 Bad Request", "a request with no image is refused");
    check(!Update.Begun, "and does not claim the partition to find that out");


    /* ---- the console password --------------------------------------------------------
       What this can say on a host is the gate: who is refused, who is let through, and that
       every route goes through the same door. What it cannot say is whether base64 over plain
       http is worth anything - it is not encryption, and the backend's notes say so. */
    auto fetchRoot = [](const char* Header) {
        Authorization = (Header == nullptr) ? "" : Header;
        SentStatus.clear();
        RootBody.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        RootHandler(&get);
    };
    /* The console goes through the same door, and it is worth asking separately: both pages
       come out of one sendPage(), and a gate that had stayed in the old handler would now be
       in one of the two callers and not in the other. */
    auto fetchConsole = [](const char* Header) {
        Authorization = (Header == nullptr) ? "" : Header;
        SentStatus.clear();
        RootBody.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        ConsoleHandler(&get);
    };

    /* Nothing stored: the state a clock ships in, and the one an update must not change. */
    check(!System::getInstance().isConsoleProtected(), "a clock with no password is not protected");
    fetchRoot(nullptr);
    check(SentStatus.empty(), "and answers a request that carries no credential");

    fetchConsole(nullptr);
    check(SentStatus.empty(), "and so does the console");

    check(System::getInstance().setConsolePassword("hunter2") == E_OK, "a password can be stored");
    check(System::getInstance().isConsoleProtected(), "which is what protected means");

    fetchRoot(nullptr);
    check(SentStatus == "401 Unauthorized", "a request without a credential is refused");
    check(SentHeaders["WWW-Authenticate"].find("Basic") != std::string::npos,
          "with the scheme a browser needs to prompt");
    check(SentHeaders["WWW-Authenticate"].find("wordclock") != std::string::npos,
          "and the user name, which is the one field a browser cannot guess");

    /* base64("wordclock:hunter2"), which is what a browser sends. */
    fetchRoot("Basic d29yZGNsb2NrOmh1bnRlcjI=");
    check(SentStatus.empty(), "the right credential is let through");
    check(RootBody.size() > 100u, "and gets the page");

    fetchRoot("Basic d29yZGNsb2NrOmh1bnRlcjM=");
    check(SentStatus == "401 Unauthorized", "a wrong password is refused");

    /* A prefix of the right credential, which is the one comparison that must not be lenient:
       a length-blind strncmp would accept this. */
    fetchRoot("Basic d29yZGNsb2NrOmh1bnRlcj");
    check(SentStatus == "401 Unauthorized", "and so is a prefix of the right one");

    fetchRoot("d29yZGNsb2NrOmh1bnRlcjI=");
    check(SentStatus == "401 Unauthorized", "a credential without the scheme is refused");

    /* Every route, not just the page: a console that asked for a password and then took
       commands on an unchecked socket would be a lock on the wrong door. */
    Authorization.clear();
    for(const auto& Route : {std::make_pair("commands", CommandsHandler), std::make_pair("display", DisplayHandler),
                             std::make_pair("manifest", ManifestHandler), std::make_pair("icon", Icon192Handler),
                             std::make_pair("socket handshake", SocketHandler)}) {
        SentStatus.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        Route.second(&get);
        check(SentStatus == "401 Unauthorized", Route.first);
    }
    {
        SentStatus.clear();
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = 4096u;
        UpdateHandler(&post);
        check(SentStatus == "401 Unauthorized", "update");
        check(!Update.Begun, "and an unauthorised update never claims the partition");
    }

    /* And the way back for a clock nobody can log into any more. */
    check(System::getInstance().setConsolePassword("") == E_OK, "the password can be cleared");
    check(!System::getInstance().isConsoleProtected(), "which unprotects the console");
    fetchRoot(nullptr);
    check(SentStatus.empty(), "and answers everybody again");

    return report();
}
