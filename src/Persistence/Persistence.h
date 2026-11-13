// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Keeps the configuration across a restart.
 *
 * Settings used to live in RAM only, which a simulator never reveals - it starts fresh anyway - but
 * a wall clock does: configure it, unplug it, and it came back on its defaults.
 *
 * What is stored is gathered from the modules that own it rather than duplicated here, and it is
 * written when it differs from what was written last. That is the same shape as DisplayManager's
 * word-set latch, and it is chosen for the same reason: there is no way to forget to mark something
 * dirty, because nothing marks anything. The brightness automatic cannot trigger a write either,
 * since what it computes is never stored back into the setting it computes from.
 *
 * Not stored, and deliberately: the time and the date, which come from the network.
 *
 * The overlay configuration is stored, its text included. The text is kept at its full buffer size
 * rather than length-prefixed: a variable field would have bought some forty bytes at the price of
 * the one property everything here rests on - that the settings are a single fixed block, which
 * memcmp compares and the checksum walks without knowing what is in it.
 *
 * The format itself - the struct, its version and its checksum - lives in the source, so that
 * adding a field cannot silently change what the platforms are compiled against.
 */

#ifndef WORDCLOCK_SRC_PERSISTENCE_PERSISTENCE_H_
#define WORDCLOCK_SRC_PERSISTENCE_PERSISTENCE_H_

#include "Arduino.h"
#include "StandardTypes.h"

/*!
 * \brief The cycle of the persistence's task, in scheduler ticks, so 200 * 10 ms = 2 s between
 *        checks.
 *
 * It is the write that is being rationed, not the check: a burst of commands within one period
 * collapses into a single write, and a period in which nothing changed costs a struct comparison.
 * The scheduler counts cycles in a byte, so this cannot be raised past 255.
 */
#define PERSISTENCE_TASK_CYCLE 200u

/*!
 * \brief Keeps the configuration across a restart: restores it at the start and writes it to the
 *        store whenever it changed.
 *
 * The settings are read from the modules that own them, not kept here; the time and the date are
 * not stored.
 */
class Persistence {
 private:
  static constexpr byte kTaskCycle{PERSISTENCE_TASK_CYCLE};

  Persistence() {}
  ~Persistence() = default;

 public:
  /*! \brief Returns the one persistence of the firmware. */
  static Persistence& GetInstance() {
    static Persistence singleton_instance;
    return singleton_instance;
  }

  /*! \brief Returns the scheduler ticks between two calls of Task(), PERSISTENCE_TASK_CYCLE. */
  static constexpr byte GetTaskCycle() { return kTaskCycle; }

  /*!
   * \brief Restores the stored configuration, if there is a usable one.
   *
   * Called once by the platform's entry point, after the display is up: the settings it restores
   * reach the display through the modules' own setters.
   *
   * A blob that cannot be used is thrown away rather than left in place: it would otherwise be
   * read and rejected on every start, and the store has nothing to offer that a fresh write does
   * not. Nothing stored at all takes the same path, where clearing is a no-op.
   *
   * What is remembered as "last written" afterwards is what the modules actually hold, not what
   * the blob said. A value one of them rejected therefore shows up as a difference on the next
   * task and gets corrected in the store, instead of being read and rejected forever.
   *
   * \return kEOk if a stored configuration was applied, or kENotOk if the store could not be read
   *         or holds no blob of this format, with its magic, version and checksum; the modules then
   *         keep the settings they have, and the store is emptied.
   */
  static StdReturnType Load();

  /*!
   * \brief Writes the configuration if it changed since the last write.
   *
   * The scheduler calls it every PERSISTENCE_TASK_CYCLE ticks. A failed write is not remembered
   * as written, so the next task tries again rather than treating the change as stored.
   */
  static void Task();

  /*!
   * \brief Writes the configuration now.
   *
   * Now rather than within the next period. What that period is there for is rationing a burst
   * of commands, which is exactly what a caller asking for this is not doing - it is about to
   * pull the plug.
   *
   * Unconditionally, unlike Task(): "nothing changed since the last write" is a state the caller
   * cannot check for itself.
   *
   * \return kEOk if the store took the configuration, or kENotOk if it did not; Task() then still
   *         compares against the configuration that was last written.
   */
  static StdReturnType Save();

  /*!
   * \brief Puts the configuration back to what a clock starts with.
   *
   * Every module back to what it starts with, and the store emptied. Emptied rather than written
   * with the defaults: an empty store is what a clock that was never configured has, and Load()
   * already treats the two the same.
   *
   * Each module answers for its own defaults, the same way the configuration is read from each
   * of them rather than from a copy kept here - so a setting added to one of them is reset by the
   * module that owns it and not forgotten in a list over here.
   *
   * What is remembered as "last written" afterwards is what the modules now hold, so the next
   * task does not write the defaults straight back into the store that was just emptied.
   *
   * \return kEOk if the store was emptied, or kENotOk if it was not; the modules are reset all the
   *         same.
   */
  static StdReturnType Reset();
};

#endif  // WORDCLOCK_SRC_PERSISTENCE_PERSISTENCE_H_
