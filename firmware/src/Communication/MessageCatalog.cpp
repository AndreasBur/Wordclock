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
#include "MessageCatalog.h"
/* The command numbers are taken from here rather than copied, so they cannot fall behind
   the firmware. Its OVERLAYS_SUPPORT_* switches reach the entries below through it too. */
#include "MsgCmdParser.h"
/* For RpcIdType alone, which the procedure names below are counted against - they are the
   one list here that is written beside the firmware rather than derived from it. */
#include "MsgCmdRemoteProcedureCallParser.h"
/* Not for any type, only for the ANIMATIONS_SUPPORT_* and TEXT_SUPPORT_FONT_* switches:
   the value lists below are guarded by exactly the same ones as the enums they name, so
   they cannot end up naming the wrong value when one is switched off. */
#include "Animations.h"
#include "Text.h"
/* For the ranges. The limits themselves are private to these classes, but the predicates
   that test them are public and constexpr, so the range can be measured rather than
   copied. */
#include "Clock.h"
#include "Display.h"
#include "RealTimeClock.h"
#include <limits>

/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S
******************************************************************************************************************************************************/
/* For the options the firmware bounds only by the width of the argument: the overlay
   periods and endurances reject nothing but zero, and the speeds nothing at all. */
#define BYTE_MAX                                        255u

/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/
/* Counted rather than written out, so a value list and its length cannot drift apart. */
#define NUMBER_OF(Array)                                static_cast<byte>(sizeof(Array) / sizeof((Array)[0]))

/* The bounds of an option, measured off the firmware's own predicate instead of copied
   from beside it. */
#define MEASURED(ValueType, IsValid)                    findMinimum<ValueType>(IsValid), findMaximum<ValueType>(IsValid)

/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
/* The tables below sit outside the class, so that they can be constexpr and be checked at
   compile time. These bring the argument types back within reach unqualified, which keeps
   the tables readable. */
constexpr MessageCatalog::ArgumentType ARGUMENT_TYPE_UINT8{MessageCatalog::ARGUMENT_TYPE_UINT8};
constexpr MessageCatalog::ArgumentType ARGUMENT_TYPE_UINT16{MessageCatalog::ARGUMENT_TYPE_UINT16};
constexpr MessageCatalog::ArgumentType ARGUMENT_TYPE_STRING{MessageCatalog::ARGUMENT_TYPE_STRING};

/* Walk the whole range of the value's own type and ask the firmware which values it
   accepts. Searching beats copying the limits: they are private, and a copy is a number
   that can quietly fall behind. The type parameter keeps the search inside what the
   predicate takes, so a byte predicate is never asked about 300. */
template <typename ValueType, typename PredicateType>
constexpr uint16_t findMinimum(PredicateType IsValid)
{
    constexpr uint32_t Limit{std::numeric_limits<ValueType>::max()};

    for(uint32_t Value = 0u; Value <= Limit; Value++) {
        if(IsValid(static_cast<ValueType>(Value))) { return static_cast<uint16_t>(Value); }
    }

    return 0u;
}

template <typename ValueType, typename PredicateType>
constexpr uint16_t findMaximum(PredicateType IsValid)
{
    constexpr uint32_t Limit{std::numeric_limits<ValueType>::max()};

    for(uint32_t Value = Limit + 1u; Value > 0u; Value--) {
        if(IsValid(static_cast<ValueType>(Value - 1u))) { return static_cast<uint16_t>(Value - 1u); }
    }

    return 0u;
}

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A
******************************************************************************************************************************************************/

/* For every option that carries a flag rather than a number. */
constexpr const char* const BooleanValueNames[] {
    "off", "on"
};

/* MsgCmdRemoteProcedureCallParser::RpcIdType, from RPC_ID_NONE + 1 onwards. NONE is the
   initial value of the parser's RpcId member, not a procedure: asking for it lands in the
   default case and answers ERROR_RPC_ID_UNKNOWN. Offering it would offer a guaranteed
   error, the same reason COMMAND_NONE is left out of the command list. */
constexpr const char* const RemoteProcedureValueNames[] {
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
    "Power off",
    "Clock refresh",
    "Animation start",
    "Animation abort",
    "Overlay date show",
    "Overlay temperature show",
    "Overlay text show",
    "Overlay abort",
    "Settings save",
    "Settings reset"
};

/* The command numbers and the value lists of the enumerations come out of the firmware
   itself; this list does not, so it is the one that can fall behind. A procedure added to
   RpcIdType without a name here would be offered as the name of the procedure before it.
   RPC_ID_NONE has no name, hence the one. */
static_assert(NUMBER_OF(RemoteProcedureValueNames) ==
              MsgCmdRemoteProcedureCallParser::RPC_ID_NUMBER_OF_PROCEDURES - 1u,
              "the remote procedure names have fallen behind MsgCmdRemoteProcedureCallParser::RpcIdType");

/* Clock::ModeType */
constexpr const char* const ClockModeValueNames[] {
    "Wessi", "Ossi", "Rhein-Ruhr", "Schwaben"
};

/* Animations::AnimationIdType, under the same switches as the enum. Its values shift as
   soon as one animation is switched off, so a fixed list would name the wrong ones. */
constexpr const char* const AnimationValueNames[] {
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
constexpr const char* const AnimationModeValueNames[] {
    "Fixed", "Random", "Sequence"
};

/* Text::FontType, under the same switches as the enum. */
constexpr const char* const FontValueNames[] {
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

/* Where the values are named, the names are the range: the first one sits at the minimum
   and there are as many values as names. */
#define NAMED(Minimum, Names)                           (Minimum), static_cast<uint16_t>((Minimum) + NUMBER_OF(Names) - 1u), Names, NUMBER_OF(Names)

/* MsgCmdRemoteProcedureCallParser. Starts at one, RPC_ID_NONE being left out. */
constexpr MessageCatalog::OptionType RemoteProcedureCallOptions[] {
    {'P', "Procedure",          ARGUMENT_TYPE_UINT8,  NAMED(1u, RemoteProcedureValueNames)}
};

/* MsgCmdDisplayColorParser */
constexpr MessageCatalog::OptionType DisplayColorOptions[] {
    {'R', "Red",                ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'G', "Green",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'B', "Blue",               ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX}
};

/* MsgCmdDisplayBrightnessParser */
constexpr MessageCatalog::OptionType DisplayBrightnessOptions[] {
    {'B', "Brightness",         ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'A', "Automatic",          ARGUMENT_TYPE_UINT8,  NAMED(0u, BooleanValueNames)},
    {'G', "Gamma correction",   ARGUMENT_TYPE_UINT8,  NAMED(0u, BooleanValueNames)}
};

/* MsgCmdDisplayPixelParser. The index is a uint16 as soon as the display has more than
   255 pixels; 110 here, so the parser compiles it as a uint8. */
constexpr MessageCatalog::OptionType DisplayPixelOptions[] {
    {'I', "Index",              ARGUMENT_TYPE_UINT8,  0u, DISPLAY_NUMBER_OF_PIXELS - 1u},
    {'S', "State",              ARGUMENT_TYPE_UINT8,  NAMED(0u, BooleanValueNames)}
};

/* MsgCmdBaseOverlayParser, shared by the date, temperature and text commands. Only the
   text overlay reads the text: the other two answer the parser's setText() and
   sendAnswerText() with empty bodies, so a text sent to them is accepted, discarded and
   never answered. It is therefore the last entry rather than one in the middle, which
   lets those two share this table and simply stop one option short of it. */
constexpr MessageCatalog::OptionType OverlayOptions[] {
    {'P', "Period (minutes)",   ARGUMENT_TYPE_UINT8,  1u, BYTE_MAX},
    {'E', "Endurance (s)",      ARGUMENT_TYPE_UINT8,  1u, BYTE_MAX},
    {'M', "Month",              ARGUMENT_TYPE_UINT8,  0u, 12u},
    {'D', "Day",                ARGUMENT_TYPE_UINT8,  0u, 31u},
    {'V', "Valid in days",      ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'A', "Active",             ARGUMENT_TYPE_UINT8,  NAMED(0u, BooleanValueNames)},
    {'S', "Speed",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'F', "Font",               ARGUMENT_TYPE_UINT8,  NAMED(0u, FontValueNames)},
    {'T', "Text",               ARGUMENT_TYPE_STRING, 0u, 0u}
};

/* The overlays that have no text of their own stop one short of the shared table. Derived
   from its length rather than written out, so it follows if an option is added. */
constexpr byte NumberOfOverlayOptionsWithoutText{static_cast<byte>(NUMBER_OF(OverlayOptions) - 1u)};

/* Stopping one short only leaves out the text as long as the text is what sits last.
   Appending an option would otherwise hide that one and offer the text instead, to two
   overlays that do not read it — quietly, since both lists would still be the right
   length. */
static_assert(OverlayOptions[NumberOfOverlayOptionsWithoutText].ShortName == 'T',
              "the text has to be the last of the overlay options, see NumberOfOverlayOptionsWithoutText");

/* MsgCmdStatusParser. Every one of them read-only: the command takes no options at all,
   and these names exist so that its answer can be read back through them. The bounds are
   therefore left at zero - nothing offers these to be typed, so nothing has a range to
   check against. */
#define READ_ONLY                                       0u, 0u, nullptr, 0u, true

constexpr MessageCatalog::OptionType StatusOptions[] {
    {'V', "Firmware version",   ARGUMENT_TYPE_STRING, READ_ONLY},
    {'I', "Illuminance (lx)",   ARGUMENT_TYPE_UINT16, READ_ONLY},
    {'T', "Temperature",        ARGUMENT_TYPE_STRING, READ_ONLY}
};

/* MsgCmdClockModeParser */
constexpr MessageCatalog::OptionType ClockModeOptions[] {
    {'M', "Mode",               ARGUMENT_TYPE_UINT8,  NAMED(0u, ClockModeValueNames)}
};

/* MsgCmdAnimationParser */
constexpr MessageCatalog::OptionType AnimationOptions[] {
    {'A', "Animation",          ARGUMENT_TYPE_UINT8,  NAMED(0u, AnimationValueNames)},
    {'M', "Mode",               ARGUMENT_TYPE_UINT8,  NAMED(0u, AnimationModeValueNames)},
    {'S', "Speed",              ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX},
    {'F', "Favourite",          ARGUMENT_TYPE_UINT8,  0u, BYTE_MAX}
};

/* MsgCmdTimeParser */
constexpr MessageCatalog::OptionType TimeOptions[] {
    {'H', "Hour",               ARGUMENT_TYPE_UINT8,  MEASURED(ClockTime::HourType, ClockTime::isHourValid)},
    {'M', "Minute",             ARGUMENT_TYPE_UINT8,  MEASURED(ClockTime::MinuteType, ClockTime::isMinuteValid)},
    {'S', "Second",             ARGUMENT_TYPE_UINT8,  MEASURED(ClockTime::SecondType, ClockTime::isSecondValid)}
};

/* MsgCmdDateParser */
constexpr MessageCatalog::OptionType DateOptions[] {
    {'Y', "Year",               ARGUMENT_TYPE_UINT16, MEASURED(ClockDate::YearType, ClockDate::isYearValid)},
    {'M', "Month",              ARGUMENT_TYPE_UINT8,  MEASURED(ClockDate::MonthType, ClockDate::isMonthValid)},
    {'D', "Day",                ARGUMENT_TYPE_UINT8,  MEASURED(ClockDate::DayType, ClockDate::isDayValid)}
};

/* The numbers come from the enumeration itself, so they follow it instead of having to be
   kept level with it — including when a support switch shortens it, which is why the
   overlay entries carry the same guards the enumerators do. COMMAND_NONE is left out, it
   is not a command that can be sent. Sending a command with no option at all asks the
   firmware for its current settings, which is why every entry is useful on its own. */
constexpr MessageCatalog::CommandType Commands[] {
    {MsgCmdParser::COMMAND_REMOTE_PROCEDURE_CALL, "Remote procedure call", RemoteProcedureCallOptions, NUMBER_OF(RemoteProcedureCallOptions)},
    {MsgCmdParser::COMMAND_DISPLAY_COLOR,         "Display colour",        DisplayColorOptions,        NUMBER_OF(DisplayColorOptions)},
    {MsgCmdParser::COMMAND_DISPLAY_BRIGHTNESS,    "Display brightness",    DisplayBrightnessOptions,   NUMBER_OF(DisplayBrightnessOptions)},
    {MsgCmdParser::COMMAND_DISPLAY_PIXEL,         "Display pixel",         DisplayPixelOptions,        NUMBER_OF(DisplayPixelOptions)},
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    {MsgCmdParser::COMMAND_OVERLAY_DATE,          "Overlay date",          OverlayOptions,             NumberOfOverlayOptionsWithoutText},
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    {MsgCmdParser::COMMAND_OVERLAY_TEMPERATURE,   "Overlay temperature",   OverlayOptions,             NumberOfOverlayOptionsWithoutText},
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    {MsgCmdParser::COMMAND_OVERLAY_TEXT,          "Overlay text",          OverlayOptions,             NUMBER_OF(OverlayOptions)},
#endif
    {MsgCmdParser::COMMAND_CLOCK_MODE,            "Clock mode",            ClockModeOptions,           NUMBER_OF(ClockModeOptions)},
    {MsgCmdParser::COMMAND_ANIMATION,             "Animation",             AnimationOptions,           NUMBER_OF(AnimationOptions)},
    {MsgCmdParser::COMMAND_TIME,                  "Time",                  TimeOptions,                NUMBER_OF(TimeOptions)},
    {MsgCmdParser::COMMAND_DATE,                  "Date",                  DateOptions,                NUMBER_OF(DateOptions)},
    {MsgCmdParser::COMMAND_STATUS,                "Status",                StatusOptions,              NUMBER_OF(StatusOptions)}
};

constexpr byte NumberOfCommands{NUMBER_OF(Commands)};

/* The message builder builds this many option rows once and shows the ones a command
   needs, so a command with more options than that would have some of them unreachable.
   Checked here rather than trusted, because the two live in different files. */
constexpr bool areOptionCountsWithinBounds()
{
    for(const MessageCatalog::CommandType& Command : Commands) {
        if(Command.NumberOfOptions > MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS) { return false; }
    }

    return true;
}

static_assert(areOptionCountsWithinBounds(),
              "a command has more options than MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS allows rows for");

/* Naming the values makes the names the range: the dropdown offers one entry per name and
   the builder sends Minimum plus the position picked, so a range wider than the list would
   promise values that cannot be picked, and a narrower one would name values outside it.
   NAMED() gets this right by construction; this keeps a range written out by hand from
   getting it wrong. */
constexpr bool areNamedRangesConsistent()
{
    for(const MessageCatalog::CommandType& Command : Commands) {
        for(byte Index = 0u; Index < Command.NumberOfOptions; Index++) {
            const MessageCatalog::OptionType& Option = Command.Options[Index];

            if(Option.ValueNames == nullptr) { continue; }
            if((Option.Maximum - Option.Minimum + 1u) != Option.NumberOfValueNames) { return false; }
        }
    }

    return true;
}

static_assert(areNamedRangesConsistent(),
              "an option with named values has a range that does not match the number of names");

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getNumberOfCommands()
******************************************************************************************************************************************************/
byte MessageCatalog::getNumberOfCommands()
{
    return NumberOfCommands;
} /* getNumberOfCommands */


/******************************************************************************************************************************************************
  getCommand()
******************************************************************************************************************************************************/
const MessageCatalog::CommandType& MessageCatalog::getCommand(byte Index)
{
    return Commands[Index];
} /* getCommand */


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
  findRemoteProcedureCallCommand()
******************************************************************************************************************************************************/
/*! \brief          the remote procedure call, for a caller that has no number to look up
 *
 *  \return         the command, or nullptr if it is not in the table
******************************************************************************************************************************************************/
const MessageCatalog::CommandType* MessageCatalog::findRemoteProcedureCallCommand()
{
    return findCommandByNumber(MsgCmdParser::COMMAND_REMOTE_PROCEDURE_CALL);
} /* findRemoteProcedureCallCommand */


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
