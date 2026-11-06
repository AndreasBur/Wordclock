// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief What the serial commands are called and which options they take.
 *
 * See the header on why this table is here and what it mirrors.
 */

#define _MESSAGE_CATALOG_SOURCE_

#include "MessageCatalog.h"
// The command numbers are taken from here rather than copied, so they cannot fall behind
// the firmware. Its OVERLAYS_SUPPORT_* switches reach the entries below through it too.
#include "MsgCmdParser.h"
// For RpcIdType alone, which the procedure names below are counted against - they are the
// one list here that is written beside the firmware rather than derived from it.
#include "MsgCmdRemoteProcedureCallParser.h"
/*
 * Not for any type, only for the ANIMATIONS_SUPPORT_* and TEXT_SUPPORT_FONT_* switches:
 * the value lists below are guarded by exactly the same ones as the enums they name, so
 * they cannot end up naming the wrong value when one is switched off.
 */
#include "Animations.h"
#include "Text.h"
/*
 * For the ranges. The limits themselves are private to these classes, but the predicates
 * that test them are public and constexpr, so the range can be measured rather than
 * copied.
 */
#include "ClockDate.h"
#include "ClockTime.h"
#include "Display.h"
#include <limits>

// Everything up to the member functions is local to this file, so Doxygen leaves it out.
/*! \cond */

// For the options the firmware bounds only by the width of the argument: the overlay
// periods and endurances reject nothing but zero, and the speeds nothing at all.
#define BYTE_MAX                                        255u

// Counted rather than written out, so a value list and its length cannot drift apart.
#define NUMBER_OF(Array)                                static_cast<byte>(sizeof(Array) / sizeof((Array)[0]))

// The bounds of an option, measured off the firmware's own predicate instead of copied
// from beside it.
#define MEASURED(ValueType, IsValid)                    FindMinimum<ValueType>(IsValid), FindMaximum<ValueType>(IsValid)

/*
 * The tables below sit outside the class, so that they can be constexpr and be checked at
 * compile time. This brings the argument types back within reach without the class's name,
 * which keeps the tables readable.
 */
using ArgumentType = MessageCatalog::ArgumentType;

/*
 * Walk the whole range of the value's own type and ask the firmware which values it
 * accepts. Searching beats copying the limits: they are private, and a copy is a number
 * that can quietly fall behind. The type parameter keeps the search inside what the
 * predicate takes, so a byte predicate is never asked about 300.
 */
template <typename ValueType, typename PredicateType>
constexpr uint16_t FindMinimum(PredicateType is_valid)
{
    constexpr uint32_t kLimit{std::numeric_limits<ValueType>::max()};

    for(uint32_t value = 0u; value <= kLimit; value++) {
        if(is_valid(static_cast<ValueType>(value))) { return static_cast<uint16_t>(value); }
    }

    return 0u;
}

template <typename ValueType, typename PredicateType>
constexpr uint16_t FindMaximum(PredicateType is_valid)
{
    constexpr uint32_t kLimit{std::numeric_limits<ValueType>::max()};

    for(uint32_t value = kLimit + 1u; value > 0u; value--) {
        if(is_valid(static_cast<ValueType>(value - 1u))) { return static_cast<uint16_t>(value - 1u); }
    }

    return 0u;
}

// The names of every option that carries a flag rather than a number.
constexpr const char* const kBooleanValueNames[] {
    "off", "on"
};

/*
 * MsgCmdRemoteProcedureCallParser::RpcIdType, from RpcIdType::kNone + 1 onwards. NONE is the
 * initial value of the parser's rpc_id_ member, not a procedure: asking for it lands in the
 * default case and answers ErrorType::kRpcIdUnknown. Offering it would offer a guaranteed
 * error, the same reason CommandType::kNone is left out of the command list.
 */
constexpr const char* const kRemoteProcedureValueNames[] {
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
    "Settings reset",
    "System restart",
    "Time resynchronise",
    "Network reconnect",
    "Animation next",
    "Animation previous",
    "Clock mode next",
    "Display toggle",
    "Brightness automatic toggle",
    "Gamma correction toggle",
    "Colour reset"
};

/*
 * The command numbers and the value lists of the enumerations come out of the firmware
 * itself; this list does not, so it is the one that can fall behind. A procedure added to
 * RpcIdType without a name here would be offered as the name of the procedure before it.
 * RpcIdType::kNone has no name, hence the one.
 */
static_assert(NUMBER_OF(kRemoteProcedureValueNames) ==
              MsgCmdRemoteProcedureCallParser::kNumberOfProcedures - 1u,
              "the remote procedure names have fallen behind MsgCmdRemoteProcedureCallParser::RpcIdType");

// The names of Clock::ModeType.
constexpr const char* const kClockModeValueNames[] {
    "Wessi", "Ossi", "Rhein-Ruhr", "Schwaben"
};

// Animations::AnimationIdType, under the same switches as the enum. Its values shift as
// soon as one animation is switched off, so a fixed list would name the wrong ones.
constexpr const char* const kAnimationValueNames[] {
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

// The names of Animations::ModeType.
constexpr const char* const kAnimationModeValueNames[] {
    "Fixed", "Random", "Sequence"
};

// The names of Text::FontType, under the same switches as the enum.
constexpr const char* const kFontValueNames[] {
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

// Where the values are named, the names are the range: the first one sits at the minimum
// and there are as many values as names.
#define NAMED(Minimum, Names)                           (Minimum), static_cast<uint16_t>((Minimum) + NUMBER_OF(Names) - 1u), Names, NUMBER_OF(Names)

// The options of MsgCmdRemoteProcedureCallParser. Starts at one, RpcIdType::kNone being left out.
constexpr MessageCatalog::OptionType kRemoteProcedureCallOptions[] {
    {'P', "Procedure",          ArgumentType::kUint8,  NAMED(1u, kRemoteProcedureValueNames)}
};

// The options of MsgCmdDisplayColorParser.
constexpr MessageCatalog::OptionType kDisplayColorOptions[] {
    {'R', "Red",                ArgumentType::kUint8,  0u, BYTE_MAX},
    {'G', "Green",              ArgumentType::kUint8,  0u, BYTE_MAX},
    {'B', "Blue",               ArgumentType::kUint8,  0u, BYTE_MAX}
};

// The options of MsgCmdDisplayBrightnessParser.
constexpr MessageCatalog::OptionType kDisplayBrightnessOptions[] {
    {'B', "Brightness",         ArgumentType::kUint8,  0u, BYTE_MAX},
    {'A', "Automatic",          ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)},
    {'G', "Gamma correction",   ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)}
};

// MsgCmdDisplayPixelParser. The index is a uint16 as soon as the display has more than
// 255 pixels; 110 here, so the parser compiles it as a uint8.
constexpr MessageCatalog::OptionType kDisplayPixelOptions[] {
    {'I', "Index",              ArgumentType::kUint8,  0u, DISPLAY_NUMBER_OF_PIXELS - 1u},
    {'S', "State",              ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)}
};

/*
 * MsgCmdBaseOverlayParser, shared by the date, temperature and text commands. Only the
 * text overlay reads the text: the other two answer the parser's SetText() and
 * SendAnswerText() with empty bodies, so a text sent to them is accepted, discarded and
 * never answered. It is therefore the last entry rather than one in the middle, which
 * lets those two share this table and simply stop one option short of it.
 */
constexpr MessageCatalog::OptionType kOverlayOptions[] {
    {'P', "Period (minutes)",   ArgumentType::kUint8,  1u, BYTE_MAX},
    {'E', "Endurance (s)",      ArgumentType::kUint8,  1u, BYTE_MAX},
    {'M', "Month",              ArgumentType::kUint8,  0u, 12u},
    {'D', "Day",                ArgumentType::kUint8,  0u, 31u},
    {'V', "Valid in days",      ArgumentType::kUint8,  0u, BYTE_MAX},
    {'A', "Active",             ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)},
    {'S', "Speed",              ArgumentType::kUint8,  0u, BYTE_MAX},
    {'F', "Font",               ArgumentType::kUint8,  NAMED(0u, kFontValueNames)},
    {'T', "Text",               ArgumentType::kString, 0u, 0u}
};

// The overlays that have no text of their own stop one short of the shared table. Derived
// from its length rather than written out, so it follows if an option is added.
constexpr byte kNumberOfOverlayOptionsWithoutText{static_cast<byte>(NUMBER_OF(kOverlayOptions) - 1u)};

/*
 * Stopping one short only leaves out the text as long as the text is what sits last.
 * Appending an option would otherwise hide that one and offer the text instead, to two
 * overlays that do not read it - quietly, since both lists would still be the right
 * length.
 */
static_assert(kOverlayOptions[kNumberOfOverlayOptionsWithoutText].short_name == 'T',
              "the text has to be the last of the overlay options, see NumberOfOverlayOptionsWithoutText");

/*
 * MsgCmdStatusParser. Every one of them read-only: the command takes no options at all,
 * and these names exist so that its answer can be read back through them. The bounds are
 * therefore left at zero - nothing offers these to be typed, so nothing has a range to
 * check against.
 */
#define READ_ONLY                                       0u, 0u, nullptr, 0u, true

constexpr MessageCatalog::OptionType kStatusOptions[] {
    {'V', "Firmware version",   ArgumentType::kString, READ_ONLY},
    /*
     * Three fields rather than one count. A single number has to be in some unit, and
     * every unit that fits a uint16 runs out on a clock that is meant to hang on a wall
     * for years - minutes did, after 45 days. Days carry the range, hours and minutes the
     * resolution, and none of them has to be divided by a reader.
     */
    {'U', "Uptime (days)",      ArgumentType::kUint16, READ_ONLY},
    {'H', "Uptime (hours)",     ArgumentType::kUint8,  READ_ONLY},
    {'N', "Uptime (minutes)",   ArgumentType::kUint8,  READ_ONLY},
    {'I', "Illuminance (lx)",   ArgumentType::kUint16, READ_ONLY},
    {'T', "Temperature",        ArgumentType::kString, READ_ONLY},
    {'A', "Address",            ArgumentType::kString, READ_ONLY},
    {'Q', "Link quality (dBm)", ArgumentType::kString, READ_ONLY},
    {'M', "Free memory (KiB)",  ArgumentType::kUint16, READ_ONLY}
};

// MsgCmdNightSwitchParser. Brightness zero is the display switched off rather than a
// dimming of nothing, which is what lets one field carry both wishes.
constexpr MessageCatalog::OptionType kNightSwitchOptions[] {
    {'A', "Active",             ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)},
    {'H', "Start hour",         ArgumentType::kUint8,  0u, 23u},
    {'M', "Start minute",       ArgumentType::kUint8,  0u, 59u},
    {'E', "End hour",           ArgumentType::kUint8,  0u, 23u},
    {'N', "End minute",         ArgumentType::kUint8,  0u, 59u},
    {'B', "Night brightness",   ArgumentType::kUint8,  0u, BYTE_MAX}
};

/*
 * MsgCmdColorCycleParser. The hue is read-only: it is where the wheel has got to, which is
 * worth seeing and not worth setting - command 2 is how a colour is chosen. Speed is the same
 * kind of number as the animation speed above, scheduler ticks per step, and zero there is a
 * cycle standing still rather than a value to refuse.
 */
constexpr MessageCatalog::OptionType kColorCycleOptions[] {
    {'A', "Active",             ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)},
    {'S', "Speed",              ArgumentType::kUint8,  0u, BYTE_MAX},
    {'H', "Hue",                ArgumentType::kUint8,  READ_ONLY}
};

/*
 * MsgCmdConsoleParser. The password is an input and never an answer, the same as the pass
 * phrase below; what is read back is whether one is set. Sending the command with no password
 * at all clears it, which is the way back for a clock nobody can log into any more.
 */
constexpr MessageCatalog::OptionType kConsoleOptions[] {
    {'P', "Console password",   ArgumentType::kString, 0u, 0u},
    {'A', "Protected",          ArgumentType::kUint8,  READ_ONLY}
};

// MsgCmdNetworkParser. The pass phrase is an input and never an answer, so it carries no
// range and is never read back - see the parser for why.
constexpr MessageCatalog::OptionType kNetworkOptions[] {
    {'S', "Network name",       ArgumentType::kString, 0u, 0u},
    {'P', "Pass phrase",        ArgumentType::kString, 0u, 0u}
};

// The options of MsgCmdClockModeParser.
constexpr MessageCatalog::OptionType kClockModeOptions[] {
    {'M', "Mode",               ArgumentType::kUint8,  NAMED(0u, kClockModeValueNames)},
    {'I', "It is permanently",  ArgumentType::kUint8,  NAMED(0u, kBooleanValueNames)}
};

// The options of MsgCmdAnimationParser.
constexpr MessageCatalog::OptionType kAnimationOptions[] {
    {'A', "Animation",          ArgumentType::kUint8,  NAMED(0u, kAnimationValueNames)},
    {'M', "Mode",               ArgumentType::kUint8,  NAMED(0u, kAnimationModeValueNames)},
    {'S', "Speed",              ArgumentType::kUint8,  0u, BYTE_MAX},
    {'F', "Favourite",          ArgumentType::kUint8,  0u, BYTE_MAX}
};

// The options of MsgCmdTimeParser.
constexpr MessageCatalog::OptionType kTimeOptions[] {
    {'H', "Hour",               ArgumentType::kUint8,  MEASURED(ClockTime::HourType, ClockTime::IsHourValid)},
    {'M', "Minute",             ArgumentType::kUint8,  MEASURED(ClockTime::MinuteType, ClockTime::IsMinuteValid)},
    {'S', "Second",             ArgumentType::kUint8,  MEASURED(ClockTime::SecondType, ClockTime::IsSecondValid)}
};

// The options of MsgCmdDateParser.
constexpr MessageCatalog::OptionType kDateOptions[] {
    {'Y', "Year",               ArgumentType::kUint16, MEASURED(ClockDate::YearType, ClockDate::IsYearValid)},
    {'M', "Month",              ArgumentType::kUint8,  MEASURED(ClockDate::MonthType, ClockDate::IsMonthValid)},
    {'D', "Day",                ArgumentType::kUint8,  MEASURED(ClockDate::DayType, ClockDate::IsDayValid)}
};

/*
 * The numbers come from the enumeration itself, so they follow it instead of having to be
 * kept level with it - including when a support switch shortens it, which is why the
 * overlay entries carry the same guards the enumerators do. CommandType::kNone is left out, it
 * is not a command that can be sent. Sending a command with no option at all asks the
 * firmware for its current settings, which is why every entry is useful on its own.
 */
constexpr MessageCatalog::CommandType kCommands[] {
    {ToUnderlying(MsgCmdParser::CommandType::kRemoteProcedureCall), "Remote procedure call", kRemoteProcedureCallOptions, NUMBER_OF(kRemoteProcedureCallOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kDisplayColor),         "Display colour",        kDisplayColorOptions,        NUMBER_OF(kDisplayColorOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kDisplayBrightness),    "Display brightness",    kDisplayBrightnessOptions,   NUMBER_OF(kDisplayBrightnessOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kDisplayPixel),         "Display pixel",         kDisplayPixelOptions,        NUMBER_OF(kDisplayPixelOptions)},
#if (OVERLAYS_SUPPORT_DATE == STD_ON)
    {ToUnderlying(MsgCmdParser::CommandType::kOverlayDate),          "Overlay date",          kOverlayOptions,             kNumberOfOverlayOptionsWithoutText},
#endif
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    {ToUnderlying(MsgCmdParser::CommandType::kOverlayTemperature),   "Overlay temperature",   kOverlayOptions,             kNumberOfOverlayOptionsWithoutText},
#endif
#if (OVERLAYS_SUPPORT_TEXT == STD_ON)
    {ToUnderlying(MsgCmdParser::CommandType::kOverlayText),          "Overlay text",          kOverlayOptions,             NUMBER_OF(kOverlayOptions)},
#endif
    {ToUnderlying(MsgCmdParser::CommandType::kClockMode),            "Clock mode",            kClockModeOptions,           NUMBER_OF(kClockModeOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kAnimation),             "Animation",             kAnimationOptions,           NUMBER_OF(kAnimationOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kTime),                  "Time",                  kTimeOptions,                NUMBER_OF(kTimeOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kDate),                  "Date",                  kDateOptions,                NUMBER_OF(kDateOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kStatus),                "Status",                kStatusOptions,              NUMBER_OF(kStatusOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kNetwork),               "Network",               kNetworkOptions,             NUMBER_OF(kNetworkOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kNightSwitch),          "Night switch",          kNightSwitchOptions,         NUMBER_OF(kNightSwitchOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kColorCycle),           "Colour cycle",          kColorCycleOptions,          NUMBER_OF(kColorCycleOptions)},
    {ToUnderlying(MsgCmdParser::CommandType::kConsole),               "Console access",        kConsoleOptions,             NUMBER_OF(kConsoleOptions)}
};

constexpr byte kNumberOfCommands{NUMBER_OF(kCommands)};

/*
 * The message builder builds this many option rows once and shows the ones a command
 * needs, so a command with more options than that would have some of them unreachable.
 * Checked here rather than trusted, because the two live in different files.
 */
constexpr bool AreOptionCountsWithinBounds()
{
    for(const MessageCatalog::CommandType& command : kCommands) {
        if(command.number_of_options > MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS) { return false; }
    }

    return true;
}

static_assert(AreOptionCountsWithinBounds(),
              "a command has more options than MESSAGE_CATALOG_MAX_NUMBER_OF_OPTIONS allows rows for");

/*
 * Naming the values makes the names the range: the dropdown offers one entry per name and
 * the builder sends Minimum plus the position picked, so a range wider than the list would
 * promise values that cannot be picked, and a narrower one would name values outside it.
 * NAMED() gets this right by construction; this keeps a range written out by hand from
 * getting it wrong.
 */
constexpr bool AreNamedRangesConsistent()
{
    for(const MessageCatalog::CommandType& command : kCommands) {
        for(byte index = 0u; index < command.number_of_options; index++) {
            const MessageCatalog::OptionType& option = command.options[index];

            if(option.value_names == nullptr) { continue; }
            if((option.maximum - option.minimum + 1u) != option.number_of_value_names) { return false; }
        }
    }

    return true;
}

static_assert(AreNamedRangesConsistent(),
              "an option with named values has a range that does not match the number of names");

/*! \endcond */
// The tables live in the source, where they can be constexpr and checked against the
// firmware at compile time. Hence out of line rather than inline in the header.
byte MessageCatalog::GetNumberOfCommands()
{
    return kNumberOfCommands;
}

const MessageCatalog::CommandType& MessageCatalog::GetCommand(byte index)
{
    return kCommands[index];
}

const MessageCatalog::CommandType* MessageCatalog::FindCommandByNumber(byte number)
{
    for(byte index = 0u; index < kNumberOfCommands; index++) {
        if(kCommands[index].number == number) { return &kCommands[index]; }
    }

    return nullptr;
}

const MessageCatalog::CommandType* MessageCatalog::FindRemoteProcedureCallCommand()
{
    return FindCommandByNumber(ToUnderlying(MsgCmdParser::CommandType::kRemoteProcedureCall));
}

const MessageCatalog::OptionType* MessageCatalog::FindOption(const CommandType& command, char short_name)
{
    for(byte index = 0u; index < command.number_of_options; index++) {
        if(command.options[index].short_name == short_name) { return &command.options[index]; }
    }

    return nullptr;
}
