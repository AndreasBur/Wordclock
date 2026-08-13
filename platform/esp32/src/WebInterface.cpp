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
#include <ESPmDNS.h>
#include <esp_http_server.h>

#include "Arduino.h"

#include "Communication.h"
#include "WebInterface.h"
#include "WebPage.h"

#include <string.h>
#include <unistd.h>

/******************************************************************************************************************************************************
 *  LOCAL DATA
******************************************************************************************************************************************************/
namespace {

httpd_handle_t Server{nullptr};

/* Handed to WordclockSerial, which takes a plain function pointer - so the singleton is
   reached from here rather than carried along. */
void sendLineToClients(const char* Line)
{
    WebInterface::getInstance().broadcastLine(Line);
}

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
/******************************************************************************************************************************************************
  handleRoot()
******************************************************************************************************************************************************/
/*! \brief          Serves the console page straight out of flash
 *  \details        Sent compressed, which is how it is stored: the page was gzipped at
 *                  build time, so this only pushes the bytes and the browser unpacks them.
******************************************************************************************************************************************************/
esp_err_t handleRoot(httpd_req_t* Request)
{
    httpd_resp_set_type(Request, "text/html");
    httpd_resp_set_hdr(Request, "Content-Encoding", "gzip");

    return httpd_resp_send(Request, reinterpret_cast<const char*>(WebPageGzip), WebPageGzipSize);
}


/******************************************************************************************************************************************************
  handleSocket()
******************************************************************************************************************************************************/
/*! \brief          Takes one web socket frame and injects it as typed characters
 *  \details        A GET is the handshake, and returning E_OK from it is what completes the
 *                  upgrade - the frame handling below only ever sees later requests.
 *
 *                  The command is injected followed by the end-of-message character, unless
 *                  the browser already sent one. That keeps the page free of protocol
 *                  bookkeeping: it sends the command text, the terminator is this side's
 *                  business.
 *
 *  \return         ESP_OK unless the frame could not be read
******************************************************************************************************************************************************/
esp_err_t handleSocket(httpd_req_t* Request)
{
    if(Request->method == HTTP_GET) {
        WebInterface::getInstance().onClientOpened(httpd_req_to_sockfd(Request));
        return ESP_OK;
    }

    uint8_t Payload[WEB_INTERFACE_MAX_FRAME_LENGTH + 1u]{};
    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = Payload;

    /* Asking with the buffer's size rather than in two steps: a frame that does not fit is
       refused whole, so half a command can never reach the parser. */
    const esp_err_t Result = httpd_ws_recv_frame(Request, &Frame, WEB_INTERFACE_MAX_FRAME_LENGTH);
    if(Result != ESP_OK) { return Result; }

    if(Frame.type == HTTPD_WS_TYPE_CLOSE) {
        WebInterface::getInstance().onClientClosed(httpd_req_to_sockfd(Request));
        return ESP_OK;
    }
    if((Frame.type != HTTPD_WS_TYPE_TEXT) || (Frame.len == 0u)) { return ESP_OK; }

    WordclockSerial& Port = WordclockSerial::getInstance();
    Port.inject(reinterpret_cast<const char*>(Payload), Frame.len);

    if(Payload[Frame.len - 1u] != Communication::getEndOfMessageChar()) {
        const char Terminator = Communication::getEndOfMessageChar();
        Port.inject(&Terminator, 1u);
    }

    return ESP_OK;
}


/******************************************************************************************************************************************************
  onSocketClosed()
******************************************************************************************************************************************************/
/*! \brief          Drops a client the server has finished with
 *  \details        Registered as the server's close hook rather than guessed from a failing
 *                  send: a browser that is simply gone never sends a close frame, and its
 *                  descriptor would otherwise be handed to send() for the rest of the run.
******************************************************************************************************************************************************/
void onSocketClosed(httpd_handle_t, int Descriptor)
{
    WebInterface::getInstance().onClientClosed(Descriptor);
    close(Descriptor);
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
    httpd_config_t Config = HTTPD_DEFAULT_CONFIG();
    Config.server_port = WEB_INTERFACE_PORT;
    /* The sockets the server keeps have to cover the console clients plus the ones fetching
       the page, or a reload can push a console off. */
    Config.max_open_sockets = WEB_INTERFACE_MAX_CLIENTS + 2u;
    Config.close_fn = onSocketClosed;
    /* Its own task, so nothing here runs inside the firmware's tick. */
    Config.core_id = 0;

    if(httpd_start(&Server, &Config) != ESP_OK) {
        Server = nullptr;
        Serial.println(F("Web: server failed to start, console stays on the UART"));
        return E_NOT_OK;
    }

    static const httpd_uri_t RootUri{"/", HTTP_GET, handleRoot, nullptr, false, false, nullptr};
    static const httpd_uri_t SocketUri{"/ws", HTTP_GET, handleSocket, nullptr, true, false, nullptr};

    httpd_register_uri_handler(Server, &RootUri);
    httpd_register_uri_handler(Server, &SocketUri);

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
 *  \details        Runs in the firmware's task, which is allowed to send asynchronously into
 *                  the server. A send that fails is not retried and the client is not
 *                  dropped here - the server's close hook owns that, and dropping a client
 *                  from this side would race with it.
******************************************************************************************************************************************************/
void WebInterface::broadcastLine(const char* Line)
{
    if((Server == nullptr) || (Line == nullptr)) { return; }

    httpd_ws_frame_t Frame{};
    Frame.type = HTTPD_WS_TYPE_TEXT;
    Frame.payload = reinterpret_cast<uint8_t*>(const_cast<char*>(Line));
    Frame.len = strlen(Line);

    if(Frame.len == 0u) { return; }

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        const int Descriptor = Clients[Slot].load(std::memory_order_acquire);

        if(Descriptor != NoClient) { httpd_ws_send_frame_async(Server, Descriptor, &Frame); }
    }
} /* broadcastLine */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  addClient()
******************************************************************************************************************************************************/
/*! \brief          Remembers a socket to broadcast to
 *  \details        A full table drops the newcomer rather than an established console, and
 *                  says so on the UART - silently serving a page whose socket then never
 *                  answers is the more confusing failure.
******************************************************************************************************************************************************/
void WebInterface::addClient(int Descriptor)
{
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == Descriptor) { return; }
    }

    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == NoClient) {
            Clients[Slot].store(Descriptor, std::memory_order_release);
            return;
        }
    }

    Serial.println(F("Web: too many consoles, this one gets no answers"));
} /* addClient */


/******************************************************************************************************************************************************
  removeClient()
******************************************************************************************************************************************************/
void WebInterface::removeClient(int Descriptor)
{
    for(size_t Slot = 0u; Slot < MaxClients; Slot++) {
        if(Clients[Slot].load(std::memory_order_relaxed) == Descriptor) {
            Clients[Slot].store(NoClient, std::memory_order_release);
            return;
        }
    }
} /* removeClient */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
