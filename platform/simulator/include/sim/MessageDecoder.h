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
/**     \file       MessageDecoder.h
 *      \brief      Turns an answer from the firmware into a readable line
 *
 *      \details    An answer arrives as the command number followed by
 *                  "<short name>=<value>" for each of its settings, which says little
 *                  without the parser at hand. This reads it back through
 *                  MessageCatalog, so "3 B=255 A=0" becomes the command and the
 *                  option names, with named values spelled out.
 *
 *                  Error lines are read too, in the three shapes ErrorMessage sends:
 *                  "Error=<number>", the same with ":<option short name>" appended,
 *                  and "Api=<number>:Error=<number>". Their names are MIRRORED from
 *                  ErrorMessage::ErrorType and ::ApiType below and have to be kept in
 *                  step with them by hand, the same as MessageCatalog.
 *
 *                  Anything that is neither — plain text — is reported as undecodable
 *                  and left to be shown as it came.
 *
******************************************************************************************************************************************************/
#ifndef MESSAGE_DECODER_H
#define MESSAGE_DECODER_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include <wx/wx.h>
#include "StandardTypes.h"
#include "MessageCatalog.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M E S S A G E   D E C O D E R
******************************************************************************************************************************************************/
class MessageDecoder
{
/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Mirrors MsgParameterParser::OptionArgumentDelimiter, which the parser keeps
       private. */
    static constexpr char OptionArgumentDelimiter{'='};
    /* Where the delimiter sits in a "<short name>=<value>" token, and so what tells one
       apart from anything else on the line. */
    static constexpr int DelimiterPosition{1};
    /* What ErrorMessage puts between the api and the error, and between an error and the
       option it belongs to. */
    static constexpr char ErrorPartDelimiter{':'};

    static const char* const ErrorPrefix;
    static const char* const ApiPrefix;
    /* What the remote procedure call answers with in place of an option short name. */
    static const char* const RemoteProcedureIdPrefix;
    static const char* const OptionLabel;
    static const char* const ErrorValueNames[];
    static const char* const ApiValueNames[];

    /* One option per line, each indented under the command and that under the raw answer.
       Short lines are what makes the indentation hold: the output field is narrow, and a
       line long enough to wrap continues at the left edge, where it no longer reads as
       belonging to anything. */
    static const char* const CommandIndent;
    static const char* const ArgumentIndent;
    static const char* const NameValueSeparator;

    /* Command numbers are a byte in the catalog, so anything wider is not one. */
    static constexpr uint16_t CommandNumberMax{UINT8_MAX};

    /* wxString parses into unsigned long and nothing narrower, so that stays confined to
       here: the value comes back in the width the catalog actually uses, and a number too
       large to fit is refused rather than truncated into a different one. */
    static bool toValue(const wxString&, uint16_t&);

    /* "<label> = <name> (<number>)" where the value is named, "<label> = <number>" where
       it is not. Serves the options of an answer as well as an error and its api, which
       differ only in where their names come from. */
    static wxString describeValue(const char*, const wxString&, const char* const*, byte, uint16_t Minimum = 0u);

    static wxString describeAnswer(const wxString&);
    static wxString describeArgument(const MessageCatalog::CommandType&, const wxString&);
    static wxString describeRemoteProcedureCall(const wxString&);
    static wxString describeError(const wxString&);
    /* The indent is a parameter because an error stands on its own on most lines, but sits
       under a command heading in the remote procedure call's answer. */
    static wxString describeErrorPart(const wxString&, const char* Indent);

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    // methods
    /* Returns the indented block, newline-terminated and ready to be appended below the
       raw answer, or an empty string when the line is not an answer this can read — which
       is the caller's signal to leave it alone. */
    static wxString describe(const wxString&);
};

#endif // MESSAGE_DECODER_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
