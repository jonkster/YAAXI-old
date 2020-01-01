#ifndef __ROTENCODER_H__
#define __ROTENCODER_H__


#include <Arduino.h>

#define MAX_ENCODERS 4

bool setRotPins(const int encNumber, const int pinA, const int pinB);

bool rotEncoderMoved(unsigned int encNumber);
void updateRot();
int getCounter(unsigned int encNumber);


#endif
