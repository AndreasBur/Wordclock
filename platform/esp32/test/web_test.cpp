/* Drives the registered handlers the way the server reaches them - through the
   registration call - so WebInterface's own wiring is covered rather than bypassed. */
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "WebInterface.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


static httpd_handler_t SocketHandler = nullptr;
static httpd_handler_t RootHandler = nullptr;
static httpd_handler_t CommandsHandler = nullptr;
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
    else                                     { RootHandler = u->handler; }
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

esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int, httpd_ws_frame_t* f)
{
    Sent.emplace_back(reinterpret_cast<const char*>(f->payload), f->len);
    return ESP_OK;
}

static int Failures = 0;

static void check(bool Ok, const char* What)
{
    printf("%-58s %s\n", What, Ok ? "ok" : "FAIL");
    if(!Ok) { Failures++; }
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
    check((SocketHandler != nullptr) && (RootHandler != nullptr) && (CommandsHandler != nullptr),
          "all three handlers were registered");

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
    size_t Commands = 0u;
    for(size_t At = Chunked.find("\"number\":"); At != std::string::npos; At = Chunked.find("\"number\":", At + 1u)) { Commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", Chunked.size(), Commands);

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

    /* an oversized frame must be refused whole rather than truncated into the parser */
    PendingFrame.assign(WEB_INTERFACE_MAX_FRAME_LENGTH + 10u, 'x');
    check(SocketHandler(&request) != ESP_OK, "an oversized frame is refused");

    printf("\n%s\n", Failures == 0 ? "all checks passed" : "FAILURES");
    return Failures == 0 ? 0 : 1;
}
