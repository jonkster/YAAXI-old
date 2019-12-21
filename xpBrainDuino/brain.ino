/*
 */


#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size

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
#define TOGGLE0    8	
#define TOGGLE1    9	
#define PUSH0    A4	
#define PUSH1    A5	
#define TWIST0    6	
#define TWIST1    7	


byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
unsigned int localPort = 8888;
bool verbose = false;


// rot encoder variables
int rotCount0 = 0; 
int previousClk0; 
int rotCount1 = 0; 
int previousClk1; 
int lastRotEncTime0;
int lastRotEncTime1;


// led outputs
int lastTime = 0;
bool ledState = false;

// switch inputs
bool toggleState0;
bool toggleState1;
bool pushState0;
bool pushState1;
bool twistState0;
bool twistState1;


// ----------------------------------------------------------------------------------------------------
//	g = get: XP ------> Arduino (eg display airspeed on Arduino device - Get From XP)
//	s = set: Arduino -> XP      (eg take switch setting on Arduino and set value in XP - Set In XP)
char *confValues[] =  {
		(char*)"R:sim/cockpit/switches/pitot_heat_on:i:s",
		(char*)"R:sim/cockpit/electrical/nav_lights_on:i:g",
		(char*)"R:sim/cockpit/engine/fuel_pump_on:vi:s",
		(char*)"R:sim/cockpit/engine/fuel_tank_selector:i:s",
		(char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:i:s",
		(char*)"R:sim/cockpit/autopilot/heading:f:s"
};
// ----------------------------------------------------------------------------------------------------

int fuelPumpState = 0;
int gearUnsafeState = -1;


// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP	Udp;
int		clock;

void replyPing();
void parseData(char*); 
void sendConf();
void handleXPData(const char*, const char*);

int bugHdg0 = 0;
int bugHdg1 = 0;

void setup() {
	Ethernet.begin(mac,ip);
	Udp.begin(localPort);
	Serial.begin(115200);

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
	pinMode (TWIST0, INPUT);
	pinMode (TWIST1, INPUT);

	// pins for LEDS
	pinMode (LED0, OUTPUT);
	pinMode (LED1, OUTPUT);
	pinMode (LED2, OUTPUT);
	pinMode (LED3, OUTPUT);

	// Read the initial state of Inputs
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
		// work out if clockwise or counter clockwise
		if (currentDT != currentClk) { 
		} else {
			delta = - delta;
		}
	} 
	return delta;
}

void setLeds() {
	//digitalWrite(LED1, ledState);
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
	/*if (twistState0 != digitalRead(TWIST0)) {
		twistState0 = !twistState0;
		Serial.print("twist switch 0: ");
		Serial.println(twistState0);
	}
	if (twistState1 != digitalRead(TWIST1)) {
		twistState1 = !twistState1;
		Serial.print("twist switch 1: ");
		Serial.println(twistState1);
	}*/

}

void loop() {
	// if there's data available, read a packet
	int packetSize = Udp.parsePacket();
	if(packetSize)
	{
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
				sendConf();
			} else {
				Udp.write("Unknown Command:");
				Udp.write(packetBuffer);
			}
			Udp.endPacket();
		}
	}


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
		bugHdg0 = hdg;
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
		bugHdg1 = hdg;
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


void replyPing() {
	char value[] = "Yes Hello!";
	Udp.write(value);
	Serial.print("replied: ");
	Serial.println(value);
}

void sendConf() {
	int count = sizeof (confValues) / sizeof (const char *);
	for (int i = 0; i < count; i++) {
		Udp.write(confValues[i]);
		Udp.write("\n");
		Serial.print("sent value: ");
		Serial.println(confValues[i]);
	}
}

void parseData(char* msg) {
	char delim[] = ":";
	char *ptr = strtok(msg, delim);
	char regIdx[4];
	char value[64];
	if (ptr != NULL) {
		if ((strncmp(ptr, "S", 1) == 0) || (strncmp(ptr, "G", 1) == 0)) {
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(regIdx, ptr, sizeof(regIdx) - 1);
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					strncpy(value, ptr, sizeof(value) - 1);
					handleXPData(regIdx, value);					
				}
			}
		}
	}
}

void getPitotHeatSwitchPos(const int idx, const char* value) {
	int s = digitalRead(TOGGLE0);
	char buf[32];
	snprintf(buf, 32, "S:%i:%i", idx, s);
	Udp.write(buf);
	//Serial.print("Set X-Plane Pitot Heat Switch to:");
	//Serial.println(value);
}

void setXPHeadingBug(const int idx, const char* value) {
	char buf[32];
	snprintf(buf, 32, "S:%i:%i", idx, bugHdg0);
	Udp.write(buf);
}

void getXPGearUnsafe(const int idx, const char* value) {
	int v = atoi(value);
	if (v != gearUnsafeState) {
		if (v == 0) { 
			digitalWrite(LED0, HIGH);
			digitalWrite(LED1, LOW);
			Serial.print("gear safe");
		} else {
			digitalWrite(LED0, LOW);
			digitalWrite(LED1, HIGH);
			Serial.print("gear unsafe");
		}
	}
	gearUnsafeState = v;
	char buf[32];
	snprintf(buf, 32, "OK:%i", idx);
	Udp.write(buf);
}

void setNavLight(const int idx, const char* value) {
	int v = atoi(value);
	if (v == 1) { 
		digitalWrite(LED2, HIGH);
	} else {
		digitalWrite(LED2, LOW);
	}
	char buf[32];
	snprintf(buf, 32, "OK:%i", idx);
	Udp.write(buf);
}

void setXPFuelPump(const int idx, const char *value) {
	char buf[32];
	int newFuelPumpState = 0;
	if (toggleState0) {
		newFuelPumpState = 2;
	} else if (toggleState1) {
		newFuelPumpState = 1;
	}
	if (fuelPumpState != newFuelPumpState) {
		fuelPumpState = newFuelPumpState;
		Serial.print("setting fp to: "); Serial.println(fuelPumpState);
		snprintf(buf, 32, "S:%i:[%i, 0, 0, 0, 0, 0, 0]", idx, fuelPumpState);
	} else {
		snprintf(buf, 32, "OK:%i", idx);
	}
	Udp.write(buf);
}

void setXPFuelTank(const int idx, const char *value) {
	char buf[32];
	if ((twistState0 != digitalRead(TWIST0)) || (twistState1 != digitalRead(TWIST1))) {
		twistState0 = digitalRead(TWIST0);
		twistState1 = digitalRead(TWIST1);
		int s = 0;
		if (twistState0 == 0) {
			s = 1;
		} else if (twistState1 == 0) {
			s = 3;
		}
		snprintf(buf, 32, "S:%i:%i", idx, s);
	} else {
		snprintf(buf, 32, "OK:%i", idx);
	}
	Udp.write(buf);
}

void handleXPData(const char* regIdx, const char* value) {
	int idx = atoi(regIdx);
	switch (idx) {
		case 0:
			getPitotHeatSwitchPos(idx, value);
			break;
		case 1:
			setNavLight(idx, value);
			break;
		case 2:
			setXPFuelPump(idx, value);
			break;
		case 3:
			setXPFuelTank(idx, value);
			break;
		case 4:
			getXPGearUnsafe(idx, value);
			break;
		case 5:
			setXPHeadingBug(idx, value);
			break;
		default:
			Serial.print("no handler for #"); Serial.println(regIdx);
			break;
	}
}
