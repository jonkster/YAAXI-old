/*
 */
#include <Arduino.h>
#include "config.h"
#include "brain.h"
#include "hardware.h"

void registerXP();

void setup() {
	Ethernet.begin(mac,ip);
	Udp.begin(localPort);
	Serial.begin(115200);

	initPins();
	lastTime = millis() / 1000;
	registerXP();
	showLights();
}



void dumpQueue(const int idx) {
	if (sendIdx > 0) {
		sendIdx--;
		Udp.write(sendQueue[sendIdx]);
	} else {
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "O");
		Udp.write(buf);
	}
}

void queueUdp(const char* msg) {
	strncpy(sendQueue[sendIdx], msg, sizeof(sendQueue[sendIdx]) - 1);	
	sendIdx++;
	if (sendIdx > UDP_QUEUE_SIZE) {
		Serial.println("udp queue limit!");
		sendIdx--;
	}
}


int bugHdg0 = 0;
bool bug0Changed = false;
int bugHdg1 = 0;
bool bug1Changed = false;
void loop() {
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if(packetSize)
	{
		char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
		memset(packetBuffer,0,sizeof(packetBuffer));
		if (verbose) {
			Serial.print("Received packet of size ");
			Serial.println(packetSize);
			Serial.print("From ");
			IPAddress remote = Udp.remoteIP();
			for (int i =0; i < 4; i++)
			{
				Serial.print(remote[i], DEC);
				if (i < 3)
				{
					Serial.print(".");
				}
			}
			Serial.print(", port ");
			Serial.println(Udp.remotePort());
		}

		// read the packet into packetBufffer
		Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
		if (verbose) {
			Serial.println("Contents:");
			Serial.println(packetBuffer);
		}
		// send a reply
		if (strlen(packetBuffer) > 0) {
			Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
			if (strncmp(packetBuffer, "Hello", strlen(packetBuffer)) == 0) {
				replyPing();
			} else if (strncmp(packetBuffer, "S:", 2) == 0) {
				parseData(packetBuffer);
			} else if (strncmp(packetBuffer, "G:", 2) == 0) {
				parseData(packetBuffer);
			} else if (strncmp(packetBuffer, "Register", 8) == 0) {
				confResponse(Udp);
			} else {
				char buf[32];
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf)-1, "Unknown command: %s", packetBuffer);
				Udp.write(buf);
			}
			Udp.endPacket();
		}

	}


	// read encoder 0
	int currentClk = digitalRead(inputCLK0);
	int currentDT = digitalRead(inputDT0);
	int delta = 0;
	delta = readEncoder(currentClk, currentDT, previousClk0, previousDT0, millis() - lastRotEncTime0);
	previousClk0 = currentClk; 
	if (delta != 0) {
		bug0Changed = true;
		lastRotEncTime0 = millis();
		rotCount0 += delta;
		int hdg = rotCount0 % 360;
		if (hdg < 1) {
			hdg += 360;
		}
		bugHdg0 = hdg;
		Serial.println(bugHdg0);
	}

	// read encoder 1
	currentClk = digitalRead(inputCLK1);
	currentDT = digitalRead(inputDT1);
	delta = readEncoder(currentClk, currentDT, previousClk1, previousDT1, millis() - lastRotEncTime1);
	previousClk1 = currentClk; 
	if (delta != 0) {
		bug1Changed = true;
		lastRotEncTime1 = millis();
		rotCount1 += delta;
		int hdg = rotCount1 % 360;
		if (hdg < 1) {
			hdg += 360;
		}
		bugHdg1 = hdg;
		Serial.println(bugHdg1);
	}

	readToggles();	
	readPushes();	

}


void replyPing() {
	char value[] = "Yes Hello!\n";
	Udp.write(value);
	if (verbose) {
		Serial.print("replied: ");
		Serial.println(value);
	}
	showLights();
}

void parseData(const char* msg) {
	char delim[] = ":";
	char buf[32];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, msg, sizeof(buf) - 1);
	char *ptr = strtok(buf, delim);
	char regIdx[4];
	memset(regIdx, 0, sizeof(regIdx));
	char value[32];
	memset(value, 0, sizeof(value));
	if (ptr != NULL) {
		if ((strncmp(ptr, "S", 1) == 0) || (strncmp(ptr, "G", 1) == 0)) {
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(regIdx, ptr, sizeof(regIdx) - 1);
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					strncpy(value, ptr, sizeof(value) - 1);
					handleXPData(regIdx, value);					
					return;
				}
			}
		}
	}
	Udp.write("parse failed");
	return;
}


void handleXPData(const char* regIdx, const char* value) {
	const int idx = atoi(regIdx);
	if (verbose) {
		Serial.print("handle request #: "); Serial.print(regIdx); Serial.print(" v:"); Serial.println(value);
	}
	drConfig entry = getHandler(idx);
	if (verbose) {
		displayItem(entry);
	}
	if (entry.id >= 0) {
		entry.handler(idx, value);
	}
	dumpQueue(idx);
}


// ------------------ Configuration/Handlers --------------------------------
int navLightState = 0;
void setNavLight(const int idx, const char* value) {
	int v = atoi(value);
	if (v != navLightState) {
		Serial.println("change detected");
		if (v == 1) { 
			digitalWrite(LED2, HIGH);
		} else {
			digitalWrite(LED2, LOW);
		}
		navLightState = v;
	}
}

int fuelPumpState = 0;
void setXPFuelPump(const int idx, const char *value) {
	int newFuelPumpState = 0;
	if (toggleState0) {
		newFuelPumpState = 2;
	} else if (toggleState1) {
		newFuelPumpState = 1;
	}

	if (fuelPumpState != newFuelPumpState) {
		fuelPumpState = newFuelPumpState;
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:[%i, 0, 0, 0, 0, 0, 0]", idx, fuelPumpState);
		queueUdp(buf);
	}
}

void setXPFuelTank(const int idx, const char *value) {
	if ((twistState0 != digitalRead(TWIST0)) || (twistState1 != digitalRead(TWIST1))) {
		twistState0 = digitalRead(TWIST0);
		twistState1 = digitalRead(TWIST1);
		int s = 0;
		if (twistState0 == 0) {
			s = 1;
		} else if (twistState1 == 0) {
			s = 3;
		}
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:%i", idx, s);
		queueUdp(buf);
	}
}

int gearUnsafeState = -1;
void getXPGearUnsafe(const int idx, const char* value) {
	int v = atoi(value);
	if (v != gearUnsafeState) {
		if (v == 0) { 
			digitalWrite(LED1, LOW);
		} else {
			digitalWrite(LED1, HIGH);
		}
	}
	gearUnsafeState = v;
}

bool gearLockedDown = false;
void getXPGearLocked(const int idx, const char* value) {
	// value looks like: [0.5,0.5,0.5....]
	char delim[] = ", ";
	char arraySt[64];
	char *g[3];
	strncpy(arraySt, value + 1, sizeof(arraySt) - 1);
	char *ptr = strtok(arraySt, delim);
	if (ptr != NULL) {
		g[0] = ptr;
		ptr = strtok(NULL, delim);
		if (ptr != NULL) {
			g[1] = ptr;
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				g[2] = ptr;
				if ((strncmp(g[0], "1.0", 3) == 0) && (strncmp(g[1], "1.0", 3) == 0) && (strncmp(g[2], "1.0", 3) == 0)) {
					gearLockedDown = true;
					digitalWrite(LED0, HIGH);
					return;
				} else {
					// gear not fully extended
				}
			}
		}
	}
	gearLockedDown = false;
	digitalWrite(LED0, LOW);
}

void setXPHeadingBug(const int idx, const char* value) {
	if (bug0Changed) {
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:%i", idx, bugHdg0);
		queueUdp(buf);
		bug0Changed = false;
	}
}

void setXPOBS(const int idx, const char* value) {
	if (bug1Changed) {
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:%i", idx, bugHdg1);
		queueUdp(buf);
		bug1Changed = false;
	}
}

int nav1AFreq = 11090;
int nav1SFreq = 11090;

void toggleXPNavFreqs(const int sfreq, const int afreq) {
	nav1AFreq = afreq;
	nav1SFreq = sfreq;
	char buf[32];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf)-1, "S:%i:%i", 5, nav1AFreq);
	queueUdp(buf);
	snprintf(buf, sizeof(buf)-1, "S:%i:%i", 6, nav1SFreq);
	queueUdp(buf);
}

void getXPNav1ActiveFreq(const int idx, const char* value) {
	int f = atoi(value);
	if (f != nav1AFreq) {
		nav1AFreq = f;
		Serial.print("act=");
		Serial.println(f);
	}
	/*if (toggleNAV) {
		Serial.println("toggle Nav!");
		toggleNAV = false;
	//	toggleXPNavFreqs(nav1AFreq, nav1SFreq);
	}*/
}

void getXPNav1StandbyFreq(const int idx, const char* value) {
	int f = atoi(value);
	if (f != nav1SFreq) {
		nav1SFreq = f;
		Serial.print("sby=");
		Serial.println(f);
	}
}


void registerXP() {
	newEntry((char*)"R:sim/cockpit/electrical/nav_lights_on:i:s", &setNavLight);
	newEntry((char*)"R:sim/cockpit/engine/fuel_pump_on:vi:s", &setXPFuelPump);
	newEntry((char*)"R:sim/cockpit/engine/fuel_tank_selector:i:s", &setXPFuelTank);
	newEntry((char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:i:s", &getXPGearUnsafe);
	newEntry((char*)"R:sim/aircraft/parts/acf_gear_deploy:vf:g", &getXPGearLocked);
	newEntry((char*)"R:sim/cockpit/autopilot/heading:f:s", &setXPHeadingBug);
	newEntry((char*)"R:sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot:f:s", &setXPOBS);
	newEntry((char*)"R:sim/cockpit/radios/nav1_freq_hz:i:g", &getXPNav1ActiveFreq);
	newEntry((char*)"R:sim/cockpit/radios/nav1_stdby_freq_hz:i:g", &getXPNav1StandbyFreq);
}
