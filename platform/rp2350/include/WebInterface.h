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
 * The HTTP server runs from lwIP's callbacks, never inside the firmware's tick: arduino-pico runs
 * lwIP in a low-priority interrupt on the core that runs loop(). What crosses over between the two
 * is the injected character stream, which WordclockSerial takes from one producer without a lock,
 * the force flag of a client that arrived, a restart the update asks for, and the line of a refused
 * update. Answers travel the other way, sent from the firmware's tick.
 */

#ifndef WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBINTERFACE_H_
#define WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBINTERFACE_H_

#include "StandardTypes.h"

// What a browser is *sent* lives there, together with the frame rate limit and the longest
// command a socket frame may carry - shared with the ESP32 backend rather than kept twice.
#include <stddef.h>

#include "WebFrontend.h"

/*
 * There is no client limit here, and the one that used to stand in this place was never one:
 * it sized a constant nothing read, because AsyncWebSocket owns the list and enforces its own
 * ceiling. The ESP32 keeps its WEB_INTERFACE_MAX_CLIENTS because a table there really is that
 * long and its server is told the same number.
 */
/*! \brief The TCP port the server listens on, and the one mDNS announces. */
#define WEB_INTERFACE_PORT 80u

/*!
 * \brief The host name the clock announces over mDNS, reached as `wordclock.local`.
 *
 * What the clock answers to on the local network, so no address has to be looked up.
 */
#define WEB_INTERFACE_HOSTNAME "wordclock"

/*!
 * \brief The web server of the RP2350: the pages, the command catalog, the panel's letters, the
 *        firmware update and the web socket, on ESPAsyncWebServer.
 *
 * Begin() starts the server, on port WEB_INTERFACE_PORT, with these routes:
 * - every file of WebFrontend's table, at its own path, as it is stored;
 * - `GET /commands` and `GET /display`, the JSON that WebFrontend::WriteCommands() and
 *   WebFrontend::WriteDisplay() write;
 * - `POST /update`, which takes a firmware image as the request body, writes it to LittleFS as
 *   `/firmware.bin`, has PicoOTA write the loader's command beside it once the image is whole,
 *   and asks for a restart with System::Restart();
 * - `/ws`, the web socket: a text frame that is a whole message is a command for
 *   WebFrontend::InjectCommand(), and the lines and the display frames go out through
 *   WebTransport.
 *
 * Every route but the web socket asks for the console's password while
 * System::IsConsoleProtected() is true, with Basic authentication over plain http for the user
 * `wordclock`; the update asks before its body is written as well. That keeps out a guest who
 * opens the page; it keeps out nobody who can watch the traffic.
 *
 * The server's handlers run from lwIP's callbacks, which arduino-pico runs in a low-priority
 * interrupt on the core that runs loop(). The application calls Begin() once, and
 * BroadcastFrame() from its tick; WordclockSerial's line sink calls BroadcastLine().
 *
 * The web socket asks for the password at its handshake, as every route does; a refused one is
 * answered with 401 by the library.
 */
class WebInterface {
 private:
  /*
   * Nothing is kept here at all any more. The list of clients never was - AsyncWebSocket
   * owns it, counts it and broadcasts to it, so a second copy could only ever disagree with
   * the first - and the frame bookkeeping that used to sit beside it is WebFrontend's now,
   * a rate limit and a comparison being the same on any backend.
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
   * The line sink is installed only once the server has begun, so a line printed during startup
   * cannot reach a half-built server.
   *
   * It then announces WEB_INTERFACE_HOSTNAME over mDNS and prints the host name it is reached
   * at, or prints that mDNS is unavailable; that is not a failure, as the address still works.
   *
   * \return Always kEOk on the RP2350: ESPAsyncWebServer's begin() answers nothing, so a server
   *         that does not start shows up as a page that never answers rather than as kENotOk.
   */
  StdReturnType Begin();

  /*!
   * \copydoc WebFrontend::BroadcastLine(const char*)
   *
   * \note On the RP2350 a line also comes through here from the web server's callbacks, which print
   *       the reason when they refuse an update; it is then sent from lwIP's context.
   */
  void BroadcastLine(char const* line);

  /*! \copydoc WebFrontend::BroadcastFrame() */
  void BroadcastFrame();

  // An instance method like the rest of this interface, for the reason WebInterface.cpp gives.
  /*!
   * \copydoc WebFrontend::OnClientOpened()
   *
   * On the RP2350 the socket's event handler calls it on a handshake, from lwIP's context. Only
   * the force flag is touched from there; who is connected is the socket's own business.
   */
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  void OnClientOpened() { WebFrontend::GetInstance().OnClientOpened(); }
};

#endif  // WORDCLOCK_PLATFORM_RP2350_INCLUDE_WEBINTERFACE_H_
