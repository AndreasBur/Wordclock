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

StdReturnType WebSocket::GetHandshakeAnswer(std::string const& request, std::string& answer) {
  std::string key;

  if (GetKey(request, key) != kEOk) {
    return kENotOk;
  }

  answer =
      "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
      "Sec-WebSocket-Accept: " +
      ConvertKeyToAcceptKey(key) + "\r\n\r\n";
  return kEOk;
}

std::string WebSocket::EncodeFrame(bool is_binary, char const* payload, size_t length) {
  std::string frame;

  frame.push_back(static_cast<char>(is_binary ? kFinalBinaryFrame : kFinalTextFrame));
  if (length < kLength16Bit) {
    frame.push_back(static_cast<char>(length));
  } else {
    frame.push_back(static_cast<char>(kLength16Bit));
    frame.push_back(static_cast<char>((length >> 8) & 0xFFu));
    frame.push_back(static_cast<char>(length & 0xFFu));
  }
  frame.append(payload, length);
  return frame;
}

StdReturnType WebSocket::DecodeFrame(std::string& buffer, uint8_t& opcode, std::string& payload) {
  if (buffer.size() < kHeaderLength) {
    return kENotOk;
  }

  uint8_t const first = static_cast<uint8_t>(buffer[0]);
  uint8_t const second = static_cast<uint8_t>(buffer[1]);
  size_t const mask = ((second & kMaskFlag) != 0u) ? kMaskLength : 0u;
  size_t length = second & kLengthMask;
  size_t offset = kHeaderLength;

  if (length == kLength64Bit) {
    /*
     * A command is never this long, and a browser that says so is not one this serves.
     * Dropping what arrived is better than trying to read it - and better than keeping
     * it, which would leave every later frame stuck behind this one.
     */
    buffer.clear();
    return kENotOk;
  }
  if (length == kLength16Bit) {
    if (buffer.size() < kHeaderLength16Bit) {
      return kENotOk;
    }
    length = (static_cast<size_t>(static_cast<uint8_t>(buffer[2])) << 8) |
             static_cast<uint8_t>(buffer[3]);
    offset = kHeaderLength16Bit;
  }
  if (buffer.size() < (offset + mask + length)) {
    return kENotOk;
  }

  payload.assign(buffer, offset + mask, length);
  if (mask > 0u) {
    for (size_t index = 0u; index < length; index++) {
      payload[index] = static_cast<char>(payload[index] ^ buffer[offset + (index % mask)]);
    }
  }

  opcode = first & kOpcodeMask;
  buffer.erase(0u, offset + mask + length);
  return kEOk;
}

std::string WebSocket::ConvertKeyToAcceptKey(std::string const& key) {
  uint8_t digest[kSha1Length];

  CalculateSha1(key + kGuid, digest);
  return ConvertToBase64(digest, sizeof(digest));
}

void WebSocket::CalculateSha1(std::string const& message, uint8_t* digest) {
  uint32_t state[kSha1StateLength]{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
  std::string const padded = PadSha1Message(message);

  for (size_t offset = 0u; offset < padded.size(); offset += kSha1BlockLength) {
    ProcessSha1Block(reinterpret_cast<uint8_t const*>(&padded[offset]), state);
  }
  for (size_t word = 0u; word < kSha1StateLength; word++) {
    for (size_t byte = 0u; byte < 4u; byte++) {
      digest[(word * 4u) + byte] = static_cast<uint8_t>(state[word] >> (24u - (8u * byte)));
    }
  }
}

std::string WebSocket::ConvertToBase64(uint8_t const* bytes, size_t length) {
  static constexpr char kAlphabet[]{
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
  std::string encoded;

  for (size_t offset = 0u; offset < length; offset += 3u) {
    size_t const remaining = length - offset;
    uint32_t group = static_cast<uint32_t>(bytes[offset]) << 16;

    if (remaining > 1u) {
      group |= static_cast<uint32_t>(bytes[offset + 1u]) << 8;
    }
    if (remaining > 2u) {
      group |= static_cast<uint32_t>(bytes[offset + 2u]);
    }

    encoded.push_back(kAlphabet[(group >> 18) & 0x3Fu]);
    encoded.push_back(kAlphabet[(group >> 12) & 0x3Fu]);
    encoded.push_back((remaining > 1u) ? kAlphabet[(group >> 6) & 0x3Fu] : '=');
    encoded.push_back((remaining > 2u) ? kAlphabet[group & 0x3Fu] : '=');
  }
  return encoded;
}

/*
 * Finds the browser's key in a request; kENotOk, with `key` left as it was, if the request has
 * no such header or no line end behind it. Searched in a lowered copy, because header names are
 * case-insensitive, and returned from the request itself, because the key is not.
 */
StdReturnType WebSocket::GetKey(std::string const& request, std::string& key) {
  std::string lowered(request);
  for (char& character : lowered) {
    character = static_cast<char>(tolower(static_cast<unsigned char>(character)));
  }

  size_t const found = lowered.find(kKeyHeader);
  if (found == std::string::npos) {
    return kENotOk;
  }

  size_t start = found + sizeof(kKeyHeader) - 1u;
  while ((start < request.size()) && (request[start] == ' ')) {
    start++;
  }
  size_t const end = request.find("\r\n", start);
  if (end == std::string::npos) {
    return kENotOk;
  }

  key = request.substr(start, end - start);
  return kEOk;
}

// Pads a message to whole blocks of kSha1BlockLength: the message, its terminator bit and its
// length in bits, which is what SHA-1 hashes.
std::string WebSocket::PadSha1Message(std::string const& message) {
  uint64_t const bits = static_cast<uint64_t>(message.size()) * 8u;
  std::string padded(message);

  padded.push_back(static_cast<char>(0x80));
  while ((padded.size() % kSha1BlockLength) != kSha1LengthPosition) {
    padded.push_back('\0');
  }
  for (int shift = 56; shift >= 0; shift -= 8) {
    padded.push_back(static_cast<char>((bits >> shift) & 0xFFu));
  }
  return padded;
}

// Mixes one block into the state: expands it into the message schedule of 80 words and runs the
// 80 rounds over it.
void WebSocket::ProcessSha1Block(uint8_t const* block, uint32_t* state) {
  uint32_t word[kSha1Rounds];
  uint32_t a{state[0]}, b{state[1]}, c{state[2]}, d{state[3]}, e{state[4]};

  for (size_t index = 0u; index < kSha1WordsPerBlock; index++) {
    uint8_t const* chunk = &block[index * 4u];
    word[index] = (static_cast<uint32_t>(chunk[0]) << 24) |
                  (static_cast<uint32_t>(chunk[1]) << 16) | (static_cast<uint32_t>(chunk[2]) << 8) |
                  static_cast<uint32_t>(chunk[3]);
  }
  for (size_t index = kSha1WordsPerBlock; index < kSha1Rounds; index++) {
    word[index] =
        RotateLeft(word[index - 3u] ^ word[index - 8u] ^ word[index - 14u] ^ word[index - 16u], 1u);
  }
  for (size_t round = 0u; round < kSha1Rounds; round++) {
    uint32_t const next = RotateLeft(a, 5u) + MixSha1(round, b, c, d) + e + word[round];
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
