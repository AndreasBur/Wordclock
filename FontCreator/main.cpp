#include <iostream>
#include <string.h>
#include "StandardTypes.h"

using namespace std;

#define DECODE                              STD_ON

#define NUMBER_OF_CHARS                     96
//#define NUMBER_OF_CHARS                     224

#define NUMBER_OF_COLUMNS                   19

#define FONT_WIDTH                          9
#define FONT_HEIGHT                         11

#define NUMBER_OF_SEGMENTS                  (NUMBER_OF_COLUMNS - 1)
#define NUMBER_OF_LOWER_SEGMENTS            (NUMBER_OF_SEGMENTS / 2)


const unsigned short Font[][NUMBER_OF_COLUMNS] =
{
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char
        0x02, 0x00, 0x00, 0x7E, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char !
        0x03, 0x0E, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char "
        0x07, 0x40, 0x00, 0xC8, 0x01, 0x78, 0x00, 0xCE, 0x01, 0x78, 0x00, 0x4E, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char #
        0x06, 0x00, 0x00, 0x98, 0x00, 0x24, 0x01, 0xFE, 0x03, 0x24, 0x01, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char $
        0x07, 0x1E, 0x01, 0x92, 0x00, 0x5E, 0x00, 0x30, 0x00, 0xE8, 0x01, 0x24, 0x01, 0xE2, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char %
        0x07, 0x00, 0x00, 0xE0, 0x00, 0x1C, 0x01, 0x32, 0x01, 0x52, 0x01, 0x8C, 0x01, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char &
        0x02, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char '
        0x04, 0x00, 0x00, 0xF0, 0x00, 0x0C, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char (
        0x03, 0x02, 0x04, 0x0C, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char )
        0x05, 0x04, 0x00, 0x1C, 0x00, 0x06, 0x00, 0x1C, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char *
        0x08, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0xFC, 0x01, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00,  // Code for char +
        0x02, 0x00, 0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ,
        0x05, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char -
        0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char .
        0x05, 0x00, 0x00, 0x00, 0x06, 0xC0, 0x01, 0x38, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char /
        0x06, 0x00, 0x00, 0xFC, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 0
        0x06, 0x00, 0x00, 0x04, 0x01, 0x02, 0x01, 0xFE, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 1
        0x06, 0x00, 0x00, 0x82, 0x01, 0x42, 0x01, 0x22, 0x01, 0x12, 0x01, 0x0C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 2
        0x06, 0x00, 0x00, 0x02, 0x01, 0x12, 0x01, 0x12, 0x01, 0x12, 0x01, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 3
        0x06, 0x00, 0x00, 0x60, 0x00, 0x58, 0x00, 0x44, 0x00, 0xFE, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 4
        0x06, 0x00, 0x00, 0x1E, 0x01, 0x12, 0x01, 0x12, 0x01, 0x12, 0x01, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 5
        0x06, 0x00, 0x00, 0xF8, 0x00, 0x24, 0x01, 0x12, 0x01, 0x12, 0x01, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 6
        0x06, 0x00, 0x00, 0x02, 0x00, 0x82, 0x01, 0x62, 0x00, 0x1A, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 7
        0x06, 0x00, 0x00, 0xEC, 0x00, 0x12, 0x01, 0x12, 0x01, 0x12, 0x01, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 8
        0x06, 0x00, 0x00, 0x1C, 0x01, 0x22, 0x01, 0x22, 0x01, 0x92, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char 9
        0x02, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char :
        0x02, 0x00, 0x04, 0x08, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ;
        0x08, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x50, 0x00, 0x50, 0x00, 0x88, 0x00, 0x88, 0x00, 0x04, 0x01, 0x00, 0x00,  // Code for char <
        0x08, 0x00, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x50, 0x00, 0x00, 0x00,  // Code for char =
        0x08, 0x00, 0x00, 0x04, 0x01, 0x88, 0x00, 0x88, 0x00, 0x50, 0x00, 0x50, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00,  // Code for char >
        0x04, 0x02, 0x00, 0x62, 0x01, 0x12, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ?
        0x09, 0x00, 0x00, 0xF0, 0x00, 0x08, 0x01, 0x74, 0x01, 0x4A, 0x01, 0x7A, 0x01, 0x4A, 0x00, 0x44, 0x00, 0x38, 0x00,  // Code for char @
        0x07, 0x00, 0x01, 0xE0, 0x00, 0x5C, 0x00, 0x42, 0x00, 0x5C, 0x00, 0xE0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char A
        0x05, 0x00, 0x00, 0xFE, 0x01, 0x12, 0x01, 0x12, 0x01, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char B
        0x07, 0x00, 0x00, 0x78, 0x00, 0x84, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char C
        0x07, 0x00, 0x00, 0xFE, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x84, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char D
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x12, 0x01, 0x12, 0x01, 0x12, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char E
        0x05, 0x00, 0x00, 0xFE, 0x01, 0x22, 0x00, 0x22, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char F
        0x07, 0x00, 0x00, 0x78, 0x00, 0x84, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char G
        0x07, 0x00, 0x00, 0xFE, 0x01, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char H
        0x02, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char I
        0x02, 0x00, 0x04, 0xFE, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char J
        0x07, 0x00, 0x00, 0xFE, 0x01, 0x10, 0x00, 0x28, 0x00, 0x44, 0x00, 0x82, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char K
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char L
        0x08, 0x00, 0x00, 0xFE, 0x01, 0x0C, 0x00, 0x70, 0x00, 0x80, 0x00, 0x70, 0x00, 0x0C, 0x00, 0xFE, 0x01, 0x00, 0x00,  // Code for char M
        0x07, 0x00, 0x00, 0xFE, 0x01, 0x04, 0x00, 0x18, 0x00, 0x60, 0x00, 0x80, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char N
        0x08, 0x00, 0x00, 0x78, 0x00, 0x84, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x84, 0x00, 0x78, 0x00, 0x00, 0x00,  // Code for char O
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x22, 0x00, 0x22, 0x00, 0x22, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char P
        0x09, 0x00, 0x00, 0x78, 0x00, 0x84, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x84, 0x02, 0x78, 0x04, 0x00, 0x04,  // Code for char Q
        0x07, 0x00, 0x00, 0xFE, 0x01, 0x22, 0x00, 0x22, 0x00, 0x62, 0x00, 0x9C, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char R
        0x05, 0x00, 0x00, 0x0C, 0x01, 0x12, 0x01, 0x22, 0x01, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char S
        0x07, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0xFE, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char T
        0x07, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char U
        0x07, 0x02, 0x00, 0x1C, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x1C, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char V
        0x09, 0x06, 0x00, 0x78, 0x00, 0x80, 0x01, 0x78, 0x00, 0x06, 0x00, 0x78, 0x00, 0x80, 0x01, 0x78, 0x00, 0x06, 0x00,  // Code for char W
        0x07, 0x02, 0x01, 0x84, 0x00, 0x48, 0x00, 0x30, 0x00, 0x48, 0x00, 0x84, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00,  // Code for char X
        0x07, 0x02, 0x00, 0x04, 0x00, 0x08, 0x00, 0xF0, 0x01, 0x08, 0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char Y
        0x06, 0x00, 0x00, 0x82, 0x01, 0x42, 0x01, 0x32, 0x01, 0x0A, 0x01, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char Z
        0x03, 0x00, 0x00, 0xFE, 0x07, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char [
        0x05, 0x00, 0x00, 0x06, 0x00, 0x38, 0x00, 0xC0, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char BackSlash
        0x03, 0x00, 0x00, 0x02, 0x04, 0xFE, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ]
        0x06, 0x00, 0x00, 0x60, 0x00, 0x18, 0x00, 0x06, 0x00, 0x18, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ^
        0x06, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char _
        0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char `
        0x06, 0x00, 0x00, 0xC8, 0x00, 0x28, 0x01, 0x28, 0x01, 0xF0, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char a
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x10, 0x01, 0x08, 0x01, 0x08, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char b
        0x05, 0x00, 0x00, 0xF0, 0x00, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char c
        0x06, 0x00, 0x00, 0xF0, 0x00, 0x08, 0x01, 0x08, 0x01, 0x88, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char d
        0x05, 0x00, 0x00, 0xF0, 0x00, 0x28, 0x01, 0x28, 0x01, 0x30, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char e
        0x04, 0x08, 0x00, 0xFC, 0x01, 0x0A, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char f
        0x06, 0x00, 0x00, 0xF0, 0x04, 0x08, 0x05, 0x08, 0x05, 0x88, 0x04, 0xF8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char g
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x10, 0x00, 0x08, 0x00, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char h
        0x02, 0x00, 0x00, 0xFA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char i
        0x02, 0x00, 0x04, 0xFA, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char j
        0x06, 0x00, 0x00, 0xFE, 0x01, 0x20, 0x00, 0x60, 0x00, 0x90, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char k
        0x02, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char l
        0x08, 0x00, 0x00, 0xF8, 0x01, 0x10, 0x00, 0x08, 0x00, 0xF8, 0x01, 0x10, 0x00, 0x08, 0x00, 0xF8, 0x01, 0x00, 0x00,  // Code for char m
        0x06, 0x00, 0x00, 0xF8, 0x01, 0x10, 0x00, 0x08, 0x00, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char n
        0x06, 0x00, 0x00, 0xF0, 0x00, 0x08, 0x01, 0x08, 0x01, 0x08, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char o
        0x06, 0x00, 0x00, 0xF8, 0x07, 0x10, 0x01, 0x08, 0x01, 0x08, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char p
        0x06, 0x00, 0x00, 0xF0, 0x00, 0x08, 0x01, 0x08, 0x01, 0x88, 0x00, 0xF8, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char q
        0x05, 0x00, 0x00, 0xF8, 0x01, 0x10, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char r
        0x05, 0x00, 0x00, 0x10, 0x01, 0x28, 0x01, 0x48, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char s
        0x03, 0x08, 0x00, 0xFC, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char t
        0x06, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x01, 0x00, 0x01, 0x80, 0x00, 0xF8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char u
        0x05, 0x18, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char v
        0x09, 0x18, 0x00, 0xE0, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x01, 0xE0, 0x00, 0x18, 0x00,  // Code for char w
        0x06, 0x00, 0x00, 0x08, 0x01, 0x90, 0x00, 0x60, 0x00, 0x90, 0x00, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char x
        0x05, 0x18, 0x00, 0x60, 0x04, 0x80, 0x03, 0x60, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char y
        0x05, 0x00, 0x00, 0x88, 0x01, 0x48, 0x01, 0x28, 0x01, 0x18, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char z
        0x03, 0x20, 0x00, 0xDC, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char {
        0x02, 0x00, 0x00, 0xFE, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char |
        0x04, 0x00, 0x00, 0x02, 0x04, 0xDC, 0x03, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char }
        0x06, 0x00, 0x00, 0x40, 0x00, 0x20, 0x00, 0x20, 0x00, 0x40, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Code for char ~
        0x02, 0xFC, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   // Code for char 
};

int FontChar[FONT_WIDTH][FONT_HEIGHT];

void setHorizontal();
void setVertical();

int main()
{
    for(int Index = 0; Index < NUMBER_OF_CHARS; Index++)
    {
        int Column = 0;

        memset(FontChar, 0, sizeof(FontChar));
        for(int Segment = 1; Segment < NUMBER_OF_COLUMNS; Segment++)
        {
            int SegmentUpper = Font[Index][Segment++];
            int SegmentLower = Font[Index][Segment];

            for(int Bit = 0; Bit < 8; Bit++) {
               FontChar[Column][Bit] = readBit(SegmentUpper, Bit);
            }

            for(int Bit = 0; Bit < FONT_HEIGHT - 8; Bit++) {
               FontChar[Column][8 + Bit] = readBit(SegmentLower, Bit);
            }
            Column++;
        }

        setHorizontal();
        //setVertical();
    }

    return 0;
}


void setVertical()
{
    for(int a = 0; a < FONT_WIDTH; a++)
    {
        uint32_t Column = 0;
        for(int i = 0; i < 32 && i < FONT_HEIGHT; i++) {
#if (DECODE == STD_OFF)
            if(FontChar[a][i] == 0) cout << " ";
            else cout << FontChar[a][i];
#endif
#if (DECODE == STD_ON)
            WRITE_BIT(Column, i, FontChar[a][i]);
            //writeBit(Column, i, FontChar[i][a]);
#endif
        }
#if (DECODE == STD_OFF)
        cout << endl;
#endif
#if (DECODE == STD_ON)
        if(Column < 16) cout << "0x00" << hex << Column << ",";
        else if(Column < 255) cout << "0x0" << hex << Column << ",";
        else cout << "0x" << hex << Column << ",";
#endif
    }
    cout << endl;
}


void setHorizontal()
{
    for(int a = 0; a < FONT_HEIGHT; a++)
    {
        uint32_t Row = 0;
        for(int i = 0; i < 32 && i < FONT_WIDTH; i++) {
#if (DECODE == STD_OFF)
            if(FontChar[i][a] == 0) cout << " ";
            else cout << FontChar[i][a];
#endif
#if (DECODE == STD_ON)
            WRITE_BIT(Row, i, FontChar[i][a]);
            //writeBit(Row, i, FontChar[i][a]);
#endif
        }
#if (DECODE == STD_OFF)
        cout << endl;
#endif
#if (DECODE == STD_ON)
        if(Row < 16) cout << "0x00" << hex << Row << ",";
        else if(Row < 255) cout << "0x0" << hex << Row << ",";
        else cout << "0x" << hex << Row << ",";
#endif
    }
    cout << endl;
}

