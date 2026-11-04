// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Shifts of the whole display or of one row or column, optionally rotating.
 */

#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

#include "StandardTypes.h"
#include "Arduino.h"

/* Transformation configuration parameter */

/* Transformation parameter */

class Transformation
{
  public:

  private:

  public:
    constexpr Transformation() { }
    ~Transformation() { }

    // get methods

    // set methods

    // methods
    static StdReturnType ShiftLeft(bool = false);
    static StdReturnType ShiftRight(bool = false);
    static StdReturnType ShiftUp(bool = false);
    static StdReturnType ShiftDown(bool = false);

    static StdReturnType ShiftRowLeft(byte, bool = false);
    static StdReturnType ShiftRowRight(byte, bool = false);
    static StdReturnType ShiftColumnUp(byte, bool = false);
    static StdReturnType ShiftColumnDown(byte, bool = false);
};

#endif
