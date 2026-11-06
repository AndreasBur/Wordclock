/*
 * Fuzzes the way a command reaches the clock from the network.
 *
 * The input is one web socket text frame, handed to WebFrontend::InjectCommand() exactly as the
 * simulator's WebHost and the two networked backends hand it a browser's frame. From there it is
 * the firmware's own way: the port, Communication putting lines together, MsgCmdParser picking
 * the command's parser, and that parser setting whatever the line asks for. A newline inside the
 * frame ends a line, so one input can be several commands in a row - which is how a command that
 * only does harm after another one gets found.
 *
 * The ticks after the last line are what lets a setting take effect: a colour reaching the strip,
 * an animation or an overlay that was just started drawing its first frames. Eight, because a tick
 * costs about 26 us with every object instrumented, and the ticks are most of what an input costs:
 * 64 of them ran 517 inputs a second, 8 run 1203. An animation at its fastest still draws eight
 * frames in them.
 */

#include "fuzz.h"

#include "WebFrontend.h"

#include <cstddef>
#include <cstdint>

namespace {
constexpr unsigned kTicksAfterTheLastLine{8u};
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) // NOLINT(readability-identifier-naming)
{
    StartFromANewClock();

    WebFrontend::InjectCommand(reinterpret_cast<const char*>(data), size);
    RunUntilThePortIsRead();
    RunTicks(kTicksAfterTheLastLine);
    return 0;
}
