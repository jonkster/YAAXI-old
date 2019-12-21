#include <SPI.h>

// Rotary Encoder Inputs
#define inputCLK0 A0
#define inputDT0 A1
#define inputCLK1 A2
#define inputDT1 A3

//Leds
#define LED0    2	
#define LED1    3	
#define LED2    4	
#define LED3    5	

// Switches
#define TOGGlE0    8	
#define TOGGLE1    9	
#define PUSH0    A4	
#define PUSH1    A5	
#define TWIST0    6	
#define TWIST1    7	

int rotCount0 = 0; 
int previousClk0; 
int rotCount1 = 0; 
int previousClk1; 
int lastRotEncTime0;
int lastRotEncTime1;


int lastTime = 0;
bool ledState = false;
bool toggleState0;
bool toggleState1;
bool pushState0;
bool pushState1;
bool twistState0;
bool twistState1;



void setup() { 

	// Set encoder pins as inputs  
	pinMode (inputCLK0, INPUT);
	pinMode (inputDT0,  INPUT);
	pinMode (inputCLK1, INPUT);
	pinMode (inputDT1,  INPUT);

	pinMode (TOGGLE0, INPUT);
	pinMode (TOGGLE1, INPUT);
	pinMode (PUSH0, INPUT);
	pinMode (PUSH1, INPUT);
	pinMode (TWIST0, INPUT);
	pinMode (TWIST1, INPUT);

	pinMode (LED0, OUTPUT);
	pinMode (LED1, OUTPUT);
	pinMode (LED2, OUTPUT);
	pinMode (LED3, OUTPUT);

	// Setup Serial Monitor
	Serial.begin (115200);

	// Read the initial state of inputCLK
	// Assign to previousStateCLK variable
	previousClk0 = digitalRead(inputCLK0);
	previousClk1 = digitalRead(inputCLK1);
	lastRotEncTime0 = millis();
	lastRotEncTime1 = millis();

	toggleState0 = digitalRead(TOGGLE0);
	toggleState1 = digitalRead(TOGGLE1);
	pushState0 = digitalRead(PUSH0);
	pushState1 = digitalRead(PUSH1);
	twistState0 = digitalRead(TWIST0);
	twistState1 = digitalRead(TWIST1);


	lastTime = millis() / 1000;
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
		// If the inputDT state is different than the inputCLK state then 
		if (currentDT != currentClk) { 
			// the encoder is rotating counterclockwise
			delta = - delta;
		} else {
			// Encoder is rotating clockwise
		}
	} 
	return delta;
}

void setLeds() {
	digitalWrite(LED0, ledState);
	digitalWrite(LED1, ledState);
	digitalWrite(LED2, !ledState);
	digitalWrite(LED3, !ledState);
	//digitalWrite(led4, ledState);
	//digitalWrite(led5, ledState);
	ledState = !ledState;
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
	if (pushState0 != digitalRead(PUSH0)) {
		pushState0 = !pushState0;
		Serial.print("push switch 0: ");
		Serial.println(pushState0);
	}
	if (pushState1 != digitalRead(PUSH1)) {
		pushState1 = !pushState1;
		Serial.print("push switch 1: ");
		Serial.println(pushState1);
	}

}

void readTwists() {
	if (twistState0 != digitalRead(TWIST0)) {
		twistState0 = !twistState0;
		Serial.print("twist switch 0: ");
		Serial.println(twistState0);
	}
	if (twistState1 != digitalRead(TWIST1)) {
		twistState1 = !twistState1;
		Serial.print("twist switch 1: ");
		Serial.println(twistState1);
	}

}

void loop() { 

	// read encoder 0
	int currentClk = digitalRead(inputCLK0);
	int currentDT = digitalRead(inputDT0);
	int delta = readEncoder(currentClk, currentDT, previousClk0, millis() - lastRotEncTime0);
	if (delta != 0) {
		lastRotEncTime0 = millis();
		rotCount0 += delta;
		int hdg = rotCount0 % 360;
		if (hdg < 1) {
			hdg += 360;
		}
		Serial.print("Heading Bug 0: ");
		Serial.println(hdg);
		previousClk0 = currentClk; 
	}

	// read encoder 1
	currentClk = digitalRead(inputCLK1);
	currentDT = digitalRead(inputDT1);
	delta = readEncoder(currentClk, currentDT, previousClk1, millis() - lastRotEncTime1);
	if (delta != 0) {
		lastRotEncTime1 = millis();
		rotCount1 += delta;
		int hdg = rotCount1 % 360;
		if (hdg < 1) {
			hdg += 360;
		}
		Serial.print("Heading Bug 1: ");
		Serial.println(hdg);
		previousClk1 = currentClk; 
	}

	int now = millis() / 1000;
	if (now - lastTime > 1) {
		setLeds();
		lastTime = now;
	}

	readToggles();	
	readPushes();	
	readTwists();


}
