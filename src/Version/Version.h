// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the firmware calls itself.
 *
 * Reported by the status command, and there for one question: whether the clock on the wall is
 * running what was last built for it. It is kept by hand rather than derived from the repository,
 * because the two platforms are built by different tools and neither of them has the working tree
 * when it runs the compiler.
 *
 * Hand-kept means it lies as soon as nobody raises it, so raise it when flashing something that is
 * worth telling apart from what came before.
 */

#ifndef WORDCLOCK_SRC_VERSION_VERSION_H_
#define WORDCLOCK_SRC_VERSION_VERSION_H_

#ifndef WORDCLOCK_VERSION
/*!
 * \brief The version of the firmware, as a string: major, minor, patch.
 *
 * A build that wants to say more - a branch, a commit - overrides it from the outside rather than
 * editing this line.
 */
#define WORDCLOCK_VERSION "0.1.0"
#endif

#endif  // WORDCLOCK_SRC_VERSION_VERSION_H_
