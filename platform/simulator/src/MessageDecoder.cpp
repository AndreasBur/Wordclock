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
/**     \file       MessageDecoder.cpp
 *      \brief      Turns an answer from the firmware into a readable line
 *
 *      \details
 *
******************************************************************************************************************************************************/
#define _MESSAGE_DECODER_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/MessageDecoder.h"
#include <wx/tokenzr.h>

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/
/* Counted rather than written out, so a value list and its length cannot drift apart. */
#define NUMBER_OF(Array)                                static_cast<byte>(sizeof(Array) / sizeof((Array)[0]))


/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const char* const MessageDecoder::CommandIndent{"   "};
const char* const MessageDecoder::ArgumentIndent{"      "};
const char* const MessageDecoder::NameValueSeparator{" = "};

const char* const MessageDecoder::ErrorPrefix{"Error="};
const char* const MessageDecoder::ApiPrefix{"Api="};
const char* const MessageDecoder::RemoteProcedureIdPrefix{"RpcId="};
const char* const MessageDecoder::OptionLabel{"Option"};

/* ErrorMessage::ErrorType */
const char* const MessageDecoder::ErrorValueNames[] {
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
const char* const MessageDecoder::ApiValueNames[] {
    "None",
    "Display show",
    "Display write pixel",
    "Animation show",
    "Clock show",
    "Animation set animation",
    "Animation set clock task cycle"
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  describe()
******************************************************************************************************************************************************/
/*! \brief          reads a line from the firmware back into the names behind it
 *  \details        An answer first, an error line second; a line is never both, because
 *                  an answer starts with a number and an error with a name.
 *
 *  \return         the readable form, or an empty string if this is neither
******************************************************************************************************************************************************/
wxString MessageDecoder::describe(const wxString& Line)
{
    const wxString Answer = describeAnswer(Line);
    if(!Answer.IsEmpty()) { return Answer; }

    const wxString RemoteProcedureCall = describeRemoteProcedureCall(Line);
    if(!RemoteProcedureCall.IsEmpty()) { return RemoteProcedureCall; }

    return describeError(Line);
} /* describe */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  describeAnswer()
******************************************************************************************************************************************************/
/*! \brief          reads an answer line back into the names behind it
 *  \details        Deliberately strict about the shape: the line has to start with the
 *                  number of a known command and carry at least one option, otherwise it
 *                  is reported as undecodable. Half-decoding whatever happens to look
 *                  similar would be worse than leaving it alone.
 *
 *  \return         the readable form, or an empty string if this is no answer
******************************************************************************************************************************************************/
wxString MessageDecoder::describeAnswer(const wxString& Line)
{
    wxStringTokenizer Tokenizer(Line, _T(" "));
    if(!Tokenizer.HasMoreTokens()) { return wxEmptyString; }

    uint16_t Number{0u};
    if(!toValue(Tokenizer.GetNextToken(), Number)) { return wxEmptyString; }
    if(Number > CommandNumberMax) { return wxEmptyString; }

    const MessageCatalog::CommandType* Command = MessageCatalog::findCommandByNumber(static_cast<byte>(Number));
    if(Command == nullptr) { return wxEmptyString; }

    wxString Arguments;

    while(Tokenizer.HasMoreTokens()) {
        const wxString Argument = describeArgument(*Command, Tokenizer.GetNextToken());
        if(Argument.IsEmpty()) { continue; }

        Arguments += ArgumentIndent + Argument + _T("\n");
    }

    if(Arguments.IsEmpty()) { return wxEmptyString; }

    return CommandIndent + wxString(Command->Label) + _T("\n") + Arguments;
} /* describeAnswer */


/******************************************************************************************************************************************************
  describeArgument()
******************************************************************************************************************************************************/
/*! \brief          reads one "<short name>=<value>" of an answer back into names
 *  \details        A short name the command does not have is kept as it is rather than
 *                  dropped: it means this table has fallen behind the parser, and hiding
 *                  that would be the wrong way round.
 *
 *  \return         the readable form, or an empty string if the token is not an argument
******************************************************************************************************************************************************/
wxString MessageDecoder::describeArgument(const MessageCatalog::CommandType& Command, const wxString& Token)
{
    if(Token.Find(OptionArgumentDelimiter) != DelimiterPosition) { return wxEmptyString; }

    const char ShortName = static_cast<char>(Token[0]);
    const wxString Value = Token.Mid(DelimiterPosition + 1);
    const MessageCatalog::OptionType* Option = MessageCatalog::findOption(Command, ShortName);

    if(Option == nullptr) { return Token; }

    return describeValue(Option->Label, Value, Option->ValueNames, Option->NumberOfValueNames, Option->Minimum);
} /* describeArgument */


/******************************************************************************************************************************************************
  describeRemoteProcedureCall()
******************************************************************************************************************************************************/
/*! \brief          reads the remote procedure call's answer back into names
 *  \details        It is the one answer that does not begin with its command number:
 *                  "RpcId=<number>" and then the error of carrying it out. Laid out like
 *                  any other answer, with the command as the heading, so the procedure and
 *                  the error read as what they are — two fields of one answer.
 *
 *                  The procedure names are the ones the catalog already carries for the
 *                  command's option, rather than a second list.
 *
 *  \return         the readable form, or an empty string if this is not that answer
******************************************************************************************************************************************************/
wxString MessageDecoder::describeRemoteProcedureCall(const wxString& Line)
{
    wxStringTokenizer Tokenizer(Line, _T(" "));
    if(!Tokenizer.HasMoreTokens()) { return wxEmptyString; }

    wxString Number;
    if(!Tokenizer.GetNextToken().StartsWith(RemoteProcedureIdPrefix, &Number)) { return wxEmptyString; }

    /* As strict about the shape as an answer: without a number behind the prefix this is
       something else that happens to begin the same way. */
    uint16_t Procedure{0u};
    if(!toValue(Number, Procedure)) { return wxEmptyString; }

    const MessageCatalog::CommandType* Command = MessageCatalog::findRemoteProcedureCallCommand();
    if((Command == nullptr) || (Command->NumberOfOptions == 0u)) { return wxEmptyString; }

    /* The procedure is the command's only option. */
    const MessageCatalog::OptionType& Option = Command->Options[0];
    wxString Described = CommandIndent + wxString(Command->Label) + _T("\n")
                       + ArgumentIndent
                       + describeValue(Option.Label, Number, Option.ValueNames, Option.NumberOfValueNames, Option.Minimum)
                       + _T("\n");

    while(Tokenizer.HasMoreTokens()) {
        Described += describeErrorPart(Tokenizer.GetNextToken(), ArgumentIndent);
    }

    return Described;
} /* describeRemoteProcedureCall */


/******************************************************************************************************************************************************
  describeError()
******************************************************************************************************************************************************/
/*! \brief          reads an error line back into the names behind it
 *  \details        ErrorMessage appends a space after each error, so one line can carry
 *                  several; each is read on its own and anything that is not one is
 *                  passed over.
 *
 *  \return         the readable form, or an empty string if this is no error line
******************************************************************************************************************************************************/
wxString MessageDecoder::describeError(const wxString& Line)
{
    wxString Described;
    wxStringTokenizer Tokenizer(Line, _T(" "));

    while(Tokenizer.HasMoreTokens()) {
        Described += describeErrorPart(Tokenizer.GetNextToken(), CommandIndent);
    }

    return Described;
} /* describeError */


/******************************************************************************************************************************************************
  describeErrorPart()
******************************************************************************************************************************************************/
/*! \brief          reads one error of a line back into names
 *  \details        Three shapes come out of ErrorMessage: the error on its own, the error
 *                  with the option it belongs to appended, and the error behind the api
 *                  call that failed. The api form carries an error of its own after the
 *                  delimiter, which is why this reaches back into itself for it.
 *
 *  \return         the readable form, or an empty string if this is no error
******************************************************************************************************************************************************/
wxString MessageDecoder::describeErrorPart(const wxString& Part, const char* Indent)
{
    wxString Rest;

    if(Part.StartsWith(ApiPrefix, &Rest)) {
        return Indent
             + describeValue(ApiPrefix, Rest.BeforeFirst(ErrorPartDelimiter), ApiValueNames, NUMBER_OF(ApiValueNames))
             + _T("\n")
             + describeErrorPart(Rest.AfterFirst(ErrorPartDelimiter), Indent);
    }

    if(Part.StartsWith(ErrorPrefix, &Rest)) {
        wxString Described = Indent
                           + describeValue(ErrorPrefix, Rest.BeforeFirst(ErrorPartDelimiter), ErrorValueNames, NUMBER_OF(ErrorValueNames))
                           + _T("\n");

        /* Only the shape that names an option has anything after the delimiter. */
        const wxString Option = Rest.AfterFirst(ErrorPartDelimiter);
        if(!Option.IsEmpty()) {
            Described += Indent + wxString(OptionLabel) + NameValueSeparator + Option + _T("\n");
        }

        return Described;
    }

    return wxEmptyString;
} /* describeErrorPart */


/******************************************************************************************************************************************************
  describeValue()
******************************************************************************************************************************************************/
/*! \brief          puts a label and its value together, spelling the value out if named
 *  \details        The number stays alongside the name: the name is what makes the line
 *                  readable, the number is what was actually on the wire.
 *
 *  \return         "<label> = <name> (<number>)", or "<label> = <value>" without a name
******************************************************************************************************************************************************/
wxString MessageDecoder::describeValue(const char* Label, const wxString& Value,
                                       const char* const* Names, byte NumberOfNames, uint16_t Minimum)
{
    /* The labels of the error shapes carry the delimiter the firmware prints them with,
       which would read oddly in front of one of ours. */
    wxString Described = wxString(Label);
    Described.Replace(wxString(OptionArgumentDelimiter), wxEmptyString);
    Described += NameValueSeparator;

    uint16_t Number{0u};
    if((Names != nullptr) && toValue(Value, Number) &&
       (Number >= Minimum) && ((Number - Minimum) < NumberOfNames)) {
        Described += wxString::Format(wxT("%s (%s)"), Names[Number - Minimum], Value);
    } else {
        Described += Value;
    }

    return Described;
} /* describeValue */


/******************************************************************************************************************************************************
  toValue()
******************************************************************************************************************************************************/
/*! \brief          reads a number out of a token in the width the catalog uses
 *  \details        wxString only parses into unsigned long, which is why this exists: it
 *                  narrows once, in one place, and refuses a value that does not fit
 *                  instead of truncating it into a different one.
 *
 *  \return         true if the token is a number that fits
******************************************************************************************************************************************************/
bool MessageDecoder::toValue(const wxString& Text, uint16_t& Value)
{
    unsigned long Parsed{0ul};

    if(!Text.ToULong(&Parsed)) { return false; }
    if(Parsed > UINT16_MAX) { return false; }

    Value = static_cast<uint16_t>(Parsed);
    return true;
} /* toValue */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
