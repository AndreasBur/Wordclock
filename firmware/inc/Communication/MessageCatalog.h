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
/**     \file       MessageCatalog.h
 *      \brief      What the serial commands are called and which options they take
 *
 *      \details    Readable names for the commands the firmware parses, so a front end can
 *                  offer them instead of leaving them to be typed, and read an answer back
 *                  through them.
 *
 *                  It describes the firmware's command set, so it lives with the firmware
 *                  rather than with one front end. The simulator's message builder is one
 *                  renderer of this table - it derives its whole dialog from it, down to
 *                  the input hints - and the intent is that a web interface becomes a
 *                  second one, so that a command added here appears in every front end and
 *                  on the serial interface at once. Nothing in the core reads it, so a
 *                  target too small to carry it can leave the source out of its build.
 *
 *                  The command numbers are taken from MsgCmdParser::CommandType itself,
 *                  and the value lists of the enumerations sit behind the same support
 *                  switches as the enumerations do, so neither can drift. What remains
 *                  MIRRORED by hand are the option short names and argument types, which
 *                  each parser keeps private in its ParameterTable, plus the labels and
 *                  ranges this table adds on top. Every entry names the parser it was
 *                  taken from.
 *
******************************************************************************************************************************************************/
#ifndef MESSAGE_CATALOG_H
#define MESSAGE_CATALOG_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
/* byte comes from here, not from StandardTypes.h */
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* The widest option list of any command, which is the overlays' shared one. Fixes how
   many option rows the message builder has to hold; a static_assert in the source keeps it
   from falling behind the table. */
#define MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS           9u

/******************************************************************************************************************************************************
 *  G L O B A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  C L A S S   M E S S A G E   C A T A L O G
******************************************************************************************************************************************************/
class MessageCatalog
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* Only the argument types the commands actually use, not the full set
       MsgParameter knows. */
    enum ArgumentType {
        ARGUMENT_TYPE_UINT8,
        ARGUMENT_TYPE_UINT16,
        ARGUMENT_TYPE_STRING
    };

    struct OptionType {
        char ShortName;
        const char* Label;
        ArgumentType Argument;
        /* Bounds for the numeric types, both zero where anything the type holds is
           allowed. Ignored for ARGUMENT_TYPE_STRING. */
        uint16_t Minimum;
        uint16_t Maximum;
        /* Names of the values, in ascending order beginning at Minimum, for options that
           carry an enumeration rather than a number. Where they are given, the message
           builder offers them as a dropdown and an answer is read back through them.
           Defaulted, so an option that is a plain number simply leaves both out instead
           of spelling out that it has no names. */
        const char* const* ValueNames{nullptr};
        byte NumberOfValueNames{0u};
        /* A field that only ever appears in an answer. Its command takes no such option -
           sending one is refused by the parser - but the name is described here so that a
           front end can read the answer back through it without offering it as an input.
           Defaulted, so the options that are settable say nothing about it. */
        bool ReadOnly{false};
    };

    struct CommandType {
        byte Number;
        const char* Label;
        const OptionType* Options;
        byte NumberOfOptions;
    };

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    /* The tables live in the source, where they can be constexpr and checked against the
       firmware at compile time. Hence out of line rather than inline here. */

    // get methods
    static byte getNumberOfCommands();
    static const CommandType& getCommand(byte Index);

    // methods
    /* Both return nullptr when there is no such entry, which is how a line that only
       looks like an answer is told apart from one that is. */
    static const CommandType* findCommandByNumber(byte Number);
    static const OptionType* findOption(const CommandType&, char ShortName);

    /* The remote procedure call is the one command whose answer names its value in words
       of its own ("RpcId=") rather than by the option's short name, so the decoder has to
       reach it without having a number to look it up by. */
    static const CommandType* findRemoteProcedureCallCommand();
};

#endif // MESSAGE_CATALOG_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
