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
#include "Arduino.h"
#include <array>

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Scheduler configuration parameter */
/*
 * Milliseconds between two Task() calls, and thereby the unit every module's task
 * cycle counts in. A platform whose tick comes from a timer it cannot set freely
 * overrides this; GetTaskIntervalMs() is what the platform's tick source reads.
 */
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
    /* One per task that TriggerTasks() drives, and in the order it drives them. */
    enum class TaskIdType {
        kIlluminance,
        kTemperature,
        kDisplayManager,
        kAnimations,
        kCommunication,
        kOverlays,
        kText,
        kPersistence,
        kUptime,
        kNightSwitch,
        kColorCycle,
        kPower,
        kNumberOfTasks
    };
    static constexpr byte kNumberOfTasks{static_cast<byte>(TaskIdType::kNumberOfTasks)};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static constexpr byte kTaskIntervalMs{SCHEDULER_TASK_INTERVAL_MS};

    using RemainingTicksType = byte;
    std::array<RemainingTicksType, kNumberOfTasks> remaining_ticks_{};

    // functions
    bool IsDue(TaskIdType, byte);
    void TriggerTasks();

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    constexpr Scheduler() { }
    ~Scheduler() { }

	// get methods
    static constexpr byte GetTaskIntervalMs() { return kTaskIntervalMs; }

	// set methods

    // methods
    void Task();
    static constexpr byte ConvertSpeedToTaskCycle(byte speed) {
        if(speed == 0u) { return 0u; }
        return static_cast<byte>((UINT8_MAX - speed) + 1u);
    }
    static constexpr byte ConvertTaskCycleToSpeed(byte task_cycle) {
        if(task_cycle == 0u) { return 0u; }
        return static_cast<byte>((UINT8_MAX - task_cycle) + 1u);
    }

};

static_assert(Scheduler::ConvertSpeedToTaskCycle(0u) == 0u, "Speed zero must stop the task");
static_assert(Scheduler::ConvertSpeedToTaskCycle(1u) == UINT8_MAX, "Slowest speed must use the longest cycle");
static_assert(Scheduler::ConvertSpeedToTaskCycle(UINT8_MAX - 1u) == 2u, "Second-fastest speed must use cycle two");
static_assert(Scheduler::ConvertSpeedToTaskCycle(UINT8_MAX) == 1u, "Fastest speed must use cycle one");
static_assert(Scheduler::ConvertTaskCycleToSpeed(0u) == 0u, "Cycle zero must report speed zero");
static_assert(Scheduler::ConvertTaskCycleToSpeed(1u) == UINT8_MAX, "Cycle one must report the fastest speed");
static_assert(Scheduler::ConvertTaskCycleToSpeed(UINT8_MAX) == 1u, "Longest cycle must report the slowest speed");

#endif

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
