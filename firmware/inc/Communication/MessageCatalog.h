// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the serial commands are called and which options they take.
 *
 * Readable names for the commands the firmware parses, so a front end can offer them instead of
 * leaving them to be typed, and read an answer back through them.
 *
 * It describes the firmware's command set, so it lives with the firmware rather than with one front
 * end. The simulator's message builder is one renderer of this table - it derives its whole dialog
 * from it, down to the input hints - and the intent is that a web interface becomes a second one,
 * so that a command added here appears in every front end and on the serial interface at once.
 * Nothing in the core reads it, so a target too small to carry it can leave the source out of its
 * build.
 *
 * The command numbers are taken from MsgCmdParser::CommandType itself, and the value lists of the
 * enumerations sit behind the same support switches as the enumerations do, so neither can drift.
 * What remains MIRRORED by hand are the option short names and argument types, which each parser
 * keeps private in its parameter_table_, plus the labels and ranges this table adds on top. Every
 * entry names the parser it was taken from.
 */

#ifndef MESSAGE_CATALOG_H
#define MESSAGE_CATALOG_H

#include "Arduino.h"

/*
 * The widest option list of any command, which is the overlays' shared one. Fixes how
 * many option rows the message builder has to hold; a static_assert in the source keeps it
 * from falling behind the table.
 */
#define MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS           9u

class MessageCatalog
{
  public:
    // Only the argument types the commands actually use, not the full set
    // MsgParameter knows.
    enum class ArgumentType {
        kUint8,
        kUint16,
        kString
    };

    struct OptionType {
        char short_name;
        const char* label;
        ArgumentType argument;
        // Bounds for the numeric types, both zero where anything the type holds is
        // allowed. Ignored for ArgumentType::kString.
        uint16_t minimum;
        uint16_t maximum;
        /*
         * Names of the values, in ascending order beginning at Minimum, for options that
         * carry an enumeration rather than a number. Where they are given, the message
         * builder offers them as a dropdown and an answer is read back through them.
         * Defaulted, so an option that is a plain number simply leaves both out instead
         * of spelling out that it has no names.
         */
        const char* const* value_names{nullptr};
        byte number_of_value_names{0u};
        /*
         * A field that only ever appears in an answer. Its command takes no such option -
         * sending one is refused by the parser - but the name is described here so that a
         * front end can read the answer back through it without offering it as an input.
         * Defaulted, so the options that are settable say nothing about it.
         */
        bool read_only{false};
    };

    struct CommandType {
        byte number;
        const char* label;
        const OptionType* options;
        byte number_of_options;
    };

  public:
    // The tables live in the source, where they can be constexpr and checked against the
    // firmware at compile time. Hence out of line rather than inline here.

    // get methods
    static byte GetNumberOfCommands();
    static const CommandType& GetCommand(byte index);

    // methods
    // Both return nullptr when there is no such entry, which is how a line that only
    // looks like an answer is told apart from one that is.
    static const CommandType* FindCommandByNumber(byte number);
    static const OptionType* FindOption(const CommandType&, char short_name);

    /*
     * The remote procedure call is the one command whose answer names its value in words
     * of its own ("RpcId=") rather than by the option's short name, so the decoder has to
     * reach it without having a number to look it up by.
     */
    static const CommandType* FindRemoteProcedureCallCommand();
};

#endif // MESSAGE_CATALOG_H
