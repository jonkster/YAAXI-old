#ifndef __BRAIN_H__
#define __BRAIN_H__

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
bool toggleVHF;
bool push0Last;
bool toggleNAV;
bool push1Last;
bool twistState0;
bool twistState1;


// ----------------------------------------------------------------------------------------------------
//	g = get: XP ------> Arduino (eg display airspeed on Arduino device - Get From XP)
//	s = set: Arduino -> XP      (eg take switch setting on Arduino and set value in XP - Set In XP)
/*char *confValues[] =  {
		(char*)"R:sim/cockpit/engine/fuel_pump_on:vi:s",
		(char*)"R:sim/cockpit/engine/fuel_tank_selector:i:s",
		(char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:i:s",
		(char*)"R:sim/cockpit/autopilot/heading:f:s",
		(char*)"R:sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot:f:s",
		(char*)"R:sim/cockpit/radios/nav1_freq_hz:i:g",
		(char*)"R:sim/cockpit/radios/nav1_stdby_freq_hz:i:g",
		(char*)"R:sim/aircraft/parts/acf_gear_deploy:vf:g"
};*/
// ----------------------------------------------------------------------------------------------------

int fuelPumpState = 0;
int gearUnsafeState = -1;
int nav1AFreq = 11090;
int nav1SFreq = 11090;
bool gearLockedDown = false;

#define UDP_QUEUE_SIZE 10
char sendQueue[UDP_QUEUE_SIZE ][32];
int sendIdx = 0;

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP	Udp;
int		clock;

void replyPing();
void parseData(char*); 
void sendConf();
void handleXPData(const char*, const char*);

int bugHdg0 = 0;
bool bug0Changed = false;
int bugHdg1 = 0;
bool bug1Changed = false;

#endif

