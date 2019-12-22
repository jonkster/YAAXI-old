#ifndef __HARDWARE_H__
#define __HARDWARE_H__

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

byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
unsigned int localPort = 8888;

int lastRotEncTime0;
int lastRotEncTime1;
int previousClk0; 
int previousClk1; 
void initPins() {
	// Set encoder pins
	pinMode (inputCLK0, INPUT);
	pinMode (inputDT0,  INPUT);
	pinMode (inputCLK1, INPUT);
	pinMode (inputDT1,  INPUT);

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

	// Read the initial state of Inputs
	previousClk0 = digitalRead(inputCLK0);
	previousClk1 = digitalRead(inputCLK1);
	lastRotEncTime0 = millis();
	lastRotEncTime1 = millis();

	toggleState0 = digitalRead(TOGGLE0);
	toggleState1 = digitalRead(TOGGLE1);
	push0Last = digitalRead(PUSH0);
	push1Last = digitalRead(PUSH1);
	push2Last = digitalRead(PUSH2);
	push3Last = digitalRead(PUSH3);
	toggleVHF = false;
	toggleNAV = false;
	twistState0 = digitalRead(TWIST0);
	twistState1 = digitalRead(TWIST1);

}

int readEncoder(const int currentClk, const int currentDT, const int previousClk, const int revTime) {
	int delta = 0;
	// If the previous and the current state of the inputCLK are different then a pulse has occured
	if (currentClk != previousClk){ 
		if (revTime < 2) {
			delta = 8;
		} else if (revTime < 20) {
			delta = 6;
		} else if (revTime < 50) {
			delta = 4;
		} else if (revTime < 100) {
			delta = 2;
		} else {
			delta = 1;
		}
		// work out if clockwise or counter clockwise
		if (currentDT != currentClk) { 
		} else {
			delta = - delta;
		}
	} 
	return delta;
}


void readToggles() {
	if (toggleState0 != digitalRead(TOGGLE0)) {
		toggleState0 = !toggleState0;
		Serial.print("toggle switch 0: ");
		Serial.println(toggleState0);
	}
	if (toggleState1 != digitalRead(TOGGLE1)) {
		toggleState1 = !toggleState1;
		Serial.print("toggle switch 1: ");
		Serial.println(toggleState1);
	}
}

void readPushes() {
	if (push0Last != digitalRead(PUSH0)) {
		push0Last = ! push0Last;
		if (! push0Last) {
			Serial.println("push switch 0: ");
			toggleVHF = true;
		}
	}
	if (push1Last != digitalRead(PUSH1)) {
		push1Last = ! push1Last;
		if (! push1Last) {
			Serial.println("push switch 1: ");
			toggleNAV = true;
		}
	}
	if (push2Last != digitalRead(PUSH2)) {
		push2Last = ! push2Last;
		if (! push2Last) {
			Serial.println("push switch 2: ");
		}
	}
	if (push3Last != digitalRead(PUSH3)) {
		push3Last = ! push3Last;
		if (! push3Last) {
			Serial.println("push switch 3: ");
		}
	}

}


void showLights() {
	for (int i = 0; i < 5; i++) {
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 0);
		delay(50);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 1);
		delay(50);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 1);
		digitalWrite(LED5, 0);
		delay(50);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 1);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 0);
		delay(50);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 1);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 0);
		delay(50);
		digitalWrite(LED0, 0);
		digitalWrite(LED1, 1);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 0);
		delay(50);
		digitalWrite(LED0, 1);
		digitalWrite(LED1, 0);
		digitalWrite(LED2, 0);
		digitalWrite(LED3, 0);
		digitalWrite(LED4, 0);
		digitalWrite(LED5, 0);
		delay(50);
	}
	digitalWrite(LED0, 1);
	digitalWrite(LED1, 1);
	digitalWrite(LED2, 1);
	digitalWrite(LED3, 1);
	digitalWrite(LED4, 1);
	digitalWrite(LED5, 1);
	delay(500);
	digitalWrite(LED0, 0);
	digitalWrite(LED1, 0);
	digitalWrite(LED2, 0);
	digitalWrite(LED3, 0);
	digitalWrite(LED4, 0);
	digitalWrite(LED5, 0);
}


#endif
