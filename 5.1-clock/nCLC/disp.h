// disp.h - interface for driver for the 303WIFILC01 display/TM1650
#ifndef _DISP_H_
#define _DISP_H_

// https://datasheet.lcsc.com/lcsc/1810281208_TM-Shenzhen-Titan-Micro-Elec-TM1650_C44444.pdf

/*
Brightness settings
MSB				LSB
B7 B6 B5 B4 B3 B2 B1 B0 Explanation
 ×  0  0  0     ×  ×	8 brightness
 ×  0  0  1     ×  ×	1 brightness
 ×  0  1  0     ×  ×	2 brightness
 ×  0  1  1     ×  ×	3 Brightness
 ×  1  0  0     ×  ×	4 brightness
 ×  1  0  1     ×  ×	5 brightness
 ×  1  1  0     ×  ×	6 brightness
 ×  1  1  1     ×  ×	7 brightness
On / off the display position
MSB				LSB
B7 B6 B5 B4 B3 B2 B1 B0 Explanation
 ×              ×  ×  0 Off Display
 ×              ×  ×  1 On Display
 */

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

class Disp303
{
private:
    enum Disp303TM1650pins : int8_t { SCL_PIN = 12, SDA_PIN = 13 };
    enum TM1650reg : uint8_t { 
        TM1650_CONTROL_BASE = 0x24,	// Address of the control register of the left-most digit
        TM1650_DISPLAY_BASE	= 0x34	// Address of the left-most digit
    };
public:
    Disp303(int bright = 4, bool segmode = false, bool power = false);
    void init();
    void setBrightness(uint8_t brightness);
    void setBrightness();
    uint8_t getBrightness() const;
    void setPower(bool power = true);
    bool getPower() const;
    void setMode(bool segmode = false);
    bool getMode() const;
    static void show(const char* s, uint8_t dots = 0);
    IRAM_ATTR static void setDigit(uint8_t d, uint8_t segs);

private:
    IRAM_ATTR static uint8_t write(uint8_t reg, uint8_t val);
    static const uint8_t dispFont[0x80] ;

    union DispSettings
    {
        struct //BitFieldSettings
        {
            unsigned char power : 1;
            unsigned char : 2;
            unsigned char mode : 1; // 0 : 8-bit, 1 : 7-bit
            unsigned char brightness : 3; // 1-8 : 001 (min) to (1)000 (max)
        };
        unsigned char all;
    } settings;

    /*
    struct DispSettings
    {
        unsigned char power : 1;
        unsigned char : 2;
        unsigned char mode : 1; // 0 : 8-bit, 1 : 7-bit
        unsigned char brightness : 3; // 1-8 : 001 (min) to (1)000 (max)
    } settings;
    */
};

#endif
