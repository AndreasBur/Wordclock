// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What WebFrontend needs from this backend's server.
 *
 * The backend half of the contract in src/Communication/WebFrontend/WebFrontend.h: where a
 * response body goes, and where a line or a frame goes. Everything above it - the catalog, the
 * letter grid, the rate limit, the frame comparison - is the firmware's and is not repeated here.
 *
 * Deliberately free of framework headers, and it has to be: the firmware includes this, and this
 * backend's Arduino.h binds Serial to a macro that must not reach the framework's own headers -
 * which is why every platform source includes them first. A header pulled into firmware sources
 * cannot make that promise, so the library's two request classes are named by forward declaration
 * instead.
 *
 * Both classes are *defined* in WebInterface.cpp rather than in a source of their own, because what
 * they reach - the server and its web socket - is that file's and stays that file's.
 */

#ifndef WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBTRANSPORT_H_
#define WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBTRANSPORT_H_

#include <stddef.h>

#include "Arduino.h"
#include "StandardTypes.h"

class AsyncWebServerRequest;
class AsyncResponseStream;

/*!
 * \brief One route's answer, being written: the body of a response, which the core writes into and
 *        finishes.
 *
 * A backend's route opens it and hands it to WebFrontend::WriteCommands() or
 * WebFrontend::WriteDisplay(), which write the document into it and finish it.
 *
 * On the RP2350 it is an `AsyncResponseStream` of ESPAsyncWebServer. This server is pulled from
 * rather than pushed to: the stream collects what is written and Finish() is what hands it to the
 * request, after which the library sends it.
 */
class WebResponseBody {
 private:
  AsyncWebServerRequest* request_;
  AsyncResponseStream* stream_;

 public:
  /*!
   * \brief Opens the answer to `request`, and sets its content type.
   *
   * It opens the stream with `beginResponseStream()`.
   *
   * \param[in] request The request of ESPAsyncWebServer the answer is for.
   * \param[in] content_type The content type of the body, terminated.
   */
  WebResponseBody(AsyncWebServerRequest* request, char const* content_type);

  /*!
   * \brief Adds `length` bytes of `data` to the body.
   *
   * On the RP2350, written into the stream, which holds it until Finish(); what the stream
   * answers is not checked.
   *
   * \param[in] data The bytes; they need not be terminated.
   * \param[in] length The number of bytes.
   */
  void Write(char const* data, size_t length);

  /*!
   * \brief Ends the body, and has the answer sent.
   *
   * Called once, after the last Write(). On the RP2350 it hands the stream to the request with
   * `send()`, and the library sends it.
   */
  void Finish();
};

/*!
 * \brief Where a finished line or a display frame leaves for whoever is watching: every client of
 *        the web socket.
 *
 * WebFrontend sends through it from the firmware's tick. It asks first whether the server is
 * listening, and before a display frame whether a client is connected.
 *
 * On the RP2350 the clients are `AsyncWebSocket`'s own client list. The socket owns that list,
 * counts it and broadcasts to it, so HasClients() is that count and there is no table here that
 * could disagree with it. IsListening() is true from the end of WebInterface::Begin() on.
 */
class WebTransport {
 public:
  /*! \brief Returns the one transport. */
  static WebTransport& GetInstance();

  /*! \brief Returns whether the server is listening for connections. */
  bool IsListening() const;

  /*! \brief Returns whether a client is connected to the web socket. */
  bool HasClients() const;

  /*!
   * \brief Sends `length` bytes of `text` to every client of the web socket, as a text frame.
   *
   * On the RP2350, with `AsyncWebSocket::textAll()`, whose answer is not checked.
   *
   * \param[in] text The text, as UTF-8, which a text frame has to be; it need not be terminated.
   * \param[in] length The number of bytes.
   */
  void SendText(char const* text, size_t length);

  /*!
   * \brief Sends `length` bytes of `bytes` to every client of the web socket, as a binary frame.
   *
   * On the RP2350, with `AsyncWebSocket::binaryAll()`, whose answer is not checked.
   *
   * \param[in] bytes The bytes.
   * \param[in] length The number of bytes.
   */
  void SendBinary(byte const* bytes, size_t length);
};

#endif  // WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBTRANSPORT_H_
