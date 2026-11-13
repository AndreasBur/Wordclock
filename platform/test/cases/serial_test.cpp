/*
 * Checks the one thing the whole web console rests on: that an injected character takes
 * exactly the same path through Communication as one typed on the wire, and that answers
 * come out one line at a time.
 */
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "Arduino.h"
#include "Communication.h"
#include "MsgCmdParser.h"
#include "MsgCmdRemoteProcedureCallParser.h"
#include "Uptime.h"
#include "Version.h"
#include "check.h"

static std::vector<std::string> lines;
static void CollectLine(char const* line) { lines.emplace_back(line); }

int main() {
  WordclockSerial& port = WordclockSerial::GetInstance();
  port.SetLineSink(CollectLine);

  // Exercise the parser directly as well: it must be safe for an empty buffer, which
  // Communication hands it for an empty line, and for out-of-range numbers.
  std::string const wrong_command =
      "Error=" + std::to_string(ToUnderlying(ErrorMessage::ErrorType::kWrongCommand));
  char const* invalid_commands[]{"",
                                 " ",
                                 "abc",
                                 "0",
                                 "-1",
                                 "17",
                                 "65537",
                                 "4294967297",
                                 "9999999999999999999999999999999999999999999999999",
                                 "-999999999999999999999999999999999999999999999999"};
  for (char const* command : invalid_commands) {
    Message message;
    for (char const* character = command; *character != '\0'; ++character) {
      Check(message.AddChar(*character) == kEOk, "the command fixture fits the buffer");
    }
    lines.clear();
    MsgCmdParser(message).Parse();
    Check(lines.size() == 1u && lines[0] == wrong_command,
          "invalid command numbers are refused without overflow or wraparound");
  }

  /* the wire is served first, so a network client cannot starve it */
  wordclock_platform::HardwarePort().Incoming = "AB";
  port.Inject("cd", 2u);
  std::string order;
  while (port.available()) {
    order.push_back(static_cast<char>(port.read()));
  }
  Check(order == "ABcd", "the UART is drained before the injected characters");

  /* one line per println, and nothing before it */
  lines.clear();
  port.print("Error=");
  port.print(4);
  Check(lines.empty(), "print alone does not emit a line");
  port.println();
  Check(lines.size() == 1u && lines[0] == "Error=4", "println emits exactly the collected line");

  /* a newline printed as a character ends the line too - the firmware does that */
  lines.clear();
  port.print("Api");
  port.print('\n');
  Check(lines.size() == 1u && lines[0] == "Api", "a printed newline ends the line as println does");

  /* and it reaches the UART unchanged, which is what an attached console sees */
  Check(wordclock_platform::HardwarePort().Written.find("Error=4") != std::string::npos,
        "the same output still goes to the wire");

  /* the real thing: an injected command must be parsed like a typed one */
  lines.clear();
  wordclock_platform::HardwarePort().Written.clear();
  char const* command = "3 -B255\n"; /* terminated with kEndOfMessageChar, as on the wire */
  Check(port.Inject(command, strlen(command)) == kEOk, "a command fits the inject buffer");
  Communication& communication = Communication::GetInstance();
  communication.Task(); /* reads the characters */
  communication.Task(); /* parses the complete message */
  Check(!lines.empty(), "the injected command produced an answer");
  if (!lines.empty()) {
    printf("   answer: \"%s\"\n", lines[0].c_str());
  }

  /*
   * The procedures that act now rather than at the next word change. What of them
   * reaches the wire is the answer, and it says which of the two happened: the display
   * was free and the procedure ran, or it was busy and the procedure stepped aside.
   */
  /*
   * Ticked until the answer appears rather than exactly twice: the task reads on one
   * call and parses on the next, and a message left over from an earlier check would
   * otherwise shift every answer by one.
   */
  auto answer_to = [&](char const* message) {
    lines.clear();
    port.Inject(message, strlen(message));
    for (int tick = 0; (tick < 8) && lines.empty(); tick++) {
      communication.Task();
    }
    return lines.empty() ? std::string() : lines[0];
  };

  Check(answer_to("1 -P22\n") == "1 RpcId=22 Error=0", "the clock refresh is carried out");
  Check(answer_to("1 -P28\n") == "1 RpcId=28 Error=8",
        "aborting with no overlay showing is refused");

  // Derived from the enumeration rather than written down: this used to name id 34, and
  // the day 34 became a procedure the check went on passing for a procedure that ran.
  {
    char message[16];
    char expected[32];
    unsigned const past_the_last = MsgCmdRemoteProcedureCallParser::kNumberOfProcedures;

    snprintf(message, sizeof(message), "1 -P%u\n", past_the_last);
    snprintf(expected, sizeof(expected), "1 RpcId=%u Error=7", past_the_last);
    Check(answer_to(message) == expected, "an id past the last procedure is unknown");
  }

  /*
   * The two procedures that need a network, on a host that has none. Refusing is the
   * whole point: answering kEOk would tell a caller that the clock is on its way back
   * to a network it never had.
   */
  Check(answer_to("1 -P32\n") == "1 RpcId=32 Error=8",
        "resynchronising without a network is refused");
  Check(answer_to("1 -P33\n") == "1 RpcId=33 Error=8",
        "reconnecting without a configured network is refused");

  /*
   * The status command answers with values and takes none. Every field that has nothing
   * behind it comes back empty rather than as a zero that reads like a value - here the
   * temperature, with no chip on the bus, and the two network fields.
   */
  Check(answer_to("12\n") == "12 V=" WORDCLOCK_VERSION " U=0 H=0 N=0 I=1 T= A= Q= M=0",
        "the status answers every field, and the ones with no answer empty");
  Check(
      answer_to("12 -V1\n") == "12 Error=3:V V=" WORDCLOCK_VERSION " U=0 H=0 N=0 I=1 T= A= Q= M=0",
      "a value sent to a read-only field is refused as an unknown option");

  Check(answer_to("00012\n").find("12 ") == 0u, "commands with leading zeroes remain decimal");
  Check(answer_to("16\n").find("16 ") == 0u, "the highest command number is accepted");

  /*
   * The uptime is the one of them a host can produce. Driven through the counter's own
   * task rather than by moving millis(), which is the point of the change: nothing
   * divides a millisecond count down any more, so nothing wraps when that count does.
   */
  for (unsigned int second = 0u; second < 3u * 60u; second++) {
    Uptime::GetInstance().Task();
  }
  Check(answer_to("12\n") == "12 V=" WORDCLOCK_VERSION " U=0 H=0 N=3 I=1 T= A= Q= M=0",
        "the uptime is reported as days, hours and minutes");

  /*
   * The network command, which is the one that used to be a reflash. What can be checked
   * on a host is the whole path except the radio: a pair goes in, the store keeps it, and
   * the answer names the network without ever carrying the pass phrase back.
   */
  Check(answer_to("13\n") == "13 S= Error=0", "a clock with no network names none");
  Check(answer_to("13 -SMyNetwork -Psecret\n") == "13 S=MyNetwork Error=0", "a pair is taken");
  Check(answer_to("13\n") == "13 S=MyNetwork Error=0",
        "and is what the clock answers with afterwards");
  Check(answer_to("13\n").find("secret") == std::string::npos, "the pass phrase never comes back");
  Check(answer_to("13 -SOther\n") == "13 S=MyNetwork Error=8", "half a pair is refused");
  Check(answer_to("13\n") == "13 S=MyNetwork Error=0", "and leaves the stored one alone");

  /*
   * An option that ends the line is read with an empty argument, as any other. The parser
   * used to stop at the end of the line before it got to the argument, and these did nothing.
   */
  Check(answer_to("13 -S -P\n") == "13 S= Error=0", "13 -S -P forgets the network");
  Check(answer_to("13 -SOpenNet -P\n") == "13 S=OpenNet Error=0",
        "an empty pass phrase joins an open network");
  Check(answer_to("9 -A\n").find("Error=5:A") != std::string::npos,
        "an option without its value at the end of the line is answered");

  // The character behind an unknown option is read again: it used to be skipped with it.
  {
    std::string const answer = answer_to("9 -X-A3\n");
    Check(answer.find("Error=3:X") != std::string::npos && answer.find("A=3") != std::string::npos,
          "an unknown option does not swallow the option behind it");
    answer_to("9 -A0\n");
  }

  /*
   * Command 16 clears the console password with an empty -P, the way back for a clock nobody
   * can log into, and only with it. The bare number asks: it cleared as well, and the panel
   * asks for every command it knows by its bare number, so opening the panel took the password
   * off the clock.
   */
  Check(answer_to("16 -Psecret\n") == "16 A=1", "a console password is set");
  Check(answer_to("16\n") == "16 A=1", "command 16 alone asks and leaves the password alone");
  Check(answer_to("16 -P\n") == "16 A=0", "an empty -P clears it, which is the way back in");
  Check(answer_to("16\n") == "16 A=0", "and the query says so afterwards");

  // A refused -I sets no pixel: the index stayed at 0, and -S lit pixel 0.
  answer_to("4 -I0 -S0\n");
  Check(answer_to("4 -I300 -S1\n") == "4 Error=4:I I=0 S=0", "a refused index sets no pixel");

  // A font that is not one is answered rather than ignored.
  Check(answer_to("5 -F9\n").find("Error=4:F") != std::string::npos,
        "an overlay font out of range is answered");

  /*
   * A line longer than the buffer is refused whole, with one error. It was answered with an
   * error per character dropped and then run cut short, here as the text of the text overlay.
   */
  {
    std::string const too_long = "7 -T" + std::string(MESSAGE_LENGTH, 'a') + "\n";
    lines.clear();
    port.Inject(too_long.data(), too_long.size());
    for (int tick = 0; tick < 8; tick++) {
      communication.Task();
    }
    Check(lines.size() == 1u && lines[0] == "Error=1",
          "a line too long is answered with one error");
    Check(answer_to("7\n").find("T= ") != std::string::npos, "and is not carried out");
  }

  /* a full buffer refuses rather than overwriting a command in flight */
  std::string too_much(WORDCLOCK_SERIAL_INJECT_BUFFER_SIZE + 8u, 'x');
  Check(port.Inject(too_much.data(), too_much.size()) == kENotOk,
        "an overlong injection is refused");
  return Report();
}
