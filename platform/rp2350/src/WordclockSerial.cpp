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

/*
 * Instance methods, though some of them read no member: they are the interface the
 * firmware core calls through GetInstance(), and every backend - the simulator's
 * stand-ins included - offers it that way. Whether this backend's implementation
 * happens to need a member is its own detail, and answering the check would carry
 * that detail into the interface.
 */
// NOLINTBEGIN(readability-convert-member-functions-to-static)
/******************************************************************************************************************************************************
  Begin()
******************************************************************************************************************************************************/
void WordclockSerial::begin(unsigned long baud_rate)
{
    wordclock_platform::HardwarePortBegin(baud_rate);
} /* Begin */


/******************************************************************************************************************************************************
  Inject()
******************************************************************************************************************************************************/
/*! \brief          Hands characters to the port from another task
 *  \details        Written by the producer only, which is what keeps this free of a lock -
 *                  see the note on the indices in the header.
 *
 *                  A full buffer drops the remainder rather than overwriting what has not
 *                  been read yet: the reader is mid-command, and overwriting would splice
 *                  two commands into one that parses as neither.
 *
 *  \return         kEOk if everything was taken
******************************************************************************************************************************************************/
StdReturnType WordclockSerial::Inject(const char* characters, size_t length)
{
    if((characters == nullptr) || (length == 0u)) { return kENotOk; }

    size_t head = inject_head_.load(std::memory_order_relaxed);

    for(size_t index = 0u; index < length; index++) {
        const size_t next = NextIndex(head);

        /* One slot stays empty, which is what tells a full buffer from an empty one. */
        if(next == inject_tail_.load(std::memory_order_acquire)) { return kENotOk; }

        inject_buffer_[head] = characters[index];
        head = next;
        // Published after the character, so the reader cannot see the index move ahead of
        // the data it points at.
        inject_head_.store(head, std::memory_order_release);
    }

    return kEOk;
} /* Inject */


/******************************************************************************************************************************************************
  available()
******************************************************************************************************************************************************/
int WordclockSerial::available() const
{
    const int from_port = wordclock_platform::HardwarePort().available();

    if(from_port > 0) { return from_port; }

    return IsInjectEmpty() ? 0 : 1;
} /* available */


/******************************************************************************************************************************************************
  Read()
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
    Stream& port = wordclock_platform::HardwarePort();

    if(port.available() > 0) { return port.read(); }
    if(!IsInjectEmpty()) { return static_cast<unsigned char>(ReadInjected()); }

    return -1;
} /* Read */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(const char* text)
{
    if(text == nullptr) { return; }

    for(const char* character = text; *character != '\0'; character++) { print(*character); }
} /* print */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(const __FlashStringHelper* text)
{
    // On this target flash is read through the same address space, so what F() wrapped can
    // be read as an ordinary string.
    print(reinterpret_cast<const char*>(text));
} /* print */


/******************************************************************************************************************************************************
  print()
******************************************************************************************************************************************************/
void WordclockSerial::print(int number)
{
    char digits[12]{};

    /* Ten digits and a sign are the most an int can print to; nothing to truncate. */
    (void)snprintf(digits, sizeof(digits), "%d", number);
    print(digits);
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
void WordclockSerial::print(char character)
{
    if(character == '\n') { println(); return; }

    wordclock_platform::HardwarePort().write(static_cast<uint8_t>(character));
    AppendToLine(character);
} /* print */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println()
{
    wordclock_platform::HardwarePort().write(static_cast<uint8_t>('\r'));
    wordclock_platform::HardwarePort().write(static_cast<uint8_t>('\n'));

    FlushLine();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(const char* text)
{
    print(text);
    println();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(int number)
{
    print(number);
    println();
} /* println */


/******************************************************************************************************************************************************
  println()
******************************************************************************************************************************************************/
void WordclockSerial::println(const __FlashStringHelper* text)
{
    print(text);
    println();
} /* println */


/******************************************************************************************************************************************************
  Flush()
******************************************************************************************************************************************************/
/*! \brief          Waits until the UART has sent what it was given
******************************************************************************************************************************************************/
void WordclockSerial::flush()
{
    wordclock_platform::HardwarePort().flush();
} /* Flush */
// NOLINTEND(readability-convert-member-functions-to-static)


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  ReadInjected()
******************************************************************************************************************************************************/
char WordclockSerial::ReadInjected()
{
    const size_t tail = inject_tail_.load(std::memory_order_relaxed);
    const char character = inject_buffer_[tail];

    // Published after the character was taken, so the producer cannot refill the slot
    // while it is still being read.
    inject_tail_.store(NextIndex(tail), std::memory_order_release);

    return character;
} /* ReadInjected */


/******************************************************************************************************************************************************
  AppendToLine()
******************************************************************************************************************************************************/
/*! \brief          Collects the line the sink will be handed
 *  \details        A line that outgrows the buffer is flushed early rather than cut: the
 *                  client then sees the answer split over two frames, which is readable,
 *                  where a truncated line would silently lose its end.
******************************************************************************************************************************************************/
void WordclockSerial::AppendToLine(char character)
{
    if(line_used_ >= (kLineLength - 1u)) { FlushLine(); }

    line_buffer_[line_used_++] = character;
} /* AppendToLine */


/******************************************************************************************************************************************************
  FlushLine()
******************************************************************************************************************************************************/
/*! \brief          Hands the collected line to whoever registered for it
 *  \details        An empty line is dropped, because println() is also used to terminate
 *                  output that print() already ended with a newline - passing that on
 *                  would send a blank frame after every answer.
******************************************************************************************************************************************************/
void WordclockSerial::FlushLine()
{
    if(line_used_ == 0u) { return; }

    line_buffer_[line_used_] = '\0';
    line_used_ = 0u;

    if(line_sink_ != nullptr) { line_sink_(line_buffer_); }
} /* FlushLine */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
