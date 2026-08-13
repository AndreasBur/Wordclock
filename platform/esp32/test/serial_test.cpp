/* Checks the one thing the whole web console rests on: that an injected character takes
   exactly the same path through Communication as one typed on the wire, and that answers
   come out one line at a time. */
#include "Arduino.h"
#include "Communication.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


static std::vector<std::string> Lines;
static void collectLine(const char* Line) { Lines.emplace_back(Line); }

static int Failures = 0;
static void check(bool Ok, const char* What) {
    printf("%-56s %s\n", What, Ok ? "ok" : "FAIL");
    if(!Ok) Failures++;
}

int main()
{
    WordclockSerial& port = WordclockSerial::getInstance();
    port.setLineSink(collectLine);

    /* the wire is served first, so a network client cannot starve it */
    WordclockPlatform::hardwarePort().Incoming = "AB";
    port.inject("cd", 2u);
    std::string order;
    while(port.available()) { order.push_back(static_cast<char>(port.read())); }
    check(order == "ABcd", "the UART is drained before the injected characters");

    /* one line per println, and nothing before it */
    Lines.clear();
    port.print("Error=");
    port.print(4);
    check(Lines.empty(), "print alone does not emit a line");
    port.println();
    check(Lines.size() == 1u && Lines[0] == "Error=4", "println emits exactly the collected line");

    /* a newline printed as a character ends the line too - the firmware does that */
    Lines.clear();
    port.print("Api");
    port.print('\n');
    check(Lines.size() == 1u && Lines[0] == "Api", "a printed newline ends the line as println does");

    /* and it reaches the UART unchanged, which is what an attached console sees */
    check(WordclockPlatform::hardwarePort().Written.find("Error=4") != std::string::npos, "the same output still goes to the wire");

    /* the real thing: an injected command must be parsed like a typed one */
    Lines.clear();
    WordclockPlatform::hardwarePort().Written.clear();
    const char* command = "3 -B255\n";   /* terminated with EndOfMessageChar, as on the wire */
    check(port.inject(command, strlen(command)) == E_OK, "a command fits the inject buffer");
    Communication& communication = Communication::getInstance();
    communication.task();                   /* reads the characters */
    communication.task();                   /* parses the complete message */
    check(!Lines.empty(), "the injected command produced an answer");
    if(!Lines.empty()) { printf("   answer: \"%s\"\n", Lines[0].c_str()); }

    /* a full buffer refuses rather than overwriting a command in flight */
    std::string tooMuch(WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE + 8u, 'x');
    check(port.inject(tooMuch.data(), tooMuch.size()) == E_NOT_OK, "an overlong injection is refused");

    printf("\n%s\n", Failures == 0 ? "all checks passed" : "FAILURES");
    return Failures == 0 ? 0 : 1;
}
