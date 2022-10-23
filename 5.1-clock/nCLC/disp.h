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

void disp_brightness_set(int brightness);       // Sets brightness level 1..8
int  disp_brightness_get();                     // Gets brightness level
void disp_power_set(int power);                 // Sets power 0 (off) or 1 (on)
int  disp_power_get();                          // Gets power level
                                              
void disp_init();                               // Initializes display (prints error to Serial)
void disp_show(const char * s, uint8_t dots=0); // Puts (first 4 chars of) `s` (padded with spaces) on display, using flags in `dots` for P

void disp_set(int d, uint8_t segs);             // Sets raw segments on the n-th digit of the display

#endif
