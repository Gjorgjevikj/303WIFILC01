// disp.cpp - driver for the 303WIFILC01 display/TM1650

#include <Arduino.h>
#include <Wire.h>
#include "disp.h"

Disp303::Disp303(int bright, bool segmode, bool disppow) 
{
    settings.brightness = bright;
    settings.mode = segmode;
    settings.power = disppow;
}

void Disp303::init()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    //int res = write(0x24, ((disp_brightness & 0b111) << 4) | (disp_mode << 3) | disp_power);
    int res = write(TM1650_CONTROL_BASE, *((unsigned char*)(&settings)));
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
