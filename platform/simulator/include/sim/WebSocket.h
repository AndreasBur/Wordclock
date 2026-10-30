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
/**     \file       WebSocket.h
 *      \brief      The web socket protocol, as far as the two pages need it
 *
 *      \details    The handshake and the frames of RFC 6455. On strings only, with no socket in
 *                  here, so that the tests can check every byte without a network.
 *
******************************************************************************************************************************************************/
#ifndef _WEB_SOCKET_H_
#define _WEB_SOCKET_H_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "StandardTypes.h"

#include <stddef.h>
#include <stdint.h>
#include <string>

/******************************************************************************************************************************************************
 *  C L A S S   W E B   S O C K E T
******************************************************************************************************************************************************/
/* No state of its own, so every function is static. */
class WebSocket
{
/******************************************************************************************************************************************************
 *  P U B L I C   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
  public:
    /* RFC 6455, section 5.2 */
    static constexpr uint8_t TextOpcode{0x1u};
    static constexpr uint8_t CloseOpcode{0x8u};

    static constexpr size_t Sha1Length{20u};

/******************************************************************************************************************************************************
 *  P R I V A T E   D A T A   A N D   F U N C T I O N S
******************************************************************************************************************************************************/
  private:
    /* Handshake, RFC 6455 section 1.3: the server hashes the client's key with this fixed string. */
    static constexpr char Guid[]{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};
    static constexpr char KeyHeader[]{"sec-websocket-key:"};

    /* Frame, RFC 6455 section 5.2 */
    static constexpr uint8_t FinalTextFrame{0x81u};
    static constexpr uint8_t FinalBinaryFrame{0x82u};
    static constexpr uint8_t OpcodeMask{0x0Fu};
    static constexpr uint8_t MaskFlag{0x80u};
    static constexpr size_t MaskLength{4u};
    static constexpr uint8_t LengthMask{0x7Fu};
    static constexpr size_t Length16Bit{126u};
    static constexpr size_t Length64Bit{127u};
    static constexpr size_t HeaderLength{2u};
    static constexpr size_t HeaderLength16Bit{4u};

    /* SHA-1, FIPS 180-4 */
    static constexpr size_t Sha1BlockLength{64u};
    static constexpr size_t Sha1LengthPosition{56u};
    static constexpr size_t Sha1StateLength{5u};
    static constexpr size_t Sha1WordsPerBlock{16u};
    static constexpr size_t Sha1Rounds{80u};

    static StdReturnType getKey(const std::string&, std::string&);
    static std::string padSha1Message(const std::string&);
    static void processSha1Block(const uint8_t*, uint32_t*);

    static constexpr uint32_t rotateLeft(uint32_t Value, unsigned Bits) {
        return (Value << Bits) | (Value >> (32u - Bits));
    }

    /* The round's function of B, C and D plus its constant, both of which change every 20 rounds. */
    static constexpr uint32_t mixSha1(size_t Round, uint32_t B, uint32_t C, uint32_t D) {
        if(Round < 20u) { return ((B & C) | ((~B) & D)) + 0x5A827999u; }
        if(Round < 40u) { return (B ^ C ^ D) + 0x6ED9EBA1u; }
        if(Round < 60u) { return ((B & C) | (B & D) | (C & D)) + 0x8F1BBCDCu; }
        return (B ^ C ^ D) + 0xCA62C1D6u;
    }

/******************************************************************************************************************************************************
 *  P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/
  public:
    // get methods
    static StdReturnType getHandshakeAnswer(const std::string&, std::string&);

    // methods
    static std::string encodeFrame(bool, const char*, size_t);
    static StdReturnType decodeFrame(std::string&, uint8_t&, std::string&);
    static std::string convertKeyToAcceptKey(const std::string&);
    static void calculateSha1(const std::string&, uint8_t*);
    static std::string convertToBase64(const uint8_t*, size_t);
};

#endif // _WEB_SOCKET_H_

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
