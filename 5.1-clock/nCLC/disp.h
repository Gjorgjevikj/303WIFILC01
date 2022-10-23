// disp.h - interface for driver for the 303WIFILC01 display/TM1650
#ifndef _DISP_H_
#define _DISP_H_

// Constants for dots in disp_show()
#define DISP_DOTNO     0
#define DISP_DOT1      1
#define DISP_DOTCOLON  2 // Hardware maps DOT3 to the colon
#define DISP_DOT3      4
#define DISP_DOT4      8
#define DISP_ALL       ( DISP_DOT1 | DISP_DOT2 | DISP_DOTCOLON | DISP_DOT4 )

enum nCLCsegs : uint8_t {
    SEG_P = 0b00000001,
    SEG_G = 0b00000010,
    SEG_C = 0b00000100,
    SEG_D = 0b00001000,
    SEG_E = 0b00010000,
    SEG_A = 0b00100000,
    SEG_F = 0b01000000,
    SEG_B = 0b10000000
};

static const uint8_t dispFont[0x80] = {
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


void disp_brightness_set(int brightness);       // Sets brightness level 1..8
int  disp_brightness_get();                     // Gets brightness level
void disp_power_set(int power);                 // Sets power 0 (off) or 1 (on)
int  disp_power_get();                          // Gets power level
                                              
void disp_init();                               // Initializes display (prints error to Serial)
void disp_show(const char * s, uint8_t dots=0); // Puts (first 4 chars of) `s` (padded with spaces) on display, using flags in `dots` for P

void disp_set(int d, uint8_t segs);             // Sets raw segments on the n-th digit of the display



class disp303
{
private:
    enum disp303pins : int8_t { SCL_PIN = 12, SDA_PIN = 13 };
public:
    disp303(int b = 4, bool m = false, bool p = false);
    /* : disp_brightness(b), disp_mode(m), disp_power(p)
    {
        init();
    }*/

    void init();
    /* {
        Wire.begin(SDA_PIN, SCL_PIN);
        int res = write(0x24, ((disp_brightness & 0b111) << 4) | (disp_mode7 << 3) | disp_power);
        if (res == 0) Serial.printf("disp: init\n");
        else Serial.printf("disp: init ERROR %d\n", res);
    }*/

    void setBrightness(uint8_t brightness);
    /* {
        if (brightness < 1) brightness = 1;
        if (brightness > 8) brightness = 8;
        disp_brightness = brightness;
        init();
    }*/

    uint8_t getBrightness() const;
    /* {
        return disp_brightness;
    }*/

    void increaseBrightness();
    void decreaseBrightness();

    void setPower(bool power = true);
    /* {
        disp_power = power;
        init();
    }*/

    bool getPower() const;
    /* {
        return disp_power;
    }*/

    static void show(const char* s, uint8_t dots = 0);
    /* {
        for (int i = 0; *s && i < 4; i++, s++) 
        {
            // Lookup for char *s which segments to enable. *s is truncated to 7 bits
            uint8_t segments = (dispFont[*s & 0x7F]); 
            if (dots & (1 << i)) segments |= SEG_P; // Add dot to segments
            disp_set(i, segments);
        }
    }*/

    IRAM_ATTR static void setDigit(uint8_t d, uint8_t segs);
    /* {
        // Send to display
        Wire.beginTransmission(0x34 + d);
        Wire.write(segs);
        Wire.endTransmission();
    }*/
private:
    IRAM_ATTR static uint8_t write(uint8_t reg, uint8_t val);
    /* {
        Wire.beginTransmission(reg); 
        Wire.write(val);
        return Wire.endTransmission();
    }*/
    uint8_t disp_brightness; // 1 (min) to 8 (max)
    bool disp_mode;         // 0 (8 segments) or 1 (7 segments)
    bool disp_power;        // 0 (off) or 1 (on)
};

#endif
