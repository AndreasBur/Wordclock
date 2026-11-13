// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What WebFrontend needs from this backend's server.
 *
 * The simulator's half of the contract in src/Communication/WebFrontend/WebFrontend.h -
 * the third one, after the ESP32's and the RP2350's, and the one that needs no board.
 *
 * What sits behind it is WebHost, a socket, HTTP and a web socket written out here because a
 * desktop build has no framework to ask. That is the price of this direction,
 * and what it buys is that the pages a clock serves can be opened from the same binary that draws
 * the window.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBTRANSPORT_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBTRANSPORT_H_

#include "StandardTypes.h"
// Where this backend's `byte` comes from - the two on hardware get it from the Arduino core,
// and this header is reached before that one on the way in from WebHost.cpp.
#include "arduino/Types.h"

#include <stddef.h>
#include <string>

/*!
 * \brief One route's answer, being written: the body of a response, which the core writes into and
 *        finishes.
 *
 * A backend's route opens it and hands it to WebFrontend::WriteCommands() or
 * WebFrontend::WriteDisplay(), which write the document into it and finish it.
 *
 * Collected whole rather than pushed out in chunks: this server answers on loopback and the
 * largest thing it writes is a page out of flash, so a length up front is simpler than a chunked
 * encoding and is what Finish() sends.
 */
class WebResponseBody
{
  private:
    int socket_;
    const char* content_type_;
    std::string collected_;

  public:
    /*!
     * \brief Opens the answer to the request that came in on `socket`.
     *
     * \param[in] socket The connection the request came in on.
     * \param[in] content_type The content type of the body, terminated.
     * \note `content_type` is kept as a pointer and only read by Finish(), so it has to stay valid
     *       until then.
     */
    WebResponseBody(int socket, const char* content_type);

    /*!
     * \brief Adds `length` bytes of `data` to the body.
     *
     * In the simulator, nothing is sent before Finish().
     *
     * \param[in] data The bytes; they need not be terminated.
     * \param[in] length The number of bytes.
     */
    void Write(const char* data, size_t length);

    /*!
     * \brief Ends the body, and has the answer sent.
     *
     * Called once, after the last Write(). In the simulator, it sends the status 200, the content
     * type, the length and the body collected, at once, on the caller's thread.
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
 * The client list is WebHost's, since nothing here comes with one. What is sent goes out at
 * once, on the caller's thread.
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
     * \param[in] text The text, as UTF-8, which a text frame has to be; it need not be terminated.
     * \param[in] length The number of bytes.
     */
    void SendText(const char* text, size_t length);

    /*!
     * \brief Sends `length` bytes of `bytes` to every client of the web socket, as a binary frame.
     *
     * \param[in] bytes The bytes.
     * \param[in] length The number of bytes.
     */
    void SendBinary(const byte* bytes, size_t length);
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBTRANSPORT_H_
