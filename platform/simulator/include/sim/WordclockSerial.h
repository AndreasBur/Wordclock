// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The port a second front end reaches, by the name the firmware knows it.
 *
 * The web front end injects a browser's command as typed characters and takes every finished answer
 * line out of a sink, and it reaches both under this name because that is what the two networked
 * backends call the port. Here the port is SerialShim, which the window already owns.
 *
 * A header of its own rather than SerialShim growing the name, because the seam belongs to whoever
 * reads it: src/Communication/WebFrontend includes "WordclockSerial.h" and must not have
 * to know that this backend spells its port differently.
 */

#ifndef _WORDCLOCK_SERIAL_H_
#define _WORDCLOCK_SERIAL_H_

#include "sim/SerialShim.h"

/*!
 * \brief The room for one answer line.
 *
 * Room for one answer, the same number the two networked backends use. What reads it is the web
 * front end, which sizes the buffer it widens a line into by it.
 */
#define WORDCLOCK_SERIAL_LINE_LENGTH                    160u

/*!
 * \brief The port the web front end reaches by this name: here SerialShim, the port Serial is bound
 *        to.
 *
 * WebFrontend hands a browser's command to it with Inject(), as though it had been typed. The
 * backend's server installs the sink every finished answer line goes to with SetLineSink(); here
 * that is WebHost.
 *
 * The port under the name the firmware reaches it by. An alias rather than a wrapper: there is one
 * port in this backend, and a second object standing in front of it would only be something to
 * keep in step.
 */
using WordclockSerial = SerialShim;

#endif // _WORDCLOCK_SERIAL_H_
