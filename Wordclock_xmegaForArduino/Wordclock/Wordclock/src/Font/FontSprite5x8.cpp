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
/**     \file       FontSprite5x8.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _FONT_SPRITE_5X8_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "FontSprite5x8.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const FontSprite5x8::FontTableType FontSprite5x8::FontTable PROGMEM
{
    FontTableElementType(3, {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}), // space
    FontTableElementType(1, {0b01011111, 0b00000000, 0b00000000, 0b00000000, 0b00000000}), // !
    FontTableElementType(3, {0b00000011, 0b00000000, 0b00000011, 0b00000000, 0b00000000}), // "
    FontTableElementType(5, {0b00010100, 0b00111110, 0b00010100, 0b00111110, 0b00010100}), // #
    FontTableElementType(4, {0b00100100, 0b01101010, 0b00101011, 0b00010010, 0b00000000}), // $
    FontTableElementType(5, {0b01100011, 0b00010011, 0b00001000, 0b01100100, 0b01100011}), // %
    FontTableElementType(5, {0b00110110, 0b01001001, 0b01010110, 0b00100000, 0b01010000}), // &
    FontTableElementType(1, {0b00000011, 0b00000000, 0b00000000, 0b00000000, 0b00000000}), // '
    FontTableElementType(3, {0b00011100, 0b00100010, 0b01000001, 0b00000000, 0b00000000}), // (
    FontTableElementType(3, {0b01000001, 0b00100010, 0b00011100, 0b00000000, 0b00000000}), // )
    FontTableElementType(5, {0b00101000, 0b00011000, 0b00001110, 0b00011000, 0b00101000}), // *
    FontTableElementType(5, {0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000}), // +
    FontTableElementType(2, {0b10110000, 0b01110000, 0b00000000, 0b00000000, 0b00000000}), // ,
    FontTableElementType(4, {0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000}), // -
    FontTableElementType(2, {0b01100000, 0b01100000, 0b00000000, 0b00000000, 0b00000000}), // .
    FontTableElementType(4, {0b01100000, 0b00011000, 0b00000110, 0b00000001, 0b00000000}), // /
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01000001, 0b00111110, 0b00000000}), // 0
    FontTableElementType(3, {0b01000010, 0b01111111, 0b01000000, 0b00000000, 0b00000000}), // 1
    FontTableElementType(4, {0b01100010, 0b01010001, 0b01001001, 0b01000110, 0b00000000}), // 2
    FontTableElementType(4, {0b00100010, 0b01000001, 0b01001001, 0b00110110, 0b00000000}), // 3
    FontTableElementType(4, {0b00011000, 0b00010100, 0b00010010, 0b01111111, 0b00000000}), // 4
    FontTableElementType(4, {0b00100111, 0b01000101, 0b01000101, 0b00111001, 0b00000000}), // 5
    FontTableElementType(4, {0b00111110, 0b01001001, 0b01001001, 0b00110000, 0b00000000}), // 6
    FontTableElementType(4, {0b01100001, 0b00010001, 0b00001001, 0b00000111, 0b00000000}), // 7
    FontTableElementType(4, {0b00110110, 0b01001001, 0b01001001, 0b00110110, 0b00000000}), // 8
    FontTableElementType(4, {0b00000110, 0b01001001, 0b01001001, 0b00111110, 0b00000000}), // 9
    FontTableElementType(2, {0b01010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}), // :
    FontTableElementType(2, {0b10000000, 0b01010000, 0b00000000, 0b00000000, 0b00000000}), // ;
    FontTableElementType(3, {0b00010000, 0b00101000, 0b01000100, 0b00000000, 0b00000000}), // <
    FontTableElementType(3, {0b00010100, 0b00010100, 0b00010100, 0b00000000, 0b00000000}), // =
    FontTableElementType(3, {0b01000100, 0b00101000, 0b00010000, 0b00000000, 0b00000000}), // >
    FontTableElementType(4, {0b00000010, 0b01011001, 0b00001001, 0b00000110, 0b00000000}), // ?
    FontTableElementType(5, {0b00111110, 0b01001001, 0b01010101, 0b01011101, 0b00001110}), // @
    FontTableElementType(4, {0b01111110, 0b00010001, 0b00010001, 0b01111110, 0b00000000}), // A
    FontTableElementType(4, {0b01111111, 0b01001001, 0b01001001, 0b00110110, 0b00000000}), // B
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01000001, 0b00100010, 0b00000000}), // C
    FontTableElementType(4, {0b01111111, 0b01000001, 0b01000001, 0b00111110, 0b00000000}), // D
    FontTableElementType(4, {0b01111111, 0b01001001, 0b01001001, 0b01000001, 0b00000000}), // E
    FontTableElementType(4, {0b01111111, 0b00001001, 0b00001001, 0b00000001, 0b00000000}), // F
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01001001, 0b01111010, 0b00000000}), // G
    FontTableElementType(4, {0b01111111, 0b00001000, 0b00001000, 0b01111111, 0b00000000}), // H
    FontTableElementType(3, {0b01000001, 0b01111111, 0b01000001, 0b00000000, 0b00000000}), // I
    FontTableElementType(4, {0b00110000, 0b01000000, 0b01000001, 0b00111111, 0b00000000}), // J
    FontTableElementType(4, {0b01111111, 0b00001000, 0b00010100, 0b01100011, 0b00000000}), // K
    FontTableElementType(4, {0b01111111, 0b01000000, 0b01000000, 0b01000000, 0b00000000}), // L
    FontTableElementType(5, {0b01111111, 0b00000010, 0b00001100, 0b00000010, 0b01111111}), // M
    FontTableElementType(5, {0b01111111, 0b00000100, 0b00001000, 0b00010000, 0b01111111}), // N
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01000001, 0b00111110, 0b00000000}), // O
    FontTableElementType(4, {0b01111111, 0b00001001, 0b00001001, 0b00000110, 0b00000000}), // P
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01000001, 0b10111110, 0b00000000}), // Q
    FontTableElementType(4, {0b01111111, 0b00001001, 0b00001001, 0b01110110, 0b00000000}), // R
    FontTableElementType(4, {0b01000110, 0b01001001, 0b01001001, 0b00110010, 0b00000000}), // S
    FontTableElementType(5, {0b00000001, 0b00000001, 0b01111111, 0b00000001, 0b00000001}), // T
    FontTableElementType(4, {0b00111111, 0b01000000, 0b01000000, 0b00111111, 0b00000000}), // U
    FontTableElementType(5, {0b00001111, 0b00110000, 0b01000000, 0b00110000, 0b00001111}), // V
    FontTableElementType(5, {0b00111111, 0b01000000, 0b00111000, 0b01000000, 0b00111111}), // W
    FontTableElementType(5, {0b01100011, 0b00010100, 0b00001000, 0b00010100, 0b01100011}), // X
    FontTableElementType(5, {0b00000111, 0b00001000, 0b01110000, 0b00001000, 0b00000111}), // Y
    FontTableElementType(4, {0b01100001, 0b01010001, 0b01001001, 0b01000111, 0b00000000}), // Z
    FontTableElementType(2, {0b01111111, 0b01000001, 0b00000000, 0b00000000, 0b00000000}), // [
    FontTableElementType(4, {0b00000001, 0b00000110, 0b00011000, 0b01100000, 0b00000000}), // \ backslash
    FontTableElementType(2, {0b01000001, 0b01111111, 0b00000000, 0b00000000, 0b00000000}), // ]
    FontTableElementType(3, {0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000000}), // ^
    FontTableElementType(4, {0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b00000000}), // _
    FontTableElementType(2, {0b00000001, 0b00000010, 0b00000000, 0b00000000, 0b00000000}), // `
    FontTableElementType(4, {0b00100000, 0b01010100, 0b01010100, 0b01111000, 0b00000000}), // a
    FontTableElementType(4, {0b01111111, 0b01000100, 0b01000100, 0b00111000, 0b00000000}), // b
    FontTableElementType(4, {0b00111000, 0b01000100, 0b01000100, 0b00101000, 0b00000000}), // c
    FontTableElementType(4, {0b00111000, 0b01000100, 0b01000100, 0b01111111, 0b00000000}), // d
    FontTableElementType(4, {0b00111000, 0b01010100, 0b01010100, 0b00011000, 0b00000000}), // e
    FontTableElementType(3, {0b00000100, 0b01111110, 0b00000101, 0b00000000, 0b00000000}), // f
    FontTableElementType(4, {0b10011000, 0b10100100, 0b10100100, 0b01111000, 0b00000000}), // g
    FontTableElementType(4, {0b01111111, 0b00000100, 0b00000100, 0b01111000, 0b00000000}), // h
    FontTableElementType(3, {0b01000100, 0b01111101, 0b01000000, 0b00000000, 0b00000000}), // i
    FontTableElementType(4, {0b01000000, 0b10000000, 0b10000100, 0b01111101, 0b00000000}), // j
    FontTableElementType(4, {0b01111111, 0b00010000, 0b00101000, 0b01000100, 0b00000000}), // k
    FontTableElementType(3, {0b01000001, 0b01111111, 0b01000000, 0b00000000, 0b00000000}), // l
    FontTableElementType(5, {0b01111100, 0b00000100, 0b01111100, 0b00000100, 0b01111000}), // m
    FontTableElementType(4, {0b01111100, 0b00000100, 0b00000100, 0b01111000, 0b00000000}), // n
    FontTableElementType(4, {0b00111000, 0b01000100, 0b01000100, 0b00111000, 0b00000000}), // o
    FontTableElementType(4, {0b11111100, 0b00100100, 0b00100100, 0b00011000, 0b00000000}), // p
    FontTableElementType(4, {0b00011000, 0b00100100, 0b00100100, 0b11111100, 0b00000000}), // q
    FontTableElementType(4, {0b01111100, 0b00001000, 0b00000100, 0b00000100, 0b00000000}), // r
    FontTableElementType(4, {0b01001000, 0b01010100, 0b01010100, 0b00100100, 0b00000000}), // s
    FontTableElementType(3, {0b00000100, 0b00111111, 0b01000100, 0b00000000, 0b00000000}), // t
    FontTableElementType(4, {0b00111100, 0b01000000, 0b01000000, 0b01111100, 0b00000000}), // u
    FontTableElementType(5, {0b00011100, 0b00100000, 0b01000000, 0b00100000, 0b00011100}), // v
    FontTableElementType(5, {0b00111100, 0b01000000, 0b00111100, 0b01000000, 0b00111100}), // w
    FontTableElementType(5, {0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100}), // x
    FontTableElementType(4, {0b10011100, 0b10100000, 0b10100000, 0b01111100, 0b00000000}), // y
    FontTableElementType(3, {0b01100100, 0b01010100, 0b01001100, 0b00000000, 0b00000000}), // z
    FontTableElementType(3, {0b00001000, 0b00110110, 0b01000001, 0b00000000, 0b00000000}), // {
    FontTableElementType(1, {0b01111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000}), // |
    FontTableElementType(3, {0b01000001, 0b00110110, 0b00001000, 0b00000000, 0b00000000}), // }
    FontTableElementType(4, {0b00001000, 0b00000100, 0b00001000, 0b00000100, 0b00000000}), // ~
    FontTableElementType(4, {0b01111111, 0b01000001, 0b01000001, 0b01111111, 0b00000000}), // 
    FontTableElementType(4, {0b01111100, 0b00010011, 0b00010011, 0b01111100, 0b00000000}), // �
    FontTableElementType(4, {0b00111100, 0b01000011, 0b01000011, 0b00111100, 0b00000000}), // �
    FontTableElementType(4, {0b00111110, 0b01000001, 0b01000001, 0b00111110, 0b00000000}), // �
    FontTableElementType(4, {0b00100000, 0b01010110, 0b01010110, 0b01111000, 0b00000000}), // �
    FontTableElementType(4, {0b00111000, 0b01000110, 0b01000110, 0b00111000, 0b00000000}), // �
    FontTableElementType(4, {0b00111100, 0b01000010, 0b01000010, 0b01111100, 0b00000000}), // �
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of FontSprite5x8
******************************************************************************************************************************************************/
/*! \brief          FontSprite5x8 Constructor
 *  \details        Instantiation of the FontSprite5x8 library
 *
 *  \return         -
******************************************************************************************************************************************************/
constexpr FontSprite5x8::FontSprite5x8()
{

} /* FontSprite5x8 */


/******************************************************************************************************************************************************
  Destructor of FontSprite5x8
******************************************************************************************************************************************************/
FontSprite5x8::~FontSprite5x8()
{

} /* ~FontSprite5x8 */


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 