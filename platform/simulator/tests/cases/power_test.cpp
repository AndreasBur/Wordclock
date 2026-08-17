/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       power_test.cpp
 *      \brief      The order the strip's supply is switched in, which is the whole of this feature
******************************************************************************************************************************************************/

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "Display.h"
#include "Pixels.h"
#include "Power.h"
#include "PowerSwitch.h"

namespace {

/* What the application does after every pass over the tasks, and what PixelsFrame does with
   it here: the buffer is handed over and stops being dirty. It stands in for the render
   because the wait this feature is built around is a wait for exactly that - and none of
   these cases has a window to do it for them. */
void renderOnce()
{
    Pixels::getInstance().clearDirty();
}

/* A clock face on a lit display, with the supply up and the sequence idle - the state every
   case below starts from, and the one it has to be left in for the next. */
void startFromRunningClock()
{
    Display& display = Display::getInstance();

    display.resumeOutput();
    display.enable();
    Clock::getInstance().setTime(10u, 5u);
    display.show();
    renderOnce();
}

} // namespace

/* The point of the whole module: the port must not drop while the blanked frame is still on
   its way, or the strip is left holding a frame with no supply to latch it. */
void testSupplyCutWaitsForTheBlankedFrame()
{
    Power& power = Power::getInstance();
    Display& display = Display::getInstance();

    startFromRunningClock();
    expect(power.isSupplyOn(), "a running clock must have its strip powered");
    expect(isAnyOutputPixelLit(), "and must be putting something out");

    expect(power.switchSupplyOff() == E_OK, "a fitted switch must accept being asked");
    expect(!isAnyOutputPixelLit(), "the ask must darken the strip over the data line first");
    expect(power.isSupplyOn(), "and must not have cut the supply yet");
    expect(!display.isOutputSuspended(), "nor gated the line the blanked frame still has to go out on");

    /* The frame is only asked for at this point, not gone. A task here must find that out
       rather than take the darkness for done. */
    power.task();
    expect(power.isSupplyOn(), "a task before the blanked frame was rendered must keep the supply");

    renderOnce();
    power.task();
    expect(!power.isSupplyOn(), "once the frame is off the wire the supply must go");
    expect(display.isOutputSuspended(), "and the data line must be gated behind it");

    /* Left as it was found, or the next case starts on a dark strip. */
    expect(power.switchSupplyOn() == E_OK, "the supply must be askable back");
    power.task();
    startFromRunningClock();
}

/* Switching on is the same order backwards, and the reason is the same diode: the strip
   wants its rail before the first frame arrives, not with it. */
void testSupplyReturnsBeforeTheDataLine()
{
    Power& power = Power::getInstance();
    Display& display = Display::getInstance();

    startFromRunningClock();

    /* Down first, all the way, so that coming back up is what is being read. */
    power.switchSupplyOff();
    renderOnce();
    power.task();
    expect(!power.isSupplyOn() && display.isOutputSuspended(), "the strip must be off and gated");

    expect(power.switchSupplyOn() == E_OK, "a fitted switch must accept being asked back");
    expect(power.isSupplyOn(), "the supply must come up on the ask");
    expect(display.isOutputSuspended(), "and the data line must stay gated for that tick");
    expect(!isAnyOutputPixelLit(), "so nothing is put out while the rail is still settling");

    power.task();
    expect(!display.isOutputSuspended(), "the next task must release the line");
    expect(isAnyOutputPixelLit(), "and give the display back");
    expect(power.getState() == Power::STATE_SUPPLY_ON, "which leaves the sequence idle again");

    /* The strip forgot its registers with its supply, so the buffer has to be marked for a
       redraw rather than trusted to still be up there. */
    expect(display.isDirty(), "coming back must ask for a redraw, not assume the strip kept the frame");

    startFromRunningClock();
}

/* The two pairs of ids are not each other. Ids 3 and 4 darken a powered strip; these cut
   its supply. Collapsing them would lose the only one that saves any current. */
void testSupplyOffIsNotDisplayDisable()
{
    Power& power = Power::getInstance();
    Display& display = Display::getInstance();

    startFromRunningClock();

    display.disable();
    expect(!isAnyOutputPixelLit(), "disabling the display must darken it");
    expect(power.isSupplyOn(), "but must leave the strip powered");
    expect(!display.isOutputSuspended(), "and must not gate the data line");

    display.enable();
    expect(isAnyOutputPixelLit(), "and it must come back without the supply having moved");

    startFromRunningClock();
}

/* Asking twice, and changing one's mind halfway. The states in between are the ones a
   caller cannot see, so they are the ones that have to behave. */
void testSupplyRequestsAreReversible()
{
    Power& power = Power::getInstance();
    Display& display = Display::getInstance();

    startFromRunningClock();

    /* Asked for what is already the case. Honoured rather than refused: twice off means the
       same as once off, and a caller with no readback would have nothing to do with a no. */
    expect(power.switchSupplyOn() == E_OK, "asking for a supply that is already on must be honoured");
    expect(power.getState() == Power::STATE_SUPPLY_ON, "and must not start a sequence");

    /* Changed mind while the blanked frame is still going out. The supply never went, so
       there is nothing to bring up - only the darkness to undo. */
    power.switchSupplyOff();
    expect(power.getState() == Power::STATE_BLANKING, "the ask must put the sequence into blanking");
    expect(power.switchSupplyOn() == E_OK, "and must be revocable while it is still there");
    expect(power.getState() == Power::STATE_SUPPLY_ON, "which takes the sequence straight back");
    expect(power.isSupplyOn() && isAnyOutputPixelLit(), "with the strip powered and lit again");

    /* Changed mind the other way, while the rail is settling. The line is still gated there,
       so the strip is dark already and the port can go straight back down. */
    power.switchSupplyOff();
    renderOnce();
    power.task();
    power.switchSupplyOn();
    expect(power.getState() == Power::STATE_SETTLING, "the ask back must settle rather than finish");
    expect(power.switchSupplyOff() == E_OK, "and must itself be revocable");
    expect(!power.isSupplyOn(), "which drops the port again");
    expect(display.isOutputSuspended(), "with the line still gated, as it never stopped being");

    power.switchSupplyOn();
    power.task();
    startFromRunningClock();
}
