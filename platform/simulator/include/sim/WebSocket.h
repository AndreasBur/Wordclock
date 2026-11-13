// Copyright 2017 Andreas Burnickl
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
 * \file
 * \brief The web socket protocol, as far as the two pages need it.
 *
 * The handshake and the frames of RFC 6455. On strings only, with no socket in here, so that the
 * tests can check every byte without a network.
 */

#ifndef WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBSOCKET_H_
#define WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBSOCKET_H_

#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

/*!
 * \brief The web socket protocol of RFC 6455, as far as the two pages need it: the handshake, and
 *        the frames in both directions.
 *
 * No state of its own, so every function is static. It works on strings only; WebHost owns the
 * sockets.
 */
class WebSocket
{
  public:
    /*! \brief The opcode of a text frame, RFC 6455 section 5.2. */
    static constexpr uint8_t kTextOpcode{0x1u};
    /*! \brief The opcode of a close frame, RFC 6455 section 5.2. */
    static constexpr uint8_t kCloseOpcode{0x8u};

    static constexpr size_t kSha1Length{20u}; /*!< The number of bytes of a SHA-1 digest. */

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

    static StdReturnType GetKey(const std::string& request, std::string& key);
    static std::string PadSha1Message(const std::string& message);
    static void ProcessSha1Block(const uint8_t* block, uint32_t* state);

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
    /*!
     * \brief Writes into `answer` the response that opens a web socket for `request`.
     *
     * The handshake is the whole of what makes a socket out of a request: the 101 response, with
     * the accept key the browser checks. Only the Sec-WebSocket-Key header is read; the method,
     * the Upgrade header and the version are not checked.
     *
     * \param[in] request The whole HTTP request, headers included.
     * \param[out] answer The response to send back.
     * \return kEOk, or kENotOk if the request has no Sec-WebSocket-Key header, or one without a
     *         line end behind it; `answer` is then left as it was.
     */
    static StdReturnType GetHandshakeAnswer(const std::string& request, std::string& answer);

    /*!
     * \brief Returns `payload` wrapped into a web socket frame, header and payload.
     *
     * A final, unmasked frame, as a server sends it. Only the two lengths a clock ever sends are
     * encoded: a line is at most 2 * WORDCLOCK_SERIAL_LINE_LENGTH bytes and a display frame is
     * 330, so the 64-bit form is unreachable and writing it would be code no run ever takes.
     *
     * \param[in] is_binary True for a binary frame, false for a text one.
     * \param[in] payload The bytes to send.
     * \param[in] length The number of bytes of `payload`.
     * \return The frame.
     * \warning A `length` of 65536 or more is not refused: the header then holds only its lowest 16
     *          bits, and the frame is broken.
     */
    static std::string EncodeFrame(bool is_binary, const char* payload, size_t length);

    /*!
     * \brief Takes the first frame out of `buffer`.
     *
     * What a browser sends is always masked, which is the one part of the format this has to
     * undo; an unmasked frame is taken as well. A frame that has not fully arrived stays in the
     * buffer for the next call. Every frame is taken as a whole message: the FIN bit is not read,
     * so a fragment comes out as a frame of its own, a continuation with opcode 0.
     *
     * A frame that announces a 64-bit length is never one a page sends. It empties `buffer`,
     * with any frame that followed it, and kENotOk comes back.
     *
     * \param[in,out] buffer What has arrived on the connection and is not yet decoded. The frame is
     *                       removed from its front.
     * \param[out] opcode The opcode of the frame.
     * \param[out] payload The payload of the frame, unmasked.
     * \return kEOk, or kENotOk if `buffer` holds no complete frame; `opcode` and `payload` are then
     *         left as they were, and so is `buffer`, unless its frame announced a 64-bit length.
     */
    static StdReturnType DecodeFrame(std::string& buffer, uint8_t& opcode, std::string& payload);

    /*!
     * \brief Returns the value of Sec-WebSocket-Accept for the browser's `key`: the base64 of the
     *        SHA-1 of the key and the fixed GUID of RFC 6455.
     *
     * \param[in] key The value of Sec-WebSocket-Key.
     * \return The accept key.
     */
    static std::string ConvertKeyToAcceptKey(const std::string& key);

    /*!
     * \brief Calculates the SHA-1 of `message`.
     *
     * \param[in] message The bytes to hash.
     * \param[out] digest The hash, kSha1Length bytes.
     */
    static void CalculateSha1(const std::string& message, uint8_t* digest);

    /*!
     * \brief Returns `length` bytes of `bytes` in base64.
     *
     * Every three bytes become four characters, and the last group is padded with '='. The text
     * has no line breaks.
     *
     * \param[in] bytes The bytes to convert.
     * \param[in] length The number of bytes of `bytes`.
     * \return The base64 text.
     */
    static std::string ConvertToBase64(const uint8_t* bytes, size_t length);
};

#endif  // WORDCLOCK_PLATFORM_SIMULATOR_INCLUDE_SIM_WEBSOCKET_H_
