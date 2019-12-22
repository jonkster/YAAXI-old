#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <EthernetUdp.h>

#define MAX_HANDLERS 20

// handler function for dealing with XP data
typedef void (*xpResponseHandler)(const int, const char*);

/* the type that stores a specific handler's details */
typedef struct {
	int  id;			// unique id#
	char const *drPath;		// XP dataRef string
	xpResponseHandler handler;	// callback function
} drConfig;


/* the array of handler details */
drConfig	drHandlers[MAX_HANDLERS];
int		drConfigIdx = 0;

/* register handler details */
void addDataRefHandler(drConfig);

/* debug methods - show handler details */
void displayHandler(int);
void displayHandlers();



// an example handler callback function
void printIdxValue(const int idx, const char* value) {
	Serial.print("idx:");
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
	&printIdxValue
};

void newEntry(const char* drPath, const xpResponseHandler handler) {
	drConfig entry = { 
		-1,
		drPath,
		handler	
	};
	if (drConfigIdx < MAX_HANDLERS) {
		entry.id = drConfigIdx;
		drHandlers[drConfigIdx++] = entry;
	} else {
		Serial.println("Max handlers registered, cannot add new handler: ");
		Serial.println(drPath);
	}
}


drConfig getHandler(const int handlerId) {
	if (handlerId < drConfigIdx) {
		return drHandlers[handlerId];
	} else {
		Serial.println("no handler!");
		return nullHandler;
	}
}

void displayItem(drConfig item) {
	Serial.print("#"); Serial.print(item.id);
	Serial.print(", dataref:"); Serial.print(item.drPath);
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

void confResponse(EthernetUDP Udp) {
	displayHandlers();
	for (int i = 0; i < drConfigIdx; i++) {
		drConfig item = drHandlers[i];
		const char* path = item.drPath;
		char msg[1024];
		snprintf(msg, sizeof(msg)-1, "%s\n", path);
		Udp.write(msg);
		Serial.print("sent value: ");
		Serial.println(path);
	}
}



#endif

