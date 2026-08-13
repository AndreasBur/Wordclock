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
/**     \file       WordclockSerial.cpp
 *      \brief      The port Serial is bound to: the UART plus an injected line source
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "Arduino.h"

#ifndef WORDCLOCK_ARDUINO_SHIM
# error "WordclockSerial: the core's Arduino.h was included instead of this platform's. Put platform/esp32/include ahead of the framework on the include path, or the web console will be silently mute."
#endif

#include <stdio.h>
#include <string.h>

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  begin()
******************************************************************************************************************************************************/
void WordclockSerial::begin(unsigned long BaudRate)
{
    WordclockPlatform::hardwarePortBegin(BaudRate);
} /* begin */


/******************************************************************************************************************************************************
  inject()
******************************************************************************************************************************************************/
/*! \brief          Hands characters to the port from another task
 *  \details        Written by the producer only, which is what keeps this free of a lock -
 *                  see the note on the indices in the header.
 *
 *                  A full buffer drops the remainder rather than overwriting what has not
 *                  been read yet: the reader is mid-command, and overwriting would splice
 *                  two commands into one that parses as neither.
 *
 *  \return         E_OK if everything was taken
******************************************************************************************************************************************************/
StdReturnType WordclockSerial::inject(const char* Characters, size_t Length)
{
    if((Characters == nullptr) || (Length == 0u)) { return E_NOT_OK; }

    size_t Head = InjectHead.load(std::memory_order_relaxed);

    for(size_t Index = 0u; Index < Length; Index++) {
        const size_t Next = nextIndex(Head);

        /* One slot stays empty, which is what tells a full buffer from an empty one. */
        if(Next == InjectTail.load(std::memory_order_acquire)) { return E_NOT_OK; }

        InjectBuffer[Head] = Characters[Index];
        Head = Next;
        /* Published after the character, so the reader cannot see the index move ahead of
           the data it points at. */
        InjectHead.store(Head, std::memory_order_release);
    }

    return E_OK;
} /* inject */


/******************************************************************************************************************************************************
  available()
******************************************************************************************************************************************************/
int WordclockSerial::available() const
{
    const int FromPort = WordclockPlatform::hardwarePort().available();

    if(FromPort > 0) { return FromPort; }

    return isInjectEmpty() ? 0 : 1;
} /* available */


/******************************************************************************************************************************************************
  read()
******************************************************************************************************************************************************/
/*! \brief          Takes the next character, wire before network
 *  \details        The UART is served first so that a console on the wire keeps working
 *                  while a network client is busy - the wire is where one debugs a clock
 *                  whose network is the problem.
 *
 *  \return         the character, or -1 when there is none
******************************************************************************************************************************************************/
int WordclockSerial::read()
{
    Stream& Port = WordclockPlatform::hardwarePort();

    if(Port.available() > 0) { return Port.read(); }
    if(!isInjectEmpty()) { return static_cast<unsigned char>(readInjected()); }

    return -1;
} /* read */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(const char* Text)
{
    if(Text == nullptr) { return; }

    for(const char* Character = Text; *Character != '\0'; Character++) { print(*Character); }
} /* print */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(const __FlashStringHelper* Text)
{
    /* On this target flash is read through the same address space, so what F() wrapped can
       be read as an ordinary string. */
    print(reinterpret_cast<const char*>(Text));
} /* print */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(int Number)
{
    char Digits[12]{};

    snprintf(Digits, sizeof(Digits), "%d", Number);
    print(Digits);
} /* print */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
/*! \brief          Writes one character, to the wire and into the line being collected
 *  \details        A newline is treated as the end of the line rather than as content,
 *                  the same way the simulator's shim does it - the firmware ends some of
 *                  its output with print('\n') instead of println(), and a line sink that
 *                  did not notice would run two answers together.
******************************************************************************************************************************************************/
void WordclockSerial::print(char Character)
{
    if(Character == '\n') { println(); return; }

    WordclockPlatform::hardwarePort().write(static_cast<uint8_t>(Character));
    appendToLine(Character);
} /* print */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println()
{
    WordclockPlatform::hardwarePort().write(static_cast<uint8_t>('\r'));
    WordclockPlatform::hardwarePort().write(static_cast<uint8_t>('\n'));

    flushLine();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(const char* Text)
{
    print(Text);
    println();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(int Number)
{
    print(Number);
    println();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(const __FlashStringHelper* Text)
{
    print(Text);
    println();
} /* println */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  readInjected()
******************************************************************************************************************************************************/
char WordclockSerial::readInjected()
{
    const size_t Tail = InjectTail.load(std::memory_order_relaxed);
    const char Character = InjectBuffer[Tail];

    /* Published after the character was taken, so the producer cannot refill the slot
       while it is still being read. */
    InjectTail.store(nextIndex(Tail), std::memory_order_release);

    return Character;
} /* readInjected */


/******************************************************************************************************************************************************
  appendToLine()
******************************************************************************************************************************************************/
/*! \brief          Collects the line the sink will be handed
 *  \details        A line that outgrows the buffer is flushed early rather than cut: the
 *                  client then sees the answer split over two frames, which is readable,
 *                  where a truncated line would silently lose its end.
******************************************************************************************************************************************************/
void WordclockSerial::appendToLine(char Character)
{
    if(LineUsed >= (LineLength - 1u)) { flushLine(); }

    LineBuffer[LineUsed++] = Character;
} /* appendToLine */


/******************************************************************************************************************************************************
  flushLine()
******************************************************************************************************************************************************/
/*! \brief          Hands the collected line to whoever registered for it
 *  \details        An empty line is dropped, because println() is also used to terminate
 *                  output that print() already ended with a newline - passing that on
 *                  would send a blank frame after every answer.
******************************************************************************************************************************************************/
void WordclockSerial::flushLine()
{
    if(LineUsed == 0u) { return; }

    LineBuffer[LineUsed] = '\0';
    LineUsed = 0u;

    if(LineSink != nullptr) { LineSink(LineBuffer); }
} /* flushLine */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
