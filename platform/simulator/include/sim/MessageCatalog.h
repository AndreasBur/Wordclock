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
 *      \details    Readable names for the commands the firmware parses, so the message
 *                  builder can offer them instead of leaving them to be typed, and the
 *                  decoder can read an answer back through them. Simulator-only: it
 *                  adds nothing the firmware needs.
 *
 *                  MIRRORS the parsers in firmware/inc/Communication/MessageParser
 *                  and has to be kept in step with them by hand. Command numbers
 *                  come from MsgCmdParser::CommandType, short names and argument
 *                  types from each parser's ParameterTable; every entry below names
 *                  the parser it was taken from. The parsers keep those tables
 *                  private and carry no labels or ranges, which is what this table
 *                  adds; teaching them about both would mean changing firmware for
 *                  a simulator convenience.
 *
******************************************************************************************************************************************************/
#ifndef MESSAGE_CATALOG_H
#define MESSAGE_CATALOG_H

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"
#include "Arduino.h"

/******************************************************************************************************************************************************
 *  G L O B A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* The widest option list of any command, which is the overlays' shared one. Fixes how
   many option rows the message builder has to hold. */
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
    };

    struct CommandType {
        byte Number;
        const char* Label;
        const OptionType* Options;
        byte NumberOfOptions;
    };

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    static const char* const BooleanValueNames[];
    static const char* const RemoteProcedureValueNames[];
    static const char* const ClockModeValueNames[];
    static const char* const AnimationValueNames[];
    static const char* const AnimationModeValueNames[];
    static const char* const FontValueNames[];

    static const OptionType RemoteProcedureCallOptions[];
    static const OptionType DisplayColorOptions[];
    static const OptionType DisplayBrightnessOptions[];
    static const OptionType DisplayPixelOptions[];
    static const OptionType OverlayOptions[];
    static const OptionType ClockModeOptions[];
    static const OptionType AnimationOptions[];
    static const OptionType TimeOptions[];
    static const OptionType DateOptions[];

    static const CommandType Commands[];
    static const byte NumberOfCommands;

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    // get methods
    static byte getNumberOfCommands() { return NumberOfCommands; }
    static const CommandType& getCommand(byte Index) { return Commands[Index]; }

    // methods
    static bool isCommandValid(byte Index) { return Index < NumberOfCommands; }

    /* Both return nullptr when there is no such entry, which is how a line that only
       looks like an answer is told apart from one that is. */
    static const CommandType* findCommandByNumber(byte Number);
    static const OptionType* findOption(const CommandType&, char ShortName);
};

#endif // MESSAGE_CATALOG_H

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
