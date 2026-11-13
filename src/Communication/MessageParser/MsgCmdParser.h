// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The commands by number, and the parser that hands a message to the one it names.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDPARSER_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDPARSER_H_

#include "StandardTypes.h"
#include "Arduino.h"
#include "Message.h"
#include "ErrorMessage.h"
#include "Overlays.h"

/*! \brief The number of MsgCmdParser::CommandType::kNone, 0, which names no command. */
# define MSG_COMMAND_NONE_NUMBER        0u

/*!
 * \brief The parser of a whole message: tells the command apart by its number and hands the rest of
 *        the message to that command's parser.
 *
 * Communication makes one for each complete message and calls Parse(), which also sends the
 * answer.
 */
class MsgCmdParser
{
  public:
    /*!
     * \brief The commands, by the number that a message starts with.
     *
     * The numbers count up from 0 in this order. The three overlay commands exist only while
     * their overlay is built in, by OVERLAYS_SUPPORT_DATE, OVERLAYS_SUPPORT_TEMPERATURE and
     * OVERLAYS_SUPPORT_TEXT, and the commands behind one that is not move down by one.
     * MessageCatalog takes its command numbers from here.
     */
    enum class CommandType {
        kNone = MSG_COMMAND_NONE_NUMBER, /*!< No command: what a message becomes that names none. */
        kRemoteProcedureCall, /*!< A remote procedure call, MsgCmdRemoteProcedureCallParser. */
        kDisplayColor, /*!< The colour of the display, MsgCmdDisplayColorParser. */
        kDisplayBrightness, /*!< The brightness of the display, MsgCmdDisplayBrightnessParser. */
        kDisplayPixel, /*!< One pixel of the display, MsgCmdDisplayPixelParser. */
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
        kOverlayDate, /*!< The date overlay, MsgCmdOverlayDateParser. */
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
        kOverlayTemperature, /*!< The temperature overlay, MsgCmdOverlayTemperatureParser. */
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
        kOverlayText, /*!< The text overlay, MsgCmdOverlayTextParser. */
#endif
        kClockMode, /*!< The wording of the time, MsgCmdClockModeParser. */
        kAnimation, /*!< The animation of a word change, MsgCmdAnimationParser. */
        kTime, /*!< The time of the clock, MsgCmdTimeParser. */
        kDate, /*!< The date of the clock, MsgCmdDateParser. */
        kStatus, /*!< The readings of the clock, which it only answers, MsgCmdStatusParser. */
        kNetwork, /*!< The network the clock joins, MsgCmdNetworkParser. */
        kNightSwitch, /*!< When the clock dims or goes dark at night, MsgCmdNightSwitchParser. */
        kColorCycle, /*!< The cycle of the colour round the wheel, MsgCmdColorCycleParser. */
        kConsole /*!< The password of the clock's pages, MsgCmdConsoleParser; the last command. */
    };

  private:
    static const char kCommandParameterDelimiter{' '};
    ErrorMessage error_;
    const Message& incoming_message_;

    static void SendAnswer(CommandType command) {
        Serial.print(ToUnderlying(command));
        Serial.print(kCommandParameterDelimiter);
    }

    static bool IsDecimalDigit(char character) { return (character >= '0') && (character <= '9'); }
    static byte ToDigitValue(char character) { return static_cast<byte>(character - '0'); }

    /*
     * Counted up digit by digit, and refused the moment the running number passes the
     * last command rather than after the whole run: atoi returns an int, so on the AVR
     * "65537" wrapped into 1 and was answered as a valid command. strtol reports that
     * overflow and was what this did first, at 566 bytes of flash for a routine that
     * parses bases, signs and a long - where a command number is one of seventeen.
     */
    static CommandType ToCommand(const char* message) {
        uint16_t number{0u};
        const char* digit = message;

        while(IsDecimalDigit(*digit)) {
            number = static_cast<uint16_t>((number * 10u) + ToDigitValue(*digit));
            if(number > static_cast<uint16_t>(ToUnderlying(CommandType::kConsole))) { return CommandType::kNone; }
            digit++;
        }
        // Not one digit read: a message that is not a number at all, which is the same
        // CommandType::kNone that a zero converts to on its own.
        if(digit == message) { return CommandType::kNone; }

        return static_cast<CommandType>(number);
    }

    CommandType GetCommand() const { return ToCommand(incoming_message_.GetMessage()); }

    /*
     * A command without a delimiter has no parameter, and the terminator is where that
     * empty string lives. It used to be the last character instead, so "12" handed the
     * parameter parser a "2" to read options out of - and an empty message asked for
     * index Length() - 1, which is a strlen of zero minus one and not a position at all.
     */
    const char* GetParameter() const {
        const char* message = incoming_message_.GetMessage();
        size_t value_pos = incoming_message_.Find(kCommandParameterDelimiter);

        if(value_pos == Message::npos) { return &message[incoming_message_.Length()]; }
        else { return &message[value_pos]; }
    }

  public:
    /*!
     * \brief Makes a parser for `message`.
     *
     * \param[in] message The message, without its end-of-message character. The parser keeps a
     *                    reference to it, not a copy.
     */
    constexpr MsgCmdParser(const Message& message) : error_(), incoming_message_(message)  { }

    /*! \brief Destroys the parser. */
    ~MsgCmdParser() = default;

    /*!
     * \brief Reads the command number of the message, runs the command's parser on the rest, and
     *        ends the answer line.
     *
     * The command number is the run of decimal digits that the message starts with, leading zeros
     * allowed: `00012` is command 12. What follows the digits up to the first space is not read,
     * so `9x` is command 9. A message that does not start with a digit, the number 0, and a
     * number past the last command, however many digits it has, are CommandType::kNone.
     *
     * The parameter string, which the command's parser reads its options from, is the message
     * from its first space on, the space included. A message without a space has an empty one:
     * `9-A3` is command 9 without an option, as `9` is.
     *
     * A command is answered with its number and a space. Then its parser is made from the
     * parameter string and run through MsgParameterParser::Parse(), `Process()` and
     * `SendAnswer()`, in this order: the errors of the options come first, as they are read, and
     * the fields of the answer behind them, as in `12 Error=3:V V=...`. CommandType::kNone is
     * answered with `Error=2`, ErrorMessage::ErrorType::kWrongCommand, and no number. Either way
     * Serial.println() ends the line.
     */
    void Parse();

};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGEPARSER_MSGCMDPARSER_H_
