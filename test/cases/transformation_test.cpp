// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

// The shifts of the display, and the roll animation that is their one user in every direction.

#include "Transformation.h"

#include <array>
#include <cstddef>

#include "Animations.h"
#include "Clock.h"
#include "Display.h"
#include "test/cases.h"
#include "test/check.h"

namespace {
using GridType = std::array<std::array<bool, DISPLAY_NUMBER_OF_COLUMNS>, DISPLAY_NUMBER_OF_ROWS>;

constexpr byte kRows{DISPLAY_NUMBER_OF_ROWS};
constexpr byte kColumns{DISPLAY_NUMBER_OF_COLUMNS};

// The models below count in int, because a step can be negative; an index they reach is not.
std::size_t ToIndex(int value) { return static_cast<std::size_t>(value); }
bool& At(GridType& grid, int row, int column) { return grid[ToIndex(row)][ToIndex(column)]; }
bool At(GridType const& grid, int row, int column) { return grid[ToIndex(row)][ToIndex(column)]; }

// Whether each pixel is lit, which is all a shift moves: a pixel is written in the colour the
// display writes now, so its colour says nothing about where it came from.
GridType ReadGrid() {
  GridType grid{};

  for (byte row = 0u; row < kRows; row++) {
    for (byte column = 0u; column < kColumns; column++) {
      grid[row][column] = Display::GetInstance().GetPixel(column, row);
    }
  }
  return grid;
}

void WriteGrid(GridType const& grid) {
  Display& display = Display::GetInstance();

  display.Clear();
  for (byte row = 0u; row < kRows; row++) {
    for (byte column = 0u; column < kColumns; column++) {
      if (grid[row][column]) {
        display.SetPixel(column, row);
      }
    }
  }
}

/*
 * A picture no shift maps onto itself: the four corners, a pixel off each edge's centre, and
 * the diagonal of a corner. Rotation shows in the corners, a lost row or column in the edges.
 */
GridType Pattern() {
  GridType grid{};

  grid[0][0] = true;
  grid[0][kColumns - 1u] = true;
  grid[kRows - 1u][0] = true;
  grid[kRows - 1u][kColumns - 1u] = true;
  grid[0][3] = true;
  grid[4][0] = true;
  grid[kRows - 1u][6] = true;
  grid[2][kColumns - 1u] = true;
  for (byte step = 1u; step < 4u; step++) {
    grid[step][step] = true;
  }
  return grid;
}

// The model each shift is checked against: a pixel taken from `step` rows and columns away,
// and from the opposite edge when rotating, or dark when not.
GridType Shifted(GridType const& grid, int row_step, int column_step, bool rotate) {
  GridType shifted{};

  for (int row = 0; row < kRows; row++) {
    for (int column = 0; column < kColumns; column++) {
      int from_row = row - row_step;
      int from_column = column - column_step;
      bool const inside =
          (from_row >= 0) && (from_row < kRows) && (from_column >= 0) && (from_column < kColumns);

      if (!inside && !rotate) {
        continue;
      }
      from_row = (from_row + kRows) % kRows;
      from_column = (from_column + kColumns) % kColumns;
      At(shifted, row, column) = At(grid, from_row, from_column);
    }
  }
  return shifted;
}

// The same, for one row or one column only; the rest of the grid stays where it was.
GridType ShiftedLine(GridType const& grid, int row, int column, int row_step, int column_step,
                     bool rotate) {
  GridType const whole = Shifted(grid, row_step, column_step, rotate);
  GridType shifted = grid;

  if (row >= 0) {
    shifted[ToIndex(row)] = whole[ToIndex(row)];
  }
  if (column >= 0) {
    for (byte line = 0u; line < kRows; line++) {
      At(shifted, line, column) = At(whole, line, column);
    }
  }
  return shifted;
}

enum class RollType { kRight, kLeft, kDown, kUp, kNumberOfRolls };
constexpr byte kNumberOfRolls{static_cast<byte>(RollType::kNumberOfRolls)};

/*
 * What the display shows after `steps` steps of a roll in `roll`: the previous time moved on
 * by that many rows or columns, and the edge it left filled with as much of the new time as
 * has come in, the part of it that ends up furthest from that edge first.
 */
bool RolledPixel(GridType const& previous, GridType const& next, RollType roll, int steps, int row,
                 int column) {
  switch (roll) {
    case RollType::kRight:
      return (column < steps) ? At(next, row, kColumns - steps + column)
                              : At(previous, row, column - steps);
    case RollType::kLeft:
      return (column >= kColumns - steps) ? At(next, row, column - (kColumns - steps))
                                          : At(previous, row, column + steps);
    case RollType::kDown:
      return (row < steps) ? At(next, kRows - steps + row, column)
                           : At(previous, row - steps, column);
    default:
      return (row >= kRows - steps) ? At(next, row - (kRows - steps), column)
                                    : At(previous, row + steps, column);
  }
}

GridType Rolled(GridType const& previous, GridType const& next, RollType roll, int steps) {
  GridType rolled{};

  for (int row = 0; row < kRows; row++) {
    for (int column = 0; column < kColumns; column++) {
      At(rolled, row, column) = RolledPixel(previous, next, roll, steps, row, column);
    }
  }
  return rolled;
}

int NumberOfSteps(RollType roll) {
  return (roll == RollType::kRight || roll == RollType::kLeft) ? kColumns : kRows;
}
}  // namespace

void TestShiftsMoveEveryPixelByOne() {
  Transformation transformation;
  GridType const pattern = Pattern();

  for (bool const rotate : {false, true}) {
    WriteGrid(pattern);
    Expect(
        transformation.ShiftRight(rotate) == kEOk && ReadGrid() == Shifted(pattern, 0, 1, rotate),
        "a shift to the right must move every pixel one column");
    WriteGrid(pattern);
    Expect(
        transformation.ShiftLeft(rotate) == kEOk && ReadGrid() == Shifted(pattern, 0, -1, rotate),
        "a shift to the left must move every pixel one column");
    WriteGrid(pattern);
    Expect(transformation.ShiftDown(rotate) == kEOk && ReadGrid() == Shifted(pattern, 1, 0, rotate),
           "a shift down must move every pixel one row");
    WriteGrid(pattern);
    Expect(transformation.ShiftUp(rotate) == kEOk && ReadGrid() == Shifted(pattern, -1, 0, rotate),
           "a shift up must move every pixel one row");

    // The first and the last line, where the edge is, and one in between.
    for (byte const row : {byte{0u}, byte{4u}, byte{kRows - 1u}}) {
      WriteGrid(pattern);
      Expect(transformation.ShiftRowRight(row, rotate) == kEOk &&
                 ReadGrid() == ShiftedLine(pattern, row, -1, 0, 1, rotate),
             "a row must shift right on its own");
      WriteGrid(pattern);
      Expect(transformation.ShiftRowLeft(row, rotate) == kEOk &&
                 ReadGrid() == ShiftedLine(pattern, row, -1, 0, -1, rotate),
             "a row must shift left on its own");
    }
    for (byte const column : {byte{0u}, byte{3u}, byte{kColumns - 1u}}) {
      WriteGrid(pattern);
      Expect(transformation.ShiftColumnDown(column, rotate) == kEOk &&
                 ReadGrid() == ShiftedLine(pattern, -1, column, 1, 0, rotate),
             "a column must shift down on its own");
      WriteGrid(pattern);
      Expect(transformation.ShiftColumnUp(column, rotate) == kEOk &&
                 ReadGrid() == ShiftedLine(pattern, -1, column, -1, 0, rotate),
             "a column must shift up on its own");
    }
  }

  WriteGrid(pattern);
  Expect(transformation.ShiftRowRight(kRows) == kENotOk &&
             transformation.ShiftRowLeft(kRows) == kENotOk,
         "a row off the grid must be refused");
  Expect(transformation.ShiftColumnDown(kColumns) == kENotOk &&
             transformation.ShiftColumnUp(kColumns) == kENotOk,
         "and so must a column");
  Expect(ReadGrid() == pattern, "with nothing moved");

  Display::GetInstance().Clear();
}

#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
namespace {

// One frame per task, the last of them the one that finishes: a roll of n steps takes n + 1
// tasks. The bound is a runaway's, not a measurement.
using FramesType = std::array<GridType, kColumns + 2u>;

// Runs the started animation to its end and returns how many frames it showed.
std::size_t RecordFrames(Animations& animations, FramesType& frames) {
  std::size_t number_of_frames = 0u;
  while (animations.GetState() == Animations::StateType::kPending &&
         number_of_frames < frames.size()) {
    animations.Task(true);
    frames[number_of_frames++] = ReadGrid();
  }
  return number_of_frames;
}

// Whether the frames are, step by step, those of a roll in `roll` from `previous` to `next`.
bool ShowsRoll(FramesType const& frames, std::size_t number_of_frames, GridType const& previous,
               GridType const& next, RollType roll) {
  if (number_of_frames != static_cast<std::size_t>(NumberOfSteps(roll)) + 1u) {
    return false;
  }
  for (int step = 1; step <= NumberOfSteps(roll); step++) {
    if (frames[ToIndex(step - 1)] != Rolled(previous, next, roll, step)) {
      return false;
    }
  }
  return true;
}

}  // namespace
#endif

/*
 * The roll animation is what reaches the vertical shifts in the firmware, and which of its four
 * directions a word change takes is decided by the time. A single time - all the case for every
 * animation uses - is a single direction, so this one goes through the five-minute steps of
 * twelve hours and checks every step of every roll against the frame it should show. Which
 * direction a roll took is read off the frames rather than computed: the case is about the
 * rolling, and it should not break when the choice of direction is made differently.
 */
void TestRollAnimationRollsEveryWay() {
#if (ANIMATIONS_SUPPORT_ROLL == STD_ON)
  Animations& animations = Animations::GetInstance();
  Clock::ModeType const clock_mode_before = Clock::GetInstance().GetMode();
  std::array<bool, kNumberOfRolls> seen{};

  Clock::GetInstance().SetMode(Clock::ModeType::kWessi);
  animations.SetMode(Animations::ModeType::kFixed);

  for (byte hour = 0u; hour < 12u; hour++) {
    for (byte minute = 0u; minute < 60u; minute += 5u) {
      DrawClockFace(hour, minute);
      GridType const next = ReadGrid();
      DrawClockFace(hour, static_cast<byte>((minute + 55u) % 60u));
      GridType const previous = ReadGrid();

      animations.SetAnimation(Animations::AnimationIdType::kRoll);
      Expect(animations.SetTime(hour, minute) == kEOk, "the roll must start on a word change");

      FramesType frames{};
      std::size_t const number_of_frames = RecordFrames(animations, frames);

      bool any = false;
      for (byte roll = 0u; roll < kNumberOfRolls; roll++) {
        if (ShowsRoll(frames, number_of_frames, previous, next, static_cast<RollType>(roll))) {
          any = true;
          seen[roll] = true;
        }
      }
      Expect(any,
             "every frame of a roll must be the previous time rolled on and the new one rolled in");
      Expect(ReadGrid() == next, "and the roll must end on the new time");
    }
  }

  Expect(seen[static_cast<byte>(RollType::kRight)] && seen[static_cast<byte>(RollType::kLeft)],
         "the roll must go right and left on some word change");
  Expect(seen[static_cast<byte>(RollType::kDown)] && seen[static_cast<byte>(RollType::kUp)],
         "and down and up on some other");

  animations.SetAnimation(Animations::AnimationIdType::kNone);
  Clock::GetInstance().SetMode(clock_mode_before);
  Display::GetInstance().Clear();
#endif
}
