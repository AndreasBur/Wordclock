// The commands as a line arrives over the serial port or the web socket: the number picks the
// parser, the options change the clock.

#include "check.h"
#include "cases.h"

#include "Clock.h"
#include "ColorCycle.h"
#include "Display.h"
#include "Message.h"
#include "MsgCmdParser.h"
#include "MsgCmdRemoteProcedureCallParser.h"
#include "NightSwitch.h"
#include "Overlays.h"
#include "RealTimeClock.h"
#include "Text.h"

#include <string>

/*
 * What is checked is what a command changes, not what it answers. The simulator's port collects
 * no output until its window is attached, and none is here; the answers are what the host suites
 * of the two networked backends read off the socket.
 */

namespace {
using CommandType = MsgCmdParser::CommandType;
using RpcIdType = MsgCmdRemoteProcedureCallParser::RpcIdType;

/*
 * The numbers the parsers' documentation, the console and the panel give - pinned, because
 * inserting an enumerator moves every number behind it and nothing else would notice. What
 * moves is what somebody has written down. The last of each list is pinned too, which is what
 * catches an insertion anywhere before it; appending stays free.
 */
#if (OVERLAYS_SUPPORT_DATE == STD_ON) && (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON) && (OVERLAYS_SUPPORT_TEXT == STD_ON)
static_assert(ToUnderlying(CommandType::kRemoteProcedureCall) == 1u, "command 1 is the procedure call");
static_assert(ToUnderlying(CommandType::kDisplayColor) == 2u, "command 2 is the display colour");
static_assert(ToUnderlying(CommandType::kOverlayTemperature) == 6u, "command 6 is the temperature overlay");
static_assert(ToUnderlying(CommandType::kClockMode) == 8u, "command 8 is the clock mode");
static_assert(ToUnderlying(CommandType::kTime) == 10u, "command 10 is the time");
static_assert(ToUnderlying(CommandType::kDate) == 11u, "command 11 is the date");
static_assert(ToUnderlying(CommandType::kNightSwitch) == 14u, "command 14 is the night switch");
static_assert(ToUnderlying(CommandType::kColorCycle) == 15u, "command 15 is the colour cycle");
static_assert(ToUnderlying(CommandType::kConsole) == 16u, "command 16 is the console, the last");
#endif
static_assert(ToUnderlying(RpcIdType::kDisplayEnable) == 3u, "procedure 3 switches the display on");
static_assert(ToUnderlying(RpcIdType::kPowerOn) == 20u, "procedure 20 asks for the supply");
static_assert(ToUnderlying(RpcIdType::kClockRefresh) == 22u, "procedure 22 redraws the time");
static_assert(ToUnderlying(RpcIdType::kNetworkReconnect) == 33u, "procedure 33 joins the network again");
static_assert(ToUnderlying(RpcIdType::kDisplayColorReset) == 40u, "procedure 40 resets the colour, the last");

std::string Option(char name, unsigned value)
{
    return std::string("-") + name + std::to_string(value);
}

// The whole way a received line takes: into a Message as the port fills one, then MsgCmdParser,
// which reads the number and hands the rest to the command's parser.
void Send(CommandType command, const std::string& options)
{
    const std::string line = std::to_string(ToUnderlying(command)) + " " + options;
    Message message;

    for(const char character : line) {
        Expect(message.AddChar(character) == kEOk, "a test command must fit the message buffer");
    }
    MsgCmdParser(message).Parse();
}

void Call(unsigned procedure)
{
    Send(CommandType::kRemoteProcedureCall, Option('P', procedure));
}

void Call(RpcIdType procedure)
{
    Call(ToUnderlying(procedure));
}

bool IsTime(byte hour, byte minute, byte second)
{
    const ClockTime time = RealTimeClock::GetInstance().GetTime();
    return (time.GetHour() == hour) && (time.GetMinute() == minute) && (time.GetSecond() == second);
}

bool IsDate(uint16_t year, byte month, byte day)
{
    const ClockDate date = RealTimeClock::GetInstance().GetDate();
    return (date.GetYear() == year) && (date.GetMonth() == month) && (date.GetDay() == day);
}

bool IsColor(Pixel color, byte red, byte green, byte blue)
{
    return (color.GetRed() == red) && (color.GetGreen() == green) && (color.GetBlue() == blue);
}

}

void TestTimeCommandSetsTheClock()
{
    const ClockTime before = RealTimeClock::GetInstance().GetTime();

    SetTime(8u, 15u, 0u);
    Send(CommandType::kTime, "-H12 -M34 -S56");
    Expect(IsTime(12u, 34u, 56u), "each option of the time command sets its part of the time");

    Send(CommandType::kTime, "-M7");
    Expect(IsTime(12u, 7u, 56u), "an option the command does not carry keeps its part");

    // Out of the clock's range but not of the option's uint8, so it is the time that refuses
    // it - and the rest of the line still applies.
    Send(CommandType::kTime, "-H24 -M8");
    Expect(IsTime(12u, 8u, 56u), "an hour of 24 is refused and the minute after it still set");
    Send(CommandType::kTime, "-M60 -S60");
    Expect(IsTime(12u, 8u, 56u), "a minute or a second of 60 is refused");

    Send(CommandType::kTime, "");
    Expect(IsTime(12u, 8u, 56u), "the command without options is a query");

    RealTimeClock::GetInstance().SetTime(before);
}

void TestDateCommandSetsTheCalendar()
{
    const ClockDate before = RealTimeClock::GetInstance().GetDate();

    RealTimeClock::GetInstance().SetDate(ClockDate(2026u, 3u, 1u));
    Send(CommandType::kDate, "-Y2030 -M6 -D15");
    Expect(IsDate(2030u, 6u, 15u), "each option of the date command sets its part of the date");

    Send(CommandType::kDate, "-D20");
    Expect(IsDate(2030u, 6u, 20u), "an option the command does not carry keeps its part");

    Send(CommandType::kDate, "-Y1999");
    Send(CommandType::kDate, "-Y2100");
    Expect(IsDate(2030u, 6u, 20u), "a year outside 2000 to 2099 is refused");
    Send(CommandType::kDate, "-M0");
    Send(CommandType::kDate, "-M13");
    Expect(IsDate(2030u, 6u, 20u), "a month outside 1 to 12 is refused");
    Send(CommandType::kDate, "-D0");
    Send(CommandType::kDate, "-D32");
    Expect(IsDate(2030u, 6u, 20u), "a day outside 1 to 31 is refused");

    RealTimeClock::GetInstance().SetDate(before);
}

void TestClockModeCommandSetsTheWording()
{
    Clock& clock = Clock::GetInstance();
    const Clock::ModeType mode_before = clock.GetMode();
    const bool it_is_before = clock.GetShowItIsPermanently();

    clock.SetMode(Clock::ModeType::kWessi);
    Send(CommandType::kClockMode, Option('M', ToUnderlying(Clock::ModeType::kOssi)));
    Expect(clock.GetMode() == Clock::ModeType::kOssi, "-M selects the regional mode");

    Send(CommandType::kClockMode, Option('M', Clock::kNumberOfModes));
    Expect(clock.GetMode() == Clock::ModeType::kOssi, "a mode past the last is refused");

    Send(CommandType::kClockMode, "-I1");
    Expect(clock.GetShowItIsPermanently(), "-I1 has ES IST stand at every time");
    Send(CommandType::kClockMode, "-I0");
    Expect(!clock.GetShowItIsPermanently(), "and -I0 only at the full and the half hour");

    clock.SetMode(mode_before);
    clock.SetShowItIsPermanently(it_is_before);
}

void TestDisplayColorCommandRecoloursTheWords()
{
    Display& display = Display::GetInstance();
    const Pixel before = display.GetColor();

    display.SetColor(255u, 255u, 255u);
    DrawClockFace(10u, 0u);
    Send(CommandType::kDisplayColor, "-R10 -G20 -B30");
    Expect(IsColor(display.GetColor(), 10u, 20u, 30u), "the colour command sets all three channels");
    // Against the same face drawn in the new colour rather than against the three values: the
    // buffer holds the colour already dimmed.
    const PixelBufferType recoloured = ReadPixels();
    Expect(ArePixelsEqual(recoloured, DrawClockFace(10u, 0u)), "and the words already lit take the colour at once");

    Send(CommandType::kDisplayColor, "-G200");
    Expect(IsColor(display.GetColor(), 10u, 200u, 30u), "a channel the command does not carry keeps its value");

    display.SetColor(before);
    display.Clear();
}

void TestColorCycleCommandStartsAndStops()
{
    ColorCycle& cycle = ColorCycle::GetInstance();
    Display& display = Display::GetInstance();
    const Pixel color_before = display.GetColor();
    const byte task_cycle_before = cycle.GetTaskCycle();

    cycle.SetIsActive(false);
    display.SetColor(10u, 20u, 30u);
    const PixelBufferType chosen = DrawClockFace(10u, 0u);

    Send(CommandType::kColorCycle, "-S7");
    Expect(cycle.GetTaskCycle() == 7u, "-S sets the ticks per step of hue");
    Expect(!cycle.GetIsActive(), "without starting the cycle");

    Send(CommandType::kColorCycle, "-A1");
    Expect(cycle.GetIsActive(), "-A1 starts the cycle");
    Expect(!ArePixelsEqual(ReadPixels(), chosen), "which paints the words in its own hue at once");
    Expect(IsColor(display.GetColor(), 10u, 20u, 30u), "while the colour of command 2 is kept");

    const byte hue = cycle.GetHue();
    Send(CommandType::kColorCycle, Option('H', (hue + 1u) % 256u));
    Expect(cycle.GetHue() == hue, "the hue is answered but cannot be set");

    Send(CommandType::kColorCycle, "-A0");
    Expect(!cycle.GetIsActive(), "-A0 stops the cycle");
    Expect(ArePixelsEqual(ReadPixels(), chosen), "and puts the colour of command 2 back at once");

    cycle.SetTaskCycle(task_cycle_before);
    display.SetColor(color_before);
    display.Clear();
}

void TestNightSwitchCommandSetsTheWindow()
{
    NightSwitch& night_switch = NightSwitch::GetInstance();
    const ClockTime time_before = RealTimeClock::GetInstance().GetTime();
    const bool active_before = night_switch.GetIsActive();
    const NightSwitch::HourType start_hour_before = night_switch.GetStartHour();
    const NightSwitch::MinuteType start_minute_before = night_switch.GetStartMinute();
    const NightSwitch::HourType end_hour_before = night_switch.GetEndHour();
    const NightSwitch::MinuteType end_minute_before = night_switch.GetEndMinute();
    const byte brightness_before = night_switch.GetNightBrightness();

    // Noon, well outside the window, so switching it on below has no night to start.
    SetTime(12u, 0u, 0u);
    night_switch.SetIsActive(false);

    Send(CommandType::kNightSwitch, "-H22 -M30 -E6 -N15 -B40");
    Expect(night_switch.GetStartHour() == 22u && night_switch.GetStartMinute() == 30u,
           "-H and -M set the start of the night");
    Expect(night_switch.GetEndHour() == 6u && night_switch.GetEndMinute() == 15u,
           "-E and -N set its end");
    Expect(night_switch.GetNightBrightness() == 40u, "-B sets what the night dims to");
    Expect(!night_switch.GetIsActive(), "none of them switches the night switch on");

    Send(CommandType::kNightSwitch, "-A1");
    Expect(night_switch.GetIsActive(), "-A1 switches it on");
    Send(CommandType::kNightSwitch, "-A0");
    Expect(!night_switch.GetIsActive(), "and -A0 off");

    night_switch.SetStartHour(start_hour_before);
    night_switch.SetStartMinute(start_minute_before);
    night_switch.SetEndHour(end_hour_before);
    night_switch.SetEndMinute(end_minute_before);
    night_switch.SetNightBrightness(brightness_before);
    night_switch.SetIsActive(active_before);
    RealTimeClock::GetInstance().SetTime(time_before);
}

// Without the overlay its command does not exist, and the case has nothing to send.
void TestOverlayCommandSetsTheTemperatureOverlay()
{
#if (OVERLAYS_SUPPORT_TEMPERATURE == STD_ON)
    Overlays& overlays = Overlays::GetInstance();
    const auto period_before = overlays.GetTemperaturePeriodInMinutes();
    const auto endurance_before = overlays.GetTemperatureEnduranceInSeconds();
    const auto month_before = overlays.GetTemperatureMonth();
    const auto day_before = overlays.GetTemperatureDay();
    const auto valid_in_days_before = overlays.GetTemperatureValidInDays();
    const byte speed_before = overlays.GetTemperatureSpeed();
    const Text::FontType font_before = overlays.GetTemperatureFont();
    const bool active_before = overlays.GetTemperatureIsActive();
    const unsigned font = (static_cast<unsigned>(ToUnderlying(font_before)) + 1u) % Text::kNumberOfFonts;

    Send(CommandType::kOverlayTemperature, "-P15 -E8 -M3 -D4 -V5 -S100 " + Option('F', font));
    Expect(overlays.GetTemperaturePeriodInMinutes() == 15u, "-P sets the period");
    Expect(overlays.GetTemperatureEnduranceInSeconds() == 8u, "-E sets the endurance");
    Expect(overlays.GetTemperatureMonth() == 3u && overlays.GetTemperatureDay() == 4u,
           "-M and -D set the date it is limited to");
    Expect(overlays.GetTemperatureValidInDays() == 5u, "-V sets the days it goes on for");
    Expect(overlays.GetTemperatureSpeed() == 100u, "-S sets the speed of the text");
    Expect(static_cast<unsigned>(ToUnderlying(overlays.GetTemperatureFont())) == font, "-F sets the font");

    Send(CommandType::kOverlayTemperature, "-P0 -E0 " + Option('F', Text::kNumberOfFonts));
    Expect(overlays.GetTemperaturePeriodInMinutes() == 15u, "a period of 0 is refused");
    Expect(overlays.GetTemperatureEnduranceInSeconds() == 8u, "an endurance of 0 is refused");
    Expect(static_cast<unsigned>(ToUnderlying(overlays.GetTemperatureFont())) == font, "a font past the last is refused");

    Send(CommandType::kOverlayTemperature, "-A1");
    Expect(overlays.GetTemperatureIsActive(), "-A1 switches the overlay on");
    Send(CommandType::kOverlayTemperature, "-A0");
    Expect(!overlays.GetTemperatureIsActive(), "and -A0 off");

    overlays.SetTemperaturePeriodInMinutes(period_before);
    overlays.SetTemperatureEnduranceInSeconds(endurance_before);
    overlays.SetTemperatureMonth(month_before);
    overlays.SetTemperatureDay(day_before);
    overlays.SetTemperatureValidInDays(valid_in_days_before);
    overlays.SetTemperatureSpeed(speed_before);
    overlays.SetTemperatureFont(font_before);
    overlays.SetTemperatureIsActive(active_before);
#endif
}

void TestProceduresChangeTheDisplay()
{
    Display& display = Display::GetInstance();
    const Pixel color_before = display.GetColor();
    const byte brightness_before = display.GetBrightness();
    const bool automatic_before = display.GetBrightnessUseAutomatic();
    const bool gamma_before = display.GetBrightnessUseGammaCorrection();
    const bool enabled_before = display.IsEnabled();

    display.SetColor(100u, 100u, 100u);
    Call(RpcIdType::kDisplayColorRedIncrement);
    Call(RpcIdType::kDisplayColorGreenIncrement);
    Call(RpcIdType::kDisplayColorGreenIncrement);
    Call(RpcIdType::kDisplayColorBlueDecrement);
    Expect(IsColor(display.GetColor(), 101u, 102u, 99u), "the colour procedures step one channel each");
    Call(RpcIdType::kDisplayColorRedDecrement);
    Call(RpcIdType::kDisplayColorGreenDecrement);
    Call(RpcIdType::kDisplayColorBlueIncrement);
    Expect(IsColor(display.GetColor(), 100u, 101u, 100u), "in both directions");
    Call(RpcIdType::kDisplayColorReset);
    Expect(IsColor(display.GetColor(), 255u, 255u, 255u), "and the reset makes it white again");

    display.SetBrightness(100u);
    Call(RpcIdType::kDisplayBrightnessIncrement);
    Expect(display.GetBrightness() == 101u, "the brightness steps up by one");
    Call(RpcIdType::kDisplayBrightnessDecrement);
    Call(RpcIdType::kDisplayBrightnessDecrement);
    Expect(display.GetBrightness() == 99u, "and down by one");

    Call(RpcIdType::kDisplayBrightnessAutomaticOn);
    Expect(display.GetBrightnessUseAutomatic(), "the automatic is switched on");
    Call(RpcIdType::kDisplayBrightnessAutomaticToggle);
    Expect(!display.GetBrightnessUseAutomatic(), "toggled off");
    Call(RpcIdType::kDisplayBrightnessAutomaticToggle);
    Expect(display.GetBrightnessUseAutomatic(), "toggled on again");
    Call(RpcIdType::kDisplayBrightnessAutomaticOff);
    Expect(!display.GetBrightnessUseAutomatic(), "and switched off");

    Call(RpcIdType::kDisplayBrightnessGammaCorrectionOn);
    Expect(display.GetBrightnessUseGammaCorrection(), "the gamma correction is switched on");
    Call(RpcIdType::kDisplayBrightnessGammaCorrectionToggle);
    Expect(!display.GetBrightnessUseGammaCorrection(), "toggled off");
    Call(RpcIdType::kDisplayBrightnessGammaCorrectionToggle);
    Expect(display.GetBrightnessUseGammaCorrection(), "toggled on again");
    Call(RpcIdType::kDisplayBrightnessGammaCorrectionOff);
    Expect(!display.GetBrightnessUseGammaCorrection(), "and switched off");

    Call(RpcIdType::kDisplayDisable);
    Expect(!display.IsEnabled(), "the display is switched off");
    Call(RpcIdType::kDisplayToggle);
    Expect(display.IsEnabled(), "toggled on");
    Call(RpcIdType::kDisplayToggle);
    Expect(!display.IsEnabled(), "toggled off again");
    Call(RpcIdType::kDisplayEnable);
    Expect(display.IsEnabled(), "and switched on");

    display.SetColor(color_before);
    display.SetBrightness(brightness_before);
    display.SetBrightnessUseAutomatic(automatic_before);
    display.SetBrightnessUseGammaCorrection(gamma_before);
    if(enabled_before) { display.Enable(); } else { display.Disable(); }
}

void TestProcedureSelectionsWrapAround()
{
    Animations& animations = Animations::GetInstance();
    Clock& clock = Clock::GetInstance();
    const Animations::AnimationIdType animation_before = animations.GetAnimation();
    const Clock::ModeType mode_before = clock.GetMode();
    const auto last_animation = static_cast<Animations::AnimationIdType>(Animations::kNumberOfAnimations - 1u);

    Expect(animations.SetAnimation(last_animation) == kEOk, "the last animation must be selectable");
    Call(RpcIdType::kAnimationNext);
    Expect(animations.GetAnimation() == Animations::AnimationIdType::kNone, "after the last animation comes id 0");
    Call(RpcIdType::kAnimationPrevious);
    Expect(animations.GetAnimation() == last_animation, "and before id 0 the last");

    clock.SetMode(static_cast<Clock::ModeType>(Clock::kNumberOfModes - 1u));
    Call(RpcIdType::kClockModeNext);
    Expect(clock.GetMode() == static_cast<Clock::ModeType>(0u), "after the last regional mode comes the first");

    animations.SetAnimation(animation_before);
    clock.SetMode(mode_before);
}

void TestUnknownProcedureRunsNothing()
{
    Display& display = Display::GetInstance();
    const Pixel color_before = display.GetColor();
    const byte brightness_before = display.GetBrightness();
    const bool enabled_before = display.IsEnabled();

    // 0 is no procedure, the count is one past the last, and 300 does not fit the option's uint8.
    Call(0u);
    Call(ToUnderlying(RpcIdType::kNumberOfProcedures));
    Call(300u);
    Send(CommandType::kRemoteProcedureCall, "");

    Expect(IsColor(display.GetColor(), color_before.GetRed(), color_before.GetGreen(), color_before.GetBlue()) &&
           display.GetBrightness() == brightness_before && display.IsEnabled() == enabled_before,
           "an id that is no procedure changes nothing");
}
