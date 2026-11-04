// Copyright 2026 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The web socket protocol, as far as the two pages need it.
 *
 * SHA-1 and base64 are here for one reason only: RFC 6455 makes the handshake answer the base64 of
 * the SHA-1 of the client's key and a fixed string. There is nothing to choose about either, so
 * they are written out rather than pulled in - a dependency for eighty lines of arithmetic would be
 * a worse trade in a build that otherwise needs only wxWidgets.
 *
 * No other use, and in particular not a security one: this is a checksum a browser demands, not a
 * secret.
 */

#include "sim/WebSocket.h"

#include <cctype>

/*! \brief          Answers a browser's request to open a web socket
 *  \details        The handshake is the whole of what makes a socket out of a request: the 101
 *                  response, with the accept key the browser checks.
 *
 *  \param[in]      request         the whole HTTP request, headers included
 *  \param[out]     answer          the response to send back
 *  \return         kEOk
 *                  kENotOk        the request has no Sec-WebSocket-Key
 */
StdReturnType WebSocket::GetHandshakeAnswer(const std::string& request, std::string& answer)
{
    std::string key;

    if(GetKey(request, key) != kEOk) { return kENotOk; }

    answer = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
             "Sec-WebSocket-Accept: " + ConvertKeyToAcceptKey(key) + "\r\n\r\n";
    return kEOk;
}

/*! \brief          Wraps a payload into a frame
 *  \details        A final, unmasked frame, as a server sends it. Only the two lengths a clock
 *                  ever sends are encoded: a line is well under 126 bytes and a display frame
 *                  is 330, so the 64-bit form is unreachable and writing it would be code no
 *                  run ever takes.
 *
 *  \param[in]      is_binary       a binary frame rather than a text one
 *  \param[in]      payload         the bytes to send
 *  \param[in]      length          the number of bytes
 *  \return         the frame, header and payload
 */
std::string WebSocket::EncodeFrame(bool is_binary, const char* payload, size_t length)
{
    std::string frame;

    frame.push_back(static_cast<char>(is_binary ? kFinalBinaryFrame : kFinalTextFrame));
    if(length < kLength16Bit) {
        frame.push_back(static_cast<char>(length));
    } else {
        frame.push_back(static_cast<char>(kLength16Bit));
        frame.push_back(static_cast<char>((length >> 8) & 0xFFu));
        frame.push_back(static_cast<char>(length & 0xFFu));
    }
    frame.append(payload, length);
    return frame;
}

/*! \brief          Takes the first frame out of what has arrived
 *  \details        What a browser sends is always masked, which is the one part of the format
 *                  this has to undo. A frame that has not fully arrived stays in the buffer for
 *                  the next call.
 *
 *  \param[in,out]  buffer          what has arrived on the connection and is not yet decoded
 *  \param[out]     opcode          the opcode of the frame
 *  \param[out]     payload         the payload of the frame, unmasked
 *  \return         kEOk
 *                  kENotOk        no complete frame in the buffer
 */
StdReturnType WebSocket::DecodeFrame(std::string& buffer, uint8_t& opcode, std::string& payload)
{
    if(buffer.size() < kHeaderLength) { return kENotOk; }

    const uint8_t first = static_cast<uint8_t>(buffer[0]);
    const uint8_t second = static_cast<uint8_t>(buffer[1]);
    const size_t mask = ((second & kMaskFlag) != 0u) ? kMaskLength : 0u;
    size_t length = second & kLengthMask;
    size_t offset = kHeaderLength;

    if(length == kLength64Bit) {
        /*
         * A command is never this long, and a browser that says so is not one this serves.
         * Dropping what arrived is better than trying to read it - and better than keeping
         * it, which would leave every later frame stuck behind this one.
         */
        buffer.clear();
        return kENotOk;
    }
    if(length == kLength16Bit) {
        if(buffer.size() < kHeaderLength16Bit) { return kENotOk; }
        length = (static_cast<size_t>(static_cast<uint8_t>(buffer[2])) << 8) | static_cast<uint8_t>(buffer[3]);
        offset = kHeaderLength16Bit;
    }
    if(buffer.size() < (offset + mask + length)) { return kENotOk; }

    payload.assign(buffer, offset + mask, length);
    if(mask > 0u) {
        for(size_t index = 0u; index < length; index++) {
            payload[index] = static_cast<char>(payload[index] ^ buffer[offset + (index % mask)]);
        }
    }

    opcode = first & kOpcodeMask;
    buffer.erase(0u, offset + mask + length);
    return kEOk;
}

/*! \brief          Converts the browser's key into the key it expects back
 *
 *  \param[in]      Key             the value of Sec-WebSocket-Key
 *  \return         the value of Sec-WebSocket-Accept: the base64 of the SHA-1 of the key and
 *                  the fixed GUID of RFC 6455
 */
std::string WebSocket::ConvertKeyToAcceptKey(const std::string& key)
{
    uint8_t digest[kSha1Length];

    CalculateSha1(key + kGuid, digest);
    return ConvertToBase64(digest, sizeof(digest));
}

/*! \brief          Calculates the SHA-1 of a message
 *
 *  \param[in]      message         the bytes to hash
 *  \param[out]     digest          the hash, kSha1Length bytes
 *  \return         -
 */
void WebSocket::CalculateSha1(const std::string& message, uint8_t* digest)
{
    uint32_t state[kSha1StateLength]{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    const std::string padded = PadSha1Message(message);

    for(size_t offset = 0u; offset < padded.size(); offset += kSha1BlockLength) {
        ProcessSha1Block(reinterpret_cast<const uint8_t*>(&padded[offset]), state);
    }
    for(size_t word = 0u; word < kSha1StateLength; word++) {
        for(size_t byte = 0u; byte < 4u; byte++) {
            digest[(word * 4u) + byte] = static_cast<uint8_t>(state[word] >> (24u - (8u * byte)));
        }
    }
}

/*! \brief          Converts bytes to base64
 *  \details        Every three bytes become four characters, and the last group is padded
 *                  with '='.
 *
 *  \param[in]      bytes           the bytes to convert
 *  \param[in]      length          the number of bytes
 *  \return         the base64 text
 */
std::string WebSocket::ConvertToBase64(const uint8_t* bytes, size_t length)
{
    static constexpr char kAlphabet[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    std::string encoded;

    for(size_t offset = 0u; offset < length; offset += 3u) {
        const size_t remaining = length - offset;
        uint32_t group = static_cast<uint32_t>(bytes[offset]) << 16;

        if(remaining > 1u) { group |= static_cast<uint32_t>(bytes[offset + 1u]) << 8; }
        if(remaining > 2u) { group |= static_cast<uint32_t>(bytes[offset + 2u]); }

        encoded.push_back(kAlphabet[(group >> 18) & 0x3Fu]);
        encoded.push_back(kAlphabet[(group >> 12) & 0x3Fu]);
        encoded.push_back((remaining > 1u) ? kAlphabet[(group >> 6) & 0x3Fu] : '=');
        encoded.push_back((remaining > 2u) ? kAlphabet[group & 0x3Fu] : '=');
    }
    return encoded;
}

/*! \brief          Finds the browser's key in a request
 *  \details        Searched in a lowered copy, because header names are case-insensitive, and
 *                  returned from the request itself, because the key is not.
 *
 *  \param[in]      request         the whole HTTP request, headers included
 *  \param[out]     Key             the value of Sec-WebSocket-Key
 *  \return         kEOk
 *                  kENotOk        the request has no such header
 */
StdReturnType WebSocket::GetKey(const std::string& request, std::string& key)
{
    std::string lowered(request);
    for(char& character : lowered) { character = static_cast<char>(tolower(static_cast<unsigned char>(character))); }

    const size_t found = lowered.find(kKeyHeader);
    if(found == std::string::npos) { return kENotOk; }

    size_t start = found + sizeof(kKeyHeader) - 1u;
    while((start < request.size()) && (request[start] == ' ')) { start++; }
    const size_t end = request.find("\r\n", start);
    if(end == std::string::npos) { return kENotOk; }

    key = request.substr(start, end - start);
    return kEOk;
}

/*! \brief          Pads a message to whole blocks
 *  \details        The message, its terminator bit and its length in bits, which is what SHA-1
 *                  hashes.
 *
 *  \param[in]      message         the bytes to hash
 *  \return         the padded message, a multiple of kSha1BlockLength long
 */
std::string WebSocket::PadSha1Message(const std::string& message)
{
    const uint64_t bits = static_cast<uint64_t>(message.size()) * 8u;
    std::string padded(message);

    padded.push_back(static_cast<char>(0x80));
    while((padded.size() % kSha1BlockLength) != kSha1LengthPosition) { padded.push_back('\0'); }
    for(int shift = 56; shift >= 0; shift -= 8) {
        padded.push_back(static_cast<char>((bits >> shift) & 0xFFu));
    }
    return padded;
}

/*! \brief          Mixes one block into the state
 *  \details        Expands the block into the message schedule of 80 words and runs the 80
 *                  rounds over it.
 *
 *  \param[in]      block           kSha1BlockLength bytes of the padded message
 *  \param[in,out]  state           the five words of the hash so far
 *  \return         -
 */
void WebSocket::ProcessSha1Block(const uint8_t* block, uint32_t* state)
{
    uint32_t word[kSha1Rounds];
    uint32_t a{state[0]}, b{state[1]}, c{state[2]}, d{state[3]}, e{state[4]};

    for(size_t index = 0u; index < kSha1WordsPerBlock; index++) {
        const uint8_t* chunk = &block[index * 4u];
        word[index] = (static_cast<uint32_t>(chunk[0]) << 24) | (static_cast<uint32_t>(chunk[1]) << 16)
                    | (static_cast<uint32_t>(chunk[2]) << 8) | static_cast<uint32_t>(chunk[3]);
    }
    for(size_t index = kSha1WordsPerBlock; index < kSha1Rounds; index++) {
        word[index] = RotateLeft(word[index - 3u] ^ word[index - 8u] ^ word[index - 14u] ^ word[index - 16u], 1u);
    }
    for(size_t round = 0u; round < kSha1Rounds; round++) {
        const uint32_t next = RotateLeft(a, 5u) + MixSha1(round, b, c, d) + e + word[round];
        e = d;
        d = c;
        c = RotateLeft(b, 30u);
        b = a;
        a = next;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}
