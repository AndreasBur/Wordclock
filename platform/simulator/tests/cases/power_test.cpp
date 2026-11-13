// The order the strip's supply is switched in, which is the whole of this feature.

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "Pixels.h"
#include "Power.h"
#include "PowerSwitch.h"

namespace {
/*
 * What the application does after every pass over the tasks, and what PixelsFrame does with
 * it here: the buffer is handed over and stops being dirty. It stands in for the render
 * because the wait this feature is built around is a wait for exactly that - and none of
 * these cases has a window to do it for them.
 */
void RenderOnce() { Pixels::GetInstance().ClearDirty(); }

// A clock face on a lit display, with the supply up and the sequence idle - the state every
// case below starts from, and the one it has to be left in for the next.
void StartFromRunningClock() {
  Display& display = Display::GetInstance();

  display.ResumeOutput();
  display.Enable();
  Expect(Clock::GetInstance().SetTime(10u, 5u) == kEOk, "test clock time must be accepted");
  display.Show();
  RenderOnce();
}

}  // namespace

// The point of the whole module: the port must not drop while the blanked frame is still on
// its way, or the strip is left holding a frame with no supply to latch it.
void TestSupplyCutWaitsForTheBlankedFrame() {
  Power& power = Power::GetInstance();
  Display& display = Display::GetInstance();

  StartFromRunningClock();
  Expect(power.IsSupplyOn(), "a running clock must have its strip powered");
  Expect(IsAnyOutputPixelLit(), "and must be putting something out");

  Expect(power.SwitchSupplyOff() == kEOk, "a fitted switch must accept being asked");
  Expect(!IsAnyOutputPixelLit(), "the ask must darken the strip over the data line first");
  Expect(power.IsSupplyOn(), "and must not have cut the supply yet");
  Expect(!display.IsOutputSuspended(),
         "nor gated the line the blanked frame still has to go out on");

  // The frame is only asked for at this point, not gone. A task here must find that out
  // rather than take the darkness for done.
  power.Task();
  Expect(power.IsSupplyOn(), "a task before the blanked frame was rendered must keep the supply");

  RenderOnce();
  power.Task();
  Expect(!power.IsSupplyOn(), "once the frame is off the wire the supply must go");
  Expect(display.IsOutputSuspended(), "and the data line must be gated behind it");

  /* Left as it was found, or the next case starts on a dark strip. */
  Expect(power.SwitchSupplyOn() == kEOk, "the supply must be askable back");
  power.Task();
  StartFromRunningClock();
}

// Switching on is the same order backwards, and the reason is the same diode: the strip
// wants its rail before the first frame arrives, not with it.
void TestSupplyReturnsBeforeTheDataLine() {
  Power& power = Power::GetInstance();
  Display& display = Display::GetInstance();

  StartFromRunningClock();

  /* Down first, all the way, so that coming back up is what is being read. */
  power.SwitchSupplyOff();
  RenderOnce();
  power.Task();
  Expect(!power.IsSupplyOn() && display.IsOutputSuspended(), "the strip must be off and gated");

  Expect(power.SwitchSupplyOn() == kEOk, "a fitted switch must accept being asked back");
  Expect(power.IsSupplyOn(), "the supply must come up on the ask");
  Expect(display.IsOutputSuspended(), "and the data line must stay gated for that tick");
  Expect(!IsAnyOutputPixelLit(), "so nothing is put out while the rail is still settling");

  power.Task();
  Expect(!display.IsOutputSuspended(), "the next task must release the line");
  Expect(IsAnyOutputPixelLit(), "and give the display back");
  Expect(power.GetState() == Power::StateType::kSupplyOn, "which leaves the sequence idle again");

  // The strip forgot its registers with its supply, so the buffer has to be marked for a
  // redraw rather than trusted to still be up there.
  Expect(display.IsDirty(),
         "coming back must ask for a redraw, not assume the strip kept the frame");

  StartFromRunningClock();
}

// The two pairs of ids are not each other. Ids 3 and 4 darken a powered strip; these cut
// its supply. Collapsing them would lose the only one that saves any current.
void TestSupplyOffIsNotDisplayDisable() {
  Power& power = Power::GetInstance();
  Display& display = Display::GetInstance();

  StartFromRunningClock();

  display.Disable();
  Expect(!IsAnyOutputPixelLit(), "disabling the display must darken it");
  Expect(power.IsSupplyOn(), "but must leave the strip powered");
  Expect(!display.IsOutputSuspended(), "and must not gate the data line");

  display.Enable();
  Expect(IsAnyOutputPixelLit(), "and it must come back without the supply having moved");

  StartFromRunningClock();
}

// Asking twice, and changing one's mind halfway. The states in between are the ones a
// caller cannot see, so they are the ones that have to behave.
void TestSupplyRequestsAreReversible() {
  Power& power = Power::GetInstance();
  Display& display = Display::GetInstance();

  StartFromRunningClock();

  // Asked for what is already the case. Honoured rather than refused: twice off means the
  // same as once off, and a caller with no readback would have nothing to do with a no.
  Expect(power.SwitchSupplyOn() == kEOk, "asking for a supply that is already on must be honoured");
  Expect(power.GetState() == Power::StateType::kSupplyOn, "and must not start a sequence");

  // Changed mind while the blanked frame is still going out. The supply never went, so
  // there is nothing to bring up - only the darkness to undo.
  power.SwitchSupplyOff();
  Expect(power.GetState() == Power::StateType::kBlanking,
         "the ask must put the sequence into blanking");
  Expect(power.SwitchSupplyOn() == kEOk, "and must be revocable while it is still there");
  Expect(power.GetState() == Power::StateType::kSupplyOn, "which takes the sequence straight back");
  Expect(power.IsSupplyOn() && IsAnyOutputPixelLit(), "with the strip powered and lit again");

  // Changed mind the other way, while the rail is settling. The line is still gated there,
  // so the strip is dark already and the port can go straight back down.
  power.SwitchSupplyOff();
  RenderOnce();
  power.Task();
  power.SwitchSupplyOn();
  Expect(power.GetState() == Power::StateType::kSettling,
         "the ask back must settle rather than finish");
  Expect(power.SwitchSupplyOff() == kEOk, "and must itself be revocable");
  Expect(!power.IsSupplyOn(), "which drops the port again");
  Expect(display.IsOutputSuspended(), "with the line still gated, as it never stopped being");

  power.SwitchSupplyOn();
  power.Task();
  StartFromRunningClock();
}
