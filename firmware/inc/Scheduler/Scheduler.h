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
/**     \file       Scheduler.h
 *      \brief
 *
 *      \details
 *
******************************************************************************************************************************************************/
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"
#include "functional"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Scheduler configuration parameter */
/* Milliseconds between two task() calls, and thereby the unit every module's task
   cycle counts in. A platform whose tick comes from a timer it cannot set freely
   overrides this; getTaskIntervalMs() is what the platform's tick source reads. */
#define SCHEDULER_TASK_INTERVAL_MS          10u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   S C H E D U L E R
******************************************************************************************************************************************************/
class Scheduler
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* One per task that triggerTasks() drives, and in the order it drives them. */
    enum TaskIdType {
        TASK_ID_ILLUMINANCE,
        TASK_ID_DISPLAY_MANAGER,
        TASK_ID_ANIMATIONS,
        TASK_ID_COMMUNICATION,
        TASK_ID_OVERLAYS,
        TASK_ID_TEXT,
        TASK_ID_NUMBER_OF_TASKS
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte TaskIntervalMs{SCHEDULER_TASK_INTERVAL_MS};

    using RemainingTicksType = byte;
    std::array<RemainingTicksType, TASK_ID_NUMBER_OF_TASKS> RemainingTicks{};

    // functions
    bool isDue(TaskIdType, byte);
    void triggerTasks();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Scheduler() { }
    ~Scheduler() { }

	// get methods
    static constexpr byte getTaskIntervalMs() { return TaskIntervalMs; }

	// set methods

    // methods
    void task();
    static constexpr byte convertSpeedToTaskCycle(byte Speed) {
        if(Speed == 0u) { return 0u; }
        return static_cast<byte>((UINT8_MAX - Speed) + 1u);
    }
    static constexpr byte convertTaskCycleToSpeed(byte TaskCycle) {
        if(TaskCycle == 0u) { return 0u; }
        return static_cast<byte>((UINT8_MAX - TaskCycle) + 1u);
    }

};

static_assert(Scheduler::convertSpeedToTaskCycle(0u) == 0u, "Speed zero must stop the task");
static_assert(Scheduler::convertSpeedToTaskCycle(1u) == UINT8_MAX, "Slowest speed must use the longest cycle");
static_assert(Scheduler::convertSpeedToTaskCycle(UINT8_MAX - 1u) == 2u, "Second-fastest speed must use cycle two");
static_assert(Scheduler::convertSpeedToTaskCycle(UINT8_MAX) == 1u, "Fastest speed must use cycle one");
static_assert(Scheduler::convertTaskCycleToSpeed(0u) == 0u, "Cycle zero must report speed zero");
static_assert(Scheduler::convertTaskCycleToSpeed(1u) == UINT8_MAX, "Cycle one must report the fastest speed");
static_assert(Scheduler::convertTaskCycleToSpeed(UINT8_MAX) == 1u, "Longest cycle must report the slowest speed");

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
