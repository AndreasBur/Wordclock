/* The clock as a host process, so the web page can be tried without a board.
 *
 * The whole firmware core runs in here at the scheduler's real interval, bound to the same
 * WordclockSerial the ESP32 uses - so a command arriving from the browser takes exactly the
 * path it takes on the device, and the answers are the device's answers.
 *
 * Line protocol on stdio, because the HTTP and web socket side is node's job:
 *   in   "C"        -> the command catalog
 *   in   anything   -> a command, injected as typed characters
 *   out  "#<json>"  -> the catalog
 *   out  ">​<line>"  -> one answer line
 */
#include <esp_http_server.h>
#include <ESPmDNS.h>
#include "Arduino.h"
#include "Communication.h"
#include "Persistence.h"
#include "Scheduler.h"
#include "WebInterface.h"

#include <cstdio>
#include <cstring>
#include <poll.h>
#include <string>
#include <unistd.h>


/* The catalog is produced by the platform's own handler, so what the browser gets here is
   byte for byte what the device would send. */
static std::string Catalog;
static httpd_handler_t CommandsHandler = nullptr;

esp_err_t httpd_start(httpd_handle_t* h, const httpd_config_t*) { *h = (httpd_handle_t)1; return ESP_OK; }
esp_err_t httpd_register_uri_handler(httpd_handle_t, const httpd_uri_t* u)
{
    if(strcmp(u->uri, "/commands") == 0) { CommandsHandler = u->handler; }
    return ESP_OK;
}
esp_err_t httpd_resp_set_type(httpd_req_t*, const char*) { return ESP_OK; }
esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char*, const char*) { return ESP_OK; }
esp_err_t httpd_resp_send(httpd_req_t*, const char*, ssize_t) { return ESP_OK; }
esp_err_t httpd_resp_send_chunk(httpd_req_t*, const char* b, ssize_t n)
{
    if(b != nullptr) { Catalog.append(b, static_cast<size_t>(n)); }
    return ESP_OK;
}
int httpd_req_to_sockfd(httpd_req_t*) { return 1; }
esp_err_t httpd_ws_recv_frame(httpd_req_t*, httpd_ws_frame_t*, size_t) { return ESP_OK; }
esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int, httpd_ws_frame_t*) { return ESP_OK; }

/* Registered as WordclockSerial's line sink, so every finished answer line reaches node. */
static void emitLine(const char* Line)
{
    std::printf(">%s\n", Line);
    std::fflush(stdout);
}

int main()
{
    WordclockSerial& Port = WordclockSerial::getInstance();

    /* Through WebInterface, so the catalog comes out of the handler rather than a copy. */
    WebInterface::getInstance().begin();

    /* After begin(), which registers a sink of its own - that one broadcasts to sockets the
       device would have and this process does not. */
    Port.setLineSink(emitLine);

    Persistence::getInstance().load();

    Scheduler Tasks;
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

                if(Line == "C") {
                    Catalog.clear();
                    httpd_req_t Request{};
                    Request.method = HTTP_GET;
                    if(CommandsHandler != nullptr) { CommandsHandler(&Request); }
                    std::printf("#%s\n", Catalog.c_str());
                    std::fflush(stdout);
                    continue;
                }

                Line.push_back(Communication::getEndOfMessageChar());
                Port.inject(Line.data(), Line.size());
            }
        }

        Tasks.task();
    }
    return 0;
}
