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
/**     \file       FontTahoma10x10.cpp
 *      \brief      
 *
 *      \details    
 *                  
 *
******************************************************************************************************************************************************/
#define _FONT_TAHOMA_10X10_SOURCE_

/******************************************************************************************************************************************************
 * I N C L U D E S
******************************************************************************************************************************************************/
#include "FontTahoma10x10.h"


/******************************************************************************************************************************************************
 *  L O C A L   C O N S T A N T   M A C R O S 
******************************************************************************************************************************************************/


/******************************************************************************************************************************************************
 *  L O C A L   F U N C T I O N   M A C R O S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  L O C A L   D A T A   T Y P E S   A N D   S T R U C T U R E S
******************************************************************************************************************************************************/
const FontTahoma10x10::FontTableType FontTahoma10x10::FontTable PROGMEM
{
    FontTableElementType(6,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char
    FontTableElementType(2,  {0x0000, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0000, 0x0002, 0x0000}),      // Code for char !
    FontTableElementType(3,  {0x0000, 0x0005, 0x0005, 0x0005, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char "
    FontTableElementType(7,  {0x0000, 0x0028, 0x0028, 0x007e, 0x0014, 0x0014, 0x003f, 0x000a, 0x000a, 0x0000}),      // Code for char #
    FontTableElementType(5,  {0x0000, 0x0004, 0x001e, 0x0005, 0x0005, 0x000e, 0x0014, 0x0014, 0x000f, 0x0004}),      // Code for char $
    FontTableElementType(10, {0x0000, 0x0046, 0x0049, 0x0029, 0x0026, 0x0190, 0x0250, 0x0248, 0x0188, 0x0000}),      // Code for char %
    FontTableElementType(7,  {0x0000, 0x0006, 0x0009, 0x0009, 0x0026, 0x0029, 0x0011, 0x0031, 0x004e, 0x0000}),      // Code for char &
    FontTableElementType(1,  {0x0000, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char '
    FontTableElementType(3,  {0x0000, 0x0004, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0004}),      // Code for char (
    FontTableElementType(3,  {0x0000, 0x0001, 0x0002, 0x0002, 0x0004, 0x0004, 0x0004, 0x0002, 0x0002, 0x0001}),      // Code for char )
    FontTableElementType(5,  {0x0000, 0x0004, 0x0015, 0x000e, 0x0015, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char *
    FontTableElementType(7,  {0x0000, 0x0000, 0x0008, 0x0008, 0x0008, 0x007f, 0x0008, 0x0008, 0x0008, 0x0000}),      // Code for char +
    FontTableElementType(2,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0002, 0x0002, 0x0001}),      // Code for char ,
    FontTableElementType(3,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0007, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char -
    FontTableElementType(2,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0002, 0x0002, 0x0000}),      // Code for char .
    FontTableElementType(3,  {0x0000, 0x0004, 0x0004, 0x0004, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001}),      // Code for char /
    FontTableElementType(5,  {0x0000, 0x000e, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x000e, 0x0000}),      // Code for char 0
    FontTableElementType(4,  {0x0000, 0x0004, 0x0006, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x000e, 0x0000}),      // Code for char 1
    FontTableElementType(5,  {0x0000, 0x000e, 0x0011, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001, 0x001f, 0x0000}),      // Code for char 2
    FontTableElementType(5,  {0x0000, 0x000e, 0x0011, 0x0010, 0x000c, 0x0010, 0x0010, 0x0011, 0x000e, 0x0000}),      // Code for char 3
    FontTableElementType(5,  {0x0000, 0x0008, 0x000c, 0x000a, 0x0009, 0x001f, 0x0008, 0x0008, 0x0008, 0x0000}),      // Code for char 4
    FontTableElementType(5,  {0x0000, 0x001f, 0x0001, 0x0001, 0x000f, 0x0010, 0x0010, 0x0011, 0x000e, 0x0000}),      // Code for char 5
    FontTableElementType(5,  {0x0000, 0x000c, 0x0002, 0x0001, 0x000f, 0x0011, 0x0011, 0x0011, 0x000e, 0x0000}),      // Code for char 6
    FontTableElementType(5,  {0x0000, 0x001f, 0x0010, 0x0008, 0x0008, 0x0004, 0x0004, 0x0002, 0x0002, 0x0000}),      // Code for char 7
    FontTableElementType(5,  {0x0000, 0x000e, 0x0011, 0x0011, 0x000e, 0x0011, 0x0011, 0x0011, 0x000e, 0x0000}),      // Code for char 8
    FontTableElementType(5,  {0x0000, 0x000e, 0x0011, 0x0011, 0x0011, 0x001e, 0x0010, 0x0008, 0x0006, 0x0000}),      // Code for char 9
    FontTableElementType(2,  {0x0000, 0x0000, 0x0000, 0x0002, 0x0002, 0x0000, 0x0000, 0x0002, 0x0002, 0x0000}),      // Code for char :
    FontTableElementType(2,  {0x0000, 0x0000, 0x0002, 0x0002, 0x0000, 0x0000, 0x0002, 0x0002, 0x0002, 0x0001}),      // Code for char ;
    FontTableElementType(7,  {0x0000, 0x0000, 0x0040, 0x0030, 0x000c, 0x0002, 0x000c, 0x0030, 0x0040, 0x0000}),      // Code for char <
    FontTableElementType(7,  {0x0000, 0x0000, 0x0000, 0x0000, 0x007f, 0x0000, 0x007f, 0x0000, 0x0000, 0x0000}),      // Code for char =
    FontTableElementType(7,  {0x0000, 0x0000, 0x0002, 0x000c, 0x0030, 0x0040, 0x0030, 0x000c, 0x0002, 0x0000}),      // Code for char >
    FontTableElementType(4,  {0x0000, 0x0007, 0x0008, 0x0008, 0x0004, 0x0002, 0x0002, 0x0000, 0x0002, 0x0000}),      // Code for char ?
    FontTableElementType(9,  {0x0000, 0x007c, 0x0082, 0x0139, 0x0125, 0x0125, 0x0125, 0x00f9, 0x0002, 0x003c}),      // Code for char @
    FontTableElementType(6,  {0x0000, 0x000c, 0x000c, 0x0012, 0x0012, 0x0012, 0x003f, 0x0021, 0x0021, 0x0000}),      // Code for char A
    FontTableElementType(5,  {0x0000, 0x000f, 0x0011, 0x0011, 0x000f, 0x0011, 0x0011, 0x0011, 0x000f, 0x0000}),      // Code for char B
    FontTableElementType(6,  {0x0000, 0x003c, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x003c, 0x0000}),      // Code for char C
    FontTableElementType(6,  {0x0000, 0x000f, 0x0011, 0x0021, 0x0021, 0x0021, 0x0021, 0x0011, 0x000f, 0x0000}),      // Code for char D
    FontTableElementType(5,  {0x0000, 0x001f, 0x0001, 0x0001, 0x000f, 0x0001, 0x0001, 0x0001, 0x001f, 0x0000}),      // Code for char E
    FontTableElementType(5,  {0x0000, 0x001f, 0x0001, 0x0001, 0x001f, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char F
    FontTableElementType(6,  {0x0000, 0x003c, 0x0002, 0x0001, 0x0001, 0x0039, 0x0021, 0x0022, 0x003c, 0x0000}),      // Code for char G
    FontTableElementType(6,  {0x0000, 0x0021, 0x0021, 0x0021, 0x003f, 0x0021, 0x0021, 0x0021, 0x0021, 0x0000}),      // Code for char H
    FontTableElementType(3,  {0x0000, 0x0007, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0007, 0x0000}),      // Code for char I
    FontTableElementType(4,  {0x0000, 0x000e, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0007, 0x0000}),      // Code for char J
    FontTableElementType(5,  {0x0000, 0x0011, 0x0009, 0x0005, 0x0003, 0x0003, 0x0005, 0x0009, 0x0011, 0x0000}),      // Code for char K
    FontTableElementType(4,  {0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x000f, 0x0000}),      // Code for char L
    FontTableElementType(7,  {0x0000, 0x0063, 0x0063, 0x0055, 0x0055, 0x0049, 0x0049, 0x0041, 0x0041, 0x0000}),      // Code for char M
    FontTableElementType(6,  {0x0000, 0x0023, 0x0023, 0x0025, 0x0025, 0x0029, 0x0029, 0x0031, 0x0031, 0x0000}),      // Code for char N
    FontTableElementType(7,  {0x0000, 0x001c, 0x0022, 0x0041, 0x0041, 0x0041, 0x0041, 0x0022, 0x001c, 0x0000}),      // Code for char O
    FontTableElementType(5,  {0x0000, 0x000f, 0x0011, 0x0011, 0x0011, 0x000f, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char P
    FontTableElementType(7,  {0x0000, 0x001c, 0x0022, 0x0041, 0x0041, 0x0041, 0x0041, 0x0022, 0x001c, 0x0070}),      // Code for char Q
    FontTableElementType(6,  {0x0000, 0x000f, 0x0011, 0x0011, 0x0011, 0x000f, 0x0009, 0x0011, 0x0021, 0x0000}),      // Code for char R
    FontTableElementType(5,  {0x0000, 0x001e, 0x0001, 0x0001, 0x000e, 0x0010, 0x0010, 0x0010, 0x000f, 0x0000}),      // Code for char S
    FontTableElementType(7,  {0x0000, 0x007f, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0008, 0x0000}),      // Code for char T
    FontTableElementType(6,  {0x0000, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x001e, 0x0000}),      // Code for char U
    FontTableElementType(5,  {0x0000, 0x0011, 0x0011, 0x0011, 0x000a, 0x000a, 0x000a, 0x0004, 0x0004, 0x0000}),      // Code for char V
    FontTableElementType(9,  {0x0000, 0x0111, 0x0111, 0x0111, 0x00aa, 0x00aa, 0x00aa, 0x0044, 0x0044, 0x0000}),      // Code for char W
    FontTableElementType(5,  {0x0000, 0x0011, 0x0011, 0x000a, 0x0004, 0x0004, 0x000a, 0x0011, 0x0011, 0x0000}),      // Code for char X
    FontTableElementType(5,  {0x0000, 0x0011, 0x0011, 0x000a, 0x000a, 0x0004, 0x0004, 0x0004, 0x0004, 0x0000}),      // Code for char Y
    FontTableElementType(5,  {0x0000, 0x001f, 0x0010, 0x0008, 0x0004, 0x0004, 0x0002, 0x0001, 0x001f, 0x0000}),      // Code for char Z
    FontTableElementType(3,  {0x0000, 0x0007, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0007}),      // Code for char [
    FontTableElementType(3,  {0x0000, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0004, 0x0004, 0x0004}),      // Code for char Back
    FontTableElementType(3,  {0x0000, 0x0007, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0004, 0x0007}),      // Code for char ]
    FontTableElementType(7,  {0x0000, 0x0008, 0x0014, 0x0022, 0x0041, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char ^
    FontTableElementType(6,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x003f}),      // Code for char _
    FontTableElementType(3,  {0x0000, 0x0002, 0x0004, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}),      // Code for char `
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x000e, 0x0010, 0x001e, 0x0011, 0x0011, 0x001e, 0x0000}),      // Code for char a
    FontTableElementType(5,  {0x0000, 0x0001, 0x0001, 0x0001, 0x000f, 0x0011, 0x0011, 0x0011, 0x000f, 0x0000}),      // Code for char b
    FontTableElementType(4,  {0x0000, 0x0000, 0x0000, 0x000e, 0x0001, 0x0001, 0x0001, 0x0001, 0x000e, 0x0000}),      // Code for char c
    FontTableElementType(5,  {0x0000, 0x0010, 0x0010, 0x0010, 0x001e, 0x0011, 0x0011, 0x0011, 0x001e, 0x0000}),      // Code for char d
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x000e, 0x0011, 0x001f, 0x0001, 0x0011, 0x000e, 0x0000}),      // Code for char e
    FontTableElementType(3,  {0x0000, 0x0006, 0x0001, 0x0001, 0x0007, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char f
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x001e, 0x0011, 0x0011, 0x001e, 0x0010, 0x0010, 0x000e}),      // Code for char g
    FontTableElementType(5,  {0x0000, 0x0001, 0x0001, 0x0001, 0x000f, 0x0011, 0x0011, 0x0011, 0x0011, 0x0000}),      // Code for char h
    FontTableElementType(1,  {0x0000, 0x0001, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char i
    FontTableElementType(2,  {0x0000, 0x0002, 0x0000, 0x0003, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001}),      // Code for char j
    FontTableElementType(5,  {0x0000, 0x0001, 0x0001, 0x0009, 0x0005, 0x0003, 0x0005, 0x0009, 0x0011, 0x0000}),      // Code for char k
    FontTableElementType(1,  {0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char l
    FontTableElementType(7,  {0x0000, 0x0000, 0x0000, 0x0037, 0x0049, 0x0049, 0x0049, 0x0049, 0x0049, 0x0000}),      // Code for char m
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x000f, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0000}),      // Code for char n
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x000e, 0x0011, 0x0011, 0x0011, 0x0011, 0x000e, 0x0000}),      // Code for char o
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x000f, 0x0011, 0x0011, 0x0011, 0x000f, 0x0001, 0x0001}),      // Code for char p
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x001e, 0x0011, 0x0011, 0x0011, 0x001e, 0x0010, 0x0010}),      // Code for char q
    FontTableElementType(3,  {0x0000, 0x0000, 0x0000, 0x0005, 0x0003, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000}),      // Code for char r
    FontTableElementType(4,  {0x0000, 0x0000, 0x0000, 0x000e, 0x0001, 0x0003, 0x000c, 0x0008, 0x0007, 0x0000}),      // Code for char s
    FontTableElementType(3,  {0x0000, 0x0001, 0x0001, 0x0007, 0x0001, 0x0001, 0x0001, 0x0001, 0x0006, 0x0000}),      // Code for char t
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x001e, 0x0000}),      // Code for char u
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x0011, 0x0011, 0x000a, 0x000a, 0x0004, 0x0004, 0x0000}),      // Code for char v
    FontTableElementType(7,  {0x0000, 0x0000, 0x0000, 0x0049, 0x0049, 0x0055, 0x0055, 0x0022, 0x0022, 0x0000}),      // Code for char w
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x0011, 0x000a, 0x0004, 0x0004, 0x000a, 0x0011, 0x0000}),      // Code for char x
    FontTableElementType(5,  {0x0000, 0x0000, 0x0000, 0x0011, 0x0011, 0x000a, 0x000a, 0x0004, 0x0004, 0x0002}),      // Code for char y
    FontTableElementType(4,  {0x0000, 0x0000, 0x0000, 0x000f, 0x0008, 0x0004, 0x0002, 0x0001, 0x000f, 0x0000}),      // Code for char z
    FontTableElementType(4,  {0x0000, 0x0008, 0x0004, 0x0004, 0x0004, 0x0003, 0x0004, 0x0004, 0x0004, 0x0008}),      // Code for char {
    FontTableElementType(2,  {0x0000, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002}),      // Code for char |
    FontTableElementType(4,  {0x0000, 0x0001, 0x0002, 0x0002, 0x0002, 0x000c, 0x0002, 0x0002, 0x0002, 0x0001}),      // Code for char }
    FontTableElementType(7,  {0x0000, 0x0000, 0x0000, 0x0000, 0x0046, 0x0049, 0x0031, 0x0000, 0x0000, 0x0000}),      // Code for char ~
    FontTableElementType(3,  {0x0000, 0x0007, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0007, 0x0000}),      // Code for char 
    FontTableElementType(6,  {0x0012, 0x0000, 0x000c, 0x000c, 0x0012, 0x0012, 0x0012, 0x003f, 0x0021, 0x0021}),      // Code for char �
    FontTableElementType(7,  {0x0014, 0x0000, 0x001c, 0x0022, 0x0041, 0x0041, 0x0041, 0x0041, 0x0022, 0x001c}),      // Code for char �
    FontTableElementType(6,  {0x0012, 0x0000, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x0021, 0x001e}),      // Code for char �
    FontTableElementType(5,  {0x0000, 0x0000, 0x000a, 0x0000, 0x000e, 0x0010, 0x001e, 0x0011, 0x0011, 0x001e}),      // Code for char �
    FontTableElementType(5,  {0x0000, 0x0000, 0x000a, 0x0000, 0x000e, 0x0011, 0x0011, 0x0011, 0x0011, 0x000e}),      // Code for char �
    FontTableElementType(5,  {0x0000, 0x0000, 0x000a, 0x0000, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x001e}),      // Code for char �
};


/******************************************************************************************************************************************************
 * P U B L I C   F U N C T I O N S
******************************************************************************************************************************************************/

/******************************************************************************************************************************************************
  Constructor of FontTahoma10x10
******************************************************************************************************************************************************/
/*! \brief          FontTahoma10x10 Constructor
 *  \details        Instantiation of the FontTahoma10x10 library
 *
 *  \return         -
******************************************************************************************************************************************************/
constexpr FontTahoma10x10::FontTahoma10x10()
{

} /* FontTahoma10x10 */


/******************************************************************************************************************************************************
  Destructor of FontTahoma10x10
******************************************************************************************************************************************************/
FontTahoma10x10::~FontTahoma10x10()
{

} /* ~FontTahoma10x10 */



/******************************************************************************************************************************************************
 * P R I V A T E   F U N C T I O N S
******************************************************************************************************************************************************/



/******************************************************************************************************************************************************
 *  E N D   O F   F I L E
******************************************************************************************************************************************************/
 