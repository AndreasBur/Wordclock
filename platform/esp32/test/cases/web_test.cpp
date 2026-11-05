// Drives the registered handlers the way the server reaches them - through the
// registration call - so WebInterface's own wiring is covered rather than bypassed.
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "System.h"
#include "Update.h"
#include "WebInterface.h"
#include "WordclockSerial.h"
#include <WiFi.h>
#include "check.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/*
 * A registered route as the server keeps it: the handler and the context handed back with
 * it. The context matters since the five files served unchanged share one handler and tell
 * each other apart by it - so a route is called through here rather than as a bare function
 * pointer, or every asset would answer as whichever was registered last.
 */
struct RouteType {
    httpd_handler_t handler = nullptr;
    void* context = nullptr;

    esp_err_t operator()(httpd_req_t* request) const {
        request->user_ctx = context;
        return handler(request);
    }

    // So a case can still ask whether a route was registered at all, which is what the
    // nullptr comparisons below are for.
    bool operator!=(std::nullptr_t) const { return handler != nullptr; }
    bool operator==(std::nullptr_t) const { return handler == nullptr; }
};

static RouteType socket_handler;
static RouteType root_handler;
static RouteType commands_handler;
static RouteType display_handler;
static RouteType manifest_handler;
static RouteType icon192_handler;
static RouteType icon512_handler;
static RouteType update_handler;
static RouteType console_handler;
static std::string sent_type;
static std::string sent_status;
/* Every header a handler set, by name - the gate's WWW-Authenticate is read out of here. */
static std::map<std::string, std::string> sent_headers;
// The body the next upload is to be fed, and how much of it the stub is willing to hand
// over - short of the announced length is a connection that went away mid-upload.
static std::string body;
static size_t body_offset = 0u;
static size_t body_limit = 0u;
static std::string pending_frame;
static std::vector<std::string> sent;
static std::string root_body;
static std::string root_encoding;
static std::string chunked;

esp_err_t httpd_start(httpd_handle_t* h, const httpd_config_t*) { *h = (httpd_handle_t)1; return ESP_OK; }

esp_err_t httpd_register_uri_handler(httpd_handle_t, const httpd_uri_t* u)
{
    const RouteType route{u->handler, u->user_ctx};

    if(strcmp(u->uri, "/ws") == 0)            { socket_handler = route; }
    else if(strcmp(u->uri, "/commands") == 0) { commands_handler = route; }
    else if(strcmp(u->uri, "/display") == 0)  { display_handler = route; }
    else if(strcmp(u->uri, "/manifest.webmanifest") == 0) { manifest_handler = route; }
    else if(strcmp(u->uri, "/icon-192.png") == 0) { icon192_handler = route; }
    else if(strcmp(u->uri, "/icon-512.png") == 0) { icon512_handler = route; }
    else if(strcmp(u->uri, "/update") == 0)   { update_handler = route; }
    else if(strcmp(u->uri, "/console") == 0)  { console_handler = route; }
    else                                      { root_handler = route; }
    return ESP_OK;
}

esp_err_t httpd_resp_set_type(httpd_req_t*, const char* t) { sent_type = t; return ESP_OK; }

esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char* k, const char* v)
{
    if(strcmp(k, "Content-Encoding") == 0) { root_encoding = v; }
    sent_headers[k] = v;
    return ESP_OK;
}

esp_err_t httpd_resp_set_status(httpd_req_t*, const char* s) { sent_status = s; return ESP_OK; }

/* The Authorization header the next request is to carry, empty for a request without one. */
static std::string authorization;

esp_err_t httpd_req_get_hdr_value_str(httpd_req_t*, const char* field, char* target, size_t capacity)
{
    if((strcmp(field, "Authorization") != 0) || authorization.empty()) { return ESP_FAIL; }
    if(authorization.size() >= capacity) { return ESP_FAIL; }

    memcpy(target, authorization.c_str(), authorization.size() + 1u);
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t*, const char* b, ssize_t n)
{
    /*
     * The update route answers with the framework's "until the terminator" sentinel, the
     * others with a length - and a body of bytes may hold a zero, so the two cannot be
     * collapsed into strlen().
     */
    root_body.assign(b, (n == HTTPD_RESP_USE_STRLEN) ? strlen(b) : static_cast<size_t>(n));
    return ESP_OK;
}

// Hands over the prepared body a chunk at a time, and stops at body_limit rather than at its
// end - which is how a dropped connection is reached without a socket to drop.
int httpd_req_recv(httpd_req_t*, char* target, size_t max)
{
    if(body_offset >= body_limit) { return 0; }

    const size_t take = std::min(max, body_limit - body_offset);
    memcpy(target, body.data() + body_offset, take);
    body_offset += take;

    return static_cast<int>(take);
}

esp_err_t httpd_resp_send_chunk(httpd_req_t*, const char* b, ssize_t n)
{
    if(b != nullptr) { chunked.append(b, static_cast<size_t>(n)); }
    return ESP_OK;
}

int httpd_req_to_sockfd(httpd_req_t*) { return 7; }

esp_err_t httpd_ws_recv_frame(httpd_req_t*, httpd_ws_frame_t* f, size_t max)
{
    if(pending_frame.size() > max) { return -1; }
    memcpy(f->payload, pending_frame.data(), pending_frame.size());
    f->len = pending_frame.size();
    f->type = HTTPD_WS_TYPE_TEXT;
    return ESP_OK;
}

static std::vector<std::string> sent_binary;
// The descriptor each frame went to, so a test can ask who was served rather than only how
// many frames left - the answer differs once more than one client is registered.
static std::vector<int> sent_binary_to;

esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int descriptor, httpd_ws_frame_t* f)
{
    std::string payload(reinterpret_cast<const char*>(f->payload), f->len);

    if(f->type == HTTPD_WS_TYPE_BINARY) { sent_binary.push_back(payload); sent_binary_to.push_back(descriptor); }
    else                                { sent.push_back(payload); }
    return ESP_OK;
}

static size_t FramesSentTo(int descriptor)
{
    size_t count = 0u;
    for(const int to : sent_binary_to) { if(to == descriptor) { count++; } }
    return count;
}

/*
 * Enough of a parse to say the document is well formed: brackets balanced and no string
 * left open. A browser would refuse the whole thing otherwise, and that failure looks
 * like a broken server rather than a broken label.
 */
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

int main()
{
    Check(WebInterface::GetInstance().Begin() == kEOk, "the server starts and registers its handlers");
    Check((socket_handler != nullptr) && (root_handler != nullptr) && (commands_handler != nullptr)
          && (display_handler != nullptr), "all four handlers were registered");
    // Separately, because the server's own handler limit is what would drop these: it is a
    // number in the configuration, and one route past it is refused rather than reported.
    Check((manifest_handler != nullptr) && (icon192_handler != nullptr) && (icon512_handler != nullptr)
          && (update_handler != nullptr),
          "and the four added since fit under the handler limit as well");
    /*
     * The ninth, which is the one that pushed the limit past its default: "/" is the panel and
     * the console moved here, so a missing registration would answer 404 on the page that
     * everything the panel does not cover goes through.
     */
    Check(console_handler != nullptr, "and the console's own route fits too");

    httpd_req_t request{};
    request.method = HTTP_GET;

    /*
     * the two pages: both gzipped, both announced as such, and not the same bytes - "/" is
     * the panel and "/console" the generated console. Told apart by their titles, because
     * that is what a browser would show and what a swapped pair of pointers would get
     * wrong while every other check here still passed.
     */
    console_handler(&request);
    const std::string console = root_body;
    Check(root_encoding == "gzip", "the console is announced as gzip");
    Check(console.size() > 100u, "and is a page");

    root_handler(&request);
    Check(root_encoding == "gzip", "the page is announced as gzip");
    Check((root_body.size() > 100u) && (static_cast<unsigned char>(root_body[0]) == 0x1fu)
                                   && (static_cast<unsigned char>(root_body[1]) == 0x8bu),
          "the page really is a gzip stream");
    Check(root_body != console, "and the two routes are two different pages");

    /* the manifest: a browser reads it, so the type is part of the answer and not decoration */
    root_encoding.clear();
    manifest_handler(&request);
    Check(sent_type == "application/manifest+json", "the manifest is announced as a manifest");
    Check(root_encoding.empty(), "and sent as it is, not gzipped");
    const std::string manifest = root_body;
    Check(IsBalancedJson(manifest), "the manifest is well formed JSON");
    Check(manifest.find("\"display\"") != std::string::npos
          && manifest.find("standalone") != std::string::npos, "it asks to start without an address bar");
    Check(manifest.find("icon-192.png") != std::string::npos
          && manifest.find("icon-512.png") != std::string::npos, "it names both icons");
    // Relative, because the clock answers on whatever address it was given: an absolute
    // start_url baked in here would send an installed console to a different clock.
    Check(manifest.find("\"start_url\": \".\"") != std::string::npos, "and starts at a relative URL");

    /* the icons: sent as they are, which is what a PNG wants */
    root_encoding.clear();
    icon192_handler(&request);
    Check(sent_type == "image/png", "the small icon is announced as a PNG");
    Check(root_encoding.empty(), "and not as gzip, a PNG being deflated already");
    Check(root_body.size() > 500u && root_body.compare(1u, 3u, "PNG") == 0, "and really is one");

    icon512_handler(&request);
    Check(root_body.size() > 5000u && root_body.compare(1u, 3u, "PNG") == 0,
          "the large icon is a PNG too, and the larger of the two");

    /* the catalog, generated from the real table */
    commands_handler(&request);
    Check(!chunked.empty() && (chunked.front() == '[') && (chunked.back() == ']'),
          "the catalog is a JSON array");
    Check(IsBalancedJson(chunked), "its brackets and strings are balanced");
    Check(chunked.find("\"number\":3") != std::string::npos, "it carries the display brightness command");
    Check(chunked.find("\"short\":\"B\"") != std::string::npos, "it carries an option short name");
    Check(chunked.find("\"max\":255") != std::string::npos, "it carries the ranges");
    Check(chunked.find("\"values\":[") != std::string::npos, "it carries named values for the enumerations");
    // Without this the page would offer the status fields as inputs, and a value typed
    // into one of them would come back as "parameter unknown".
    Check(chunked.find("\"readonly\":true") != std::string::npos, "it marks the fields that only appear in answers");
    size_t commands = 0u;
    for(size_t at = chunked.find("\"number\":"); at != std::string::npos; at = chunked.find("\"number\":", at + 1u)) { commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", chunked.size(), commands);

    /* the panel description: shape and letters, the letters as UTF-8 */
    chunked.clear();
    display_handler(&request);
    Check(IsBalancedJson(chunked), "the display description is well formed");
    Check(chunked.find("\"columns\":11") != std::string::npos, "it carries the column count");
    Check(chunked.find("\"rows\":10") != std::string::npos, "it carries the row count");
    Check(chunked.find("ESKISTLF") != std::string::npos, "it carries the letters");
    // U with an umlaut is 0xDC in the firmware's table and has to arrive as two UTF-8 bytes;
    // the raw byte would make the whole document invalid.
    Check(chunked.find("\xc3\x9c") != std::string::npos, "the umlauts arrive as UTF-8");
    Check(chunked.find("\xdc") == std::string::npos, "no raw Latin-1 byte escapes into the JSON");

    /* the handshake registers the client, so answers have somewhere to go */
    socket_handler(&request);

    /* a frame without a terminator - the bridge has to add it */
    sent.clear();
    pending_frame = "3 -B200";
    request.method = HTTP_POST;
    Check(socket_handler(&request) == ESP_OK, "a command frame is accepted");

    Communication& communication = Communication::GetInstance();
    communication.Task();
    communication.Task();

    bool found = false;
    for(const auto& line : sent) { if(line.find("B=200") != std::string::npos) { found = true; } }
    Check(found, "the answer came back over the socket");
    for(const auto& line : sent) { printf("   sent: \"%s\"\n", line.c_str()); }

    /*
     * An answer carrying a Latin-1 byte, which is what the overlay text command answers with
     * once somebody sets a text with an umlaut in it - the thing the font tables carry
     * umlauts for. A web socket text frame has to be valid UTF-8, so a raw 0xFC here does
     * not draw one character wrong, it makes the browser close the connection.
     *
     * The strings are split around the escapes on purpose: "\xFCck" would swallow the c as a
     * further hex digit and stop compiling.
     */
    sent.clear();
    WebInterface::GetInstance().BroadcastLine("T=Gl\xFC" "ck");
    Check(!sent.empty(), "an answer with an umlaut is sent at all");
    Check(!sent.empty() && sent.back() == "T=Gl\xC3\xBC" "ck",
          "a Latin-1 byte in an answer leaves as UTF-8");
    Check(!sent.empty() && sent.back().find('\xFC') == std::string::npos,
          "and no raw Latin-1 byte survives into the frame");

    /* the display frame */
    WebInterface& web = WebInterface::GetInstance();
    constexpr unsigned kInterval{WEB_FRONTEND_FRAME_INTERVAL_TICKS};
    auto run_interval = [&web](unsigned count) {
        for(unsigned tick = 0u; tick < (count * kInterval); tick++) { web.BroadcastFrame(); }
    };

    // The handshake above counted as a client arriving, so one frame is already owed. Drain
    // it before measuring anything, or every count below is one too high.
    run_interval(1u);
    sent_binary.clear();
    sent_binary_to.clear();

    run_interval(3u);
    Check(sent_binary.empty(), "a display that never changed sends no frame");

    Pixels::GetInstance().SetPixel(0u, 10u, 20u, 30u);
    run_interval(3u);
    Check(sent_binary.size() == 1u, "a change is sent once and not repeated");

    if(!sent_binary.empty()) {
        const unsigned char* frame = reinterpret_cast<const unsigned char*>(sent_binary[0].data());

        Check(sent_binary[0].size() == 110u * 3u, "the frame is 110 * 3 bytes");
        Check((frame[0] == 20u) && (frame[1] == 10u) && (frame[2] == 30u),
              "the frame carries green, red, blue like the strip");
        printf("   frame: %zu bytes, one per %u ticks\n", sent_binary[0].size(), kInterval);
    }

    sent_binary.clear();
    Pixels::GetInstance().DisablePixels();
    run_interval(1u);
    const bool browser_dark = !sent_binary.empty() &&
        std::all_of(sent_binary[0].begin(), sent_binary[0].end(), [](char value) { return value == 0; });
    Check(browser_dark, "disabling the display also blanks the browser frame");

    sent_binary.clear();
    Pixels::GetInstance().EnablePixels();
    run_interval(1u);
    Check(!sent_binary.empty() &&
          static_cast<unsigned char>(sent_binary[0][0]) == 20u &&
          static_cast<unsigned char>(sent_binary[0][1]) == 10u &&
          static_cast<unsigned char>(sent_binary[0][2]) == 30u,
          "enabling the display restores the browser frame");

    /*
     * A client arriving between two changes still has to be shown the panel - and every
     * client that is already watching gets the same frame, which is why this counts by
     * descriptor rather than by frames.
     */
    sent_binary.clear();
    sent_binary_to.clear();
    web.OnClientOpened(9);
    run_interval(1u);
    Check(FramesSentTo(9) == 1u, "a client that arrives is shown the standing display");
    Check(FramesSentTo(7) == 1u, "and the one already watching gets it too");

    // A descriptor the system handed out again is a new client, not the old one, so it also
    // has to be shown the panel - the early return on the duplicate used to skip that.
    sent_binary.clear();
    sent_binary_to.clear();
    web.OnClientOpened(9);
    run_interval(1u);
    Check(FramesSentTo(9) == 1u, "a reused descriptor counts as a client arriving");

    /* Changed on every tick, and still one frame per interval rather than one per tick. */
    sent_binary.clear();
    sent_binary_to.clear();
    for(unsigned tick = 0u; tick < (4u * kInterval); tick++) {
        Pixels::GetInstance().SetPixel(1u, static_cast<byte>(tick + 1u), 0u, 0u);
        web.BroadcastFrame();
    }
    Check(FramesSentTo(9) == 4u, "a display changing every tick still sends one per interval");

    /* an oversized frame must be refused whole rather than truncated into the parser */
    pending_frame.assign(WEB_FRONTEND_MAX_FRAME_LENGTH + 10u, 'x');
    Check(socket_handler(&request) != ESP_OK, "an oversized frame is refused");

    /*
     * ---- the firmware update ----------------------------------------------------------
     * What a host can say about this is the sequence: begun with the size the request
     * announced, written what arrived, ended only when all of it did, and the reboot asked
     * for afterwards rather than taken inside the handler. Whether the bytes land in the
     * other partition is the framework's business and no test here can speak to it.
     */
    auto upload = [](const std::string& image, size_t deliver) {
        Update.forget();
        body = image;
        body_offset = 0u;
        body_limit = deliver;
        sent_status.clear();

        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = image.size();
        update_handler(&post);
    };

    // An image of no particular content: what is being checked is the byte count, and the
    // header the real Update would insist on is the framework's check rather than this.
    const std::string image(4096u, '\xe9');

    upload(image, image.size());
    Check(sent_status == "200 OK", "a complete image is accepted");
    Check(root_body == "{\"ok\":true}", "and answered so the page can say it landed");
    Check(Update.Announced == image.size(), "the partition is claimed for the size the request announced");
    Check(Update.Written == image.size(), "every byte that arrived was written");
    Check(Update.Ended && !Update.Aborted, "and the image was finished rather than abandoned");

    // The reboot is the point of the deferral: asked for here, carried out by the tick after
    // the answer has gone out. A handler that restarted on the spot would send nothing.
    Check(!ESP.Restarted, "the handler does not restart the controller itself");
    System::GetInstance().PerformPendingRestart();
    Check(ESP.Restarted, "the tick after the answer does");

    /*
     * Half an image, which is what a connection that goes away mid-upload leaves. The clock
     * has to keep running what it was running, so nothing may be finished and the partition
     * has to be given back.
     */
    upload(image, image.size() / 2u);
    Check(sent_status == "400 Bad Request", "an upload that stops halfway is refused");
    Check(!Update.Ended, "nothing is finished on a partial image");
    Check(Update.Aborted, "and the partition is released rather than left claimed");
    Check(root_body.find("stopped before") != std::string::npos,
          "with a reason that says what happened rather than that something did");

    /*
     * Refused before a byte is written, which is what an image too large for the partition
     * does - the factory file instead of the OTA one, most likely. The reason is the
     * framework's own text, because it is the one that names which limit was hit.
     */
    Update.forget();
    Update.RefuseBegin = true;
    Update.Error = "Not Enough Space";
    body = image;
    body_offset = 0u;
    body_limit = image.size();
    sent_status.clear();
    {
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = image.size();
        update_handler(&post);
    }
    Check(sent_status == "400 Bad Request", "an image the partition cannot hold is refused");
    Check(Update.Written == 0u, "before anything is written");
    Check(root_body.find("Not Enough Space") != std::string::npos, "and the framework's reason is passed on");

    // No body at all, which is what an empty file picker sends. Refused without claiming the
    // partition, or a mis-click would take the running firmware's spare with it.
    Update.forget();
    sent_status.clear();
    {
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = 0u;
        update_handler(&post);
    }
    Check(sent_status == "400 Bad Request", "a request with no image is refused");
    Check(!Update.Begun, "and does not claim the partition to find that out");

    /*
     * ---- the console password --------------------------------------------------------
     * What this can say on a host is the gate: who is refused, who is let through, and that
     * every route goes through the same door. What it cannot say is whether base64 over plain
     * http is worth anything - it is not encryption, and the backend's notes say so.
     */
    auto fetch_root = [](const char* header) {
        authorization = (header == nullptr) ? "" : header;
        sent_status.clear();
        root_body.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        root_handler(&get);
    };
    /*
     * The console goes through the same door, and it is worth asking separately: both pages
     * come out of one sendPage(), and a gate that had stayed in the old handler would now be
     * in one of the two callers and not in the other.
     */
    auto fetch_console = [](const char* header) {
        authorization = (header == nullptr) ? "" : header;
        sent_status.clear();
        root_body.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        console_handler(&get);
    };

    /* Nothing stored: the state a clock ships in, and the one an update must not change. */
    Check(!System::GetInstance().IsConsoleProtected(), "a clock with no password is not protected");
    fetch_root(nullptr);
    Check(sent_status.empty(), "and answers a request that carries no credential");

    fetch_console(nullptr);
    Check(sent_status.empty(), "and so does the console");

    Check(System::GetInstance().SetConsolePassword("hunter2") == kEOk, "a password can be stored");
    Check(System::GetInstance().IsConsoleProtected(), "which is what protected means");

    fetch_root(nullptr);
    Check(sent_status == "401 Unauthorized", "a request without a credential is refused");
    Check(sent_headers["WWW-Authenticate"].find("Basic") != std::string::npos,
          "with the scheme a browser needs to prompt");
    Check(sent_headers["WWW-Authenticate"].find("wordclock") != std::string::npos,
          "and the user name, which is the one field a browser cannot guess");

    /* base64("wordclock:hunter2"), which is what a browser sends. */
    fetch_root("Basic d29yZGNsb2NrOmh1bnRlcjI=");
    Check(sent_status.empty(), "the right credential is let through");
    Check(root_body.size() > 100u, "and gets the page");

    fetch_root("Basic d29yZGNsb2NrOmh1bnRlcjM=");
    Check(sent_status == "401 Unauthorized", "a wrong password is refused");

    // A prefix of the right credential, which is the one comparison that must not be lenient:
    // a length-blind strncmp would accept this.
    fetch_root("Basic d29yZGNsb2NrOmh1bnRlcj");
    Check(sent_status == "401 Unauthorized", "and so is a prefix of the right one");

    fetch_root("d29yZGNsb2NrOmh1bnRlcjI=");
    Check(sent_status == "401 Unauthorized", "a credential without the scheme is refused");

    // Every route, not just the page: a console that asked for a password and then took
    // commands on an unchecked socket would be a lock on the wrong door.
    authorization.clear();
    for(const auto& route : {std::make_pair("commands", commands_handler), std::make_pair("display", display_handler),
                             std::make_pair("manifest", manifest_handler), std::make_pair("icon", icon192_handler),
                             std::make_pair("socket handshake", socket_handler)}) {
        sent_status.clear();
        httpd_req_t get{};
        get.method = HTTP_GET;
        route.second(&get);
        Check(sent_status == "401 Unauthorized", route.first);
    }
    {
        sent_status.clear();
        httpd_req_t post{};
        post.method = HTTP_POST;
        post.content_len = 4096u;
        update_handler(&post);
        Check(sent_status == "401 Unauthorized", "update");
        Check(!Update.Begun, "and an unauthorised update never claims the partition");
    }

    /* And the way back for a clock nobody can log into any more. */
    Check(System::GetInstance().SetConsolePassword("") == kEOk, "the password can be cleared");
    Check(!System::GetInstance().IsConsoleProtected(), "which unprotects the console");
    fetch_root(nullptr);
    Check(sent_status.empty(), "and answers everybody again");

    /*
     * The network, against a store that answers as the framework's: getString() counts the
     * terminator, so an empty value reads as stored. An empty name stored as such made 13 -S -P
     * join a network with no name; RPC 33 asked for a compiled-in network only.
     */
    {
        System& system = System::GetInstance();
        const int reconnects = WiFi.Reconnects;

        Check(system.SetNetworkCredentials("Home", "homepass") == kEOk && WiFi.Mode == WIFI_STA &&
              WiFi.Joined == "Home" && WiFi.JoinedWith == "homepass", "a stored network is joined");
        Check(system.ReconnectNetwork() == kEOk && WiFi.Reconnects == reconnects + 1,
              "and RPC 33 reconnects to a network entered with command 13");
        Check(system.SetNetworkCredentials("OpenNet", "") == kEOk && WiFi.Joined == "OpenNet" &&
              WiFi.JoinedWith.empty(), "an empty pass phrase joins an open network");

        WiFi.AccessPoint.clear();
        Check(system.SetNetworkCredentials("", "") == kEOk && !system.IsNetworkConfigured() &&
              WiFi.Mode == WIFI_AP && WiFi.AccessPoint == SYSTEM_ACCESS_POINT_SSID,
              "an empty name forgets the network and opens the access point");
        Check(system.ReconnectNetwork() == kENotOk, "after which RPC 33 has nothing to join");
    }

    /*
     * Two tasks printing at once, as the firmware's and the web server's do. Every line that
     * reaches the sink is one task's line, whole: without the lock the two ran together.
     */
    {
        static std::vector<std::string> sunk;
        static std::mutex sunk_mutex;
        WordclockSerial& port = WordclockSerial::GetInstance();
        constexpr int kLines{200};

        port.SetLineSink([](const char* line) {
            const std::lock_guard<std::mutex> lock(sunk_mutex);
            sunk.emplace_back(line);
        });
        const std::string as(40u, 'a');
        const std::string bs(40u, 'b');
        std::thread first([&] { for(int line = 0; line < kLines; line++) { port.println(as.c_str()); } });
        std::thread second([&] { for(int line = 0; line < kLines; line++) { port.println(bs.c_str()); } });
        first.join();
        second.join();
        port.SetLineSink(nullptr);

        const bool whole = std::all_of(sunk.begin(), sunk.end(), [&](const std::string& line) {
            return (line == as) || (line == bs);
        });
        Check((sunk.size() == 2u * kLines) && whole, "lines printed from two tasks at once stay whole");
    }

    return Report();
}
