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
 * from it, down to the input hints - and the web console is a second one, through WebFrontend, so
 * that a command added here appears in every front end and on the serial interface at once.
 * Nothing in the core reads it but WebFrontend, which is built only with WEB_FRONTEND_SUPPORT, so
 * a target too small to carry it can leave the source out of its build.
 *
 * The command numbers are taken from MsgCmdParser::CommandType itself, and the value lists of the
 * enumerations sit behind the same support switches as the enumerations do, so neither can drift.
 * What remains MIRRORED by hand are the option short names and argument types, which each parser
 * keeps private in its parameter_table_, plus the labels and ranges this table adds on top. Every
 * entry names the parser it was taken from.
 */

#ifndef WORDCLOCK_SRC_COMMUNICATION_MESSAGECATALOG_H_
#define WORDCLOCK_SRC_COMMUNICATION_MESSAGECATALOG_H_

#include "Arduino.h"

/*!
 * \brief The widest option list of any command, which is the overlays' shared one.
 *
 * Fixes how many option rows the message builder has to hold; a static_assert in the source keeps
 * it from falling behind the table.
 */
#define MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS 9u

/*!
 * \brief The message catalog: the commands the firmware parses, each with its number, label and
 *        options.
 *
 * The tables are constant and live in the source; the functions look entries up in them. A
 * command whose `OVERLAYS_SUPPORT_*` switch is STD_OFF is not in them.
 */
class MessageCatalog {
 public:
  /*!
   * \brief The type of an option's argument.
   *
   * Only the argument types the commands actually use, not the full set MsgParameter knows.
   */
  enum class ArgumentType {
    kUint8,  /*!< A number of type `uint8_t`. */
    kUint16, /*!< A number of type `uint16_t`. */
    kString  /*!< A text. */
  };

  /*! \brief An option of a command: its short name, label, argument type, range and names. */
  struct OptionType {
    char short_name;       /*!< The letter behind the `-` that introduces the option. */
    char const* label;     /*!< The name a front end shows for it. */
    ArgumentType argument; /*!< The type of its argument. */
    /*!
     * \brief The smallest value of a number.
     *
     * Zero both for a text, where the bounds are ignored, and for a read-only option, which
     * nothing offers to be typed.
     */
    uint16_t minimum;
    uint16_t maximum; /*!< The largest value of a number; see `minimum`. */
    /*!
     * \brief The names of the values, or nullptr.
     *
     * Names of the values, in ascending order beginning at `minimum`, for options that carry an
     * enumeration rather than a number. Where they are given, the message builder offers them
     * as a dropdown and an answer is read back through them. Defaulted, so an option that is a
     * plain number simply leaves both out instead of spelling out that it has no names.
     */
    char const* const* value_names{nullptr};
    byte number_of_value_names{0u}; /*!< The number of names in `value_names`. */
    /*!
     * \brief Whether the option only ever appears in an answer.
     *
     * A field that only ever appears in an answer. Its command takes no such option - sending
     * one is refused by the parser - but the name is described here so that a front end can
     * read the answer back through it without offering it as an input. Defaulted, so the
     * options that are settable say nothing about it.
     */
    bool read_only{false};
  };

  /*! \brief A command: its number, its label and its options. */
  struct CommandType {
    byte number;               /*!< The number it is sent with, a MsgCmdParser::CommandType. */
    char const* label;         /*!< The name a front end shows for it. */
    OptionType const* options; /*!< Its options. */
    byte number_of_options;    /*!< The number of its options, from the start of `options`. */
  };

 public:
  /*! \brief Returns the number of commands in the catalog. */
  static byte GetNumberOfCommands();

  /*!
   * \brief Returns the command at `index` of the catalog.
   *
   * \param[in] index The index, counted from 0.
   * \return The command.
   * \pre `index` is below GetNumberOfCommands(); it is not checked.
   */
  static CommandType const& GetCommand(byte index);

  /*!
   * \brief Looks a command up by the number it is sent with.
   *
   * Not the same as the index into the table: the numbers happen to run from one without a gap
   * today, but nothing keeps them that way.
   *
   * It returns nullptr when there is no such entry, which is how a line that only looks like an
   * answer is told apart from one that is.
   *
   * \param[in] number The number of the command.
   * \return The command, or nullptr if no command carries that number.
   */
  static CommandType const* FindCommandByNumber(byte number);

  /*!
   * \brief Looks an option of a command up by its short name.
   *
   * It returns nullptr when there is no such entry, which is how a line that only looks like an
   * answer is told apart from one that is.
   *
   * \param[in] command The command.
   * \param[in] short_name The short name of the option.
   * \return The option, or nullptr if the command has no such option.
   */
  static OptionType const* FindOption(CommandType const& command, char short_name);

  /*!
   * \brief Returns the remote procedure call, for a caller that has no number to look up.
   *
   * The remote procedure call is the one command whose answer names its value in words of its
   * own ("RpcId=") rather than by the option's short name, so the decoder has to reach it
   * without having a number to look it up by.
   *
   * \return The command, or nullptr if it is not in the table.
   */
  static CommandType const* FindRemoteProcedureCallCommand();
};

#endif  // WORDCLOCK_SRC_COMMUNICATION_MESSAGECATALOG_H_
