/******************************************************************************************************************************************************
 *  COPYRIGHT
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  \verbatim
 *  Copyright (c) Andreas Burnickl                                                                                                 All rights reserved.
 *
 *  \endverbatim
 *  ---------------------------------------------------------------------------------------------------------------------------------------------------
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       Power.h
 *      \brief      Cuts the strip's supply, and gives it back, in the order the hardware needs
 *
 *      \details    A display told to be dark is still a strip drawing its quiescent current -
 *                  about 1 mA per LED, so some 110 mA for this display, all night. Switching
 *                  the 5 V away is what that current costs nothing, and this is the module
 *                  that does it.
 *
 *                  It is not the display's enable and disable, which stay what they were: a
 *                  brightness of zero, a frame of black pixels, and a strip that is still
 *                  powered. Both pairs exist and neither replaces the other.
 *
 *                  Why this is a task and not two lines in the procedure that asks. Data must
 *                  not reach DIN while the supply is off - it pushes current into a dead rail
 *                  through the LED's own protection diode - so switching off has to blank the
 *                  strip over the data line first and wait for that frame to be gone. The
 *                  frame is not gone when the procedure returns: nothing is transmitted until
 *                  the application's tick renders, and the transmission outlives that call as
 *                  well. So the procedure can only ask, and the waiting happens here, one
 *                  tick at a time.
 *
 *                  Switching on is the same order read backwards. The supply goes up first
 *                  and the data line is released a tick later, rather than both at once, so
 *                  the strip has its rail before the first frame arrives.
 *
 *                  Blanking alone would not be enough to keep the line quiet afterwards. The
 *                  clock keeps running with the supply off, and every pixel written marks the
 *                  buffer, so the next tick would transmit again. That is why the display's
 *                  output is gated rather than only darkened, and why the gate is released
 *                  before the strip is told it may light up again.
 *
******************************************************************************************************************************************************/
#ifndef _POWER_H_
#define _POWER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "Display.h"
#include "PowerSwitch.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Power configuration parameter */
/* Every tick. The two states this counts through are waits on the strip, and a frame takes
   about a third of one tick - so anything slower would idle with the supply in mid-air for
   no reason. A tick that has nothing to wait for costs one comparison. */
#define POWER_TASK_CYCLE                                1u

/******************************************************************************************************************************************************
 *  C L A S S   P O W E R
******************************************************************************************************************************************************/
class Power
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    enum StateType {
        /* The supply is on and frames are going out - what a clock runs in. */
        STATE_SUPPLY_ON,
        /* Asked to go off. The display is dark; what is left is to see the blanked frame
           off the wire before the port drops. */
        STATE_BLANKING,
        /* The supply is off and the data line is gated. */
        STATE_SUPPLY_OFF,
        /* Asked to come on. The supply is up; the data line is released on the next task,
           which gives the strip a tick to find its rail. */
        STATE_SETTLING
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskCycle{POWER_TASK_CYCLE};

    StateType State{STATE_SUPPLY_ON};

    Power() { }
    ~Power() { }

    // functions
    static Display& getDisplay() { return Display::getInstance(); }
    static PowerSwitch& getSwitch() { return PowerSwitch::getInstance(); }

    /* Both waits, as one question. A buffer still marked dirty means the blanked frame has
       not been handed to the strip yet; a frame on the wire means it has and is still going
       out. Only when neither holds is the strip dark and idle.

       Anything still drawing - an animation that is running out, the clock changing minute -
       marks the buffer again and makes this wait another tick. That is left as it is rather
       than cut short: the display is dark from the first step onwards, so what a longer wait
       postpones is the current the switch saves, not the darkness somebody asked for. */
    static bool isStripDarkAndIdle() {
        if(getDisplay().isDirty()) { return false; }

        return !getDisplay().isFrameOnTheWire();
    }

    /* The gate before the port, so that no tick can slip a frame in between the two. */
    void cutSupply() {
        getDisplay().suspendOutput();
        getSwitch().switchSupplyOff();
        State = STATE_SUPPLY_OFF;
    }

    /* The port before the gate, for the same reason read the other way round. */
    void releaseSupply() {
        getDisplay().resumeOutput();
        getDisplay().enable();
        State = STATE_SUPPLY_ON;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    static Power& getInstance() {
        static Power SingletonInstance;
        return SingletonInstance;
    }

    // get methods
    static constexpr byte getTaskCycle() { return TaskCycle; }

    StateType getState() const { return State; }

    /* What the status of the supply is, rather than which step the sequence is on: a switch
       that is on its way off still has its supply up, and the two ticks it takes to get
       there are nobody else's business. */
    bool isSupplyOn() const { return getSwitch().isSupplyOn(); }

    /* Whether there is a switch to ask at all. A board that was built without one has the
       high side MOSFET bridged, and then these procedures have nothing to act on. */
    static constexpr bool isSwitchFitted() { return PowerSwitch::isFitted(); }

    // methods
    /* Asks for the supply. Answers E_NOT_OK only where there is no switch - an ask that is
       already the case, or already under way, is honoured rather than refused, because a
       caller that says "off" twice means the same thing both times. */
    StdReturnType switchSupplyOn() {
        if(!isSwitchFitted()) { return E_NOT_OK; }

        switch(State) {
            case STATE_SUPPLY_OFF :
                /* The port first; the data line follows on the next task. */
                getSwitch().switchSupplyOn();
                State = STATE_SETTLING;
                break;
            /* Never got as far as cutting it, so there is nothing to bring up and nothing
               to settle - only the darkness to undo. */
            case STATE_BLANKING :
                releaseSupply();
                break;
            case STATE_SUPPLY_ON :
            case STATE_SETTLING :
                break;
        }

        return E_OK;
    }

    StdReturnType switchSupplyOff() {
        if(!isSwitchFitted()) { return E_NOT_OK; }

        switch(State) {
            case STATE_SUPPLY_ON :
                /* Over the data line, not by cutting the supply: this is the frame the
                   strip has to be holding when its rail goes. */
                getDisplay().disable();
                State = STATE_BLANKING;
                break;
            /* Asked while the supply is coming up. The data line is still gated, so the
               strip is dark already and the port can go straight back down. */
            case STATE_SETTLING :
                getSwitch().switchSupplyOff();
                State = STATE_SUPPLY_OFF;
                break;
            case STATE_BLANKING :
            case STATE_SUPPLY_OFF :
                break;
        }

        return E_OK;
    }

    void task() {
        switch(State) {
            case STATE_BLANKING :
                if(isStripDarkAndIdle()) { cutSupply(); }
                break;
            case STATE_SETTLING :
                releaseSupply();
                break;
            case STATE_SUPPLY_ON :
            case STATE_SUPPLY_OFF :
                break;
        }
    }
};

#endif // _POWER_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
