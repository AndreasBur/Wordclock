// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief How the simulator application starts, ticks the firmware and stops.
 */

#include "WordclockApp.h"

#include "WebFrontend.h"
#include "sim/PixelsFrame.h"
#include "sim/WebHost.h"

// wxWidgets' macros for the entry point and the event table, which Doxygen would take for
// functions of this file.
/*! \cond */
IMPLEMENT_APP(WordclockApp);

wxBEGIN_EVENT_TABLE(WordclockApp, wxApp)
  EVT_TIMER(kTimerId, WordclockApp::OnTimer)
wxEND_EVENT_TABLE()
/*! \endcond */

WordclockApp::WordclockApp() : timer_(this, kTimerId) {
  timer_.Start(Scheduler::GetTaskIntervalMs());
}

bool WordclockApp::OnInit() {
  PixelsFrame& frame = PixelsFrame::GetInstance();

  /*
   * Bound to the window rather than listed in the application's table, where it never ran:
   * the window handles its close itself and does not pass it on, so the timer went on
   * ticking into a window that was gone. A bound handler runs before the window's own.
   */
  frame.Bind(wxEVT_CLOSE_WINDOW, &WordclockApp::OnClose, this);
  frame.Show();
  WebHost::GetInstance().Begin();
  return true;
}

void WordclockApp::OnClose(wxCloseEvent& event) {
  timer_.Stop();
  WebHost::GetInstance().End();
  // On to the window's own handler, which destroys it.
  event.Skip();
}

void WordclockApp::OnTimer(wxTimerEvent& event) {
  // Before the firmware, so a command that arrived since the last tick is in the port by
  // the time the parser reads it - one tick earlier than taking it afterwards would be.
  WebHost::GetInstance().Task();

  wordclock_.Task();
  // After the firmware, not from inside its pixel writes: the window shows whatever
  // the buffer holds once the whole pass over the tasks is done.
  PixelsFrame::GetInstance().Render();
  // And after the window, on the same buffer it just drew, so both front ends show the
  // frame the tick produced rather than one of them showing the previous one.
  WebFrontend::GetInstance().BroadcastFrame();
  UNUSED(event);
}
