// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief Turns an answer from the firmware into a readable line.
 */

#include "sim/MessageDecoder.h"
#include <wx/tokenzr.h>

// Local to this file, so Doxygen leaves it out.
/*! \cond */
/* Counted rather than written out, so a value list and its length cannot drift apart. */
#define NUMBER_OF(Array)                                static_cast<byte>(sizeof(Array) / sizeof((Array)[0]))
/*! \endcond */

const char* const MessageDecoder::kCommandIndent{"   "};
const char* const MessageDecoder::kArgumentIndent{"      "};
const char* const MessageDecoder::kNameValueSeparator{" = "};

const char* const MessageDecoder::kErrorPrefix{"Error="};
const char* const MessageDecoder::kApiPrefix{"Api="};
const char* const MessageDecoder::kRemoteProcedureIdPrefix{"RpcId="};
const char* const MessageDecoder::kOptionLabel{"Option"};

/* ErrorMessage::ErrorType */
const char* const MessageDecoder::kErrorValueNames[] {
    "No error",
    "Message too long",
    "Wrong command",
    "Parameter unknown",
    "Value out of bounds",
    "No value given",
    "Display pending",
    "Rpc id unknown",
    "Unknown"
};

/* ErrorMessage::ApiType */
const char* const MessageDecoder::kApiValueNames[] {
    "None",
    "Display show",
    "Display write pixel",
    "Animation show",
    "Clock show",
    "Animation set animation",
    "Animation set clock task cycle"
};

wxString MessageDecoder::Describe(const wxString& line)
{
    wxString answer = DescribeAnswer(line);
    if(!answer.IsEmpty()) { return answer; }

    wxString remote_procedure_call = DescribeRemoteProcedureCall(line);
    if(!remote_procedure_call.IsEmpty()) { return remote_procedure_call; }

    return DescribeError(line);
}

/*
 * Reads an answer line back into the names behind it, or returns an empty string if this is no
 * answer. Deliberately strict about the shape: the line has to start with the number of a known
 * command and carry at least one option, otherwise it is reported as undecodable. Half-decoding
 * whatever happens to look similar would be worse than leaving it alone.
 */
wxString MessageDecoder::DescribeAnswer(const wxString& line)
{
    wxStringTokenizer tokenizer(line, _T(" "));
    if(!tokenizer.HasMoreTokens()) { return wxEmptyString; }

    uint16_t number{0u};
    if(!ToValue(tokenizer.GetNextToken(), number)) { return wxEmptyString; }
    if(number > kCommandNumberMax) { return wxEmptyString; }

    const MessageCatalog::CommandType* command = MessageCatalog::FindCommandByNumber(static_cast<byte>(number));
    if(command == nullptr) { return wxEmptyString; }

    wxString arguments;

    while(tokenizer.HasMoreTokens()) {
        const wxString argument = DescribeArgument(*command, tokenizer.GetNextToken());
        if(argument.IsEmpty()) { continue; }

        arguments += kArgumentIndent + argument + _T("\n");
    }

    if(arguments.IsEmpty()) { return wxEmptyString; }

    return kCommandIndent + wxString(command->label) + _T("\n") + arguments;
}

/*
 * Reads one "<short name>=<value>" of an answer back into names, or returns an empty string if
 * the token is not an argument. A short name the command does not have is kept as it is rather
 * than dropped: it means this table has fallen behind the parser, and hiding that would be the
 * wrong way round.
 */
wxString MessageDecoder::DescribeArgument(const MessageCatalog::CommandType& command, const wxString& token)
{
    if(token.Find(kOptionArgumentDelimiter) != kDelimiterPosition) { return wxEmptyString; }

    const char short_name = static_cast<char>(token[0]);
    const wxString value = token.Mid(kDelimiterPosition + 1);
    const MessageCatalog::OptionType* option = MessageCatalog::FindOption(command, short_name);

    if(option == nullptr) { return token; }

    return DescribeValue(option->label, value, option->value_names, option->number_of_value_names, option->minimum);
}

/*
 * Reads the remote procedure call's answer back into names, or returns an empty string if this
 * is not that answer. It is the one answer that does not begin with its command number:
 * "RpcId=<number>" and then the error of carrying it out. Laid out like any other answer, with
 * the command as the heading, so the procedure and the error read as what they are - two fields
 * of one answer.
 *
 * The procedure names are the ones the catalog already carries for the command's option,
 * rather than a second list.
 */
wxString MessageDecoder::DescribeRemoteProcedureCall(const wxString& line)
{
    wxStringTokenizer tokenizer(line, _T(" "));
    if(!tokenizer.HasMoreTokens()) { return wxEmptyString; }

    wxString number;
    if(!tokenizer.GetNextToken().StartsWith(kRemoteProcedureIdPrefix, &number)) { return wxEmptyString; }

    // As strict about the shape as an answer: without a number behind the prefix this is
    // something else that happens to begin the same way.
    uint16_t procedure{0u};
    if(!ToValue(number, procedure)) { return wxEmptyString; }

    const MessageCatalog::CommandType* command = MessageCatalog::FindRemoteProcedureCallCommand();
    if((command == nullptr) || (command->number_of_options == 0u)) { return wxEmptyString; }

    /* The procedure is the command's only option. */
    const MessageCatalog::OptionType& option = command->options[0];
    wxString described = kCommandIndent + wxString(command->label) + _T("\n")
                       + kArgumentIndent
                       + DescribeValue(option.label, number, option.value_names, option.number_of_value_names, option.minimum)
                       + _T("\n");

    while(tokenizer.HasMoreTokens()) {
        described += DescribeErrorPart(tokenizer.GetNextToken(), kArgumentIndent);
    }

    return described;
}

/*
 * Reads an error line back into the names behind it, or returns an empty string if this is no
 * error line. ErrorMessage appends a space after each error, so one line can carry several;
 * each is read on its own and anything that is not one is passed over.
 */
wxString MessageDecoder::DescribeError(const wxString& line)
{
    wxString described;
    wxStringTokenizer tokenizer(line, _T(" "));

    while(tokenizer.HasMoreTokens()) {
        described += DescribeErrorPart(tokenizer.GetNextToken(), kCommandIndent);
    }

    return described;
}

/*
 * Reads one error of a line back into names, or returns an empty string if this is no error.
 * Three shapes come out of ErrorMessage: the error on its own, the error with the option it
 * belongs to appended, and the error behind the api call that failed. The api form carries an
 * error of its own after the delimiter, which is why this reaches back into itself for it.
 */
wxString MessageDecoder::DescribeErrorPart(const wxString& part, const char* indent)
{
    wxString rest;

    if(part.StartsWith(kApiPrefix, &rest)) {
        return indent
             + DescribeValue(kApiPrefix, rest.BeforeFirst(kErrorPartDelimiter), kApiValueNames, NUMBER_OF(kApiValueNames))
             + _T("\n")
             + DescribeErrorPart(rest.AfterFirst(kErrorPartDelimiter), indent);
    }

    if(part.StartsWith(kErrorPrefix, &rest)) {
        wxString described = indent
                           + DescribeValue(kErrorPrefix, rest.BeforeFirst(kErrorPartDelimiter), kErrorValueNames, NUMBER_OF(kErrorValueNames))
                           + _T("\n");

        /* Only the shape that names an option has anything after the delimiter. */
        const wxString option = rest.AfterFirst(kErrorPartDelimiter);
        if(!option.IsEmpty()) {
            described += indent + wxString(kOptionLabel) + kNameValueSeparator + option + _T("\n");
        }

        return described;
    }

    return wxEmptyString;
}

/*
 * Puts a label and its value together, spelling the value out if named: "<label> = <name>
 * (<number>)", or "<label> = <value>" without a name. The number stays alongside the name: the
 * name is what makes the line readable, the number is what was actually on the wire.
 */
wxString MessageDecoder::DescribeValue(const char* label, const wxString& value,
                                       const char* const* names, byte number_of_names, uint16_t minimum)
{
    // The labels of the error shapes carry the delimiter the firmware prints them with,
    // which would read oddly in front of one of ours.
    wxString described = wxString(label);
    described.Replace(wxString(kOptionArgumentDelimiter), wxEmptyString);
    described += kNameValueSeparator;

    uint16_t number{0u};
    if((names != nullptr) && ToValue(value, number) &&
       (number >= minimum) && ((number - minimum) < number_of_names)) {
        described += wxString::Format(wxT("%s (%s)"), names[number - minimum], value);
    } else {
        described += value;
    }

    return described;
}

/*
 * Reads a number out of a token in the width the catalog uses, and returns whether the token is
 * a number that fits. wxString only parses into unsigned long, which is why this exists: it
 * narrows once, in one place, and refuses a value that does not fit instead of truncating it
 * into a different one.
 */
bool MessageDecoder::ToValue(const wxString& text, uint16_t& value)
{
    unsigned long parsed{0ul};

    if(!text.ToULong(&parsed)) { return false; }
    if(parsed > UINT16_MAX) { return false; }

    value = static_cast<uint16_t>(parsed);
    return true;
}
