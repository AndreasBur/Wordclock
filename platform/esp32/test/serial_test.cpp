/* Checks the one thing the whole web console rests on: that an injected character takes
   exactly the same path through Communication as one typed on the wire, and that answers
   come out one line at a time. */
#include "Arduino.h"
#include "Communication.h"
#include "Version.h"
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

    /* The procedures that act now rather than at the next word change. What of them
       reaches the wire is the answer, and it says which of the two happened: the display
       was free and the procedure ran, or it was busy and the procedure stepped aside. */
    /* Ticked until the answer appears rather than exactly twice: the task reads on one
       call and parses on the next, and a message left over from an earlier check would
       otherwise shift every answer by one. */
    auto answerTo = [&](const char* Message) {
        Lines.clear();
        port.inject(Message, strlen(Message));
        for(int Tick = 0; (Tick < 8) && Lines.empty(); Tick++) { communication.task(); }
        return Lines.empty() ? std::string() : Lines[0];
    };

    check(answerTo("1 -P22\n") == "1 RpcId=22 Error=0", "the clock refresh is carried out");
    check(answerTo("1 -P28\n") == "1 RpcId=28 Error=8", "aborting with no overlay showing is refused");
    check(answerTo("1 -P34\n") == "1 RpcId=34 Error=7", "an id past the last procedure is unknown");

    /* The two procedures that need a network, on a host that has none. Refusing is the
       whole point: answering E_OK would tell a caller that the clock is on its way back
       to a network it never had. */
    check(answerTo("1 -P32\n") == "1 RpcId=32 Error=8", "resynchronising without a network is refused");
    check(answerTo("1 -P33\n") == "1 RpcId=33 Error=8", "reconnecting without a configured network is refused");

    /* The status command answers with values and takes none. Every field that has nothing
       behind it comes back empty rather than as a zero that reads like a value - here the
       temperature, with no chip on the bus, and the two network fields. */
    check(answerTo("12\n") == "12 V=" WORDCLOCK_VERSION " U=0 I=1 T= A= Q= M=0",
          "the status answers every field, and the ones with no answer empty");
    check(answerTo("12 -V1\n") == "12 Error=3:V V=" WORDCLOCK_VERSION " U=0 I=1 T= A= Q= M=0",
          "a value sent to a read-only field is refused as an unknown option");

    /* The uptime is the one of them a host can produce. */
    TestMillis = 3u * 60u * 1000u;
    check(answerTo("12\n") == "12 V=" WORDCLOCK_VERSION " U=3 I=1 T= A= Q= M=0",
          "the uptime is reported in minutes");

    /* a full buffer refuses rather than overwriting a command in flight */
    std::string tooMuch(WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE + 8u, 'x');
    check(port.inject(tooMuch.data(), tooMuch.size()) == E_NOT_OK, "an overlong injection is refused");

    printf("\n%s\n", Failures == 0 ? "all checks passed" : "FAILURES");
    return Failures == 0 ? 0 : 1;
}
