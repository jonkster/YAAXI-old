/*
 */


#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size



byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
unsigned int localPort = 8888;
bool verbose = false;

#define SWITCH_PITOT_HEAT_PIN 8
#define LED 9

// ----------------------------------------------------------------------------------------------------
//	g = get: XP ------> Arduino (eg display airspeed on Arduino device - Get From XP)
//	s = set: Arduino -> XP      (eg take switch setting on Arduino and set value in XP - Set In XP)
char *confValues[] =  {
		(char*)"R:sim/cockpit/switches/pitot_heat_on:i:s",
		(char*)"R:sim/cockpit/switches/pitot_heat_on:i:g",
		(char*)"R:sim/cockpit/radios/nav1_freq_hz:i:g"
};
// ----------------------------------------------------------------------------------------------------

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP	Udp;
int		clock;

void replyPing();
void parseData(char*); 
void sendConf();
void handleXPData(const char*, const char*);

void setup() {
	Ethernet.begin(mac,ip);
	Udp.begin(localPort);
	Serial.begin(115200);
	clock = 0;
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH);
	pinMode(SWITCH_PITOT_HEAT_PIN, INPUT);
}

void loop() {
	clock++;
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
	delay(1);
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
	/*int s = digitalRead(SWITCH_PITOT_HEAT_PIN);
	Udp.write(idx);
	Udp.write(":");
	Udp.write(s);
	Udp.write("\n");
	Serial.print("Sending heat switch pos to XP:");
	Serial.println(s);*/
	Udp.write(idx);
	Udp.write("OK\n");
}

void setPitotHeatLight(const int idx, const char* value) {
	int v = atoi(value);
	if (v == 1) { 
		digitalWrite(LED, HIGH);
		Serial.println("Turn my Pitot Heat Light On");
	} else {
		digitalWrite(LED, LOW);
		Serial.println("Turn my Pitot Heat Light Off");
	}
	Udp.write(idx);
	Udp.write("OK\n");
}

void setNavRadioFreq(const int idx, const char *value) {
	Serial.print("Set my Nav Radio Freq:");
	Serial.println(value);
	Udp.write(idx);
	Udp.write("OK\n");
}

void handleXPData(const char* regIdx, const char* value) {
	int idx = atoi(regIdx);
	switch (idx) {
		case 0:
			getPitotHeatSwitchPos(idx, value);
			break;
		case 1:
			setPitotHeatLight(idx, value);
			break;
		case 2:
			setNavRadioFreq(idx, value);
			break;
		default:
			Serial.print("no handler for #"); Serial.println(regIdx);
			break;
	}
}
