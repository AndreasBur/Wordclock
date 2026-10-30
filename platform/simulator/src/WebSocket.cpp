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
/**     \file       WebSocket.cpp
 *      \brief      The web socket protocol, as far as the two pages need it
 *
 *      \details    SHA-1 and base64 are here for one reason only: RFC 6455 makes the handshake
 *                  answer the base64 of the SHA-1 of the client's key and a fixed string. There
 *                  is nothing to choose about either, so they are written out rather than pulled
 *                  in - a dependency for eighty lines of arithmetic would be a worse trade in a
 *                  build that otherwise needs only wxWidgets.
 *
 *                  No other use, and in particular not a security one: this is a checksum a
 *                  browser demands, not a secret.
 *
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "sim/WebSocket.h"

#include <cctype>

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getHandshakeAnswer()
******************************************************************************************************************************************************/
/*! \brief          Answers a browser's request to open a web socket
 *  \details        The handshake is the whole of what makes a socket out of a request: the 101
 *                  response, with the accept key the browser checks.
 *
 *  \param[in]      Request         the whole HTTP request, headers included
 *  \param[out]     Answer          the response to send back
 *  \return         E_OK
 *                  E_NOT_OK        the request has no Sec-WebSocket-Key
******************************************************************************************************************************************************/
StdReturnType WebSocket::getHandshakeAnswer(const std::string& Request, std::string& Answer)
{
    std::string Key;

    if(getKey(Request, Key) != E_OK) { return E_NOT_OK; }

    Answer = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
             "Sec-WebSocket-Accept: " + convertKeyToAcceptKey(Key) + "\r\n\r\n";
    return E_OK;
} /* getHandshakeAnswer */


/******************************************************************************************************************************************************
  encodeFrame()
******************************************************************************************************************************************************/
/*! \brief          Wraps a payload into a frame
 *  \details        A final, unmasked frame, as a server sends it. Only the two lengths a clock
 *                  ever sends are encoded: a line is well under 126 bytes and a display frame
 *                  is 330, so the 64-bit form is unreachable and writing it would be code no
 *                  run ever takes.
 *
 *  \param[in]      IsBinary        a binary frame rather than a text one
 *  \param[in]      Payload         the bytes to send
 *  \param[in]      Length          the number of bytes
 *  \return         the frame, header and payload
******************************************************************************************************************************************************/
std::string WebSocket::encodeFrame(bool IsBinary, const char* Payload, size_t Length)
{
    std::string Frame;

    Frame.push_back(static_cast<char>(IsBinary ? FinalBinaryFrame : FinalTextFrame));
    if(Length < Length16Bit) {
        Frame.push_back(static_cast<char>(Length));
    } else {
        Frame.push_back(static_cast<char>(Length16Bit));
        Frame.push_back(static_cast<char>((Length >> 8) & 0xFFu));
        Frame.push_back(static_cast<char>(Length & 0xFFu));
    }
    Frame.append(Payload, Length);
    return Frame;
} /* encodeFrame */


/******************************************************************************************************************************************************
  decodeFrame()
******************************************************************************************************************************************************/
/*! \brief          Takes the first frame out of what has arrived
 *  \details        What a browser sends is always masked, which is the one part of the format
 *                  this has to undo. A frame that has not fully arrived stays in the buffer for
 *                  the next call.
 *
 *  \param[in,out]  Buffer          what has arrived on the connection and is not yet decoded
 *  \param[out]     Opcode          the opcode of the frame
 *  \param[out]     Payload         the payload of the frame, unmasked
 *  \return         E_OK
 *                  E_NOT_OK        no complete frame in the buffer
******************************************************************************************************************************************************/
StdReturnType WebSocket::decodeFrame(std::string& Buffer, uint8_t& Opcode, std::string& Payload)
{
    if(Buffer.size() < HeaderLength) { return E_NOT_OK; }

    const uint8_t First = static_cast<uint8_t>(Buffer[0]);
    const uint8_t Second = static_cast<uint8_t>(Buffer[1]);
    const size_t Mask = ((Second & MaskFlag) != 0u) ? MaskLength : 0u;
    size_t Length = Second & LengthMask;
    size_t Offset = HeaderLength;

    if(Length == Length64Bit) {
        /* A command is never this long, and a browser that says so is not one this serves.
           Dropping what arrived is better than trying to read it - and better than keeping
           it, which would leave every later frame stuck behind this one. */
        Buffer.clear();
        return E_NOT_OK;
    }
    if(Length == Length16Bit) {
        if(Buffer.size() < HeaderLength16Bit) { return E_NOT_OK; }
        Length = (static_cast<size_t>(static_cast<uint8_t>(Buffer[2])) << 8) | static_cast<uint8_t>(Buffer[3]);
        Offset = HeaderLength16Bit;
    }
    if(Buffer.size() < (Offset + Mask + Length)) { return E_NOT_OK; }

    Payload.assign(Buffer, Offset + Mask, Length);
    if(Mask > 0u) {
        for(size_t Index = 0u; Index < Length; Index++) {
            Payload[Index] = static_cast<char>(Payload[Index] ^ Buffer[Offset + (Index % Mask)]);
        }
    }

    Opcode = First & OpcodeMask;
    Buffer.erase(0u, Offset + Mask + Length);
    return E_OK;
} /* decodeFrame */


/******************************************************************************************************************************************************
  convertKeyToAcceptKey()
******************************************************************************************************************************************************/
/*! \brief          Converts the browser's key into the key it expects back
 *
 *  \param[in]      Key             the value of Sec-WebSocket-Key
 *  \return         the value of Sec-WebSocket-Accept: the base64 of the SHA-1 of the key and
 *                  the fixed GUID of RFC 6455
******************************************************************************************************************************************************/
std::string WebSocket::convertKeyToAcceptKey(const std::string& Key)
{
    uint8_t Digest[Sha1Length];

    calculateSha1(Key + Guid, Digest);
    return convertToBase64(Digest, sizeof(Digest));
} /* convertKeyToAcceptKey */


/******************************************************************************************************************************************************
  calculateSha1()
******************************************************************************************************************************************************/
/*! \brief          Calculates the SHA-1 of a message
 *
 *  \param[in]      Message         the bytes to hash
 *  \param[out]     Digest          the hash, Sha1Length bytes
 *  \return         -
******************************************************************************************************************************************************/
void WebSocket::calculateSha1(const std::string& Message, uint8_t* Digest)
{
    uint32_t State[Sha1StateLength]{0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    const std::string Padded = padSha1Message(Message);

    for(size_t Offset = 0u; Offset < Padded.size(); Offset += Sha1BlockLength) {
        processSha1Block(reinterpret_cast<const uint8_t*>(&Padded[Offset]), State);
    }
    for(size_t Word = 0u; Word < Sha1StateLength; Word++) {
        for(size_t Byte = 0u; Byte < 4u; Byte++) {
            Digest[(Word * 4u) + Byte] = static_cast<uint8_t>(State[Word] >> (24u - (8u * Byte)));
        }
    }
} /* calculateSha1 */


/******************************************************************************************************************************************************
  convertToBase64()
******************************************************************************************************************************************************/
/*! \brief          Converts bytes to base64
 *  \details        Every three bytes become four characters, and the last group is padded
 *                  with '='.
 *
 *  \param[in]      Bytes           the bytes to convert
 *  \param[in]      Length          the number of bytes
 *  \return         the base64 text
******************************************************************************************************************************************************/
std::string WebSocket::convertToBase64(const uint8_t* Bytes, size_t Length)
{
    static constexpr char Alphabet[]{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    std::string Encoded;

    for(size_t Offset = 0u; Offset < Length; Offset += 3u) {
        const size_t Remaining = Length - Offset;
        uint32_t Group = static_cast<uint32_t>(Bytes[Offset]) << 16;

        if(Remaining > 1u) { Group |= static_cast<uint32_t>(Bytes[Offset + 1u]) << 8; }
        if(Remaining > 2u) { Group |= static_cast<uint32_t>(Bytes[Offset + 2u]); }

        Encoded.push_back(Alphabet[(Group >> 18) & 0x3Fu]);
        Encoded.push_back(Alphabet[(Group >> 12) & 0x3Fu]);
        Encoded.push_back((Remaining > 1u) ? Alphabet[(Group >> 6) & 0x3Fu] : '=');
        Encoded.push_back((Remaining > 2u) ? Alphabet[Group & 0x3Fu] : '=');
    }
    return Encoded;
} /* convertToBase64 */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  getKey()
******************************************************************************************************************************************************/
/*! \brief          Finds the browser's key in a request
 *  \details        Searched in a lowered copy, because header names are case-insensitive, and
 *                  returned from the request itself, because the key is not.
 *
 *  \param[in]      Request         the whole HTTP request, headers included
 *  \param[out]     Key             the value of Sec-WebSocket-Key
 *  \return         E_OK
 *                  E_NOT_OK        the request has no such header
******************************************************************************************************************************************************/
StdReturnType WebSocket::getKey(const std::string& Request, std::string& Key)
{
    std::string Lowered(Request);
    for(char& Character : Lowered) { Character = static_cast<char>(tolower(static_cast<unsigned char>(Character))); }

    const size_t Found = Lowered.find(KeyHeader);
    if(Found == std::string::npos) { return E_NOT_OK; }

    size_t Start = Found + sizeof(KeyHeader) - 1u;
    while((Start < Request.size()) && (Request[Start] == ' ')) { Start++; }
    const size_t End = Request.find("\r\n", Start);
    if(End == std::string::npos) { return E_NOT_OK; }

    Key = Request.substr(Start, End - Start);
    return E_OK;
} /* getKey */


/******************************************************************************************************************************************************
  padSha1Message()
******************************************************************************************************************************************************/
/*! \brief          Pads a message to whole blocks
 *  \details        The message, its terminator bit and its length in bits, which is what SHA-1
 *                  hashes.
 *
 *  \param[in]      Message         the bytes to hash
 *  \return         the padded message, a multiple of Sha1BlockLength long
******************************************************************************************************************************************************/
std::string WebSocket::padSha1Message(const std::string& Message)
{
    const uint64_t Bits = static_cast<uint64_t>(Message.size()) * 8u;
    std::string Padded(Message);

    Padded.push_back(static_cast<char>(0x80));
    while((Padded.size() % Sha1BlockLength) != Sha1LengthPosition) { Padded.push_back('\0'); }
    for(int Shift = 56; Shift >= 0; Shift -= 8) {
        Padded.push_back(static_cast<char>((Bits >> Shift) & 0xFFu));
    }
    return Padded;
} /* padSha1Message */


/******************************************************************************************************************************************************
  processSha1Block()
******************************************************************************************************************************************************/
/*! \brief          Mixes one block into the state
 *  \details        Expands the block into the message schedule of 80 words and runs the 80
 *                  rounds over it.
 *
 *  \param[in]      Block           Sha1BlockLength bytes of the padded message
 *  \param[in,out]  State           the five words of the hash so far
 *  \return         -
******************************************************************************************************************************************************/
void WebSocket::processSha1Block(const uint8_t* Block, uint32_t* State)
{
    uint32_t Word[Sha1Rounds];
    uint32_t A{State[0]}, B{State[1]}, C{State[2]}, D{State[3]}, E{State[4]};

    for(size_t Index = 0u; Index < Sha1WordsPerBlock; Index++) {
        const uint8_t* Chunk = &Block[Index * 4u];
        Word[Index] = (static_cast<uint32_t>(Chunk[0]) << 24) | (static_cast<uint32_t>(Chunk[1]) << 16)
                    | (static_cast<uint32_t>(Chunk[2]) << 8) | static_cast<uint32_t>(Chunk[3]);
    }
    for(size_t Index = Sha1WordsPerBlock; Index < Sha1Rounds; Index++) {
        Word[Index] = rotateLeft(Word[Index - 3u] ^ Word[Index - 8u] ^ Word[Index - 14u] ^ Word[Index - 16u], 1u);
    }
    for(size_t Round = 0u; Round < Sha1Rounds; Round++) {
        const uint32_t Next = rotateLeft(A, 5u) + mixSha1(Round, B, C, D) + E + Word[Round];
        E = D;
        D = C;
        C = rotateLeft(B, 30u);
        B = A;
        A = Next;
    }

    State[0] += A;
    State[1] += B;
    State[2] += C;
    State[3] += D;
    State[4] += E;
} /* processSha1Block */

/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
