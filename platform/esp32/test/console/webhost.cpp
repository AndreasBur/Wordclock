/* The clock as a host process, so the web page can be tried without a board.
 *
 * The whole firmware core runs in here at the scheduler's real interval, bound to the same
 * WordclockSerial the ESP32 uses - so a command arriving from the browser takes exactly the
 * path it takes on the device, and the answers are the device's answers.
 *
 * Line protocol on stdio, because the HTTP and web socket side is node's job:
 *   in   "C"        -> the command catalog
 *   in   "D"        -> the panel's shape and letters
 *   in   "R"        -> a client arrived; resend the display as it stands
 *   in   anything   -> a command, injected as typed characters
 *   out  "#<json>"  -> whichever description was asked for
 *   out  ">​<line>"  -> one answer line
 *   out  "F<hex>"   -> one display frame
 *
 * The answers and frames are not produced here: they come out of WebInterface's own
 * broadcast, whose send call is the stub below. That way the rate limiting and the
 * unchanged-frame suppression are exercised rather than reimplemented.
 */
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "Scheduler.h"
#include "WebInterface.h"
#include "WordclockMain.h"

#include <cstdio>
#include <cstring>
#include <poll.h>
#include <string>
#include <unistd.h>


/* The catalog is produced by the platform's own handler, so what the browser gets here is
   byte for byte what the device would send. */
static std::string Described;
static httpd_handler_t CommandsHandler = nullptr;
static httpd_handler_t DisplayHandler = nullptr;

esp_err_t httpd_start(httpd_handle_t* h, const httpd_config_t*) { *h = (httpd_handle_t)1; return ESP_OK; }
esp_err_t httpd_register_uri_handler(httpd_handle_t, const httpd_uri_t* u)
{
    if(strcmp(u->uri, "/commands") == 0)     { CommandsHandler = u->handler; }
    else if(strcmp(u->uri, "/display") == 0)  { DisplayHandler = u->handler; }
    return ESP_OK;
}
esp_err_t httpd_resp_set_type(httpd_req_t*, const char*) { return ESP_OK; }
esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char*, const char*) { return ESP_OK; }
esp_err_t httpd_resp_send(httpd_req_t*, const char*, ssize_t) { return ESP_OK; }
esp_err_t httpd_resp_send_chunk(httpd_req_t*, const char* b, ssize_t n)
{
    if(b != nullptr) { Described.append(b, static_cast<size_t>(n)); }
    return ESP_OK;
}
int httpd_req_to_sockfd(httpd_req_t*) { return 1; }
esp_err_t httpd_ws_recv_frame(httpd_req_t*, httpd_ws_frame_t*, size_t) { return ESP_OK; }
/* Where everything WebInterface broadcasts leaves this process. */
esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int, httpd_ws_frame_t* Frame)
{
    if(Frame->type == HTTPD_WS_TYPE_BINARY) {
        std::printf("F");
        for(size_t Index = 0u; Index < Frame->len; Index++) { std::printf("%02x", Frame->payload[Index]); }
        std::printf("\n");
    } else {
        std::printf(">%.*s\n", static_cast<int>(Frame->len), reinterpret_cast<const char*>(Frame->payload));
    }
    std::fflush(stdout);
    return ESP_OK;
}

static std::string describe(httpd_handler_t Handler)
{
    Described.clear();

    if(Handler != nullptr) {
        httpd_req_t Request{};
        Request.method = HTTP_GET;
        Handler(&Request);
    }
    return Described;
}

int main()
{
    WordclockSerial& Port = WordclockSerial::getInstance();
    WebInterface& Web = WebInterface::getInstance();
    WordclockMain Wordclock;

    /* The same backend entry points and order as setup()/loop() on the board. */
    Wordclock.init();
    Web.begin();

    /* One client, or the broadcast has nobody to gather a frame for. Node is that client. */
    Web.onClientOpened(1);

    std::string Pending;

    for(;;) {
        struct pollfd Poll{STDIN_FILENO, POLLIN, 0};

        if(poll(&Poll, 1, static_cast<int>(Scheduler::getTaskIntervalMs())) > 0) {
            char Chunk[512];
            const ssize_t Read = read(STDIN_FILENO, Chunk, sizeof(Chunk));

            if(Read <= 0) { break; }
            Pending.append(Chunk, static_cast<size_t>(Read));

            for(size_t End = Pending.find('\n'); End != std::string::npos; End = Pending.find('\n')) {
                std::string Line = Pending.substr(0u, End);
                Pending.erase(0u, End + 1u);

                if(Line == "R") {
                    /* What the device does when a socket is opened: the next frame goes out
                       even though nothing changed. */
                    Web.onClientOpened(1);
                    continue;
                }

                if((Line == "C") || (Line == "D")) {
                    std::printf("#%s\n", describe(Line == "C" ? CommandsHandler : DisplayHandler).c_str());
                    std::fflush(stdout);
                    continue;
                }

                Line.push_back(Communication::getEndOfMessageChar());
                Port.inject(Line.data(), Line.size());
            }
        }

        Wordclock.task();
        Pixels::getInstance().render();
        Web.broadcastFrame();
    }
    return 0;
}
