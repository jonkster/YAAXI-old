#include "./Communication.h"
#include "./Hardware.h"

byte mac[] =  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip( 192, 168, 0, 177 );
unsigned int localPort = 8888;
EthernetUDP	Udp;

int sendIdx = 0;

/* the array of handler details */
drConfig	drHandlers[MAX_HANDLERS];
int		drConfigIdx;


bool verbose = false;

void setupEthernet() {
	Ethernet.begin(mac, ip);
	Udp.begin(localPort);
	drConfigIdx = 0;
}

#define UDP_QUEUE_SIZE 10
char sendQueue[UDP_QUEUE_SIZE ][32];

void queueUdp(const char* msg) {
	strncpy(sendQueue[sendIdx], msg, sizeof(sendQueue[sendIdx]) - 1);	
	sendIdx++;
	if (sendIdx > UDP_QUEUE_SIZE) {
		Serial.println("udp queue limit!");
		sendIdx--;
	}
}

void dumpQueue() {
	if (sendIdx > 0) {
		sendIdx--;
		Udp.write(sendQueue[sendIdx]);
	} else {
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "O\n");
		Udp.write(buf);
	}
}



// an example handler callback function
void printIdxValue(const int idx, const char* value) {
	Serial.print("dummy handler - idx:");
	Serial.print(idx);
	Serial.print(", value:");
	Serial.println(value);
}

void addDataRefHandler(drConfig item) {
	if (drConfigIdx < MAX_HANDLERS) {
		item.id = drConfigIdx;
		drHandlers[drConfigIdx++] = item;
	}
	if (drConfigIdx >= MAX_HANDLERS) {
		Serial.println("Handler registrations @ maximum");
		drConfigIdx--;
	}
}

drConfig nullHandler = {
	-1,
	(char*)"no handler configured with this id",
	&printIdxValue,
	false
};

bool pathHasReplyFlag(const char* drPath) {
	char delim[] = ":";
	char buf[32];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, drPath, sizeof(buf) - 1);
	char *ptr = strtok(buf, delim);
	if (ptr != NULL) {
		// ptr should be R
		ptr = strtok(NULL, delim);
		if (ptr != NULL) {
			// ptr should be path
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				// ptr should be data type flag
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					// ptr should be reply (R or N) flag
					return (strncmp(ptr, "R", 1) == 0);
				}
			}
		}
	}
	return false;
}

drConfig getHandler(const int handlerId) {
	if (handlerId < drConfigIdx) {
		return drHandlers[handlerId];
	} else {
		Serial.println("no handler!");
		return nullHandler;
	}
}


bool handlerHasReplyFlag(const int idx) {
	drConfig handler = getHandler(idx);
	return handler.reply;
}


void newEntry(const char* drPath, const xpResponseHandler handler) {
	drConfig entry = { 
		-1,
		drPath,
		handler,
		pathHasReplyFlag(drPath)
	};

	if (drConfigIdx < MAX_HANDLERS) {
		entry.id = drConfigIdx;
		drHandlers[drConfigIdx++] = entry;
	} else {
		Serial.println("Max handlers registered, cannot add new handler: ");
		Serial.println(drPath);
	}
}


void displayItem(drConfig item) {
	Serial.print("#"); Serial.print(item.id);
	Serial.print(", dataref:"); Serial.print(item.drPath);
	Serial.print(", send reply:"); Serial.print(item.reply);
	void* cbPtr = (void*)item.handler;
	Serial.print(", callback:"); Serial.println((int)cbPtr);
}

void displayHandler(int id) {
	drConfig item = drHandlers[id];
	displayItem(item);
}

void displayHandlers() {
	for (int i = 0; i < drConfigIdx; i++) {
		displayHandler(i);
	}
}


void handleXPData(const char* regIdx, const char* value, const char* replyFlag) {
	const int idx = atoi(regIdx);
	drConfig entry = getHandler(idx);
	if (entry.id >= 0) {
		//Serial.print("handling: "); Serial.println(idx);
		entry.handler(idx, value);
	}
	dumpQueue();
}


void confResponse() {
	displayHandlers();
	for (int i = 0; i < drConfigIdx; i++) {
		drConfig item = drHandlers[i];
		const char* path = item.drPath;
		char msg[1024];
		snprintf(msg, sizeof(msg)-1, "D:%s\n", path);
		Udp.write(msg);
	}
	Udp.write("O\n");
}

void replyPing() {
	char value[] = "Yes Hello!\n";
	Udp.write(value);
	showPingLight();
}


void serialSetup() {
	Serial.begin(115200);
}


void parseData(const char* msg) {
	//Serial.print("parsing: "); Serial.println(msg);
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
		char replyFlag[4];
		strncpy(replyFlag, ptr, sizeof(replyFlag) - 1);
		if ((strncmp(replyFlag, "N", 1) == 0) || (strncmp(replyFlag, "R", 1) == 0)) {
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(regIdx, ptr, sizeof(regIdx) - 1);
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					strncpy(value, ptr, sizeof(value) - 1);
					handleXPData(regIdx, value, replyFlag);					
					return;
				}
			}
		}
	}
	return;
}

void updateComms() {
	int packetSize = Udp.parsePacket();
	// if there's data available, read a packet
	if(packetSize)
	{
		char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
		memset(packetBuffer, 0, sizeof(packetBuffer));
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
			} else if (strncmp(packetBuffer, "R:", 2) == 0) {
				parseData(packetBuffer);
			} else if (strncmp(packetBuffer, "N:", 2) == 0) {
				parseData(packetBuffer);
			} else if (strncmp(packetBuffer, "Register", 8) == 0) {
			    	confResponse();
			} else {
				char buf[32];
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf)-1, "Unknown command: %s", packetBuffer);
				Udp.write(buf);
			}
			Udp.endPacket();
		}
	}
}


