#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 512 //increase UDP size

#define MAX_HANDLERS 20

#define IP_ADDRESS 
#define MAC_ADDRESS 


// handler function for dealing with XP data
typedef void (*xpResponseHandler)(const int, const char*);

/* the type that stores a specific handler's details */
typedef struct {
	int  id;			// unique id#
	char const *drPath;		// XP dataRef string
	xpResponseHandler handler;	// callback function
} drConfig;

/* register handler details */
void addDataRefHandler(drConfig);

/* debug methods - show handler details */
void displayHandler(int);
void displayHandlers();

void newEntry(const char* drPath, const xpResponseHandler handler);

void printIdxValue(const int idx, const char* value);

void queueUdp(const char* msg);

void setupEthernet();

void serialSetup();

void updateComms();

#endif

