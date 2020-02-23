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
/**     \file       FontLucidaSans9x10.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _FONT_LUCIDA_SANS_9X10_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "FontLucidaSans9x10.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const FontLucidaSans9x10::FontTableType FontLucidaSans9x10::FontTable PROGMEM
{
    FontTableElementType(5, {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char
    FontTableElementType(2, {0x0000, 0x017e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char !
    FontTableElementType(3, {0x000e, 0x0000, 0x000e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char "
    FontTableElementType(7, {0x0040, 0x01c8, 0x0078, 0x01ce, 0x0078, 0x004e, 0x0008, 0x0000, 0x0000}),   // Code for char #
    FontTableElementType(6, {0x0000, 0x0098, 0x0124, 0x03fe, 0x0124, 0x00c8, 0x0000, 0x0000, 0x0000}),   // Code for char $
    FontTableElementType(7, {0x011e, 0x0092, 0x005e, 0x0030, 0x01e8, 0x0124, 0x01e2, 0x0000, 0x0000}),   // Code for char %
    FontTableElementType(7, {0x0000, 0x00e0, 0x011c, 0x0132, 0x0152, 0x018c, 0x0160, 0x0000, 0x0000}),   // Code for char &
    FontTableElementType(2, {0x0000, 0x000e, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char '
    FontTableElementType(4, {0x0000, 0x00f0, 0x030c, 0x0402, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char (
    FontTableElementType(3, {0x0402, 0x030c, 0x00f0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char )
    FontTableElementType(5, {0x0004, 0x001c, 0x0006, 0x001c, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char *
    FontTableElementType(8, {0x0000, 0x0020, 0x0020, 0x0020, 0x01fc, 0x0020, 0x0020, 0x0020, 0x0000}),   // Code for char +
    FontTableElementType(2, {0x0400, 0x0300, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char ,
    FontTableElementType(5, {0x0000, 0x0020, 0x0020, 0x0020, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char -
    FontTableElementType(2, {0x0000, 0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char .
    FontTableElementType(5, {0x0000, 0x0600, 0x01c0, 0x0038, 0x0006, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char /
    FontTableElementType(6, {0x0000, 0x00fc, 0x0102, 0x0102, 0x0102, 0x00fc, 0x0000, 0x0000, 0x0000}),   // Code for char 0
    FontTableElementType(6, {0x0000, 0x0104, 0x0102, 0x01fe, 0x0100, 0x0100, 0x0000, 0x0000, 0x0000}),   // Code for char 1
    FontTableElementType(6, {0x0000, 0x0182, 0x0142, 0x0122, 0x0112, 0x010c, 0x0000, 0x0000, 0x0000}),   // Code for char 2
    FontTableElementType(6, {0x0000, 0x0102, 0x0112, 0x0112, 0x0112, 0x00ec, 0x0000, 0x0000, 0x0000}),   // Code for char 3
    FontTableElementType(6, {0x0000, 0x0060, 0x0058, 0x0044, 0x01fe, 0x0040, 0x0000, 0x0000, 0x0000}),   // Code for char 4
    FontTableElementType(6, {0x0000, 0x011e, 0x0112, 0x0112, 0x0112, 0x00e2, 0x0000, 0x0000, 0x0000}),   // Code for char 5
    FontTableElementType(6, {0x0000, 0x00f8, 0x0124, 0x0112, 0x0112, 0x00e2, 0x0000, 0x0000, 0x0000}),   // Code for char 6
    FontTableElementType(6, {0x0000, 0x0002, 0x0182, 0x0062, 0x001a, 0x0006, 0x0000, 0x0000, 0x0000}),   // Code for char 7
    FontTableElementType(6, {0x0000, 0x00ec, 0x0112, 0x0112, 0x0112, 0x00ec, 0x0000, 0x0000, 0x0000}),   // Code for char 8
    FontTableElementType(6, {0x0000, 0x011c, 0x0122, 0x0122, 0x0092, 0x007c, 0x0000, 0x0000, 0x0000}),   // Code for char 9
    FontTableElementType(2, {0x0000, 0x0108, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char :
    FontTableElementType(2, {0x0400, 0x0308, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char ;
    FontTableElementType(8, {0x0000, 0x0020, 0x0020, 0x0050, 0x0050, 0x0088, 0x0088, 0x0104, 0x0000}),   // Code for char <
    FontTableElementType(8, {0x0000, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0000}),   // Code for char =
    FontTableElementType(8, {0x0000, 0x0104, 0x0088, 0x0088, 0x0050, 0x0050, 0x0020, 0x0020, 0x0000}),   // Code for char >
    FontTableElementType(4, {0x0002, 0x0162, 0x0012, 0x000c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char ?
    FontTableElementType(9, {0x0000, 0x00f0, 0x0108, 0x0174, 0x014a, 0x017a, 0x004a, 0x0044, 0x0038}),   // Code for char @
    FontTableElementType(7, {0x0100, 0x00e0, 0x005c, 0x0042, 0x005c, 0x00e0, 0x0100, 0x0000, 0x0000}),   // Code for char A
    FontTableElementType(5, {0x0000, 0x01fe, 0x0112, 0x0112, 0x00ec, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char B
    FontTableElementType(7, {0x0000, 0x0078, 0x0084, 0x0102, 0x0102, 0x0102, 0x0102, 0x0000, 0x0000}),   // Code for char C
    FontTableElementType(7, {0x0000, 0x01fe, 0x0102, 0x0102, 0x0102, 0x0084, 0x0078, 0x0000, 0x0000}),   // Code for char D
    FontTableElementType(6, {0x0000, 0x01fe, 0x0112, 0x0112, 0x0112, 0x0102, 0x0000, 0x0000, 0x0000}),   // Code for char E
    FontTableElementType(5, {0x0000, 0x01fe, 0x0022, 0x0022, 0x0022, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char F
    FontTableElementType(7, {0x0000, 0x0078, 0x0084, 0x0102, 0x0102, 0x0102, 0x01c2, 0x0000, 0x0000}),   // Code for char G
    FontTableElementType(7, {0x0000, 0x01fe, 0x0010, 0x0010, 0x0010, 0x0010, 0x01fe, 0x0000, 0x0000}),   // Code for char H
    FontTableElementType(2, {0x0000, 0x01fe, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char I
    FontTableElementType(2, {0x0400, 0x03fe, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char J
    FontTableElementType(7, {0x0000, 0x01fe, 0x0010, 0x0028, 0x0044, 0x0082, 0x0100, 0x0000, 0x0000}),   // Code for char K
    FontTableElementType(6, {0x0000, 0x01fe, 0x0100, 0x0100, 0x0100, 0x0100, 0x0000, 0x0000, 0x0000}),   // Code for char L
    FontTableElementType(8, {0x0000, 0x01fe, 0x000c, 0x0070, 0x0080, 0x0070, 0x000c, 0x01fe, 0x0000}),   // Code for char M
    FontTableElementType(7, {0x0000, 0x01fe, 0x0004, 0x0018, 0x0060, 0x0080, 0x01fe, 0x0000, 0x0000}),   // Code for char N
    FontTableElementType(8, {0x0000, 0x0078, 0x0084, 0x0102, 0x0102, 0x0102, 0x0084, 0x0078, 0x0000}),   // Code for char O
    FontTableElementType(6, {0x0000, 0x01fe, 0x0022, 0x0022, 0x0022, 0x001c, 0x0000, 0x0000, 0x0000}),   // Code for char P
    FontTableElementType(9, {0x0000, 0x0078, 0x0084, 0x0102, 0x0102, 0x0102, 0x0284, 0x0478, 0x0400}),   // Code for char Q
    FontTableElementType(7, {0x0000, 0x01fe, 0x0022, 0x0022, 0x0062, 0x009c, 0x0100, 0x0000, 0x0000}),   // Code for char R
    FontTableElementType(5, {0x0000, 0x010c, 0x0112, 0x0122, 0x00c2, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char S
    FontTableElementType(7, {0x0002, 0x0002, 0x0002, 0x01fe, 0x0002, 0x0002, 0x0002, 0x0000, 0x0000}),   // Code for char T
    FontTableElementType(7, {0x0000, 0x00fe, 0x0100, 0x0100, 0x0100, 0x0100, 0x00fe, 0x0000, 0x0000}),   // Code for char U
    FontTableElementType(7, {0x0002, 0x001c, 0x00e0, 0x0100, 0x00e0, 0x001c, 0x0002, 0x0000, 0x0000}),   // Code for char V
    FontTableElementType(9, {0x0006, 0x0078, 0x0180, 0x0078, 0x0006, 0x0078, 0x0180, 0x0078, 0x0006}),   // Code for char W
    FontTableElementType(7, {0x0102, 0x0084, 0x0048, 0x0030, 0x0048, 0x0084, 0x0102, 0x0000, 0x0000}),   // Code for char X
    FontTableElementType(7, {0x0002, 0x0004, 0x0008, 0x01f0, 0x0008, 0x0004, 0x0002, 0x0000, 0x0000}),   // Code for char Y
    FontTableElementType(6, {0x0000, 0x0182, 0x0142, 0x0132, 0x010a, 0x0106, 0x0000, 0x0000, 0x0000}),   // Code for char Z
    FontTableElementType(3, {0x0000, 0x07fe, 0x0402, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char [
    FontTableElementType(5, {0x0000, 0x0006, 0x0038, 0x01c0, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char BackSlash
    FontTableElementType(3, {0x0000, 0x0402, 0x07fe, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char ]
    FontTableElementType(6, {0x0000, 0x0060, 0x0018, 0x0006, 0x0018, 0x0060, 0x0000, 0x0000, 0x0000}),   // Code for char ^
    FontTableElementType(6, {0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0000, 0x0000, 0x0000}),   // Code for char _
    FontTableElementType(5, {0x0000, 0x0000, 0x0000, 0x0002, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char `
    FontTableElementType(6, {0x0000, 0x00c8, 0x0128, 0x0128, 0x01f0, 0x0100, 0x0000, 0x0000, 0x0000}),   // Code for char a
    FontTableElementType(6, {0x0000, 0x01fe, 0x0110, 0x0108, 0x0108, 0x00f0, 0x0000, 0x0000, 0x0000}),   // Code for char b
    FontTableElementType(5, {0x0000, 0x00f0, 0x0108, 0x0108, 0x0108, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char c
    FontTableElementType(6, {0x0000, 0x00f0, 0x0108, 0x0108, 0x0088, 0x01fe, 0x0000, 0x0000, 0x0000}),   // Code for char d
    FontTableElementType(5, {0x0000, 0x00f0, 0x0128, 0x0128, 0x0130, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char e
    FontTableElementType(4, {0x0008, 0x01fc, 0x000a, 0x0002, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char f
    FontTableElementType(6, {0x0000, 0x04f0, 0x0508, 0x0508, 0x0488, 0x03f8, 0x0000, 0x0000, 0x0000}),   // Code for char g
    FontTableElementType(6, {0x0000, 0x01fe, 0x0010, 0x0008, 0x0008, 0x01f0, 0x0000, 0x0000, 0x0000}),   // Code for char h
    FontTableElementType(2, {0x0000, 0x01fa, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char i
    FontTableElementType(2, {0x0400, 0x03fa, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char j
    FontTableElementType(6, {0x0000, 0x01fe, 0x0020, 0x0060, 0x0090, 0x0108, 0x0000, 0x0000, 0x0000}),   // Code for char k
    FontTableElementType(2, {0x0000, 0x01fe, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char l
    FontTableElementType(8, {0x0000, 0x01f8, 0x0010, 0x0008, 0x01f8, 0x0010, 0x0008, 0x01f8, 0x0000}),   // Code for char m
    FontTableElementType(6, {0x0000, 0x01f8, 0x0010, 0x0008, 0x0008, 0x01f0, 0x0000, 0x0000, 0x0000}),   // Code for char n
    FontTableElementType(6, {0x0000, 0x00f0, 0x0108, 0x0108, 0x0108, 0x00f0, 0x0000, 0x0000, 0x0000}),   // Code for char o
    FontTableElementType(6, {0x0000, 0x07f8, 0x0110, 0x0108, 0x0108, 0x00f0, 0x0000, 0x0000, 0x0000}),   // Code for char p
    FontTableElementType(6, {0x0000, 0x00f0, 0x0108, 0x0108, 0x0088, 0x07f8, 0x0000, 0x0000, 0x0000}),   // Code for char q
    FontTableElementType(5, {0x0000, 0x01f8, 0x0010, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char r
    FontTableElementType(5, {0x0000, 0x0110, 0x0128, 0x0148, 0x0088, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char s
    FontTableElementType(3, {0x0008, 0x00fc, 0x0108, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char t
    FontTableElementType(6, {0x0000, 0x00f8, 0x0100, 0x0100, 0x0080, 0x01f8, 0x0000, 0x0000, 0x0000}),   // Code for char u
    FontTableElementType(5, {0x0018, 0x00e0, 0x0100, 0x00e0, 0x0018, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char v
    FontTableElementType(9, {0x0018, 0x00e0, 0x0100, 0x00f0, 0x0008, 0x00f0, 0x0100, 0x00e0, 0x0018}),   // Code for char w
    FontTableElementType(6, {0x0000, 0x0108, 0x0090, 0x0060, 0x0090, 0x0108, 0x0000, 0x0000, 0x0000}),   // Code for char x
    FontTableElementType(5, {0x0018, 0x0460, 0x0380, 0x0060, 0x0018, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char y
    FontTableElementType(5, {0x0000, 0x0188, 0x0148, 0x0128, 0x0118, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char z
    FontTableElementType(3, {0x0020, 0x03dc, 0x0402, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char {
    FontTableElementType(2, {0x0000, 0x07fe, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char |
    FontTableElementType(4, {0x0000, 0x0402, 0x03dc, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char }
    FontTableElementType(6, {0x0000, 0x0040, 0x0020, 0x0020, 0x0040, 0x0020, 0x0000, 0x0000, 0x0000}),   // Code for char ~
    FontTableElementType(2, {0x00fc, 0x00fc, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),   // Code for char 
    FontTableElementType(7, {0x0200, 0x01c0, 0x00b9, 0x0084, 0x00b9, 0x01c0, 0x0200, 0x0000, 0x0000}),   // Code for char �
    FontTableElementType(8, {0x0000, 0x00f0, 0x0108, 0x0205, 0x0204, 0x0205, 0x0108, 0x00f0, 0x0000}),   // Code for char �
    FontTableElementType(7, {0x0000, 0x01fc, 0x0200, 0x0201, 0x0200, 0x0201, 0x01fc, 0x0000, 0x0000}),   // Code for char �
    FontTableElementType(6, {0x0000, 0x0190, 0x0254, 0x0250, 0x03e4, 0x0200, 0x0000, 0x0000, 0x0000}),   // Code for char �
    FontTableElementType(6, {0x0000, 0x01e0, 0x0214, 0x0210, 0x0214, 0x01e0, 0x0000, 0x0000, 0x0000}),   // Code for char �
    FontTableElementType(6, {0x0000, 0x01f0, 0x0204, 0x0200, 0x0104, 0x03f0, 0x0000, 0x0000, 0x0000}),   // Code for char �
};

/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 