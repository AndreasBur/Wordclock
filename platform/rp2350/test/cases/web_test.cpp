/* Drives the registered handlers the way the server reaches them - through the registration
   call - so WebInterface's own wiring is covered rather than bypassed.

   One thing this can no longer ask, and the ESP32's version can: which client a frame went
   to. On the target that accounting has moved out of our code and into the socket, so what
   is left to check here is what the backend still decides - whether a frame is sent at all
   and when - rather than who received it. */
#include <ESPAsyncWebServer.h>
#include <LEAmDNS.h>

#include "Arduino.h"
#include "Communication.h"
#include "Pixels.h"
#include "WebInterface.h"
#include "check.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

/* Enough of a parse to say the document is well formed: brackets balanced and no string
   left open. A browser would refuse the whole thing otherwise, and that failure looks
   like a broken server rather than a broken label. */
static bool isBalancedJson(const std::string& Text)
{
    int Depth = 0;
    bool InString = false;

    for(size_t Index = 0u; Index < Text.size(); Index++) {
        const char Character = Text[Index];

        if(InString) {
            if(Character == '\\')      { Index++; }
            else if(Character == '"')  { InString = false; }
            continue;
        }
        if(Character == '"')                                 { InString = true; }
        else if((Character == '[') || (Character == '{'))     { Depth++; }
        else if((Character == ']') || (Character == '}'))     { Depth--; if(Depth < 0) { return false; } }
    }
    return (Depth == 0) && !InString;
}

/* Runs one route and hands back what it produced. The request owns its response, so it
   outlives neither - hence the copy out. */
static std::string fetch(const char* Route, std::string* ContentEncoding = nullptr)
{
    AsyncWebServerRequest Request;
    const auto Found = webStubState().Routes.find(Route);

    if(Found == webStubState().Routes.end()) { return ""; }

    Found->second(&Request);

    if(Request.Response == nullptr) { return ""; }
    if(ContentEncoding != nullptr) {
        const auto Header = Request.Response->Headers.find("Content-Encoding");
        *ContentEncoding = (Header == Request.Response->Headers.end()) ? "" : Header->second;
    }
    return Request.Response->Body;
}

/* One text frame into the socket, the way the library reports one. */
static void sendFrame(const std::string& Payload, bool Final = true)
{
    AwsFrameInfo Info{};
    Info.final = Final;
    Info.index = 0u;
    Info.len = Payload.size();
    Info.opcode = WS_TEXT;

    std::vector<uint8_t> Bytes(Payload.begin(), Payload.end());
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_DATA, &Info,
                                 Bytes.empty() ? nullptr : Bytes.data(), Bytes.size());
}


int main()
{
    check(WebInterface::getInstance().begin() == E_OK, "the server starts and registers its handlers");
    check(webStubState().Listening, "and it is listening");
    check((webStubState().SocketHandler != nullptr) && (webStubState().Routes.count("/") == 1u)
          && (webStubState().Routes.count("/commands") == 1u)
          && (webStubState().Routes.count("/display") == 1u),
          "all four handlers were registered");

    /* the page: gzipped, and announced as such */
    std::string Encoding;
    const std::string Page = fetch("/", &Encoding);
    check(Encoding == "gzip", "the page is announced as gzip");
    check((Page.size() > 100u) && (static_cast<unsigned char>(Page[0]) == 0x1fu)
                               && (static_cast<unsigned char>(Page[1]) == 0x8bu),
          "the page really is a gzip stream");

    /* the catalog, generated from the real table */
    const std::string Catalog = fetch("/commands");
    check(!Catalog.empty() && (Catalog.front() == '[') && (Catalog.back() == ']'),
          "the catalog is a JSON array");
    check(isBalancedJson(Catalog), "its brackets and strings are balanced");
    check(Catalog.find("\"number\":3") != std::string::npos, "it carries the display brightness command");
    check(Catalog.find("\"short\":\"B\"") != std::string::npos, "it carries an option short name");
    check(Catalog.find("\"max\":255") != std::string::npos, "it carries the ranges");
    check(Catalog.find("\"values\":[") != std::string::npos, "it carries named values for the enumerations");
    /* Without this the page would offer the status fields as inputs, and a value typed
       into one of them would come back as "parameter unknown". */
    check(Catalog.find("\"readonly\":true") != std::string::npos, "it marks the fields that only appear in answers");
    size_t Commands = 0u;
    for(size_t At = Catalog.find("\"number\":"); At != std::string::npos; At = Catalog.find("\"number\":", At + 1u)) { Commands++; }
    printf("   catalog: %zu bytes, %zu commands\n", Catalog.size(), Commands);

    /* the panel description: shape and letters, the letters as UTF-8 */
    const std::string Panel = fetch("/display");
    check(isBalancedJson(Panel), "the display description is well formed");
    check(Panel.find("\"columns\":11") != std::string::npos, "it carries the column count");
    check(Panel.find("\"rows\":10") != std::string::npos, "it carries the row count");
    check(Panel.find("ESKISTLF") != std::string::npos, "it carries the letters");
    /* U with an umlaut is 0xDC in the firmware's table and has to arrive as two UTF-8 bytes;
       the raw byte would make the whole document invalid. */
    check(Panel.find("\xc3\x9c") != std::string::npos, "the umlauts arrive as UTF-8");
    check(Panel.find("\xdc") == std::string::npos, "no raw Latin-1 byte escapes into the JSON");

    /* the handshake, which is an event here rather than a GET to answer */
    webStubState().SocketHandler(nullptr, nullptr, WS_EVT_CONNECT, nullptr, nullptr, 0u);
    webStubState().ClientCount = 1u;

    /* a frame without a terminator - the bridge has to add it */
    webStubState().Text.clear();
    sendFrame("3 -B200");

    Communication& communication = Communication::getInstance();
    communication.task();
    communication.task();

    bool Found = false;
    for(const auto& Line : webStubState().Text) { if(Line.find("B=200") != std::string::npos) { Found = true; } }
    check(Found, "the answer came back over the socket");
    for(const auto& Line : webStubState().Text) { printf("   sent: \"%s\"\n", Line.c_str()); }

    /* An answer carrying a Latin-1 byte, which is what the overlay text command answers with
       once somebody sets a text with an umlaut in it - the thing the font tables carry
       umlauts for. A web socket text frame has to be valid UTF-8, so a raw 0xFC here does
       not draw one character wrong, it makes the browser close the connection.

       The strings are split around the escapes on purpose: "\xFCck" would swallow the c as a
       further hex digit and stop compiling. */
    webStubState().Text.clear();
    WebInterface::getInstance().broadcastLine("T=Gl\xFC" "ck");
    check(!webStubState().Text.empty(), "an answer with an umlaut is sent at all");
    check(!webStubState().Text.empty() && webStubState().Text.back() == "T=Gl\xC3\xBC" "ck",
          "a Latin-1 byte in an answer leaves as UTF-8");
    check(!webStubState().Text.empty() && webStubState().Text.back().find('\xFC') == std::string::npos,
          "and no raw Latin-1 byte survives into the frame");

    /* the display frame */
    WebInterface& web = WebInterface::getInstance();
    const unsigned interval = WEB_INTERFACE_FRAME_INTERVAL_TICKS;
    auto runInterval = [&web, interval](unsigned count) {
        for(unsigned tick = 0u; tick < (count * interval); tick++) { web.broadcastFrame(); }
    };

    /* The handshake above counted as a client arriving, so one frame is already owed. Drain
       it before measuring anything, or every count below is one too high. */
    runInterval(1u);
    webStubState().Binary.clear();

    runInterval(3u);
    check(webStubState().Binary.empty(), "a display that never changed sends no frame");

    Pixels::getInstance().setPixel(0u, 10u, 20u, 30u);
    runInterval(3u);
    check(webStubState().Binary.size() == 1u, "a change is sent once and not repeated");

    if(!webStubState().Binary.empty()) {
        const std::string& Frame = webStubState().Binary[0];
        const unsigned char* Bytes = reinterpret_cast<const unsigned char*>(Frame.data());

        check(Frame.size() == 110u * 3u, "the frame is 110 * 3 bytes");
        check((Bytes[0] == 20u) && (Bytes[1] == 10u) && (Bytes[2] == 30u),
              "the frame carries green, red, blue like the strip");
        printf("   frame: %zu bytes, one per %u ticks\n", Frame.size(), interval);
    }

    webStubState().Binary.clear();
    Pixels::getInstance().disablePixels();
    runInterval(1u);
    const bool BrowserDark = !webStubState().Binary.empty() &&
        std::all_of(webStubState().Binary[0].begin(), webStubState().Binary[0].end(),
                    [](char Value) { return Value == 0; });
    check(BrowserDark, "disabling the display also blanks the browser frame");

    webStubState().Binary.clear();
    Pixels::getInstance().enablePixels();
    runInterval(1u);
    check(!webStubState().Binary.empty() &&
          static_cast<unsigned char>(webStubState().Binary[0][0]) == 20u &&
          static_cast<unsigned char>(webStubState().Binary[0][1]) == 10u &&
          static_cast<unsigned char>(webStubState().Binary[0][2]) == 30u,
          "enabling the display restores the browser frame");

    /* A client arriving between two changes still has to be shown the panel. Who it goes to
       is the socket's business here, so what is checked is that a frame goes out at all on
       an unchanged display - which is the decision that is still ours. */
    webStubState().Binary.clear();
    web.onClientOpened();
    runInterval(1u);
    check(webStubState().Binary.size() == 1u, "a client that arrives is shown the standing display");

    /* Nothing changed and nobody new arrived, so the next interval is silent again. */
    webStubState().Binary.clear();
    runInterval(2u);
    check(webStubState().Binary.empty(), "and the intervals after it stay quiet");

    /* Changed on every tick, and still one frame per interval rather than one per tick. */
    webStubState().Binary.clear();
    for(unsigned tick = 0u; tick < (4u * interval); tick++) {
        Pixels::getInstance().setPixel(1u, static_cast<byte>(tick + 1u), 0u, 0u);
        web.broadcastFrame();
    }
    check(webStubState().Binary.size() == 4u, "a display changing every tick still sends one per interval");

    /* Nothing to send to, so nothing is gathered - the check the client count exists for. */
    webStubState().ClientCount = 0u;
    webStubState().Binary.clear();
    Pixels::getInstance().setPixel(2u, 5u, 5u, 5u);
    runInterval(2u);
    check(webStubState().Binary.empty(), "with nobody watching, no frame is built at all");
    webStubState().ClientCount = 1u;

    /* An oversized frame must be refused whole rather than truncated into the parser, and a
       split one dropped rather than reassembled. Both would put half a command in front of
       it. */
    webStubState().Text.clear();
    sendFrame(std::string(WEB_INTERFACE_MAX_FRAME_LENGTH + 10u, 'x'));
    communication.task();
    communication.task();
    check(webStubState().Text.empty(), "an oversized frame is refused");

    sendFrame("3 -B7", false);
    communication.task();
    communication.task();
    check(webStubState().Text.empty(), "and so is a split one");

    return report();
}
