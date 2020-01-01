#include "./Communication.h"
#include "./Hardware.h"
#include "./XPlane.h"

#define MAX_TASKS 10
unsigned long timeNow[MAX_TASKS];


void taskUpdateSwitches();
void taskUpdateRotaryEncoders();
void taskToggleStatusLed();
void registerXP();

// the setup routine runs once when you press reset:
void setup() {                
  setPinModes();
  lightTest(100);
  serialSetup();
  setupEthernet();
  registerXP();
  for (int i = 0; i < MAX_TASKS; i++) {
	  timeNow[i] = millis();
  }
}


// the loop routine runs over and over again forever:

int statusDelayMs = 500;
int switchDelayMs = 20;
int rotEncDelayMs = 1;
int netDelayMs = 10;
unsigned long i = 0;
void loop() {
	unsigned long t = millis();
	if (t > timeNow[0] + statusDelayMs) {
		taskToggleStatusLed();
		timeNow[0] = t;
	}

	if (t > timeNow[1] + switchDelayMs) {
		taskUpdateSwitches();
		timeNow[1] = t;
	}

	if (t > timeNow[2] + rotEncDelayMs) {
		taskUpdateRotaryEncoders();
		timeNow[2] = t;
	}

	if (t > timeNow[3] + netDelayMs) {
		updateComms();
		timeNow[3] = t;
	}
}

void taskUpdateSwitches() {
	updateSwitchStates();
	//testSwitchAndLeds();
}

void taskUpdateRotaryEncoders() {
	updateRot();
}

void taskToggleStatusLed() {
	toggleLightState(5);
}

void registerXP() {
	newEntry((char*)"R:sim/cockpit/engine/fuel_pump_on:vi:w", &setXPFuelPump);
	newEntry((char*)"R:sim/cockpit/engine/fuel_tank_selector:i:w", &setXPFuelTank);
	newEntry((char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:i:r", &getXPGearUnsafe);
	newEntry((char*)"R:sim/aircraft/parts/acf_gear_deploy:vf:r", &getXPGearLocked);
	newEntry((char*)"R:sim/cockpit/autopilot/heading:f:w", &setXPHeadingBug);
	newEntry((char*)"R:sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot:f:w", &setXPOBS);
	newEntry((char*)"R:sim/cockpit/radios/nav1_freq_hz:i:w", &getXPNav1ActiveFreq);
	newEntry((char*)"R:sim/cockpit/radios/nav1_stdby_freq_hz:i:w", &getXPNav1StandbyFreq);
}
