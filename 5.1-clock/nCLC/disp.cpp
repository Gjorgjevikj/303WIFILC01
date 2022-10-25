// disp.cpp - driver for the 303WIFILC01 display/TM1650

#include <Arduino.h>
#include <Wire.h>
#include "disp.h"

// The 303WIFILC01 board does not connect pin X of the TM1650 to pin X of the 4x7 segment display.
// The DIG1, DIG2, DIG3, and DIG4 or 1-1, so are segments C, D, E, but the other segments are mixed.
//   to light segment power pin
//   ---------------- ---------
//           A            F   
//           B            P   
//           C            C   
//           D            D   
//           E            E   
//           F            G   
//           G            B   
//           P            A   

// https://github.com/maarten-pennings/SevenSegment-over-Serial/tree/main/font#lookalike7s
// A font, optimized for readability, for a 7 segment display.
// Support characters 0..127 (but first 32 are empty).
const uint8_t Disp303::dispFont[0x80] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // padding for 0x0_
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // padding for 0x1_
  //bfaedcgp
  0b00000000, // 20 spc
  0b01001000, // 21 !
  0b11000000, // 22 "
  0b11100010, // 23 #
  0b00101010, // 24 $
  0b01000100, // 25 %
  0b11011110, // 26 &
  0b10000000, // 27 '
  0b01111000, // 28 (
  0b10101100, // 29 )
  0b00100000, // 2A *
  0b10000010, // 2B +
  0b00001100, // 2C ,
  0b00000010, // 2D -
  0b00010000, // 2E .
  0b10010010, // 2F /
  //bfaedcgp
  0b11111100, // 30 0
  0b10000100, // 31 1
  0b10111010, // 32 2
  0b10101110, // 33 3
  0b11000110, // 34 4
  0b01101110, // 35 5
  0b01111110, // 36 6
  0b10100100, // 37 7
  0b11111110, // 38 8
  0b11101110, // 39 9
  0b00101000, // 3A :
  0b10001000, // 3B ;
  0b00011010, // 3C <
  0b00001010, // 3D =
  0b00001110, // 3E >
  0b10101010, // 3F ?
  //bfaedcgp
  0b11111000, // 40 @
  0b11110110, // 41 A
  0b01011110, // 42 B
  0b01111000, // 43 C
  0b10011110, // 44 D
  0b01111010, // 45 E
  0b01110010, // 46 F
  0b01111100, // 47 G
  0b11010110, // 48 H
  0b01010000, // 49 I
  0b10011100, // 4A J
  0b01110110, // 4B K
  0b01011100, // 4C L
  0b00110110, // 4D M
  0b11110100, // 4E N
  0b11111100, // 4F O
  //bfaedcgp
  0b11110010, // 50 P
  0b11101010, // 51 Q
  0b11110000, // 52 R
  0b01101110, // 53 S
  0b01011010, // 54 T
  0b11011100, // 55 U
  0b11010010, // 56 V
  0b11001010, // 57 W
  0b11010100, // 58 X
  0b11001110, // 59 Y
  0b10111010, // 5A Z
  0b01111000, // 5B [
  0b01000110, // 5C \ (\ shall not end line in C)
  0b10101100, // 5D ]
  0b11100000, // 5E ^
  0b00001000, // 5F _
  //bfaedcgp
  0b01000000, // 60 `
  0b10111110, // 61 a
  0b01011110, // 62 b
  0b00011010, // 63 c
  0b10011110, // 64 d
  0b11111010, // 65 e
  0b01110010, // 66 f
  0b11101110, // 67 g
  0b01010110, // 68 h
  0b00100100, // 69 i
  0b00101100, // 6A j
  0b01110110, // 6B k
  0b01011000, // 6C l
  0b00110110, // 6D m
  0b00010110, // 6E n
  0b00011110, // 6F o
  //bfaedcgp
  0b11110010, // 70 p
  0b11100110, // 71 q
  0b00010010, // 72 r
  0b01101110, // 73 s
  0b01011010, // 74 t
  0b00011100, // 75 u
  0b11010010, // 76 v
  0b11001010, // 77 w
  0b00010100, // 78 x
  0b11001100, // 79 y
  0b10111010, // 7A z
  0b10000110, // 7B {
  0b10000100, // 7C |
  0b01010010, // 7D }
  0b00100010, // 7E ~
  0b00111110  // 7F del
};


Disp303::Disp303(int bright, bool segmode, bool disppow) 
{
    //settings.all = ((bright & 0b111) << 4) | (segmode << 3) | disppow;
    settings.brightness = bright;
    settings.mode = segmode;
    settings.power = disppow;
}

void Disp303::init()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    //int res = write(TM1650_CONTROL_BASE, *(reinterpret_cast<unsigned char*>(&settings)));
    int res = write(TM1650_CONTROL_BASE, settings.all);
    if (res == 0) Serial.printf("disp: init\n");
    else Serial.printf("disp: init ERROR %d\n", res);
}

void Disp303::setBrightness(uint8_t brightness)
{
    if (brightness < 1) brightness = 1;
    if (brightness > 8) brightness = 8;
    settings.brightness = brightness & 0x07;
    init();
}

void Disp303::setBrightness()
{
    settings.brightness++;
    init();
}

uint8_t Disp303::getBrightness() const
{
    return settings.brightness;
}

void Disp303::setPower(bool power) {
    settings.power = power;
    init();
}

bool Disp303::getPower() const
{
    return settings.power; // disp_power;
}

void Disp303::setMode(bool segmode)
{
    settings.mode = segmode;
}

bool Disp303::getMode() const
{
    return settings.mode;
}

void Disp303::show(const char* s, uint8_t dots)
{
    for (uint8_t i = 0; *s && i < 4; i++, s++)
    {
        // Lookup for char *s which segments to enable. *s is truncated to 7 bits
        uint8_t segments = (dispFont[*s & 0x7F]);
        if (dots & (1 << i)) 
            segments |= SEG_P; // Add dot to segments
        setDigit(i, segments);
    }
}

IRAM_ATTR void Disp303::setDigit(uint8_t d, uint8_t segs)
{
    write(TM1650_DISPLAY_BASE + d, segs);
}

IRAM_ATTR uint8_t Disp303::write(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(reg);
    Wire.write(val);
    return Wire.endTransmission();
}
