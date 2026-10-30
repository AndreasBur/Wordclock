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

std::string toSha1Hex(const std::string& Message)
{
    uint8_t Digest[WebSocket::Sha1Length];
    char Hex[3];
    std::string Text;

    WebSocket::calculateSha1(Message, Digest);
    for(const uint8_t Byte : Digest) {
        /* Two hex digits always fit the three bytes, so there is no truncation to report. */
        (void)snprintf(Hex, sizeof(Hex), "%02x", Byte);
        Text += Hex;
    }
    return Text;
}

std::string toBase64(const std::string& Text)
{
    return WebSocket::convertToBase64(reinterpret_cast<const uint8_t*>(Text.data()), Text.size());
}

/* A text frame as a browser sends it: final, masked, and with a mask that is not zeros,
   which would leave a decoder that unmasks wrongly undetected. */
std::string toClientFrame(const std::string& Payload)
{
    const uint8_t Mask[4]{0x12u, 0x34u, 0x56u, 0x78u};
    std::string Frame;

    Frame.push_back(static_cast<char>(0x81));
    if(Payload.size() < 126u) {
        Frame.push_back(static_cast<char>(0x80u | Payload.size()));
    } else {
        Frame.push_back(static_cast<char>(0x80u | 126u));
        Frame.push_back(static_cast<char>(Payload.size() >> 8));
        Frame.push_back(static_cast<char>(Payload.size() & 0xFFu));
    }
    Frame.append(reinterpret_cast<const char*>(Mask), sizeof(Mask));
    for(size_t Index = 0u; Index < Payload.size(); Index++) {
        Frame.push_back(static_cast<char>(Payload[Index] ^ Mask[Index % 4u]));
    }
    return Frame;
}

std::string toServerFrame(const std::string& Payload)
{
    return WebSocket::encodeFrame(false, Payload.data(), Payload.size());
}

} // namespace

/* The examples of FIPS 180-2, appendix A, the last one two blocks long - and the RFC 4648
   examples for base64, one for every length the last group can have. Then the example of
   RFC 6455 section 1.3, which names its answer and so checks both together. */
void testWebSocketHashAndEncoding()
{
    expect(toSha1Hex("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709", "SHA-1 of the empty message");
    expect(toSha1Hex("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d", "SHA-1 of a one-block message");
    expect(toSha1Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
               == "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
           "SHA-1 of a two-block message");

    expect(toBase64("").empty(), "base64 of nothing is nothing");
    expect(toBase64("f") == "Zg==", "base64 pads one byte with two '='");
    expect(toBase64("fo") == "Zm8=", "base64 pads two bytes with one '='");
    expect(toBase64("foo") == "Zm9v", "base64 of a whole group has no padding");
    expect(toBase64("foobar") == "Zm9vYmFy", "base64 of two whole groups");

    expect(WebSocket::convertKeyToAcceptKey("dGhlIHNhbXBsZSBub25jZQ==") == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=",
           "the accept key must match RFC 6455's example");
}

/* Header names are case-insensitive, and a browser may put more than one space after the
   colon. */
void testWebSocketHandshake()
{
    std::string Answer;

    expect(WebSocket::getHandshakeAnswer("GET /ws HTTP/1.1\r\nSEC-WEBSOCKET-KEY:   dGhlIHNhbXBsZSBub25jZQ==\r\n\r\n",
                                         Answer) == E_OK,
           "the key must be found in any case");
    expect(Answer.compare(0u, 13u, "HTTP/1.1 101 ") == 0, "the answer switches protocols");
    expect(Answer.find("Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n") != std::string::npos,
           "the answer carries the accept key");
    expect((Answer.size() >= 4u) && (Answer.substr(Answer.size() - 4u) == "\r\n\r\n"),
           "the answer ends its headers");

    expect(WebSocket::getHandshakeAnswer("GET /ws HTTP/1.1\r\nHost: localhost\r\n\r\n", Answer) == E_NOT_OK,
           "a request without a key is no handshake");
}

/* 125 bytes is the longest payload whose length fits in the second byte; a display frame is
   past it and takes the 16-bit length. */
void testWebSocketEncodesBothLengths()
{
    const std::string Short = toServerFrame(std::string(125u, 'x'));
    expect(Short.size() == (2u + 125u), "a short frame has a two-byte header");
    expect(static_cast<uint8_t>(Short[0]) == 0x81u, "a text frame is final and has the text opcode");
    expect(static_cast<uint8_t>(Short[1]) == 125u, "and carries its length in the second byte");

    const std::string Long = toServerFrame(std::string(330u, 'x'));
    expect(Long.size() == (4u + 330u), "a long frame has a four-byte header");
    expect(static_cast<uint8_t>(Long[1]) == 126u, "and says so in the second byte");
    expect((static_cast<uint8_t>(Long[2]) == (330u >> 8)) && (static_cast<uint8_t>(Long[3]) == (330u & 0xFFu)),
           "and carries its length big-endian in the next two");
    expect(Long.substr(4u) == std::string(330u, 'x'), "the payload follows unmasked");

    const char Bytes[]{'\x01', '\x02'};
    const std::string Binary = WebSocket::encodeFrame(true, Bytes, sizeof(Bytes));
    expect(static_cast<uint8_t>(Binary[0]) == 0x82u, "a binary frame has the binary opcode");
}

void testWebSocketDecodesWhatABrowserSends()
{
    std::string Payload;
    uint8_t Opcode{0u};

    std::string Buffer = toClientFrame("2 0");
    expect(WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK, "a whole masked frame decodes");
    expect(Opcode == WebSocket::TextOpcode, "as text");
    expect(Payload == "2 0", "and unmasked");
    expect(Buffer.empty(), "and is taken out of the buffer");

    Buffer = toClientFrame(std::string(200u, 'x'));
    expect((WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK) && (Payload == std::string(200u, 'x')),
           "a frame with the 16-bit length decodes");

    /* A frame may arrive over several reads; until it is whole, nothing is taken from the
       buffer. */
    const std::string Frame = toClientFrame("hello");
    Buffer.clear();
    for(size_t Index = 0u; (Index + 1u) < Frame.size(); Index++) {
        Buffer.push_back(Frame[Index]);
        if((WebSocket::decodeFrame(Buffer, Opcode, Payload) != E_NOT_OK) || (Buffer.size() != (Index + 1u))) {
            expect(false, "a partial frame must stay in the buffer undecoded");
            break;
        }
    }
    Buffer.push_back(Frame.back());
    expect((WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK) && (Payload == "hello"),
           "the last byte completes it");

    /* Two frames in one read come out one at a time, in order. */
    Buffer = toClientFrame("first") + toClientFrame("second");
    expect((WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK) && (Payload == "first"), "the first frame first");
    expect((WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK) && (Payload == "second"), "then the second");
    expect(WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_NOT_OK, "then nothing");

    /* Kept, a frame announcing a 64-bit length would block every frame behind it. */
    Buffer = std::string("\x81\xFF", 2u) + std::string(12u, '\0');
    expect(WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_NOT_OK, "a 64-bit length is refused");
    expect(Buffer.empty(), "and dropped");

    Buffer = toServerFrame(std::string(330u, 'y'));
    expect((WebSocket::decodeFrame(Buffer, Opcode, Payload) == E_OK) && (Payload == std::string(330u, 'y')),
           "what encodeFrame() writes decodes again");
}
