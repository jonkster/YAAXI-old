#include "./Hardware.h"

bool lightState[]		= { false, false, false, false, false, false };
unsigned int ledLookup[]	= { LED0, LED1, LED2, LED3, LED4, LED5 };
unsigned int switchState[]	= { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned int switchLookup[]	= { TOGGLE0, TOGGLE1, PUSH0, PUSH1, PUSH2, PUSH3, TWIST0, TWIST1 };
bool switchChanged[]		= { false, false, false, false, false, false, false, false };

// the setup routine runs once when you press reset:
void setPinModes() {                
        // Set encoder pins
        pinMode (inputCLK0, INPUT);
        pinMode (inputDT0,  INPUT);
        pinMode (inputCLK0, INPUT_PULLUP);
        pinMode (inputDT0,  INPUT_PULLUP);
        pinMode (inputCLK1, INPUT);
        pinMode (inputDT1,  INPUT);
        pinMode (inputCLK1, INPUT_PULLUP);
        pinMode (inputDT1,  INPUT_PULLUP);

        // pins for switches
        pinMode (TOGGLE0, INPUT);
        pinMode (TOGGLE1, INPUT);
        pinMode (PUSH0, INPUT);
        pinMode (PUSH1, INPUT);
        pinMode (PUSH2, INPUT);
        pinMode (PUSH3, INPUT);
        pinMode (TWIST0, INPUT);
        pinMode (TWIST1, INPUT);

        // pins for LEDS
        pinMode (LED0, OUTPUT);
        pinMode (LED1, OUTPUT);
        pinMode (LED2, OUTPUT);
        pinMode (LED3, OUTPUT);
        pinMode (LED4, OUTPUT);
        pinMode (LED5, OUTPUT);

	// setup rotary encoders
	setRotPins(0, A0, A1);
	setRotPins(1, A2, A3);

}

int ledCount() {
	return sizeof(ledLookup)/sizeof(ledLookup[0]);
}

int switchCount() {
	return sizeof(switchLookup)/sizeof(switchLookup[0]);
}

void updateSwitchStates() {
	for (int i = 0; i < switchCount(); i++) {
		int pin = switchLookup[i];
		unsigned int v = digitalRead(pin);
		if (v != switchState[i]) {
			switchState[i] = v;
			switchChanged[i] = true;
		}
	}
}

unsigned int getSwitchState(const int switchNum) {
	return switchState[switchNum];
}

void showRotaryEncoderValues() {
	if (rotEncoderMoved(0)) {
		Serial.print("re0="); Serial.println(getCounter(0));
	}
	if (rotEncoderMoved(1)) {
		Serial.print("re1="); Serial.println(getCounter(1));
	}
}

// turn switch name to an index #
byte lookupSwitchIdx(const byte switchName) {
	switch (switchName) {
		case TOGGLE0:
			return 0;
			break;
		case TOGGLE1:
			return 1;
			break;
		case PUSH0:
			return 2;
			break;
		case PUSH1:
			return 3;
			break;
		case PUSH2:
			return 4;
			break;
		case PUSH3:
			return 5;
			break;
		case TWIST0:
			return 6;
			break;
		case TWIST1:
			return 7;
			break;
		default:
			break;
	}
	return 255;
}

// check value of switch (by NAME not idx)
bool hasSwitchChanged(const unsigned int switchName) {
	int idx = lookupSwitchIdx(switchName);
	if (idx == 255) {
		return false;
	}
	if (switchChanged[idx]) {
		switchChanged[idx] = false;
		return true;
	}
	return false;
}

// return value of switch (by NAME not idx)
unsigned int switchValue(const unsigned int switchName) {
	int idx = lookupSwitchIdx(switchName);
	if (idx != 255) {
		switchChanged[idx] = false;
		return getSwitchState(idx);
	}
	return 0;
}

// set led based on state value (led index NOT name)
void lightAct(const int ledNum) {
	int pin = LED0;
	if (ledNum < ledCount()) {
		pin = ledLookup[ledNum];
	}
	digitalWrite(pin, lightState[ledNum]);
}

// returns if led lit or not, index by led number NOT pin name
bool getLed(const int ledNum) {
	int pin = LED0;
	if (ledNum < ledCount()) {
		pin = ledLookup[ledNum];
	}
	return digitalRead(pin);
}

// set led, index by led number NOT pin name
void lightSet(const int ledNum, const bool onoff) {
	if (ledNum >= ledCount()) {
		return;
	}
	if (lightState[ledNum] != onoff) {
		lightState[ledNum] = onoff;
	}
	lightAct(ledNum);
}

// visually check leds all work
void lightTest(const int delayms) {
	for (int i = 0; i < ledCount(); i++) {
		toggleLightState(i);
		delay(delayms);
	}
	delay(2 * delayms);
	for (int i = 0; i < ledCount(); i++) {
		toggleLightState(i);
		delay(delayms);
	}
}

// toggle leds, index by led number NOT name
void toggleLightState(const int ledNum) {
	lightState[ledNum] = ! lightState[ledNum];
	lightAct(ledNum);
}

void showPingLight() {
	lightTest(50);
}

void testSwitchAndLeds() {
	if (hasSwitchChanged(TOGGLE0)) {
		Serial.println("Toggle 0 changed");
		toggleLightState(0);
	}
	if (hasSwitchChanged(TOGGLE1)) {
		Serial.println("Toggle 1 changed");
		toggleLightState(1);
	}
	if (hasSwitchChanged(PUSH0)) {
		Serial.println("Pushbutton 0 changed");
		toggleLightState(2);
	}
	if (hasSwitchChanged(PUSH1)) {
		Serial.println("Pushbutton 1 changed");
		toggleLightState(3);
	}
	if (hasSwitchChanged(PUSH2)) {
		Serial.println("Pushbutton 2 changed");
		toggleLightState(4);
	}
	if (hasSwitchChanged(PUSH3)) {
		Serial.println("Pushbutton 3 changed");
		toggleLightState(0);
	}
	if (hasSwitchChanged(TWIST0)) {
		Serial.println("Twist switch 0 changed");
		toggleLightState(1);
	}
	if (hasSwitchChanged(TWIST1)) {
		Serial.println("Twist switch 1 changed");
		toggleLightState(2);
	}
	showRotaryEncoderValues();
}
