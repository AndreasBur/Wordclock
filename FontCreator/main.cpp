#include <iostream>
#include <string.h>
#include "StandardTypes.h"

using namespace std;

#define DECODE                              STD_ON

//#define NUMBER_OF_CHARS                     96
#define NUMBER_OF_CHARS                     224

#define NUMBER_OF_COLUMNS                   15

#define FONT_HEIGHT                         15
#define FONT_WIDTH                          7

#define NUMBER_OF_SEGMENTS                  (NUMBER_OF_COLUMNS - 1)
#define NUMBER_OF_LOWER_SEGMENTS            (NUMBER_OF_SEGMENTS / 2)


const unsigned short Font[][NUMBER_OF_COLUMNS] =
{
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char !
        {0x06, 0x00, 0x00, 0x70, 0x00, 0x10, 0x00, 0x00, 0x00, 0x70, 0x00, 0x10, 0x00, 0x00, 0x00},  // Code for char "
        {0x06, 0x00, 0x00, 0x80, 0x1A, 0xC0, 0x07, 0xB0, 0x1A, 0xC0, 0x07, 0xB0, 0x02, 0x00, 0x00},  // Code for char #
        {0x05, 0x00, 0x00, 0xC0, 0x06, 0x20, 0x05, 0x30, 0x1D, 0x20, 0x03, 0x00, 0x00, 0x00, 0x00},  // Code for char $
        {0x06, 0x00, 0x00, 0x20, 0x01, 0x50, 0x01, 0x20, 0x05, 0x80, 0x0A, 0x80, 0x04, 0x00, 0x00},  // Code for char %
        {0x06, 0x00, 0x00, 0x00, 0x06, 0x80, 0x09, 0x40, 0x0A, 0x40, 0x04, 0x00, 0x0A, 0x00, 0x00},  // Code for char &
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char '
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0F, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00},  // Code for char (
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char )
        {0x06, 0x00, 0x00, 0x20, 0x00, 0xA0, 0x01, 0x70, 0x00, 0xA0, 0x01, 0x20, 0x00, 0x00, 0x00},  // Code for char *
        {0x07, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0xE0, 0x0F, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01},  // Code for char +
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x1C, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00},  // Code for char ,
        {0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00},  // Code for char -
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char .
        {0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0C, 0x00, 0x03, 0xC0, 0x00, 0x30, 0x00, 0x00, 0x00},  // Code for char /
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char 0
        {0x06, 0x00, 0x00, 0x00, 0x08, 0x10, 0x08, 0xF0, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char 1
        {0x06, 0x00, 0x00, 0x20, 0x0C, 0x10, 0x0A, 0x10, 0x09, 0x90, 0x08, 0x60, 0x0C, 0x00, 0x00},  // Code for char 2
        {0x06, 0x00, 0x00, 0x20, 0x04, 0x10, 0x08, 0x90, 0x08, 0x90, 0x08, 0x60, 0x07, 0x00, 0x00},  // Code for char 3
        {0x07, 0x00, 0x00, 0x00, 0x03, 0x80, 0x02, 0x60, 0x02, 0x10, 0x0A, 0xF0, 0x0F, 0x00, 0x0A},  // Code for char 4
        {0x06, 0x00, 0x00, 0x00, 0x04, 0xF0, 0x08, 0x90, 0x08, 0x90, 0x08, 0x10, 0x07, 0x00, 0x00},  // Code for char 5
        {0x06, 0x00, 0x00, 0xC0, 0x07, 0xA0, 0x08, 0x90, 0x08, 0x90, 0x08, 0x10, 0x07, 0x00, 0x00},  // Code for char 6
        {0x06, 0x00, 0x00, 0x30, 0x00, 0x10, 0x00, 0x10, 0x0C, 0x90, 0x03, 0x70, 0x00, 0x00, 0x00},  // Code for char 7
        {0x06, 0x00, 0x00, 0x60, 0x07, 0x90, 0x08, 0x90, 0x08, 0x90, 0x08, 0x60, 0x07, 0x00, 0x00},  // Code for char 8
        {0x06, 0x00, 0x00, 0xE0, 0x08, 0x10, 0x09, 0x10, 0x09, 0x10, 0x05, 0xE0, 0x03, 0x00, 0x00},  // Code for char 9
        {0x04, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x0C, 0xC0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char :
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xC0, 0x0C, 0xC0, 0x04, 0x00, 0x00, 0x00, 0x00},  // Code for char ;
        {0x06, 0x00, 0x01, 0x80, 0x02, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x20, 0x08, 0x00, 0x00},  // Code for char <
        {0x06, 0x00, 0x00, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x80, 0x02, 0x00, 0x00},  // Code for char =
        {0x06, 0x20, 0x08, 0x20, 0x08, 0x40, 0x04, 0x80, 0x02, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00},  // Code for char >
        {0x06, 0x00, 0x00, 0x00, 0x00, 0x40, 0x08, 0x20, 0x0A, 0x20, 0x01, 0xC0, 0x00, 0x00, 0x00},  // Code for char ?
        {0x06, 0x00, 0x00, 0xF0, 0x0F, 0x08, 0x10, 0x88, 0x11, 0x48, 0x12, 0xF0, 0x0B, 0x00, 0x00},  // Code for char @
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD0, 0x0B, 0x30, 0x02, 0xC0, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char A
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0x90, 0x08, 0x90, 0x08, 0x60, 0x07, 0x00, 0x00},  // Code for char B
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x30, 0x04, 0x00, 0x00},  // Code for char C
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x08, 0x10, 0x08, 0x20, 0x04, 0xC0, 0x03, 0x00, 0x00},  // Code for char D
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0xD0, 0x09, 0x10, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char E
        {0x07, 0x00, 0x00, 0x10, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0xD0, 0x01, 0x10, 0x00, 0x30, 0x00},  // Code for char F
        {0x07, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x10, 0x08, 0x10, 0x09, 0x30, 0x07, 0x00, 0x01},  // Code for char G
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0x80, 0x00, 0x90, 0x08, 0xF0, 0x0F, 0x10, 0x08},  // Code for char H
        {0x06, 0x00, 0x00, 0x10, 0x08, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x08, 0x10, 0x08, 0x00, 0x00},  // Code for char I
        {0x06, 0x00, 0x00, 0x00, 0x07, 0x10, 0x08, 0x10, 0x08, 0xF0, 0x07, 0x10, 0x00, 0x00, 0x00},  // Code for char J
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x09, 0x80, 0x01, 0x50, 0x02, 0x30, 0x0C, 0x10, 0x08},  // Code for char K
        {0x06, 0x00, 0x00, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x08, 0x00, 0x08, 0x00, 0x0E, 0x00, 0x00},  // Code for char L
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x70, 0x08, 0x80, 0x01, 0x70, 0x08, 0xF0, 0x0F, 0x10, 0x08},  // Code for char M
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x70, 0x08, 0x80, 0x03, 0x10, 0x0C, 0xF0, 0x0F, 0x10, 0x00},  // Code for char N
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char O
        {0x06, 0x00, 0x00, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x09, 0x10, 0x01, 0xE0, 0x00, 0x00, 0x00},  // Code for char P
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x10, 0x18, 0x10, 0x18, 0xE0, 0x17, 0x00, 0x00},  // Code for char Q
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x09, 0x10, 0x01, 0x10, 0x03, 0xE0, 0x04, 0x00, 0x08},  // Code for char R
        {0x06, 0x00, 0x00, 0x60, 0x0C, 0x90, 0x04, 0x90, 0x08, 0xA0, 0x08, 0x30, 0x07, 0x00, 0x00},  // Code for char S
        {0x07, 0x30, 0x00, 0x10, 0x00, 0x10, 0x08, 0xF0, 0x0F, 0x10, 0x08, 0x10, 0x00, 0x30, 0x00},  // Code for char T
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x10, 0x08, 0x00, 0x08, 0x10, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char U
        {0x07, 0x10, 0x00, 0x70, 0x00, 0x90, 0x03, 0x00, 0x0C, 0x90, 0x03, 0x70, 0x00, 0x10, 0x00},  // Code for char V
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x10, 0x08, 0x80, 0x07, 0x10, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char W
        {0x07, 0x10, 0x08, 0x30, 0x0C, 0x40, 0x02, 0x80, 0x01, 0x40, 0x02, 0x30, 0x0C, 0x10, 0x08},  // Code for char X
        {0x07, 0x10, 0x00, 0x30, 0x00, 0xD0, 0x08, 0x00, 0x0F, 0xD0, 0x08, 0x30, 0x00, 0x10, 0x00},  // Code for char Y
        {0x06, 0x00, 0x00, 0x30, 0x0C, 0x10, 0x0A, 0x90, 0x09, 0x50, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char Z
        {0x05, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3F, 0x10, 0x20, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00},  // Code for char [
        {0x05, 0x00, 0x00, 0x10, 0x00, 0xE0, 0x00, 0x00, 0x03, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00},  // Code for char BackSlash
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x10, 0x20, 0xF0, 0x3F, 0x00, 0x00, 0x00, 0x00},  // Code for char ]
        {0x06, 0x00, 0x00, 0x80, 0x00, 0x40, 0x00, 0x30, 0x00, 0x40, 0x00, 0x80, 0x00, 0x00, 0x00},  // Code for char ^
        {0x07, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40},  // Code for char _
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char `
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x40, 0x09, 0x40, 0x09, 0x40, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char a
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x80, 0x08, 0x40, 0x08, 0x40, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char b
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x08, 0x40, 0x08, 0x40, 0x08, 0xC0, 0x04, 0x00, 0x00},  // Code for char c
        {0x07, 0x00, 0x00, 0x80, 0x07, 0x40, 0x08, 0x40, 0x08, 0x90, 0x08, 0xF0, 0x0F, 0x00, 0x08},  // Code for char d
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x09, 0x40, 0x09, 0x40, 0x09, 0x80, 0x09, 0x00, 0x00},  // Code for char e
        {0x06, 0x00, 0x00, 0x40, 0x08, 0xE0, 0x0F, 0x50, 0x08, 0x50, 0x08, 0x50, 0x08, 0x00, 0x00},  // Code for char f
        {0x07, 0x00, 0x00, 0x80, 0x07, 0x40, 0x28, 0x40, 0x28, 0x80, 0x28, 0xC0, 0x1F, 0x40, 0x00},  // Code for char g
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x80, 0x08, 0x40, 0x00, 0x40, 0x08, 0x80, 0x0F, 0x00, 0x08},  // Code for char h
        {0x06, 0x00, 0x00, 0x40, 0x08, 0x40, 0x08, 0xD0, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char i
        {0x05, 0x00, 0x00, 0x40, 0x20, 0x40, 0x20, 0x50, 0x20, 0xC0, 0x1F, 0x00, 0x00, 0x00, 0x00},  // Code for char j
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x00, 0x01, 0x40, 0x0B, 0xC0, 0x0C, 0x40, 0x08, 0x00, 0x00},  // Code for char k
        {0x06, 0x00, 0x00, 0x00, 0x08, 0x10, 0x08, 0xF0, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char l
        {0x07, 0x40, 0x08, 0xC0, 0x0F, 0x40, 0x08, 0x80, 0x0F, 0x40, 0x08, 0x80, 0x0F, 0x00, 0x08},  // Code for char m
        {0x07, 0x40, 0x08, 0xC0, 0x0F, 0x80, 0x08, 0x40, 0x00, 0x40, 0x08, 0x80, 0x0F, 0x00, 0x08},  // Code for char n
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x08, 0x40, 0x08, 0x40, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char o
        {0x06, 0x40, 0x20, 0xC0, 0x3F, 0x80, 0x28, 0x40, 0x08, 0x40, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char p
        {0x07, 0x00, 0x00, 0x80, 0x07, 0x40, 0x08, 0x40, 0x08, 0x80, 0x28, 0xC0, 0x3F, 0x40, 0x20},  // Code for char q
        {0x06, 0x00, 0x00, 0x40, 0x08, 0xC0, 0x0F, 0x80, 0x08, 0x40, 0x08, 0x40, 0x08, 0x00, 0x00},  // Code for char r
        {0x06, 0x00, 0x00, 0x80, 0x0C, 0x40, 0x09, 0x40, 0x09, 0x40, 0x09, 0xC0, 0x06, 0x00, 0x00},  // Code for char s
        {0x07, 0x00, 0x00, 0x40, 0x00, 0xE0, 0x07, 0x40, 0x08, 0x40, 0x08, 0x40, 0x08, 0x00, 0x04},  // Code for char t
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x00, 0x08, 0x00, 0x08, 0x40, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char u
        {0x07, 0x40, 0x00, 0xC0, 0x01, 0x40, 0x06, 0x00, 0x08, 0x40, 0x06, 0xC0, 0x01, 0x40, 0x00},  // Code for char v
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x40, 0x08, 0x00, 0x07, 0x40, 0x08, 0xC0, 0x07, 0x40, 0x00},  // Code for char w
        {0x06, 0x40, 0x08, 0xC0, 0x0C, 0x00, 0x03, 0x00, 0x03, 0xC0, 0x0C, 0x40, 0x08, 0x00, 0x00},  // Code for char x
        {0x07, 0x40, 0x00, 0xC0, 0x20, 0x40, 0x23, 0x00, 0x3C, 0x40, 0x26, 0xC0, 0x01, 0x40, 0x00},  // Code for char y
        {0x06, 0x00, 0x00, 0xC0, 0x0C, 0x40, 0x0A, 0x40, 0x09, 0xC0, 0x08, 0x40, 0x0C, 0x00, 0x00},  // Code for char z
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xE0, 0x1D, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00},  // Code for char {
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char |
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0xE0, 0x1D, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00},  // Code for char }
        {0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00},  // Code for char ~
        {0x03, 0xF0, 0x1F, 0x10, 0x10, 0xF0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char 
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x03, 0x80, 0x04, 0xC0, 0x0C, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x00, 0x09, 0xE0, 0x0D, 0x10, 0x0B, 0x20, 0x09, 0x00, 0x0C, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x00, 0x20, 0x04, 0xC0, 0x03, 0x40, 0x02, 0x40, 0x02, 0xC0, 0x03, 0x20, 0x04},  // Code for char �
        {0x07, 0x10, 0x00, 0xB0, 0x02, 0xD0, 0x0A, 0x80, 0x0F, 0xD0, 0x0A, 0xB0, 0x02, 0x10, 0x00},  // Code for char �
        {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x00, 0xC0, 0x18, 0x70, 0x11, 0x50, 0x12, 0x90, 0x14, 0x10, 0x1D, 0x30, 0x06},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x80, 0x03, 0x40, 0x04, 0xA0, 0x0B, 0xA0, 0x0A, 0xA0, 0x0A, 0x40, 0x04, 0x80, 0x03},  // Code for char �
        {0x06, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x00, 0xD0, 0x00, 0xF0, 0x00, 0x80, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x05, 0x80, 0x08, 0x00, 0x02, 0x00, 0x05, 0x80, 0x08},  // Code for char �
        {0x07, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x0E},  // Code for char �
        {0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00},  // Code for char �
        {0x07, 0x80, 0x03, 0x40, 0x06, 0xE0, 0x0B, 0x60, 0x0B, 0xE0, 0x0A, 0x40, 0x04, 0x80, 0x03},  // Code for char �
        {0x07, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x28, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x09, 0x00, 0x09, 0x00, 0x09, 0xE0, 0x0F, 0x00, 0x09, 0x00, 0x09, 0x00, 0x09},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0xD0, 0x00, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0xB0, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x3F, 0x00, 0x08, 0x00, 0x08, 0x40, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0xE0, 0x00, 0x10, 0x11, 0x10, 0x11, 0xF0, 0x1F, 0x10, 0x00, 0xF0, 0x1F, 0x10, 0x10},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0xF0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x05, 0x00, 0x00, 0x60, 0x00, 0x90, 0x00, 0x90, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x06, 0x80, 0x08, 0x00, 0x05, 0x00, 0x02, 0x80, 0x08, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00},  // Code for char �
        {0x07, 0x90, 0x00, 0xF0, 0x04, 0x80, 0x02, 0x00, 0x05, 0x80, 0x0E, 0x80, 0x0F, 0x40, 0x0C},  // Code for char �
        {0x07, 0x90, 0x00, 0xF0, 0x02, 0x80, 0x01, 0x80, 0x00, 0x80, 0x09, 0x40, 0x0D, 0x20, 0x0B},  // Code for char �
        {0x07, 0x90, 0x00, 0xB0, 0x04, 0xF0, 0x02, 0x00, 0x05, 0x80, 0x0E, 0x40, 0x0F, 0x20, 0x0C},  // Code for char �
        {0x05, 0x00, 0x00, 0x00, 0x18, 0x00, 0x24, 0x40, 0x22, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD2, 0x0B, 0x34, 0x02, 0xC0, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD4, 0x0B, 0x32, 0x02, 0xC0, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD4, 0x0B, 0x32, 0x02, 0xC4, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x08, 0x06, 0x0E, 0xD2, 0x0B, 0x34, 0x02, 0xC6, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD4, 0x0B, 0x30, 0x02, 0xC4, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x08, 0x00, 0x0E, 0xD7, 0x0B, 0x35, 0x02, 0xC7, 0x0B, 0x00, 0x0E, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x0E, 0xD0, 0x09, 0x30, 0x01, 0xF0, 0x0F, 0x90, 0x08, 0x90, 0x08, 0xB0, 0x0C},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x48, 0x10, 0x58, 0x10, 0x68, 0x30, 0x04, 0x00, 0x00},  // Code for char �
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x92, 0x08, 0xD4, 0x09, 0x10, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char �
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0xD4, 0x09, 0x12, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char �
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x94, 0x08, 0xD2, 0x09, 0x14, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char �
        {0x06, 0x10, 0x08, 0xF0, 0x0F, 0x94, 0x08, 0xD0, 0x09, 0x14, 0x08, 0x30, 0x0C, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x10, 0x08, 0x12, 0x08, 0xF4, 0x0F, 0x10, 0x08, 0x10, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x10, 0x08, 0x10, 0x08, 0xF4, 0x0F, 0x12, 0x08, 0x10, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x10, 0x08, 0x14, 0x08, 0xF2, 0x0F, 0x14, 0x08, 0x10, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x10, 0x08, 0x14, 0x08, 0xF0, 0x0F, 0x14, 0x08, 0x10, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x90, 0x08, 0xF0, 0x0F, 0x90, 0x08, 0x10, 0x08, 0x20, 0x04, 0xC0, 0x03, 0x00, 0x00},  // Code for char �
        {0x07, 0x10, 0x08, 0xF0, 0x0F, 0x76, 0x08, 0x82, 0x03, 0x14, 0x0C, 0xF6, 0x0F, 0x10, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x12, 0x08, 0x14, 0x08, 0x10, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x10, 0x08, 0x14, 0x08, 0x12, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x14, 0x08, 0x12, 0x08, 0x14, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x16, 0x08, 0x12, 0x08, 0x14, 0x08, 0xE6, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0xE0, 0x07, 0x14, 0x08, 0x10, 0x08, 0x14, 0x08, 0xE0, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x80, 0x01, 0x80, 0x01, 0x40, 0x02, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x00, 0xE0, 0x0B, 0x10, 0x0C, 0x10, 0x0B, 0x90, 0x08, 0xE0, 0x07, 0x10, 0x00},  // Code for char �
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x12, 0x08, 0x04, 0x08, 0x10, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char �
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x10, 0x08, 0x04, 0x08, 0x12, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char �
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x14, 0x08, 0x02, 0x08, 0x14, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char �
        {0x07, 0x10, 0x00, 0xF0, 0x07, 0x14, 0x08, 0x00, 0x08, 0x14, 0x08, 0xF0, 0x07, 0x10, 0x00},  // Code for char �
        {0x07, 0x10, 0x00, 0x30, 0x00, 0xD0, 0x08, 0x04, 0x0F, 0xD2, 0x08, 0x30, 0x00, 0x10, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x10, 0x08, 0xF0, 0x0F, 0x50, 0x0A, 0x40, 0x02, 0x80, 0x01, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x08, 0xE0, 0x0F, 0x10, 0x08, 0x90, 0x0C, 0x60, 0x09, 0x00, 0x07, 0x00, 0x00},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x48, 0x09, 0x50, 0x09, 0x40, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x40, 0x09, 0x50, 0x09, 0x48, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x50, 0x09, 0x48, 0x09, 0x50, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x58, 0x09, 0x48, 0x09, 0x50, 0x09, 0x98, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x50, 0x09, 0x40, 0x09, 0x50, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x00, 0x80, 0x06, 0x5C, 0x09, 0x54, 0x09, 0x5C, 0x09, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x00, 0x06, 0x40, 0x09, 0x40, 0x09, 0x80, 0x0F, 0x40, 0x09, 0x40, 0x09, 0x80, 0x09},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x48, 0x40, 0x58, 0x40, 0x68, 0xC0, 0x04, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x48, 0x09, 0x50, 0x09, 0x40, 0x09, 0x80, 0x09, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x09, 0x50, 0x09, 0x48, 0x09, 0x80, 0x09, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x50, 0x09, 0x48, 0x09, 0x50, 0x09, 0x80, 0x09, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x50, 0x09, 0x40, 0x09, 0x50, 0x09, 0x80, 0x09, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x40, 0x08, 0x48, 0x08, 0xD0, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x40, 0x08, 0x50, 0x08, 0xC8, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x40, 0x08, 0x50, 0x08, 0xC8, 0x0F, 0x10, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x40, 0x08, 0x50, 0x08, 0xC0, 0x0F, 0x10, 0x08, 0x00, 0x08, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x40, 0x07, 0xB0, 0x08, 0xA0, 0x08, 0x50, 0x09, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x07, 0x40, 0x08, 0xD8, 0x0F, 0x88, 0x08, 0x50, 0x00, 0x58, 0x08, 0x80, 0x0F, 0x00, 0x08},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x48, 0x08, 0x50, 0x08, 0x40, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x40, 0x08, 0x50, 0x08, 0x48, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x50, 0x08, 0x48, 0x08, 0x50, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x98, 0x07, 0x48, 0x08, 0x50, 0x08, 0x58, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x07, 0x50, 0x08, 0x40, 0x08, 0x50, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x40, 0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00},  // Code for char �
        {0x06, 0x00, 0x00, 0x80, 0x0B, 0x40, 0x04, 0x40, 0x0B, 0x80, 0x08, 0x40, 0x07, 0x00, 0x00},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x08, 0x08, 0x10, 0x08, 0x40, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x00, 0x08, 0x10, 0x08, 0x48, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x10, 0x08, 0x08, 0x08, 0x50, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x07, 0x10, 0x08, 0x00, 0x08, 0x50, 0x04, 0xC0, 0x0F, 0x00, 0x08},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x20, 0x40, 0x23, 0x10, 0x3C, 0x48, 0x26, 0xC0, 0x01, 0x40, 0x00},  // Code for char �
        {0x06, 0x10, 0x20, 0xF0, 0x3F, 0x80, 0x24, 0x40, 0x08, 0x40, 0x08, 0x80, 0x07, 0x00, 0x00},  // Code for char �
        {0x07, 0x40, 0x00, 0xC0, 0x20, 0x50, 0x23, 0x00, 0x3C, 0x50, 0x26, 0xC0, 0x01, 0x40, 0x00}   // Code for char �
};

int main()
{
    for(int Index = 0; Index < NUMBER_OF_CHARS; Index++)
    {
        int Column = 0;
        int FontChar[FONT_WIDTH][FONT_HEIGHT];
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

        for(int a = 0; a < FONT_HEIGHT; a++)
        {
            int Row = 0;
            for(int i = 0; i < 8 && i < FONT_WIDTH; i++) {
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
            if(Row < 16) cout << "0x0" << hex << Row << ",";
            else cout << "0x" << hex << Row << ",";
#endif
        }
        cout << endl;
    }

    return 0;
}
