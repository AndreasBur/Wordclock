#include <cstdlib>
#include <iostream>

#include "Clock.h"
#include "Scheduler.h"

namespace {

int Failures{0};

void expect(bool condition, const char* description)
{
    if(condition) { return; }

    std::cerr << "FAILED: " << description << '\n';
    Failures++;
}

ClockWords wordsAt(byte hour, byte minute)
{
    ClockWords words;
    expect(Clock::getInstance().getClockWords(hour, minute, words) == E_OK,
           "valid time must produce clock words");
    return words;
}

void testSchedulerSpeedMapping()
{
    expect(Scheduler::convertSpeedToTaskCycle(0u) == 0u, "speed zero stops the task");
    expect(Scheduler::convertSpeedToTaskCycle(1u) == UINT8_MAX, "speed one uses cycle 255");
    expect(Scheduler::convertSpeedToTaskCycle(254u) == 2u, "speed 254 uses cycle two");
    expect(Scheduler::convertSpeedToTaskCycle(255u) == 1u, "speed 255 uses cycle one");

    for(unsigned int speed = 0u; speed <= UINT8_MAX; speed++) {
        const byte originalSpeed = static_cast<byte>(speed);
        const byte cycle = Scheduler::convertSpeedToTaskCycle(originalSpeed);
        expect(Scheduler::convertTaskCycleToSpeed(cycle) == originalSpeed,
               "speed and task cycle conversion must round-trip");
    }
}

void testClockWordSteps()
{
    Clock& clock = Clock::getInstance();
    clock.setModeFast(Clock::MODE_WESSI);

    expect(wordsAt(10u, 0u) == wordsAt(10u, 4u),
           "words must stay unchanged within the first five-minute step");
    expect(wordsAt(10u, 4u) != wordsAt(10u, 5u),
           "words must change at five past");
    expect(wordsAt(10u, 55u) != wordsAt(11u, 0u),
           "words must change across the hour boundary");

    ClockWords invalidWords;
    expect(clock.getClockWords(24u, 0u, invalidWords) == E_NOT_OK,
           "hour 24 must be rejected");
    expect(clock.getClockWords(10u, 60u, invalidWords) == E_NOT_OK,
           "minute 60 must be rejected");
}

void testClockModes()
{
    Clock& clock = Clock::getInstance();
    clock.setModeFast(Clock::MODE_WESSI);
    const ClockWords wessiWords = wordsAt(10u, 15u);

    clock.setModeFast(Clock::MODE_OSSI);
    expect(wessiWords != wordsAt(10u, 15u),
           "changing the clock mode must change the words where modes differ");

    clock.setModeFast(Clock::MODE_WESSI);
}

} // namespace

int main()
{
    testSchedulerSpeedMapping();
    testClockWordSteps();
    testClockModes();

    if(Failures == 0) {
        std::cout << "All Wordclock tests passed.\n";
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}
