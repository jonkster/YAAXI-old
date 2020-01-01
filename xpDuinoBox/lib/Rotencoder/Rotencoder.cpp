#include "Rotencoder.h"

int encoderPins[MAX_ENCODERS][2] = {};	// pin assignments
byte encOldPairs[MAX_ENCODERS] = {};	// last time encoder was read
int counter[MAX_ENCODERS] = {};
int oldCounter[MAX_ENCODERS] = {};

byte readPinA(unsigned int encNumber) {
	if (encNumber >= MAX_ENCODERS) {
		return 0;
	}
	const int pin = encoderPins[encNumber][0];
	byte b = digitalRead(pin);
	return b;
}

byte readPinB(unsigned int encNumber) {
	if (encNumber >= MAX_ENCODERS) {
		return 0;
	}
	const int pin = encoderPins[encNumber][1];
	byte b = digitalRead(pin);
	return b;
}

const byte RotaryDecodeTable[4][4] = {
	{B00, B10, B01, B11},
	{B01, B00, B11, B10},
	{B10, B11, B00, B01},
	{B11, B01, B10, B00}
};

bool rotEncoderMoved(unsigned int encNumber) {
	if (encNumber >= MAX_ENCODERS) {
		return false;
	} else if (oldCounter[encNumber] == counter[encNumber]) {
		return false;
	}
	oldCounter[encNumber] = counter[encNumber];
	return true;
}

bool isUsedEncoder(const int encNumber) {
	int pinA = encoderPins[encNumber][0];
	int pinB = encoderPins[encNumber][1];
	// if assigned to encoder, pin #'s will differ
	return (pinA != pinB);
}

void updateRot() {
	for (int encNumber = 0; encNumber < MAX_ENCODERS; encNumber++) {
		if (isUsedEncoder(encNumber)) {
			byte newPairs = 0;
			newPairs = ( readPinA(encNumber) <<1 | readPinB(encNumber) ); // read the current state of encoder pins
			//if (newPairs != 0 && newPairs != 3) {
				//Serial.print("new:"); Serial.print(newPairs);
			//}

			byte oldPairs = encOldPairs[encNumber];
			//if (newPairs != 0 && newPairs != 3) {
				//Serial.print("  old:"); Serial.print(newPairs);
			//}
			byte encMove = RotaryDecodeTable[oldPairs][newPairs]; // used RotaryDecodeTable to decide movement, if any
			//if (newPairs != 0 && newPairs != 3) {
				//Serial.print("  decode:"); Serial.println(encMove);
			//}

			if (encMove == B01){ // if result was move right (CW), increment counter
				counter[encNumber]++;
			} else if (encMove == B10){ // if result was move left (anti-CW), decrement counter
				counter[encNumber]--;
			} else {
			}

			encOldPairs[encNumber] = newPairs;

		}
	}
}

bool setRotPins(const int encNumber, const int pinA, const int pinB) {
	if (encNumber >= MAX_ENCODERS) {
		return false;
	}
	encoderPins[encNumber][0] = pinA;
	encoderPins[encNumber][1] = pinB;
	pinMode(pinA, INPUT);
	pinMode(pinB, INPUT);
	pinMode(pinA, INPUT_PULLUP); // turn on internal pull-up resistor for each input
	pinMode(pinB, INPUT_PULLUP);
	encOldPairs[encNumber] = ( readPinA(encNumber) <<1 | readPinB(encNumber) ); // read the current state of encoder pins
	return true;
}

int getCounter(unsigned int encNumber) {
	if (encNumber < MAX_ENCODERS) {
		return counter[encNumber];
	}
	return 0;
}
