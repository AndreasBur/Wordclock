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
byte toUtf8(byte Latin1, char* Target)
{
    if(Latin1 < 0x80u) {
        Target[0u] = static_cast<char>(Latin1);
        return 1u;
    }
    Target[0u] = static_cast<char>(0xC0u | (Latin1 >> 6u));
    Target[1u] = static_cast<char>(0x80u | (Latin1 & 0x3Fu));
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
    static constexpr size_t Capacity{256u};

    WebResponseBody& Body;
    char Buffer[Capacity]{};
    size_t Used{0u};

    void flush() {
        if(Used == 0u) { return; }

        Body.write(Buffer, Used);
        Used = 0u;
    }

  public:
    explicit ChunkWriter(WebResponseBody& sBody) : Body(sBody) { }

    void put(char Character) {
        if(Used == Capacity) { flush(); }

        Buffer[Used++] = Character;
    }

    void put(const char* Text) {
        if(Text == nullptr) { return; }

        for(const char* Character = Text; *Character != '\0'; Character++) { put(*Character); }
    }

    void putNumber(uint16_t Number) {
        char Digits[8]{};

        snprintf(Digits, sizeof(Digits), "%u", static_cast<unsigned>(Number));
        put(Digits);
    }

    /* Escaped, because a label is data: a quote in one would otherwise produce a document
       the browser refuses whole, and that failure looks like a server fault. */
    void putString(const char* Text) {
        put('"');
        for(const char* Character = Text; (Text != nullptr) && (*Character != '\0'); Character++) {
            if((*Character == '"') || (*Character == '\\')) { put('\\'); }
            put(*Character);
        }
        put('"');
    }

    /* Whatever ends a body on this backend - an empty chunk for one, handing the stream over
       for another. */
    void finish() {
        flush();
        Body.finish();
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
constexpr WebFrontend::AssetType Assets[]{
    {"/",                      WebAppGzip,   WebAppGzipSize,   "text/html",                 true},
    {"/console",               WebPageGzip,  WebPageGzipSize,  "text/html",                 true},
    {"/manifest.webmanifest",  WebManifest,  WebManifestSize,  "application/manifest+json", false},
    {"/icon-192.png",          WebIcon192,   WebIcon192Size,   "image/png",                 false},
    {"/icon-512.png",          WebIcon512,   WebIcon512Size,   "image/png",                 false},
};

/* A backend sizes its route array with MaxAssets, which it can see and this count it cannot.
   Adding a file here without raising that fails the build, rather than quietly registering
   the first five of six. */
static_assert((sizeof(Assets) / sizeof(Assets[0])) <= WebFrontend::MaxAssets,
              "WebFrontend::MaxAssets has to grow with the asset table");

} // namespace

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getNumberOfAssets() / getAsset()
******************************************************************************************************************************************************/
/*! \brief          The files a clock serves unchanged, for a backend to register routes from
 *
 *  \details        A table rather than a handler per file, because that is what the ten
 *                  handlers this replaces actually were: each one set a content type, set a
 *                  Content-Encoding header or did not, and pushed a blob. The difference
 *                  between them was data, and the sameness between the two backends was
 *                  complete.
******************************************************************************************************************************************************/
byte WebFrontend::getNumberOfAssets()
{
    return static_cast<byte>(sizeof(Assets) / sizeof(Assets[0]));
}

const WebFrontend::AssetType& WebFrontend::getAsset(byte Index)
{
    return Assets[Index];
}


/******************************************************************************************************************************************************
  writeCommands()
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
void WebFrontend::writeCommands(WebResponseBody& Body)
{
    ChunkWriter Writer(Body);
    Writer.put('[');

    for(byte Index = 0u; Index < MessageCatalog::getNumberOfCommands(); Index++) {
        const MessageCatalog::CommandType& Command = MessageCatalog::getCommand(Index);

        if(Index > 0u) { Writer.put(','); }
        Writer.put("{\"number\":");
        Writer.putNumber(Command.Number);
        Writer.put(",\"label\":");
        Writer.putString(Command.Label);
        Writer.put(",\"options\":[");

        for(byte OptionIndex = 0u; OptionIndex < Command.NumberOfOptions; OptionIndex++) {
            const MessageCatalog::OptionType& Option = Command.Options[OptionIndex];

            if(OptionIndex > 0u) { Writer.put(','); }
            Writer.put("{\"short\":\"");
            Writer.put(Option.ShortName);
            Writer.put("\",\"label\":");
            Writer.putString(Option.Label);
            Writer.put(",\"type\":");
            Writer.putNumber(static_cast<uint16_t>(Option.Argument));
            Writer.put(",\"min\":");
            Writer.putNumber(Option.Minimum);
            Writer.put(",\"max\":");
            Writer.putNumber(Option.Maximum);

            /* Only where it is set, so the page's form stays as it was for every option
               that can be sent, and the read-only fields it must not offer are the ones
               that say so. */
            if(Option.ReadOnly) { Writer.put(",\"readonly\":true"); }

            if((Option.ValueNames != nullptr) && (Option.NumberOfValueNames > 0u)) {
                Writer.put(",\"values\":[");
                for(byte NameIndex = 0u; NameIndex < Option.NumberOfValueNames; NameIndex++) {
                    if(NameIndex > 0u) { Writer.put(','); }
                    Writer.putString(Option.ValueNames[NameIndex]);
                }
                Writer.put(']');
            }
            Writer.put('}');
        }
        Writer.put("]}");
    }

    Writer.put(']');
    Writer.finish();
} /* writeCommands */


/******************************************************************************************************************************************************
  writeDisplay()
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
void WebFrontend::writeDisplay(WebResponseBody& Body)
{
    const DisplayCharacters Letters;
    ChunkWriter Writer(Body);

    Writer.put("{\"columns\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_COLUMNS);
    Writer.put(",\"rows\":");
    Writer.putNumber(DISPLAY_CHARACTERS_NUMBER_OF_ROWS);
    Writer.put(",\"letters\":\"");

    for(byte Index = 0u; Index < DISPLAY_CHARACTERS_NUMBER_OF_CHARACTERS; Index++) {
        char Utf8[2u];
        const byte Length = toUtf8(static_cast<byte>(Letters.getCharacter(Index)), Utf8);

        for(byte Byte = 0u; Byte < Length; Byte++) { Writer.put(Utf8[Byte]); }
    }

    Writer.put("\"}");
    Writer.finish();
} /* writeDisplay */


/******************************************************************************************************************************************************
  injectCommand()
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
void WebFrontend::injectCommand(const char* Text, size_t Length)
{
    if((Text == nullptr) || (Length == 0u) || (Length > WEB_FRONTEND_MAX_FRAME_LENGTH)) { return; }

    WordclockSerial& Port = WordclockSerial::getInstance();
    Port.inject(Text, Length);

    if(Text[Length - 1u] != Communication::getEndOfMessageChar()) {
        const char Terminator = Communication::getEndOfMessageChar();
        Port.inject(&Terminator, 1u);
    }
} /* injectCommand */


/******************************************************************************************************************************************************
  broadcastLine()
******************************************************************************************************************************************************/
/*! \brief          Sends one finished line to whoever is connected
 *
 *  \details        Runs in the firmware's task. A send that fails is not retried and the
 *                  client is not dropped here - a backend's own close hook owns that, and
 *                  dropping a client from this side would race with it.
******************************************************************************************************************************************************/
void WebFrontend::broadcastLine(const char* Line)
{
    WebTransport& Transport = WebTransport::getInstance();

    if(!Transport.isListening() || (Line == nullptr)) { return; }

    /* Widened rather than sent as it stands - see toUtf8(). Two bytes per Latin-1 byte is
       the worst case, so a line that is nothing but umlauts still fits and there is no
       truncation to expect. Sized from the producer's own line length rather than from a
       number of its own, because that is the buffer the line was assembled in.

       The bound is still checked. Nothing longer can arrive today, but this takes a plain
       pointer and the sink it is installed as is a function pointer - the day a second
       producer appears, an assumption written only in a comment is the one that gives. */
    char Payload[2u * WORDCLOCK_SERIAL_LINE_LENGTH];
    size_t Used{0u};

    for(const char* Character = Line; (*Character != '\0') && ((Used + 2u) <= sizeof(Payload)); Character++) {
        Used += toUtf8(static_cast<byte>(*Character), &Payload[Used]);
    }

    if(Used == 0u) { return; }

    Transport.sendText(Payload, Used);
} /* broadcastLine */


/******************************************************************************************************************************************************
  broadcastFrame()
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
void WebFrontend::broadcastFrame()
{
    WebTransport& Transport = WebTransport::getInstance();

    if(!Transport.isListening()) { return; }

    const bool Forced = ForceFrame.exchange(false, std::memory_order_acq_rel);

    if(!Forced) {
        if(FrameCountdown > 0u) { FrameCountdown--; return; }
    }
    FrameCountdown = WEB_FRONTEND_FRAME_INTERVAL_TICKS - 1u;

    /* Nothing to send to, so nothing is even gathered. */
    if(!Transport.hasClients()) { return; }

    byte Frame[FrameSize];
    byte* Target = Frame;
    const Pixels& Strip = Pixels::getInstance();

    for(byte Index = 0u; Index < PIXELS_NUMBER_OF_LEDS; Index++) {
        const Pixel Colour = Strip.getOutputPixel(Index);

        *Target++ = Colour.getGreen();
        *Target++ = Colour.getRed();
        *Target++ = Colour.getBlue();
    }

    if(!Forced && (memcmp(Frame, LastFrame, FrameSize) == 0)) { return; }
    memcpy(LastFrame, Frame, FrameSize);

    Transport.sendBinary(Frame, FrameSize);
} /* broadcastFrame */

#endif // (WEB_FRONTEND_SUPPORT == STD_ON)

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
