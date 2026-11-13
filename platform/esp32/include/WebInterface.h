// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Serves the console page and bridges its web socket to Serial.
 *
 * The pages and the other files arrive out of flash, from WebFrontend's table, and the socket is
 * wired to WordclockSerial: what a browser sends is injected as characters, and every line the
 * firmware finishes is sent back as one frame. There is no protocol here beyond that - the commands
 * are the ones in docs/serial-commands.md, parsed by the same parser as the wire.
 *
 * The HTTP server runs in its own task, never inside the firmware's tick. What crosses over between
 * the two is the injected character stream, which WordclockSerial takes from one producer without a
 * lock, the table of socket clients, and a restart the update asks for. Answers travel the other
 * way, sent from the firmware's own task.
 */

#ifndef WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBINTERFACE_H_
#define WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBINTERFACE_H_

#include "StandardTypes.h"

// What a browser is *sent* lives there, together with the frame rate limit and the longest
// command a socket frame may carry - shared with the RP2350 backend rather than kept twice.
#include <stddef.h>

#include "WebFrontend.h"

/*!
 * \brief The most web socket clients that are sent the lines and the frames at once.
 *
 * How many browsers may watch at once. Each costs a socket in the server and a slot in the
 * table of clients; a wall clock does not need more, and the server's own limit has to be at least
 * this. A further client is served the page, but its socket gets no answers.
 */
#define WEB_INTERFACE_MAX_CLIENTS 4u

/*! \brief The TCP port the server listens on, and the one mDNS announces. */
#define WEB_INTERFACE_PORT 80u

/*!
 * \brief The host name the clock announces over mDNS, reached as `wordclock.local`.
 *
 * What the clock answers to on the local network, so no address has to be looked up.
 */
#define WEB_INTERFACE_HOSTNAME "wordclock"

/*!
 * \brief The web server of the ESP32: the pages, the command catalog, the panel's letters, the
 *        firmware update and the web socket, on esp_http_server.
 *
 * Begin() starts the server, on port WEB_INTERFACE_PORT, with these routes:
 * - every file of WebFrontend's table, at its own path, as it is stored;
 * - `GET /commands` and `GET /display`, the JSON that WebFrontend::WriteCommands() and
 *   WebFrontend::WriteDisplay() write;
 * - `POST /update`, which takes a firmware image as the request body, writes it to the app slot
 *   that is not running, and asks for a restart with System::Restart();
 * - `/ws`, the web socket: a text frame is a command for WebFrontend::InjectCommand(), and the
 *   lines and the display frames go out through WebTransport.
 *
 * Every route, the web socket's handshake included, asks for the console's password while
 * System::IsConsoleProtected() is true, with Basic authentication over plain http for the user
 * `wordclock`. That keeps out a guest who opens the page; it keeps out nobody who can watch the
 * traffic.
 *
 * The server's handlers run in the server's own task. The application calls Begin() once, and
 * BroadcastFrame() from its tick; WordclockSerial's line sink calls BroadcastLine().
 */
class WebInterface {
 private:
  /*
   * The table of socket descriptors this server hands out is not here but in the anonymous
   * namespace of WebInterface.cpp, beside the server handle it is used with and beside
   * WebTransport, which is the only thing that reads it. Keeping it a member would have
   * meant naming httpd's frame type in this header, and this header is reached from sources
   * that have not included the framework's own first - the ordering this backend's
   * Arduino.h depends on.
   */
  WebInterface() {}
  ~WebInterface() = default;

 public:
  /*! \brief Returns the one web interface. */
  static WebInterface& GetInstance() {
    static WebInterface singleton_instance;
    return singleton_instance;
  }

  /*!
   * \brief Starts the server, registers its routes, and takes over the finished output lines.
   *
   * Safe to call before the network is up: the server listens on whatever address arrives later.
   * It keeps at most WEB_INTERFACE_MAX_CLIENTS + 2 sockets open, so a reload of the page does not
   * push a console off. The line sink is installed only once the routes are registered, so a line
   * printed during startup cannot reach a half-built server.
   *
   * It then announces WEB_INTERFACE_HOSTNAME over mDNS and prints the address it is reached at,
   * or prints that mDNS is unavailable; that is not a failure, as the address still works.
   *
   * \return kEOk if the server started, or kENotOk if it did not; nothing is then registered, the
   *         line sink is left as it was, and the clock runs on the UART alone rather than failing
   *         to boot.
   */
  StdReturnType Begin();

  /*! \copydoc WebFrontend::BroadcastLine(const char*) */
  void BroadcastLine(char const* line);

  /*! \copydoc WebFrontend::BroadcastFrame() */
  void BroadcastFrame();

  /*!
   * \brief Takes a web socket client on, so that the lines and the frames reach it.
   *
   * For the socket's route, which calls it from the server's task on the handshake. It has
   * WebFrontend send the next frame whether or not the display changed, so the client is shown
   * the display as it stands. A descriptor already in the table is kept once: the system reuses
   * them, so the same number coming back is a new client rather than the old one. With
   * WEB_INTERFACE_MAX_CLIENTS clients in the table, the newcomer is left out and the console says
   * so: an established console keeps its answers, and the newcomer gets none.
   *
   * \param[in] descriptor The socket descriptor of the client.
   */
  void OnClientOpened(int descriptor);

  /*!
   * \brief Takes a web socket client off, so that nothing more is sent to it.
   *
   * For the server's task: the socket's route calls it on a close frame, and the server's close
   * hook when the server has finished with a socket. A descriptor not in the table is ignored.
   *
   * \param[in] descriptor The socket descriptor of the client.
   */
  void OnClientClosed(int descriptor);
};

#endif  // WORDCLOCK_PLATFORM_ESP32_INCLUDE_WEBINTERFACE_H_
