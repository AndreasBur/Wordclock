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
 * backend's Arduino.h binds Serial to a macro that must not reach the IDF's own headers - which is
 * why every platform source includes them first. A header pulled into firmware sources cannot make
 * that promise, so it names httpd's request type by pointer to an incomplete struct instead. That
 * also sidesteps a difference nothing else would have caught: httpd_handle_t is void* in the IDF
 * and struct httpd_t* in the test stub, so a handle in this file would not compile against both.
 *
 * Both classes are *defined* in WebInterface.cpp rather than in a source of their own, because what
 * they reach - the server handle and the table of client descriptors - is that file's and stays
 * that file's.
 */

#ifndef WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBTRANSPORT_H_
#define WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBTRANSPORT_H_

#include "StandardTypes.h"
#include "Arduino.h"

#include <stddef.h>

/*!
 * \brief One route's answer, being written: the body of a response, which the core writes into and
 *        finishes.
 *
 * A backend's route opens it and hands it to WebFrontend::WriteCommands() or
 * WebFrontend::WriteDisplay(), which write the document into it and finish it.
 *
 * On the ESP32 it is a chunked response of esp_http_server. This server is pushed to: every
 * Write() is a chunk on the wire, and Finish() is the empty chunk that ends a chunked response.
 */
class WebResponseBody
{
  private:
    struct httpd_req* request_;

  public:
    /*!
     * \brief Opens the answer to `request`, and sets its content type.
     *
     * \param[in] request The request of esp_http_server the answer is for.
     * \param[in] content_type The content type of the body, terminated.
     */
    WebResponseBody(struct httpd_req* request, const char* content_type);

    /*!
     * \brief Adds `length` bytes of `data` to the body.
     *
     * On the ESP32, sent at once as one chunk with `httpd_resp_send_chunk()`, whose answer is not
     * checked.
     *
     * \param[in] data The bytes; they need not be terminated.
     * \param[in] length The number of bytes.
     */
    void Write(const char* data, size_t length);

    /*!
     * \brief Ends the body, and has the answer sent.
     *
     * Called once, after the last Write(). On the ESP32, it sends the empty chunk that ends the
     * response.
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
 * On the ESP32 the clients are a table of web socket descriptors kept in WebInterface.cpp, at most
 * WEB_INTERFACE_MAX_CLIENTS of them, which the server's task fills on a handshake and empties on a
 * close. This server hands out socket descriptors that the backend has to keep itself, so
 * HasClients() is a walk over that table rather than a count somebody else keeps.
 */
class WebTransport
{
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
     * On the ESP32, with `httpd_ws_send_frame_async()` per client. Nothing is sent for a `length`
     * of 0 or while the server is not running. A send that fails is not retried and the client is
     * not dropped here: the server's close hook owns that.
     *
     * \param[in] text The text, as UTF-8, which a text frame has to be; it need not be terminated.
     * \param[in] length The number of bytes.
     */
    void SendText(const char* text, size_t length);

    /*!
     * \brief Sends `length` bytes of `bytes` to every client of the web socket, as a binary frame.
     *
     * On the ESP32, with `httpd_ws_send_frame_async()` per client. Nothing is sent for a `length`
     * of 0 or while the server is not running. A send that fails is not retried and the client is
     * not dropped here: the server's close hook owns that.
     *
     * \param[in] bytes The bytes.
     * \param[in] length The number of bytes.
     */
    void SendBinary(const byte* bytes, size_t length);
};

#endif  // WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBTRANSPORT_H_
