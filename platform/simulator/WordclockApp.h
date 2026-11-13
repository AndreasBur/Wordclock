// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The wxWidgets application of the simulator.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_WORDCLOCKAPP_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_WORDCLOCKAPP_H_

#include <wx/app.h>
#include <wx/timer.h>

#include "WordclockMain.h"

/*!
 * \brief The wxWidgets application of the simulator: it owns the firmware, WordclockMain, and the
 *        timer that ticks it, and opens the window and the web host.
 */
class WordclockApp : public wxApp {
 public:
  /*!
   * \brief Makes the application and starts the timer at Scheduler::GetTaskIntervalMs().
   *
   * Making it makes WordclockMain first, which restores the stored settings.
   */
  WordclockApp();
  /*!
   * \brief Shows the window of PixelsFrame and starts the web host.
   *
   * The web host is started after the window, so the address it prints has somewhere to be
   * printed. A port that is taken is not fatal - the window is still worth having, which is
   * what WebHost::Begin() answers.
   *
   * \return Always true, so the application runs.
   */
  bool OnInit() override;
  /*!
   * \brief Runs one tick: WebHost::Task(), WordclockMain::Task(), PixelsFrame::Render() and
   *        WebFrontend::BroadcastFrame(), in this order.
   *
   * \param[in] event The timer's event; it is not read.
   */
  void OnTimer(wxTimerEvent& event);
  /*!
   * \brief Stops the timer and closes the web host's sockets.
   *
   * Bound to the window's close event in OnInit(), it runs before the window's own handler,
   * and passes the event on to it, which destroys the window.
   *
   * \param[in,out] event The close event, passed on.
   */
  void OnClose(wxCloseEvent& event);

 private:
  WordclockMain wordclock_;
  wxTimer timer_;
  wxDECLARE_EVENT_TABLE();

  enum { kTimerId = 1500 };
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_WORDCLOCKAPP_H_
