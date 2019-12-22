#ifndef __BRAIN_H__
#define __BRAIN_H__

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 256 //increase UDP size


bool verbose = false;


// rot encoder variables
int rotCount0 = 0; 
int rotCount1 = 0; 


// led outputs
int lastTime = 0;
bool ledState = false;

// switch inputs
bool toggleState0;
bool toggleState1;
bool toggleVHF;
bool toggleNAV;
bool push0Last;
bool push1Last;
bool push2Last;
bool push3Last;
bool twistState0;
bool twistState1;


#define UDP_QUEUE_SIZE 10
char sendQueue[UDP_QUEUE_SIZE ][32];
int sendIdx = 0;


EthernetUDP	Udp;
int		clock;

void replyPing();
void parseData(const char*); 
void handleXPData(const char*, const char*);


#endif

