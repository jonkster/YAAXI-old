#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <Rotencoder.h>

//Leds
#define LED0    2
#define LED1    3
#define LED2    4
#define LED3    5
#define LED4    24
#define LED5    26

// Rotary Encoder Inputs
#define inputCLK0 A0
#define inputDT0 A1
#define inputCLK1 A2
#define inputDT1 A3

// Switches
#define TOGGLE0    8
#define TOGGLE1    9
#define PUSH0    A4
#define PUSH1    A5
#define PUSH2    28
#define PUSH3    30
#define TWIST0    6
#define TWIST1    7



// the setup routine runs once when you press reset:
void setPinModes();

void showPingLight();

void lightTest(const int delayms);

void lightAct(const int ledNum);

void lightSet(const int ledNum, const bool onoff);

bool hasSwitchChanged(const unsigned int switchName);

void updateSwitchStates();

void showRotaryEncoderValues();

void showTwistSwitch();

unsigned int switchValue(const unsigned int switchName);

unsigned int switchValue(const unsigned int switchName);

void toggleLightState(const int ledNum);

bool getLed(const int ledNum);

void testSwitchAndLeds();

#endif
