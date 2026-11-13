// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Cuts the strip's supply, and gives it back, in the order the hardware needs.
 *
 * A display told to be dark is still a strip drawing its quiescent current - about 1 mA per LED, so
 * some 110 mA for this display, all night. Switching the 5 V away is what that current costs
 * nothing, and this is the module that does it.
 *
 * It is not the display's enable and disable, which stay what they were: a brightness of zero, a
 * frame of black pixels, and a strip that is still powered. Both pairs exist and neither replaces
 * the other.
 *
 * Why this is a task and not two lines in the procedure that asks. Data must not reach DIN while
 * the supply is off - it pushes current into a dead rail through the LED's own protection diode -
 * so switching off has to blank the strip over the data line first and wait for that frame to be
 * gone. The frame is not gone when the procedure returns: nothing is transmitted until the
 * application's tick renders, and the transmission outlives that call as well. So the procedure can
 * only ask, and the waiting happens here, one tick at a time.
 *
 * Switching on is the same order read backwards. The supply goes up first and the data line is
 * released a tick later, rather than both at once, so the strip has its rail before the first frame
 * arrives.
 *
 * Blanking alone would not be enough to keep the line quiet afterwards. The clock keeps running
 * with the supply off, and every pixel written marks the buffer, so the next tick would transmit
 * again. That is why the display's output is gated rather than only darkened, and why the gate is
 * released before the strip is told it may light up again.
 */

#ifndef WORDCLOCK_SRC_POWER_POWER_H_
#define WORDCLOCK_SRC_POWER_POWER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "PowerSwitch.h"

/*!
 * \brief Scheduler ticks between two calls of Power::Task(): every tick.
 *
 * The two states this counts through are waits on the strip, and a frame takes about a third of
 * one tick - so anything slower would idle with the supply in mid-air for no reason. A tick that
 * has nothing to wait for costs one comparison.
 */
#define POWER_TASK_CYCLE                                1u

/*!
 * \brief The strip's supply: switches it off and on in the order the hardware needs.
 *
 * SwitchSupplyOff() and SwitchSupplyOn() only ask; Task() finishes the sequence on the ticks
 * after. Where no switch is fitted, both refuse.
 */
class Power
{
  public:
    /*! \brief The steps of switching the supply off and on. */
    enum class StateType {
        kSupplyOn, /*!< The supply is on and frames are going out - what a clock runs in. */
        /*!
         * \brief Asked to go off.
         *
         * The display is dark; what is left is to see the blanked frame off the wire before the
         * port drops.
         */
        kBlanking,
        kSupplyOff, /*!< The supply is off and the data line is gated. */
        /*!
         * \brief Asked to come on.
         *
         * The supply is up; the data line is released on the next task, which gives the strip a
         * tick to find its rail.
         */
        kSettling
    };

  private:
    static constexpr byte kTaskCycle{POWER_TASK_CYCLE};

    StateType state_{StateType::kSupplyOn};

    Power() { }
    ~Power() = default;

    static PowerSwitch& GetSwitch() { return PowerSwitch::GetInstance(); }

    static bool IsStripDarkAndIdle();
    void CutSupply();
    void ReleaseSupply();

  public:
    /*! \brief Returns the one supply. */
    static Power& GetInstance() {
        static Power singleton_instance;
        return singleton_instance;
    }

    /*! \brief Returns the scheduler ticks between two calls of Task(), POWER_TASK_CYCLE. */
    static constexpr byte GetTaskCycle() { return kTaskCycle; }

    /*! \brief Returns the step the sequence of switching is on. */
    StateType GetState() const { return state_; }

    /*!
     * \brief Returns whether the supply is on.
     *
     * What the status of the supply is, rather than which step the sequence is on: a switch
     * that is on its way off still has its supply up, and the two ticks it takes to get there
     * are nobody else's business.
     *
     * \return True while the supply is on, as the platform's PowerSwitch reports it.
     */
    static bool IsSupplyOn() { return GetSwitch().IsSupplyOn(); }

    /*!
     * \brief Returns whether there is a switch to ask at all.
     *
     * A board that was built without one has the high side MOSFET bridged, and then these
     * procedures have nothing to act on.
     *
     * \return True if the board has the switch.
     */
    static constexpr bool IsSwitchFitted() { return PowerSwitch::IsFitted(); }

    /*!
     * \brief Asks for the supply to come on.
     *
     * A supply that is off is switched on at once, and the next Task() releases the data line
     * and enables the display. A supply that is still being blanked was never cut, so the
     * display is only enabled again, at once.
     *
     * An ask that is already the case, or already under way, is honoured rather than refused,
     * because a caller that says "off" twice means the same thing both times.
     *
     * \return kEOk, or kENotOk if no switch is fitted; nothing changes then.
     */
    StdReturnType SwitchSupplyOn();

    /*!
     * \brief Asks for the supply to go off.
     *
     * A supply that is on is not cut at once: the display is disabled, which blanks the strip
     * over the data line, and Task() cuts the supply once that frame has left the wire. A supply
     * that is still coming up has its data line gated still, so it goes straight back off.
     *
     * An ask that is already the case, or already under way, is honoured rather than refused,
     * as for SwitchSupplyOn().
     *
     * \return kEOk, or kENotOk if no switch is fitted; nothing changes then.
     */
    StdReturnType SwitchSupplyOff();

    /*!
     * \brief Takes the sequence of switching one step further.
     *
     * While blanking, it waits until the display's buffer holds nothing unsent and no frame is
     * on the wire, then gates the data line and cuts the supply. While settling, it releases the
     * data line and enables the display. In the other two states it does nothing.
     */
    void Task();
};

#endif  // WORDCLOCK_SRC_POWER_POWER_H_
