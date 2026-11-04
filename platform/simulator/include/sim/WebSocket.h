// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The web socket protocol, as far as the two pages need it.
 *
 * The handshake and the frames of RFC 6455. On strings only, with no socket in here, so that the
 * tests can check every byte without a network.
 */

#ifndef _WEB_SOCKET_H_
#define _WEB_SOCKET_H_

#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

/* No state of its own, so every function is static. */
class WebSocket
{
  public:
    /* RFC 6455, section 5.2 */
    static constexpr uint8_t kTextOpcode{0x1u};
    static constexpr uint8_t kCloseOpcode{0x8u};

    static constexpr size_t kSha1Length{20u};

  private:
    /* Handshake, RFC 6455 section 1.3: the server hashes the client's key with this fixed string. */
    static constexpr char kGuid[]{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};
    static constexpr char kKeyHeader[]{"sec-websocket-key:"};

    /* Frame, RFC 6455 section 5.2 */
    static constexpr uint8_t kFinalTextFrame{0x81u};
    static constexpr uint8_t kFinalBinaryFrame{0x82u};
    static constexpr uint8_t kOpcodeMask{0x0Fu};
    static constexpr uint8_t kMaskFlag{0x80u};
    static constexpr size_t kMaskLength{4u};
    static constexpr uint8_t kLengthMask{0x7Fu};
    static constexpr size_t kLength16Bit{126u};
    static constexpr size_t kLength64Bit{127u};
    static constexpr size_t kHeaderLength{2u};
    static constexpr size_t kHeaderLength16Bit{4u};

    /* SHA-1, FIPS 180-4 */
    static constexpr size_t kSha1BlockLength{64u};
    static constexpr size_t kSha1LengthPosition{56u};
    static constexpr size_t kSha1StateLength{5u};
    static constexpr size_t kSha1WordsPerBlock{16u};
    static constexpr size_t kSha1Rounds{80u};

    static StdReturnType GetKey(const std::string&, std::string&);
    static std::string PadSha1Message(const std::string&);
    static void ProcessSha1Block(const uint8_t*, uint32_t*);

    static constexpr uint32_t RotateLeft(uint32_t value, unsigned bits) {
        return (value << bits) | (value >> (32u - bits));
    }

    /* The round's function of B, C and D plus its constant, both of which change every 20 rounds. */
    static constexpr uint32_t MixSha1(size_t round, uint32_t b, uint32_t c, uint32_t d) {
        if(round < 20u) { return ((b & c) | ((~b) & d)) + 0x5A827999u; }
        if(round < 40u) { return (b ^ c ^ d) + 0x6ED9EBA1u; }
        if(round < 60u) { return ((b & c) | (b & d) | (c & d)) + 0x8F1BBCDCu; }
        return (b ^ c ^ d) + 0xCA62C1D6u;
    }

  public:
    // get methods
    static StdReturnType GetHandshakeAnswer(const std::string&, std::string&);

    // methods
    static std::string EncodeFrame(bool, const char*, size_t);
    static StdReturnType DecodeFrame(std::string&, uint8_t&, std::string&);
    static std::string ConvertKeyToAcceptKey(const std::string&);
    static void CalculateSha1(const std::string&, uint8_t*);
    static std::string ConvertToBase64(const uint8_t*, size_t);
};

#endif // _WEB_SOCKET_H_
