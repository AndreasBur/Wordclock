// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Turns an answer from the firmware into a readable line.
 *
 * An answer arrives as the command number followed by "<short name>=<value>" for each of its
 * settings, which says little without the parser at hand. This reads it back through
 * MessageCatalog, so "3 B=255 A=0" becomes the command and the option names, with named values
 * spelled out.
 *
 * Error lines are read too, in the three shapes ErrorMessage sends: "Error=<number>", the same with
 * ":<option short name>" appended, and "Api=<number>:Error=<number>". Their names are MIRRORED from
 * ErrorMessage::ErrorType and ::ApiType below and have to be kept in step with them by hand, the
 * same as MessageCatalog.
 *
 * Anything that is neither — plain text — is reported as undecodable and left to be shown as it
 * came.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEDECODER_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEDECODER_H_

#include <wx/wx.h>

#include "MessageCatalog.h"
#include "StandardTypes.h"

/*!
 * \brief Reads a line the firmware printed back into the names behind it, through MessageCatalog
 *        and the names of ErrorMessage's errors and apis.
 *
 * No state of its own; Describe() is all it offers.
 */
class MessageDecoder {
 private:
  // Mirrors MsgParameterParser::kOptionArgumentDelimiter, which the parser keeps
  // private.
  static constexpr char kOptionArgumentDelimiter{'='};
  // Where the delimiter sits in a "<short name>=<value>" token, and so what tells one
  // apart from anything else on the line.
  static constexpr int kDelimiterPosition{1};
  // What ErrorMessage puts between the api and the error, and between an error and the
  // option it belongs to.
  static constexpr char kErrorPartDelimiter{':'};

  static char const* const kErrorPrefix;
  static char const* const kApiPrefix;
  /* What the remote procedure call answers with in place of an option short name. */
  static char const* const kRemoteProcedureIdPrefix;
  static char const* const kOptionLabel;
  static char const* const kErrorValueNames[];
  static char const* const kApiValueNames[];

  /*
   * One option per line, each indented under the command and that under the raw answer.
   * Short lines are what makes the indentation hold: the output field is narrow, and a
   * line long enough to wrap continues at the left edge, where it no longer reads as
   * belonging to anything.
   */
  static char const* const kCommandIndent;
  static char const* const kArgumentIndent;
  static char const* const kNameValueSeparator;

  /* Command numbers are a byte in the catalog, so anything wider is not one. */
  static constexpr uint16_t kCommandNumberMax{UINT8_MAX};

  /*
   * wxString parses into unsigned long and nothing narrower, so that stays confined to
   * here: the value comes back in the width the catalog actually uses, and a number too
   * large to fit is refused rather than truncated into a different one.
   */
  static bool ToValue(wxString const& text, uint16_t& value);

  /*
   * "<label> = <name> (<number>)" where the value is named, "<label> = <number>" where
   * it is not. Serves the options of an answer as well as an error and its api, which
   * differ only in where their names come from.
   */
  static wxString DescribeValue(char const* label, wxString const& value, char const* const* names,
                                byte number_of_names, uint16_t minimum = 0u);

  static wxString DescribeAnswer(wxString const& line);
  static wxString DescribeArgument(MessageCatalog::CommandType const& command,
                                   wxString const& token);
  static wxString DescribeRemoteProcedureCall(wxString const& line);
  static wxString DescribeError(wxString const& line);
  // The indent is a parameter because an error stands on its own on most lines, but sits
  // under a command heading in the remote procedure call's answer.
  static wxString DescribeErrorPart(wxString const& part, char const* indent);

 public:
  /*!
   * \brief Returns the readable form of `line`, or an empty string if it is not a line this can
   *        read.
   *
   * The readable form is an indented block, newline-terminated and ready to be appended below
   * the raw answer; an empty string is the caller's signal to leave the line alone.
   *
   * An answer first, then the answer of the remote procedure call, then an error line; a line
   * is never both an answer and an error, because an answer starts with a number and an error
   * with a name.
   *
   * - An answer has to start with the number of a known command and carry at least one
   *   `<short name>=<value>`; it is then read into the command's label and a line per option.
   *   A token that is not an option is passed over, and a short name the command does not have
   *   is kept as it is.
   * - The remote procedure call's answer, `RpcId=<number>` and the errors behind it, is read
   *   into the command's label, the procedure and the errors.
   * - In an error line, every `Error=` and `Api=` is read, and every other token passed over.
   *
   * A value the catalog names reads `<label> = <name> (<number>)`, any other `<label> = <value>`.
   *
   * \code{.unparsed}
   * 3 B=255 A=1 G=0
   *    Display brightness
   *       Brightness = 255
   *       Automatic = on (1)
   *       Gamma correction = off (0)
   * \endcode
   *
   * \param[in] line The line, without its line end.
   * \return The readable form, or an empty string.
   */
  static wxString Describe(wxString const& line);
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_MESSAGEDECODER_H_
