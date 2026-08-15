/* Drives the registered handlers the way the server reaches them - through the
   registration call - so WebInterface's own wiring is covered rather than bypassed. */
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "WebInterface.h"
#include "check.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


static httpd_handler_t SocketHandler = nullptr;
static httpd_handler_t RootHandler = nullptr;
static httpd_handler_t CommandsHandler = nullptr;
static httpd_handler_t DisplayHandler = nullptr;
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
    else                                      { RootHandler = u->handler; }
    return ESP_OK;
}

esp_err_t httpd_resp_set_type(httpd_req_t*, const char*) { return ESP_OK; }

esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char* k, const char* v)
{
    if(strcmp(k, "Content-Encoding") == 0) { RootEncoding = v; }
    return ESP_OK;
}

esp_err_t httpd_resp_send(httpd_req_t*, const char* b, ssize_t n)
{
    RootBody.assign(b, static_cast<size_t>(n));
    return ESP_OK;
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

    httpd_req_t request{};
    request.method = HTTP_GET;

    /* the page: gzipped, and announced as such */
    RootHandler(&request);
    check(RootEncoding == "gzip", "the page is announced as gzip");
    check((RootBody.size() > 100u) && (static_cast<unsigned char>(RootBody[0]) == 0x1fu)
                                   && (static_cast<unsigned char>(RootBody[1]) == 0x8bu),
          "the page really is a gzip stream");

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
    const unsigned interval = WEB_INTERFACE_FRAME_INTERVAL_TICKS;
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
    PendingFrame.assign(WEB_INTERFACE_MAX_FRAME_LENGTH + 10u, 'x');
    check(SocketHandler(&request) != ESP_OK, "an oversized frame is refused");
    return report();
}
