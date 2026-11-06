// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The scheduler: calls every module's task at its own multiple of the tick.
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "Arduino.h"
#include <array>

/*!
 * \brief Milliseconds between two Task() calls, and thereby the unit every module's task cycle
 *        counts in.
 *
 * A platform whose tick comes from a timer it cannot set freely overrides this;
 * GetTaskIntervalMs() is what the platform's tick source reads.
 */
#define SCHEDULER_TASK_INTERVAL_MS          10u

/*!
 * \brief The scheduler: calls every module's task at its own multiple of the tick.
 *
 * The platform calls Task() once per tick, every SCHEDULER_TASK_INTERVAL_MS milliseconds.
 */
class Scheduler
{
  public:
    /*! \brief The tasks the scheduler drives, one per module, in the order it drives them. */
    enum class TaskIdType : byte {
        kIlluminance,    /*!< The task of Illuminance. */
        kTemperature,    /*!< The task of Temperature. */
        kDisplayManager, /*!< The task of DisplayManager. */
        kAnimations,     /*!< The task of Animations. */
        kCommunication,  /*!< The task of Communication. */
        kOverlays,       /*!< The task of Overlays. */
        kText,           /*!< The task of Text. */
        kPersistence,    /*!< The task of Persistence. */
        kUptime,         /*!< The task of Uptime. */
        kNightSwitch,    /*!< The task of NightSwitch. */
        kColorCycle,     /*!< The task of ColorCycle. */
        kPower,          /*!< The task of Power. */
        kNumberOfTasks   /*!< The number of tasks; not a task itself. */
    };

    /*! \brief The number of tasks the scheduler drives. */
    static constexpr byte kNumberOfTasks{static_cast<byte>(TaskIdType::kNumberOfTasks)};

  private:
    static constexpr byte kTaskIntervalMs{SCHEDULER_TASK_INTERVAL_MS};

    using RemainingTicksType = byte;
    std::array<RemainingTicksType, kNumberOfTasks> remaining_ticks_{};

    bool IsDue(TaskIdType task_id, byte cycle);
    void TriggerTasks();

  public:
    /*! \brief Makes a scheduler on whose first tick every task that is switched on is due. */
    constexpr Scheduler() { }

    /*! \brief Destroys the scheduler. */
    ~Scheduler() = default;

    /*! \brief Returns the milliseconds between two Task() calls, SCHEDULER_TASK_INTERVAL_MS. */
    static constexpr byte GetTaskIntervalMs() { return kTaskIntervalMs; }

    /*!
     * \brief Runs every module's task that is due on this tick, in the order of TaskIdType.
     *
     * A task is due on the first tick, and then on every n-th tick, n being the cycle its
     * module's `GetTaskCycle()` returns; a cycle that changed takes effect when the task next
     * runs. A cycle of 0 switches the task off, and what was left of its period with it, so that
     * switching it back on runs it right away instead of somewhere inside a period left over from
     * before.
     */
    void Task();

    /*!
     * \brief Converts a speed into a task cycle: the higher the speed, the shorter the cycle.
     *
     * \param[in] speed The speed, 1 the slowest to 255 the fastest, or 0 to stop the task.
     * \return The task cycle in ticks, 255 for a speed of 1 down to 1 for a speed of 255, or 0 for
     *         a speed of 0.
     */
    static constexpr byte ConvertSpeedToTaskCycle(byte speed) {
        if(speed == 0u) { return 0u; }
        return static_cast<byte>((UINT8_MAX - speed) + 1u);
    }

    /*!
     * \brief Converts a task cycle into a speed, the reverse of ConvertSpeedToTaskCycle().
     *
     * \param[in] task_cycle The task cycle in ticks, or 0 for a stopped task.
     * \return The speed, 255 for a cycle of 1 down to 1 for a cycle of 255, or 0 for a cycle of 0.
     */
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
