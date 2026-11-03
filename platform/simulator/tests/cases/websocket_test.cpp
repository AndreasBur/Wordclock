/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       websocket_test.cpp
 *      \brief      The web socket protocol, against the RFCs rather than against a browser
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "sim/WebSocket.h"

#include <cstdio>
#include <string>

namespace {

std::string ToSha1Hex(const std::string& message)
{
    uint8_t digest[WebSocket::kSha1Length];
    char hex[3];
    std::string text;

    WebSocket::CalculateSha1(message, digest);
    for(const uint8_t byte : digest) {
        /* Two hex digits always fit the three bytes, so there is no truncation to report. */
        (void)snprintf(hex, sizeof(hex), "%02x", byte);
        text += hex;
    }
    return text;
}

std::string ToBase64(const std::string& text)
{
    return WebSocket::ConvertToBase64(reinterpret_cast<const uint8_t*>(text.data()), text.size());
}

/* A text frame as a browser sends it: final, masked, and with a mask that is not zeros,
   which would leave a decoder that unmasks wrongly undetected. */
std::string ToClientFrame(const std::string& payload)
{
    const uint8_t mask[4]{0x12u, 0x34u, 0x56u, 0x78u};
    std::string frame;

    frame.push_back(static_cast<char>(0x81));
    if(payload.size() < 126u) {
        frame.push_back(static_cast<char>(0x80u | payload.size()));
    } else {
        frame.push_back(static_cast<char>(0x80u | 126u));
        frame.push_back(static_cast<char>(payload.size() >> 8));
        frame.push_back(static_cast<char>(payload.size() & 0xFFu));
    }
    frame.append(reinterpret_cast<const char*>(mask), sizeof(mask));
    for(size_t index = 0u; index < payload.size(); index++) {
        frame.push_back(static_cast<char>(payload[index] ^ mask[index % 4u]));
    }
    return frame;
}

std::string ToServerFrame(const std::string& payload)
{
    return WebSocket::EncodeFrame(false, payload.data(), payload.size());
}

} // namespace

/* The examples of FIPS 180-2, appendix A, the last one two blocks long - and the RFC 4648
   examples for base64, one for every length the last group can have. Then the example of
   RFC 6455 section 1.3, which names its answer and so checks both together. */
void TestWebSocketHashAndEncoding()
{
    Expect(ToSha1Hex("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 of the empty message");
    Expect(ToSha1Hex("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d", "SHA-1 of a one-block message");
    Expect(ToSha1Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
               == "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
           "SHA-1 of a two-block message");

    Expect(ToBase64("").empty(), "base64 of nothing is nothing");
    Expect(ToBase64("f") == "Zg==", "base64 pads one byte with two '='");
    Expect(ToBase64("fo") == "Zm8=", "base64 pads two bytes with one '='");
    Expect(ToBase64("foo") == "Zm9v", "base64 of a whole group has no padding");
    Expect(ToBase64("foobar") == "Zm9vYmFy", "base64 of two whole groups");

    Expect(WebSocket::ConvertKeyToAcceptKey("dGhlIHNhbXBsZSBub25jZQ==") == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=",
           "the accept key must match RFC 6455's example");
}

/* Header names are case-insensitive, and a browser may put more than one space after the
   colon. */
void TestWebSocketHandshake()
{
    std::string answer;

    Expect(WebSocket::GetHandshakeAnswer("GET /ws HTTP/1.1\r\nSEC-WEBSOCKET-KEY:   dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n",
                                         answer) == kEOk,
           "the key must be found in any case");
    Expect(answer.compare(0u, 13u, "HTTP/1.1 101 ") == 0, "the answer switches protocols");
    Expect(answer.find("Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n") != std::string::npos,
           "the answer carries the accept key");
    Expect((answer.size() >= 4u) && (answer.substr(answer.size() - 4u) == "\r\n\r\n"),
           "the answer ends its headers");

    Expect(WebSocket::GetHandshakeAnswer("GET /ws HTTP/1.1\r\nHost: localhost\r\n\r\n", answer) == kENotOk,
           "a request without a key is no handshake");
}

/* 125 bytes is the longest payload whose length fits in the second byte; a display frame is
   past it and takes the 16-bit length. */
void TestWebSocketEncodesBothLengths()
{
    const std::string short_frame = ToServerFrame(std::string(125u, 'x'));
    Expect(short_frame.size() == (2u + 125u), "a short frame has a two-byte header");
    Expect(static_cast<uint8_t>(short_frame[0]) == 0x81u, "a text frame is final and has the text opcode");
    Expect(static_cast<uint8_t>(short_frame[1]) == 125u, "and carries its length in the second byte");

    const std::string long_frame = ToServerFrame(std::string(330u, 'x'));
    Expect(long_frame.size() == (4u + 330u), "a long frame has a four-byte header");
    Expect(static_cast<uint8_t>(long_frame[1]) == 126u, "and says so in the second byte");
    Expect((static_cast<uint8_t>(long_frame[2]) == (330u >> 8)) && (static_cast<uint8_t>(long_frame[3]) == (330u & 0xFFu)),
           "and carries its length big-endian in the next two");
    Expect(long_frame.substr(4u) == std::string(330u, 'x'), "the payload follows unmasked");

    const char bytes[]{'\x01', '\x02'};
    const std::string binary = WebSocket::EncodeFrame(true, bytes, sizeof(bytes));
    Expect(static_cast<uint8_t>(binary[0]) == 0x82u, "a binary frame has the binary opcode");
}

void TestWebSocketDecodesWhatABrowserSends()
{
    std::string payload;
    uint8_t opcode{0u};

    std::string buffer = ToClientFrame("2 0");
    Expect(WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk, "a whole masked frame decodes");
    Expect(opcode == WebSocket::kTextOpcode, "as text");
    Expect(payload == "2 0", "and unmasked");
    Expect(buffer.empty(), "and is taken out of the buffer");

    buffer = ToClientFrame(std::string(200u, 'x'));
    Expect((WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk) && (payload == std::string(200u, 'x')),
           "a frame with the 16-bit length decodes");

    /* A frame may arrive over several reads; until it is whole, nothing is taken from the
       buffer. */
    const std::string frame = ToClientFrame("hello");
    buffer.clear();
    for(size_t index = 0u; (index + 1u) < frame.size(); index++) {
        buffer.push_back(frame[index]);
        if((WebSocket::DecodeFrame(buffer, opcode, payload) != kENotOk) || (buffer.size() != (index + 1u))) {
            Expect(false, "a partial frame must stay in the buffer undecoded");
            break;
        }
    }
    buffer.push_back(frame.back());
    Expect((WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk) && (payload == "hello"),
           "the last byte completes it");

    /* Two frames in one read come out one at a time, in order. */
    buffer = ToClientFrame("first") + ToClientFrame("second");
    Expect((WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk) && (payload == "first"), "the first frame first");
    Expect((WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk) && (payload == "second"), "then the second");
    Expect(WebSocket::DecodeFrame(buffer, opcode, payload) == kENotOk, "then nothing");

    /* Kept, a frame announcing a 64-bit length would block every frame behind it. */
    buffer = std::string("\x81\xFF", 2u) + std::string(12u, '\0');
    Expect(WebSocket::DecodeFrame(buffer, opcode, payload) == kENotOk, "a 64-bit length is refused");
    Expect(buffer.empty(), "and dropped");

    buffer = ToServerFrame(std::string(330u, 'y'));
    Expect((WebSocket::DecodeFrame(buffer, opcode, payload) == kEOk) && (payload == std::string(330u, 'y')),
           "what encodeFrame() writes decodes again");
}
