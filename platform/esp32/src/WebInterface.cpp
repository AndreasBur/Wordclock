// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Serves the console page and bridges its web socket to Serial.
 */

// clang-format off
/* The framework's headers first, ahead of the Arduino.h that binds Serial to a macro. */
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_http_server.h>

#include "Arduino.h"

#include "System.h"
#include "WebFrontend.h"
#include "WebInterface.h"
#include "WebTransport.h"
#include "WordclockSerial.h"

#include <atomic>  // NOLINT(build/include_order)
#include <string.h>  // NOLINT(build/include_order)
#include <unistd.h>  // NOLINT(build/include_order)
// clang-format on

namespace {
// Named http_server rather than Server: the Arduino core has a class Server in the global
// namespace, and an unqualified Server here is then ambiguous.
httpd_handle_t http_server{nullptr};

constexpr size_t kMaxClients{WEB_INTERFACE_MAX_CLIENTS};
constexpr int kNoClient{-1};

/*
 * The descriptors this server hands out, which it expects to be kept: unlike a library that
 * owns its client list, the IDF's server reports a handshake and a close and leaves the
 * bookkeeping here.
 *
 * Written only by the HTTP server's task - on a handshake and on a close - and read only by
 * the firmware's task when it broadcasts. One writer is what makes the plain atomics enough,
 * the same reasoning as for WordclockSerial's ring buffer.
 *
 * Reached through a function rather than left as an object with static storage duration,
 * because the slots start at kNoClient and not at zero, and zero is a descriptor a socket can
 * actually be given. Initialising on first use is also what the singleton it used to live on
 * did.
 */
struct ClientTableType {
  std::atomic<int> slots[kMaxClients];

  ClientTableType() {
    for (size_t slot = 0u; slot < kMaxClients; slot++) {
      slots[slot].store(kNoClient);
    }
  }
};

ClientTableType& ClientTable() {
  static ClientTableType table;
  return table;
}

/*
 * One frame to everybody, which is what both of WebTransport's send calls come down to - the
 * type and the payload are all that differ between a line and a display frame. A send that
 * fails is not retried and the client is not dropped here: the server's close hook owns that,
 * and dropping a client from this side would race with it.
 */
void SendToEveryClient(httpd_ws_type_t type, uint8_t* payload, size_t length) {
  if ((http_server == nullptr) || (length == 0u)) {
    return;
  }

  httpd_ws_frame_t frame{};
  frame.type = type;
  frame.payload = payload;
  frame.len = length;

  for (size_t slot = 0u; slot < kMaxClients; slot++) {
    int const descriptor = ClientTable().slots[slot].load(std::memory_order_acquire);

    if (descriptor != kNoClient) {
      httpd_ws_send_frame_async(http_server, descriptor, &frame);
    }
  }
}

// Handed to WordclockSerial, which takes a plain function pointer - so the singleton is
// reached from here rather than carried along.
void SendLineToClients(char const* line) { WebInterface::GetInstance().BroadcastLine(line); }

/*
 * Whether this request may be answered, and the 401 when it may not: true when the handler may
 * go on.
 *
 * Basic authentication, and what it is worth saying plainly: the credential travels
 * base64-encoded over plain http, which is not encryption. What this keeps out is a guest who
 * opens the page and starts sending commands, or installing firmware. It keeps out nobody who
 * can watch the traffic, and that would need https, which a clock cannot offer credibly.
 *
 * Off unless a password was stored, so an update locks nobody out of a clock that is already
 * on a wall, and the access point of a first setup still answers.
 *
 * Every route goes through this, the web socket's handshake included - a console that asked
 * for a password and then took commands on an unchecked socket would be a lock on the wrong
 * door.
 */
bool IsRequestAuthorised(httpd_req_t* request) {
  if (!System::GetInstance().IsConsoleProtected()) {
    return true;
  }

  // "Basic " and the blob. Sized for the longest credential the store can hold, and a
  // header longer than that cannot be one this clock would accept.
  char header[SYSTEM_PASSWORD_STRING_LENGTH + 128u]{};
  static constexpr char kScheme[]{"Basic "};

  if (httpd_req_get_hdr_value_str(request, "Authorization", header, sizeof(header)) == ESP_OK) {
    if (strncmp(header, kScheme, sizeof(kScheme) - 1u) == 0) {
      if (System::GetInstance().IsConsoleCredentialValid(&header[sizeof(kScheme) - 1u])) {
        return true;
      }
    }
  }

  // The realm carries the user name, because it is the one field a browser cannot guess and
  // it is not a secret - the password beside it is.
  httpd_resp_set_status(request, "401 Unauthorized");
  httpd_resp_set_hdr(request, "WWW-Authenticate", "Basic realm=\"Wordclock, user wordclock\"");
  httpd_resp_set_type(request, "text/plain");
  httpd_resp_send(request, "this clock asks for a password\n", HTTPD_RESP_USE_STRLEN);

  return false;
}

/*
 * Serves one of the files the clock hands out unchanged.
 *
 * One handler for all five - the two pages, the manifest and the two icons - because what
 * separates them is data and the data is WebFrontend's table. Which entry this request is for
 * arrives in the server's own user context, which is what that field exists for.
 *
 * A gzipped asset is sent as it is stored and the browser unpacks it; that is the whole of what
 * the header does here, and the table says which.
 */
esp_err_t HandleAsset(httpd_req_t* request) {
  if (!IsRequestAuthorised(request)) {
    return ESP_OK;
  }

  WebFrontend::AssetType const& asset =
      *static_cast<WebFrontend::AssetType const*>(request->user_ctx);

  httpd_resp_set_type(request, asset.content_type);
  if (asset.is_gzipped) {
    httpd_resp_set_hdr(request, "Content-Encoding", "gzip");
  }

  return httpd_resp_send(request, reinterpret_cast<char const*>(asset.bytes),
                         static_cast<ssize_t>(asset.size));
}

/*
 * Answers the update route, in the one shape the page reads: JSON like the other two routes
 * that answer data, and the HTTP status set with it - so the page can branch on the status and
 * still have a sentence to show. The reason travels as text on purpose: what goes wrong here is
 * a wrong file or a dropped connection, and a number for that would only have to be translated
 * back somewhere.
 */
esp_err_t SendUpdateResult(httpd_req_t* request, bool ok, char const* reason) {
  char body[192u];

  httpd_resp_set_type(request, "application/json");

  if (ok) {
    httpd_resp_set_status(request, "200 OK");
    (void)snprintf(body, sizeof(body), "{\"ok\":true}");
  } else {
    httpd_resp_set_status(request, "400 Bad Request");
    // %.120s, because errorString() is the framework's text and the buffer is ours - and
    // with it the answer cannot be cut short, so the length snprintf returns says nothing.
    (void)snprintf(body, sizeof(body), "{\"ok\":false,\"error\":\"%.120s\"}", reason);
    Serial.print(F("Web: update refused - "));
    Serial.println(reason);
  }

  return httpd_resp_send(request, body, HTTPD_RESP_USE_STRLEN);
}

/*
 * Takes a new firmware image over HTTP and reboots into it. Every way out sends an answer
 * first, whichever answer that is.
 *
 * The body is the image itself, not a form: a multipart parser in flash would exist to undo
 * something the browser only does because a <form> asked it to, and the page sends the file as
 * the request body instead. That is also what makes `curl --data-binary` a first-class way in,
 * which is the one that works when the page is the thing that broke.
 *
 * The image goes to the partition the running one is not in, and the bootloader is pointed at
 * it only by end() - so an upload that stops halfway leaves the clock running what it was
 * running. That is the whole safety argument for doing this at all, and it is the framework's
 * rather than ours.
 *
 * The reboot is asked for and not carried out: this handler's answer is still in the server's
 * buffer, and a controller that restarts here sends the browser nothing at all - which reads as
 * a failed update rather than a finished one. System::Restart() already defers to the
 * application's tick for exactly this reason, so it is what carries it out.
 */
esp_err_t HandleUpdate(httpd_req_t* request) {
  if (!IsRequestAuthorised(request)) {
    return ESP_OK;
  }

  /*
   * Announced rather than measured, and it has to be: Update needs the size before the
   * first byte, to know which partition can hold it and to erase it. A body that then
   * turns out shorter is caught by end() below, which refuses to finish an image that is
   * not all there.
   */
  size_t const announced = request->content_len;

  if (announced == 0u) {
    return SendUpdateResult(request, false, "no image in the request body");
  }

  if (!Update.begin(announced)) {
    /*
     * The usual one is an image larger than the partition, which is worth saying as
     * itself rather than as "update failed": it means the wrong file was picked - the
     * factory image instead of the plain one, most likely.
     */
    return SendUpdateResult(request, false, Update.errorString());
  }

  /*
   * One kilobyte at a time, on the server's own task. Larger buffers buy nothing here:
   * the flash write is what takes the time, and this runs beside the clock rather than
   * inside its tick, so the display keeps its frames throughout.
   */
  char chunk[1024u];
  size_t received = 0u;

  while (received < announced) {
    int const read = httpd_req_recv(request, chunk, sizeof(chunk));

    // A dropped connection halfway through, which is the case the abort exists for:
    // without it the next attempt would find the partition still claimed.
    if (read <= 0) {
      Update.abort();
      return SendUpdateResult(request, false, "the upload stopped before the image was complete");
    }
    if (Update.write(reinterpret_cast<uint8_t*>(chunk), static_cast<size_t>(read)) !=
        static_cast<size_t>(read)) {
      char const* reason = Update.errorString();
      Update.abort();
      return SendUpdateResult(request, false, reason);
    }
    received += static_cast<size_t>(read);
  }

  if (!Update.end(true)) {
    return SendUpdateResult(request, false, Update.errorString());
  }

  System::GetInstance().Restart();

  return SendUpdateResult(request, true, "");
}

/*
 * Serves the command catalog; HandleDisplay() below serves the panel's letters.
 *
 * Both documents are written by WebFrontend, shared with the RP2350 backend: what a page is
 * sent is a property of the catalog and the letter table, not of a controller. They were
 * generated here once, and identically in the other backend - two functions that had to be
 * kept the same by hand and whose divergence would have surfaced in a browser rather than in a
 * test.
 *
 * What is left here is what is genuinely this server's: the password, the content type, and a
 * body to write into.
 */
esp_err_t HandleCommands(httpd_req_t* request) {
  if (!IsRequestAuthorised(request)) {
    return ESP_OK;
  }

  WebResponseBody body(request, "application/json");
  WebFrontend::WriteCommands(body);

  return ESP_OK;
}

esp_err_t HandleDisplay(httpd_req_t* request) {
  if (!IsRequestAuthorised(request)) {
    return ESP_OK;
  }

  WebResponseBody body(request, "application/json");
  WebFrontend::WriteDisplay(body);

  return ESP_OK;
}

/*
 * Takes one web socket frame and injects it as typed characters; ESP_OK unless the frame could
 * not be read.
 *
 * A GET is the handshake, and returning ESP_OK from it is what completes the upgrade - the
 * frame handling below only ever sees later requests.
 *
 * The command is injected followed by the end-of-message character, unless the browser already
 * sent one. That keeps the page free of protocol bookkeeping: it sends the command text, the
 * terminator is this side's business.
 */
esp_err_t HandleSocket(httpd_req_t* request) {
  /*
   * The handshake only. A frame arriving later has no headers to carry a credential, so
   * the check belongs at the one moment the browser does send them - which is the GET the
   * server calls this handler with before the socket exists.
   */
  if (request->method == HTTP_GET) {
    if (!IsRequestAuthorised(request)) {
      return ESP_OK;
    }
  }

  if (request->method == HTTP_GET) {
    WebInterface::GetInstance().OnClientOpened(httpd_req_to_sockfd(request));
    return ESP_OK;
  }

  uint8_t payload[WEB_FRONTEND_MAX_FRAME_LENGTH + 1u]{};
  httpd_ws_frame_t frame{};
  frame.type = HTTPD_WS_TYPE_TEXT;
  frame.payload = payload;

  // Asking with the buffer's size rather than in two steps: a frame that does not fit is
  // refused whole, so half a command can never reach the parser.
  esp_err_t const result = httpd_ws_recv_frame(request, &frame, WEB_FRONTEND_MAX_FRAME_LENGTH);
  if (result != ESP_OK) {
    return result;
  }

  if (frame.type == HTTPD_WS_TYPE_CLOSE) {
    WebInterface::GetInstance().OnClientClosed(httpd_req_to_sockfd(request));
    return ESP_OK;
  }
  if (frame.type != HTTPD_WS_TYPE_TEXT) {
    return ESP_OK;
  }

  // The terminator and the empty-frame case are WebFrontend's, so that a command coming off
  // a socket reaches the parser the same way on either backend.
  WebFrontend::InjectCommand(reinterpret_cast<char const*>(payload), frame.len);

  return ESP_OK;
}

/*
 * Drops a client the server has finished with. Registered as the server's close hook rather
 * than guessed from a failing send: a browser that is simply gone never sends a close frame,
 * and its descriptor would otherwise be handed to send() for the rest of the run.
 */
void OnSocketClosed(httpd_handle_t, int descriptor) {
  WebInterface::GetInstance().OnClientClosed(descriptor);
  close(descriptor);
}

}  // namespace

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
StdReturnType WebInterface::Begin() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = WEB_INTERFACE_PORT;
  // The sockets the server keeps have to cover the console clients plus the ones fetching
  // the page, or a reload can push a console off.
  config.max_open_sockets = WEB_INTERFACE_MAX_CLIENTS + 2u;
  /*
   * Said rather than left to the default, which is 8 and was exactly the count until the
   * console moved to its own route. A route past the limit is refused by the registration
   * below and nothing here reads that answer, so the failure would be one route quietly
   * missing - and the test that counts them is what would find it, on a host, rather than a
   * browser on somebody's wall. The next route to be added has to raise this number with it.
   */
  config.max_uri_handlers = 9u;
  config.close_fn = OnSocketClosed;
  /* Its own task, so nothing here runs inside the firmware's tick. */
  config.core_id = 0;

  if (httpd_start(&http_server, &config) != ESP_OK) {
    http_server = nullptr;
    Serial.println(F("Web: server failed to start, console stays on the UART"));
    return kENotOk;
  }

  /*
   * The files served unchanged, one route per entry of WebFrontend's table. The array is
   * static because httpd keeps the pointer it is registered with rather than a copy, and
   * sized from the table so that a file added to web/ needs no edit here - only
   * max_uri_handlers above has to still be large enough, which is why that number is said
   * out loud rather than left at its default.
   */
  static httpd_uri_t asset_uris[WebFrontend::kMaxAssets]{};

  for (byte index = 0u; index < WebFrontend::GetNumberOfAssets(); index++) {
    WebFrontend::AssetType const& asset = WebFrontend::GetAsset(index);

    asset_uris[index] =
        httpd_uri_t{asset.path, HTTP_GET, HandleAsset, const_cast<WebFrontend::AssetType*>(&asset),
                    false,      false,    nullptr};
    httpd_register_uri_handler(http_server, &asset_uris[index]);
  }

  static httpd_uri_t const kCommandsUri{"/commands", HTTP_GET, HandleCommands, nullptr,
                                        false,       false,    nullptr};
  static httpd_uri_t const kDisplayUri{"/display", HTTP_GET, HandleDisplay, nullptr,
                                       false,      false,    nullptr};
  static httpd_uri_t const kUpdateUri{"/update", HTTP_POST, HandleUpdate, nullptr,
                                      false,     false,     nullptr};
  static httpd_uri_t const kSocketUri{"/ws", HTTP_GET, HandleSocket, nullptr, true, false, nullptr};

  httpd_register_uri_handler(http_server, &kCommandsUri);
  httpd_register_uri_handler(http_server, &kDisplayUri);
  httpd_register_uri_handler(http_server, &kUpdateUri);
  httpd_register_uri_handler(http_server, &kSocketUri);

  /* Only now, so a line printed during startup cannot reach a half-built server. */
  WordclockSerial::GetInstance().SetLineSink(SendLineToClients);

  if (MDNS.begin(WEB_INTERFACE_HOSTNAME)) {
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

/*
 * The widening to UTF-8 and the bound on it are WebFrontend's, shared with the other backend;
 * what is this file's is the walk over the descriptors, in SendText() below. Both of these
 * forward into WebFrontend and stay on this class because the application and the line sink
 * call them by this name.
 */
void WebInterface::BroadcastLine(char const* line) {
  WebFrontend::GetInstance().BroadcastLine(line);
}

/*
 * The rate limit, the gathering and the comparison against the last frame are WebFrontend's,
 * shared with the other backend - none of the three has anything to do with which server is
 * underneath. What is this file's is SendBinary() below.
 */
void WebInterface::BroadcastFrame() { WebFrontend::GetInstance().BroadcastFrame(); }

/*
 * This backend's half of the contract WebFrontend.h states. Defined here rather than in a
 * source of its own because what it reads is this file's: the server handle and the descriptor
 * table above.
 */

WebTransport& WebTransport::GetInstance() {
  static WebTransport singleton_instance;
  return singleton_instance;
}

bool WebTransport::IsListening() const { return http_server != nullptr; }

bool WebTransport::HasClients() const {
  for (size_t slot = 0u; slot < kMaxClients; slot++) {
    if (ClientTable().slots[slot].load(std::memory_order_acquire) != kNoClient) {
      return true;
    }
  }
  return false;
}

void WebTransport::SendText(char const* text, size_t length) {
  // const_cast because httpd's frame struct is the same type used for receiving, where the
  // payload is written into. Nothing on the sending path writes through it.
  SendToEveryClient(HTTPD_WS_TYPE_TEXT, reinterpret_cast<uint8_t*>(const_cast<char*>(text)),
                    length);
}

void WebTransport::SendBinary(byte const* bytes, size_t length) {
  SendToEveryClient(HTTPD_WS_TYPE_BINARY, const_cast<uint8_t*>(bytes), length);
}

// A chunked response, which is what this server offers: a chunk handed over is a chunk on the
// wire, and the empty one at the end is what closes it.

WebResponseBody::WebResponseBody(struct httpd_req* request, char const* content_type)
    : request_(request) {
  httpd_resp_set_type(request_, content_type);
}

void WebResponseBody::Write(char const* data, size_t length) {
  httpd_resp_send_chunk(request_, data, static_cast<ssize_t>(length));
}

void WebResponseBody::Finish() { httpd_resp_send_chunk(request_, nullptr, 0); }

/*
 * A full table drops the newcomer rather than an established console, and says so on the
 * console - silently serving a page whose socket then never answers is the more confusing
 * failure.
 */
void WebInterface::OnClientOpened(int descriptor) {
  /*
   * Told before the table is even looked at: either way a client just arrived, and it has to
   * be shown the display as it stands rather than as it next changes. That includes the
   * descriptor already being in the table - the system reuses them, so the same number
   * coming back is a new client rather than the old one.
   */
  WebFrontend::GetInstance().OnClientOpened();

  for (size_t slot = 0u; slot < kMaxClients; slot++) {
    if (ClientTable().slots[slot].load(std::memory_order_relaxed) == descriptor) {
      return;
    }
  }

  for (size_t slot = 0u; slot < kMaxClients; slot++) {
    if (ClientTable().slots[slot].load(std::memory_order_relaxed) == kNoClient) {
      ClientTable().slots[slot].store(descriptor, std::memory_order_release);
      return;
    }
  }

  Serial.println(F("Web: too many consoles, this one gets no answers"));
}

void WebInterface::OnClientClosed(int descriptor) {
  for (size_t slot = 0u; slot < kMaxClients; slot++) {
    if (ClientTable().slots[slot].load(std::memory_order_relaxed) == descriptor) {
      ClientTable().slots[slot].store(kNoClient, std::memory_order_release);
      return;
    }
  }
}
// NOLINTEND(readability-convert-member-functions-to-static)
