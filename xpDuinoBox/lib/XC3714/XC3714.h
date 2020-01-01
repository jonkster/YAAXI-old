#ifndef __XC3714_H__
#define __XC3714_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>


#define MAX7219DIN 4
#define MAX7219CS 5
#define MAX7219CLK 6

#define NOOP_REG        0x00
#define DIGIT0_REG	0x01
#define DIGIT1_REG	0x02
#define DIGIT2_REG	0x03
#define DIGIT3_REG	0x04
#define DIGIT4_REG	0x05
#define DIGIT5_REG	0x06
#define DIGIT6_REG	0x07
#define DIGIT7_REG	0x08
#define DECODE_REG      0x09
#define BRIGHTNESS_REG	0x0A
#define SCAN_LIMIT_REG  0x0B
#define SHUTDOWN_REG    0x0C
#define TESTMODE_REG	0x0F

const byte DP = 0b10000000;

void clockToggle();

void MAX7219senddata(byte reg, byte data);

void MAX7219brightness(byte b);

void blankDigit(const int);

void MAX7219init();

void MAX7219shownum(unsigned long, const int);

#ifdef __cplusplus
}
#endif
#endif
