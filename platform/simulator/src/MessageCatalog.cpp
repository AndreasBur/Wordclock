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
/**     \file       MessageCatalog.cpp
 *      \brief      What the serial commands are called and which options they take
 *
 *      \details    See the header on why this table is here and what it mirrors.
 *
******************************************************************************************************************************************************/
#define _MESSAGE_CATALOG_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/MessageCatalog.h"
/* Not for any type, only for the ANIMATIONS_SUPPORT_* and TEXT_SUPPORT_FONT_* switches:
   the value lists below are guarded by exactly the same ones as the enums they name, so
   they cannot end up naming the wrong value when one is switched off. */
#include "Animations.h"
#include "Text.h"

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* Spelled out rather than taken from the firmware, because the parsers keep their
   tables private. */
#define BYTE_MAX                                        255u
#define WORD_MAX                                        65535u

/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/
/* Counted rather than written out, so a value list and its length cannot drift apart. */
#define NUMBER_OF(Array)                                static_cast<byte>(sizeof(Array) / sizeof((Array)[0]))

/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A
******************************************************************************************************************************************************/

/* For every option that carries a flag rather than a number. */
const char* const MessageCatalog::BooleanValueNames[] {
    "off", "on"
};

/* MsgCmdRemoteProcedureCallParser::RpcIdType */
const char* const MessageCatalog::RemoteProcedureValueNames[] {
    "None",
    "Illuminance calibration maximum",
    "Illuminance calibration minimum",
    "Display enable",
    "Display disable",
    "Display show",
    "Display clear",
    "Display test",
    "Colour red increment",
    "Colour green increment",
    "Colour blue increment",
    "Colour red decrement",
    "Colour green decrement",
    "Colour blue decrement",
    "Brightness increment",
    "Brightness decrement",
    "Brightness automatic on",
    "Brightness automatic off",
    "Gamma correction on",
    "Gamma correction off",
    "Power on",
    "Power off"
};

/* Clock::ModeType */
const char* const MessageCatalog::ClockModeValueNames[] {
    "Wessi", "Ossi", "Rhein-Ruhr", "Schwaben"
};

/* Animations::AnimationIdType, under the same switches as the enum. Its values shift as
   soon as one animation is switched off, so a fixed list would name the wrong ones. */
const char* const MessageCatalog::AnimationValueNames[] {
    "None",
#if(ANIMATIONS_SUPPORT_CURSOR == STD_ON)
    "Cursor",
#endif
#if(ANIMATIONS_SUPPORT_TELETYPE == STD_ON)
    "Teletype",
#endif
#if(ANIMATIONS_SUPPORT_DROP == STD_ON)
    "Drop",
#endif
#if(ANIMATIONS_SUPPORT_SHIFT == STD_ON)
    "Shift",
#endif
#if(ANIMATIONS_SUPPORT_FADE == STD_ON)
    "Fade",
#endif
#if(ANIMATIONS_SUPPORT_SNAKE == STD_ON)
    "Snake",
#endif
#if(ANIMATIONS_SUPPORT_WIPE == STD_ON)
    "Wipe",
#endif
#if(ANIMATIONS_SUPPORT_CUBE == STD_ON)
    "Cube",
#endif
#if(ANIMATIONS_SUPPORT_FLICKER == STD_ON)
    "Flicker",
#endif
#if(ANIMATIONS_SUPPORT_SQUEEZE == STD_ON)
    "Squeeze",
#endif
#if(ANIMATIONS_SUPPORT_EXPLODE_PIXELS == STD_ON)
    "Explode pixels",
#endif
#if(ANIMATIONS_SUPPORT_EXPLODE_WORDS == STD_ON)
    "Explode words",
#endif
#if(ANIMATIONS_SUPPORT_MATRIX == STD_ON)
    "Matrix",
#endif
#if(ANIMATIONS_SUPPORT_ROLL == STD_ON)
    "Roll",
#endif
#if(ANIMATIONS_SUPPORT_COLLAPSE == STD_ON)
    "Collapse"
#endif
};

/* Animations::ModeType */
const char* const MessageCatalog::AnimationModeValueNames[] {
    "Fixed", "Random", "Sequence"
};

/* Text::FontType, under the same switches as the enum. */
const char* const MessageCatalog::FontValueNames[] {
#if(TEXT_SUPPORT_FONT_5X8 == STD_ON)
    "5x8",
#endif
#if(TEXT_SUPPORT_FONT_7X9 == STD_ON)
    "7x9",
#endif
#if(TEXT_SUPPORT_FONT_7X10 == STD_ON)
    "7x10",
#endif
#if(TEXT_SUPPORT_FONT_9X10 == STD_ON)
    "9x10",
#endif
#if(TEXT_SUPPORT_FONT_10X10 == STD_ON)
    "10x10"
#endif
};

/* MsgCmdRemoteProcedureCallParser */
const MessageCatalog::OptionType MessageCatalog::RemoteProcedureCallOptions[] {
    {'P', "Procedure",          ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX, RemoteProcedureValueNames, NUMBER_OF(RemoteProcedureValueNames)}
};

/* MsgCmdDisplayColorParser */
const MessageCatalog::OptionType MessageCatalog::DisplayColorOptions[] {
    {'R', "Red",                ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'G', "Green",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'B', "Blue",               ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX}
};

/* MsgCmdDisplayBrightnessParser */
const MessageCatalog::OptionType MessageCatalog::DisplayBrightnessOptions[] {
    {'B', "Brightness",         ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'A', "Automatic",          ARGUMENT_TYPE_UINT8,  0u, 1u,       BooleanValueNames, NUMBER_OF(BooleanValueNames)},
    {'G', "Gamma correction",   ARGUMENT_TYPE_UINT8,  0u, 1u,       BooleanValueNames, NUMBER_OF(BooleanValueNames)}
};

/* MsgCmdDisplayPixelParser. The index is a uint16 as soon as the display has more than
   255 pixels; 110 here, so the parser compiles it as a uint8. */
const MessageCatalog::OptionType MessageCatalog::DisplayPixelOptions[] {
    {'I', "Index",              ARGUMENT_TYPE_UINT8,  0u, 109u},
    {'S', "State",              ARGUMENT_TYPE_UINT8,  0u, 1u,       BooleanValueNames, NUMBER_OF(BooleanValueNames)}
};

/* MsgCmdBaseOverlayParser, shared by the date, temperature and text commands. Not every
   overlay makes use of every option — the text is only carried by the text overlay — but
   all three accept the full list, so it is offered for all three. */
const MessageCatalog::OptionType MessageCatalog::OverlayOptions[] {
    {'P', "Period (minutes)",   ARGUMENT_TYPE_UINT8,  1u, BYTE_MAX},
    {'E', "Endurance (s)",      ARGUMENT_TYPE_UINT8,  1u, BYTE_MAX},
    {'M', "Month",              ARGUMENT_TYPE_UINT8,  0u, 12u},
    {'D', "Day",                ARGUMENT_TYPE_UINT8,  0u, 31u},
    {'V', "Valid in days",      ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'A', "Active",             ARGUMENT_TYPE_UINT8,  0u, 1u,       BooleanValueNames, NUMBER_OF(BooleanValueNames)},
    {'T', "Text",               ARGUMENT_TYPE_STRING, 0u, 0u},
    {'S', "Speed",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'F', "Font",               ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX, FontValueNames, NUMBER_OF(FontValueNames)}
};

/* MsgCmdClockModeParser */
const MessageCatalog::OptionType MessageCatalog::ClockModeOptions[] {
    {'M', "Mode",               ARGUMENT_TYPE_UINT8,  0u, 3u,       ClockModeValueNames, NUMBER_OF(ClockModeValueNames)}
};

/* MsgCmdAnimationParser */
const MessageCatalog::OptionType MessageCatalog::AnimationOptions[] {
    {'A', "Animation",          ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX, AnimationValueNames, NUMBER_OF(AnimationValueNames)},
    {'M', "Mode",               ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX, AnimationModeValueNames, NUMBER_OF(AnimationModeValueNames)},
    {'S', "Speed",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'F', "Favourite",          ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX}
};

/* MsgCmdTimeParser */
const MessageCatalog::OptionType MessageCatalog::TimeOptions[] {
    {'H', "Hour",               ARGUMENT_TYPE_UINT8,  0u, 23u},
    {'M', "Minute",             ARGUMENT_TYPE_UINT8,  0u, 59u},
    {'S', "Second",             ARGUMENT_TYPE_UINT8,  0u, 59u}
};

/* MsgCmdDateParser */
const MessageCatalog::OptionType MessageCatalog::DateOptions[] {
    {'Y', "Year",               ARGUMENT_TYPE_UINT16, 0u, WORD_MAX},
    {'M', "Month",              ARGUMENT_TYPE_UINT8,  1u, 12u},
    {'D', "Day",                ARGUMENT_TYPE_UINT8,  1u, 31u}
};

/* Numbers and order from MsgCmdParser::CommandType. COMMAND_NONE is left out, it is not
   a command that can be sent. Sending a command with no option at all asks the firmware
   for its current settings, which is why every entry is useful on its own. */
const MessageCatalog::CommandType MessageCatalog::Commands[] {
    { 1u, "Remote procedure call", RemoteProcedureCallOptions, NUMBER_OF(RemoteProcedureCallOptions)},
    { 2u, "Display colour",        DisplayColorOptions,        NUMBER_OF(DisplayColorOptions)},
    { 3u, "Display brightness",    DisplayBrightnessOptions,   NUMBER_OF(DisplayBrightnessOptions)},
    { 4u, "Display pixel",         DisplayPixelOptions,        NUMBER_OF(DisplayPixelOptions)},
    { 5u, "Overlay date",          OverlayOptions,             NUMBER_OF(OverlayOptions)},
    { 6u, "Overlay temperature",   OverlayOptions,             NUMBER_OF(OverlayOptions)},
    { 7u, "Overlay text",          OverlayOptions,             NUMBER_OF(OverlayOptions)},
    { 8u, "Clock mode",            ClockModeOptions,           NUMBER_OF(ClockModeOptions)},
    { 9u, "Animation",             AnimationOptions,           NUMBER_OF(AnimationOptions)},
    {10u, "Time",                  TimeOptions,                NUMBER_OF(TimeOptions)},
    {11u, "Date",                  DateOptions,                NUMBER_OF(DateOptions)}
};

const byte MessageCatalog::NumberOfCommands{NUMBER_OF(Commands)};

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  findCommandByNumber()
******************************************************************************************************************************************************/
/*! \brief          looks a command up by the number it is sent with
 *  \details        Not the same as the index into the table: the numbers happen to run
 *                  from one without a gap today, but nothing keeps them that way.
 *
 *  \return         the command, or nullptr if no command carries that number
******************************************************************************************************************************************************/
const MessageCatalog::CommandType* MessageCatalog::findCommandByNumber(byte Number)
{
    for(byte Index = 0u; Index < NumberOfCommands; Index++) {
        if(Commands[Index].Number == Number) { return &Commands[Index]; }
    }

    return nullptr;
} /* findCommandByNumber */


/******************************************************************************************************************************************************
  findOption()
******************************************************************************************************************************************************/
/*! \brief          looks an option of a command up by its short name
 *
 *  \return         the option, or nullptr if the command has no such option
******************************************************************************************************************************************************/
const MessageCatalog::OptionType* MessageCatalog::findOption(const CommandType& Command, char ShortName)
{
    for(byte Index = 0u; Index < Command.NumberOfOptions; Index++) {
        if(Command.Options[Index].ShortName == ShortName) { return &Command.Options[Index]; }
    }

    return nullptr;
} /* findOption */


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
