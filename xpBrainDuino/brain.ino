/*
 */


#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size



byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 177);
unsigned int localPort = 8888;

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP	Udp;
int		clock;

void replyPing();
void getData(char*); 
void setData(char*);
void sendConf();

void setup() {
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
  Serial.begin(115200);
  clock = 0;
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
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

    // read the packet into packetBufffer
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    if (strncmp(packetBuffer, "Hello", strlen(packetBuffer)) == 0) {
	replyPing();
    } else if (strncmp(packetBuffer, "SetData", 7) == 0) {
	setData(packetBuffer);
    } else if (strncmp(packetBuffer, "GetData", 7) == 0) {
	getData(packetBuffer);
    } else if (strncmp(packetBuffer, "Register", 8) == 0) {
	sendConf();
    } else {
    	Udp.write("Unknown Command:");
    	Udp.write(packetBuffer);
    }
    Udp.write("\n");
    Udp.endPacket();
  } if (clock++ % 100) {
  }
  delay(10);
}


void replyPing() {
	char value[] = "Yes Hello!";
	Udp.write(value);
    	Serial.print("replied: ");
    	Serial.println(value);
}

void sendConf() {
	char *value[] =  {
		(char*)"R:sim/cockpit/engine/fuel_pump_on:w",               // arduino wants to set XP fuel pump
		(char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:r", // arduino wants to read XP gear unsafe annunciator
		(char*)"R:sim/cockpit/autopilot/autopilot_mode:w"          // arduino wants to set XP autopilot mode
	};
	int count = sizeof (value) / sizeof (const char *);
	for (int i = 0; i < count; i++) {
		Udp.write(value[i]);
		Serial.print("sent value: ");
		Serial.println(value[i]);
	}
}

void getData(char* source) {
	char value[] = "123";
	Udp.write("V:");
	Udp.write(source);
	Udp.write("=");
	Udp.write(value);
    	Serial.print("sent value: ");
	Serial.write("V:");
	Serial.write(source);
	Serial.write("=");
    	Serial.println(value);
}

void setData(char* destination) {
	Udp.write("OK");
    	Serial.print("set value: ");
    	Serial.println(destination);
}
