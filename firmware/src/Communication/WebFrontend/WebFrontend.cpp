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
/**     \file       WebFrontend.cpp
 *      \brief      The half of the web front end that is not a server
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "WebFrontend.h"

#if (WEB_FRONTEND_SUPPORT == STD_ON)

#include "Communication.h"
#include "DisplayCharacters.h"
#include "MessageCatalog.h"
#include "Pixels.h"
/* The build generates this from web/. Included here and nowhere else: its arrays have
   internal linkage, so every further includer would be a second copy of both pages in
   flash. */
#include "WebPage.h"
#include "WordclockSerial.h"

#include <stdio.h>
#include <string.h>

/******************************************************************************************************************************************************
 *  LOCAL FUNCTIONS
******************************************************************************************************************************************************/
namespace {

/* One Latin-1 byte as UTF-8, into Target, answering how many bytes that took. The whole of
   Latin-1 widens by this one rule, so no table is needed: below 0x80 a byte is itself, and
   above it becomes two.

   Both ways out of here need it, for different reasons. The letters served by /display are
   Latin-1 because the table holds one byte per letter, and JSON is UTF-8 - a raw 0xDC there
   makes the whole document invalid rather than one letter wrong.

   An answer line needs it for a harder reason: it leaves as a web socket *text* frame, and
   RFC 6455 requires those to be valid UTF-8. A raw 0xF6 in one does not draw a wrong
   character, it makes the browser close the connection. That is reachable rather than
   theoretical - command 8 answers with the overlay text it was given, and an umlaut in that
   text is exactly what the font tables carry umlauts for. */
byte ToUtf8(byte latin1, char* target)
{
    if(latin1 < 0x80u) {
        target[0u] = static_cast<char>(latin1);
        return 1u;
    }
    target[0u] = static_cast<char>(0xC0u | (latin1 >> 6u));
    target[1u] = static_cast<char>(0x80u | (latin1 & 0x3Fu));
    return 2u;
}


/******************************************************************************************************************************************************
  C H U N K   W R I T E R
******************************************************************************************************************************************************/
/* Collects a document in a small buffer and hands it to the body in chunks, so neither the
   whole thing nor any allocation is needed to serve it.

   The buffer is kept even for a backend whose body is pulled from rather than pushed to,
   which is the one behaviour this file changed on its way out of the two backends: the
   RP2350's writer used to hand its response stream one character at a time, on the argument
   that a library building everything out of String would not notice a buffer of ours. True,
   but it cuts both ways - the stream does not notice 256 bytes arriving at once either, and
   one writer that behaves the same on both is worth more than a saved memcpy. */
class ChunkWriter
{
  private:
    static constexpr size_t kCapacity{256u};

    WebResponseBody& body_;
    char buffer_[kCapacity]{};
    size_t used_{0u};

    void Flush() {
        if(used_ == 0u) { return; }

        body_.Write(buffer_, used_);
        used_ = 0u;
    }

  public:
    explicit ChunkWriter(WebResponseBody& body) : body_(body) { }

    void Put(char character) {
        if(used_ == kCapacity) { Flush(); }

        buffer_[used_++] = character;
    }

    void Put(const char* text) {
        if(text == nullptr) { return; }

        for(const char* character = text; *character != '\0'; character++) { Put(*character); }
    }

    void PutNumber(uint16_t number) {
        char digits[8]{};
        /* Taken rather than dropped, and then used: it is the length, so the digits go out
           without a second walk to find their terminator. Negative is snprintf's encoding
           failure, which cannot happen for "%u" and is refused rather than trusted. */
        const int length = snprintf(digits, sizeof(digits), "%u", static_cast<unsigned>(number));

        if(length <= 0) { return; }

        for(int index = 0; index < length; index++) { Put(digits[index]); }
    }

    /* Escaped, because a label is data: a quote in one would otherwise produce a document
       the browser refuses whole, and that failure looks like a server fault. */
    void PutString(const char* text) {
        Put('"');
        for(const char* character = text; (text != nullptr) && (*character != '\0'); character++) {
            if((*character == '"') || (*character == '\\')) { Put('\\'); }
            Put(*character);
        }
        Put('"');
    }

    /* Whatever ends a body on this backend - an empty chunk for one, handing the stream over
       for another. */
    void Finish() {
        Flush();
        body_.Finish();
    }
};

} // namespace

/******************************************************************************************************************************************************
 *  L O C A L   D A T A
******************************************************************************************************************************************************/
namespace {

/* Every file a clock hands out unchanged, at the path it hands it out at.
 *
 * "/" is the panel and "/console" the console, which is the whole arrangement rather than a
 * detail: nobody types a path, they type the clock's address and take what comes, so what
 * comes is the page for the things somebody changes often, with a link to the other one in
 * its header.
 *
 * The three that follow are neither page but the home screen icon - a manifest and two PNGs -
 * which is what lets the console be installed with an icon to tap and a start without an
 * address bar.
 */
constexpr WebFrontend::AssetType kAssets[]{
    {"/",                      kWebAppGzip,   kWebAppGzipSize,   "text/html",                 true},
    {"/console",               kWebPageGzip,  kWebPageGzipSize,  "text/html",                 true},
    {"/manifest.webmanifest",  kWebManifest,  kWebManifestSize,  "application/manifest+json", false},
    {"/icon-192.png",          kWebIcon192,   kWebIcon192Size,   "image/png",                 false},
    {"/icon-512.png",          kWebIcon512,   kWebIcon512Size,   "image/png",                 false},
};

/* A backend sizes its route array with kMaxAssets, which it can see and this count it cannot.
   Adding a file here without raising that fails the build, rather than quietly registering
   the first five of six. */
static_assert((sizeof(kAssets) / sizeof(kAssets[0])) <= WebFrontend::kMaxAssets,
              "WebFrontend::MaxAssets has to grow with the asset table");

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  GetNumberOfAssets() / GetAsset()
******************************************************************************************************************************************************/
/*! \brief          The files a clock serves unchanged, for a backend to register routes from
 *
 *  \details        A table rather than a handler per file, because that is what the ten
 *                  handlers this replaces actually were: each one set a content type, set a
 *                  Content-Encoding header or did not, and pushed a blob. The difference
 *                  between them was data, and the sameness between the two backends was
 *                  complete.
******************************************************************************************************************************************************/
byte WebFrontend::GetNumberOfAssets()
{
    return static_cast<byte>(sizeof(kAssets) / sizeof(kAssets[0]));
}

const WebFrontend::AssetType& WebFrontend::GetAsset(byte index)
{
    return kAssets[index];
}


/******************************************************************************************************************************************************
  WriteCommands()
******************************************************************************************************************************************************/
/*! \brief          Writes the command catalog, so the page can build its own form
 *
 *  \details        Generated from MessageCatalog rather than written out a second time.
 *                  That table is what the simulator's message builder derives its whole
 *                  dialog from, down to the input hints; serving it makes the browser a
 *                  second renderer of the same description, so a command added to the
 *                  catalog shows up in both front ends and on the wire at once.
 *
 *                  Written here rather than once per backend, which is what it was: the two
 *                  copies were identical down to their comments, and both feed the same
 *                  page's parser.
******************************************************************************************************************************************************/
void WebFrontend::WriteCommands(WebResponseBody& body)
{
    ChunkWriter writer(body);
    writer.Put('[');

    for(byte index = 0u; index < MessageCatalog::GetNumberOfCommands(); index++) {
        const MessageCatalog::CommandType& command = MessageCatalog::GetCommand(index);

        if(index > 0u) { writer.Put(','); }
        writer.Put("{\"number\":");
        writer.PutNumber(command.number);
        writer.Put(",\"label\":");
        writer.PutString(command.label);
        writer.Put(",\"options\":[");

        for(byte option_index = 0u; option_index < command.number_of_options; option_index++) {
            const MessageCatalog::OptionType& option = command.options[option_index];

            if(option_index > 0u) { writer.Put(','); }
            writer.Put("{\"short\":\"");
            writer.Put(option.short_name);
            writer.Put("\",\"label\":");
            writer.PutString(option.label);
            writer.Put(",\"type\":");
            writer.PutNumber(static_cast<uint16_t>(option.argument));
            writer.Put(",\"min\":");
            writer.PutNumber(option.minimum);
            writer.Put(",\"max\":");
            writer.PutNumber(option.maximum);

            /* Only where it is set, so the page's form stays as it was for every option
               that can be sent, and the read-only fields it must not offer are the ones
               that say so. */
            if(option.read_only) { writer.Put(",\"readonly\":true"); }

            if((option.value_names != nullptr) && (option.number_of_value_names > 0u)) {
                writer.Put(",\"values\":[");
                for(byte name_index = 0u; name_index < option.number_of_value_names; name_index++) {
                    if(name_index > 0u) { writer.Put(','); }
                    writer.PutString(option.value_names[name_index]);
                }
                writer.Put(']');
            }
            writer.Put('}');
        }
        writer.Put("]}");
    }

    writer.Put(']');
    writer.Finish();
} /* WriteCommands */


/******************************************************************************************************************************************************
  WriteDisplay()
******************************************************************************************************************************************************/
/*! \brief          Writes the panel's shape and its letters
 *
 *  \details        The page draws a grid of letters and has to know which, and there is
 *                  exactly one table of them - DisplayCharacters. Serving it keeps the
 *                  browser from carrying a second copy, which is the duplication the
 *                  simulator used to have and no longer does.
 *
 *                  The table stores one byte per letter, so the umlauts are Latin-1 and are
 *                  widened to UTF-8 on the way out: JSON is UTF-8, and a raw 0xDC makes the
 *                  whole document invalid rather than one letter wrong.
******************************************************************************************************************************************************/
void WebFrontend::WriteDisplay(WebResponseBody& body)
{
    const DisplayCharacters letters;
    ChunkWriter writer(body);

    writer.Put("{\"columns\":");
    writer.PutNumber(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS);
    writer.Put(",\"rows\":");
    writer.PutNumber(DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    writer.Put(",\"letters\":\"");

    for(byte index = 0u; index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; index++) {
        char utf8[2u];
        const byte length = ToUtf8(static_cast<byte>(letters.GetCharacter(index)), utf8);

        for(byte position = 0u; position < length; position++) { writer.Put(utf8[position]); }
    }

    writer.Put("\"}");
    writer.Finish();
} /* WriteDisplay */


/******************************************************************************************************************************************************
  InjectCommand()
******************************************************************************************************************************************************/
/*! \brief          Takes one socket frame's text and injects it as typed characters
 *
 *  \details        The command is injected followed by the end-of-message character, unless
 *                  the browser already sent one. That keeps the page free of protocol
 *                  bookkeeping: it sends the command text, the terminator is this side's
 *                  business.
 *
 *                  A frame too long to be a command is refused whole rather than truncated,
 *                  so half a command can never reach the parser. A backend whose server
 *                  hands over a bounded buffer has already made that impossible; one whose
 *                  library hands over whatever arrived has not, and this is where both end
 *                  up equal.
******************************************************************************************************************************************************/
void WebFrontend::InjectCommand(const char* text, size_t length)
{
    if((text == nullptr) || (length == 0u) || (length > WEB_FRONTEND_MAX_FRAME_LENGTH)) { return; }

    WordclockSerial& port = WordclockSerial::GetInstance();
    port.Inject(text, length);

    if(text[length - 1u] != Communication::GetEndOfMessageChar()) {
        const char terminator = Communication::GetEndOfMessageChar();
        port.Inject(&terminator, 1u);
    }
} /* InjectCommand */


/******************************************************************************************************************************************************
  BroadcastLine()
******************************************************************************************************************************************************/
/*! \brief          Sends one finished line to whoever is connected
 *
 *  \details        Runs in the firmware's task. A send that fails is not retried and the
 *                  client is not dropped here - a backend's own close hook owns that, and
 *                  dropping a client from this side would race with it.
******************************************************************************************************************************************************/
void WebFrontend::BroadcastLine(const char* line)
{
    WebTransport& transport = WebTransport::GetInstance();

    if(!transport.IsListening() || (line == nullptr)) { return; }

    /* Widened rather than sent as it stands - see ToUtf8(). Two bytes per Latin-1 byte is
       the worst case, so a line that is nothing but umlauts still fits and there is no
       truncation to expect. Sized from the producer's own line length rather than from a
       number of its own, because that is the buffer the line was assembled in.

       The bound is still checked. Nothing longer can arrive today, but this takes a plain
       pointer and the sink it is installed as is a function pointer - the day a second
       producer appears, an assumption written only in a comment is the one that gives. */
    char payload[2u * WORDCLOCK_SERIAL_LINE_LENGTH];
    size_t used{0u};

    for(const char* character = line; (*character != '\0') && ((used + 2u) <= sizeof(payload)); character++) {
        used += ToUtf8(static_cast<byte>(*character), &payload[used]);
    }

    if(used == 0u) { return; }

    transport.SendText(payload, used);
} /* BroadcastLine */


/******************************************************************************************************************************************************
  BroadcastFrame()
******************************************************************************************************************************************************/
/*! \brief          Sends the pixel buffer to whoever is connected, when it changed
 *
 *  \details        Rate limited first and compared second, so a display that changes on
 *                  every tick still costs one frame per interval, and one that stands still
 *                  costs a comparison.
 *
 *                  Sent in the strip's own byte order, green first: it is what the buffer
 *                  already holds, and the page is told the shape by the display description
 *                  rather than guessing it. Taken from the output pixel, so what goes out is
 *                  dimmed the way the strip is - which is what lets the page blank itself
 *                  when the display is switched off, without a redraw to tell it so. The
 *                  page reads the bytes for that on/off state only and paints its own
 *                  colour; the wx window is the colour-accurate view.
******************************************************************************************************************************************************/
void WebFrontend::BroadcastFrame()
{
    WebTransport& transport = WebTransport::GetInstance();

    if(!transport.IsListening()) { return; }

    const bool forced = force_frame_.exchange(false, std::memory_order_acq_rel);

    if(!forced) {
        if(frame_countdown_ > 0u) { frame_countdown_--; return; }
    }
    frame_countdown_ = WEB_FRONTEND_FRAME_INTERVAL_TICKS - 1u;

    /* Nothing to send to, so nothing is even gathered. */
    if(!transport.HasClients()) { return; }

    byte frame[kFrameSize];
    byte* target = frame;
    const Pixels& strip = Pixels::GetInstance();

    for(byte index = 0u; index < PIXELS_NUMBER_OF_LEDS; index++) {
        const Pixel colour = strip.GetOutputPixel(index);

        *target++ = colour.GetGreen();
        *target++ = colour.GetRed();
        *target++ = colour.GetBlue();
    }

    if(!forced && (memcmp(frame, last_frame_, kFrameSize) == 0)) { return; }
    memcpy(last_frame_, frame, kFrameSize);

    transport.SendBinary(frame, kFrameSize);
} /* BroadcastFrame */

#endif // (WEB_FRONTEND_SUPPORT == STD_ON)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
