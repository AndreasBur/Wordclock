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
/**     \file       Version.h
 *      \brief      What the firmware calls itself
 *
 *      \details    Reported by the status command, and there for one question: whether the
 *                  clock on the wall is running what was last built for it. It is kept by
 *                  hand rather than derived from the repository, because the two platforms
 *                  are built by different tools and neither of them has the working tree
 *                  when it runs the compiler.
 *
 *                  Hand-kept means it lies as soon as nobody raises it, so raise it when
 *                  flashing something that is worth telling apart from what came before.
 *
******************************************************************************************************************************************************/
#ifndef _VERSION_H_
#define _VERSION_H_

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Major, minor, patch. A build that wants to say more - a branch, a commit - overrides it
   from the outside rather than editing this line. */
#ifndef WORDCLOCK_VERSION
# define WORDCLOCK_VERSION                              "0.1.0"
#endif

#endif // _VERSION_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
