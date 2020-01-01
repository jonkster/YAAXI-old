/*
 * 
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "XPLMDataAccess.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
//#include "XPStandardWidgets.h"
//#include "XPWidgets.h"


#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif
#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif


#define ARD_PORT 8888
#define ARD_IP "192.168.0.177"
#define ARD_CMD_PING +9999999
#define ARD_CMD_REGISTER +9999998
#define ARD_CMD_PAUSEPI +9999997
#define ARD_CMD_PAUSE 0

#define REFRESH_RATE_SECS  0.002
//#define REFRESH_RATE_SECS  0.005

char ardLog[50][128] = {
	"",
	"",
	"",
	"",
	"",
	"",
	"end"
};
int		ardLogIdx = 0;
bool		ardLogVisible = false;

XPLMDataRef	activeDataRefs[32];
char		activeDataPaths[32][64];
char		activeDataTypes[32][4];
char		activeDataActions[32];
int		activeDataIdx = 0;
int		sendIdx = 0;
bool		sendData = false;
bool		active = false;
bool		udpOK = false;
int		sock = -1;
struct		sockaddr_in server;

char		rxMsg[256];
int 		pingArduino();

static void	menuHandlerCallback(void*, void*); 
float		flightLoopCallback(float, float, int, void*);
int		readArduinoConf();
int 		sendArduino(char*, bool);
bool		getReply(int sock);
void		addLogMessage(const char*, const char*);
void		addLogMessagei(const char*, const int);
void		closeSocket(int);

PLUGIN_API int XPluginStart( char *  outName, char *  outSig, char *  outDesc)
{
	XPLMMenuID	myMenu;
	int		mySubMenuItem;

	strncpy(outName, "ArduinoBroker", sizeof(outName) - 1);
	strncpy(outSig,  "motioncapture.interface.arduinobroker", sizeof(outSig) - 1);
	strncpy(outDesc, "A plugin that talks to an Arduino.", sizeof(outDesc) - 1);

	// setup Menu
	mySubMenuItem = XPLMAppendMenuItem(
			XPLMFindPluginsMenu(),	/* Put in plugins menu */
			"Arduino Broker",	/* Item Title */
			0,			/* Item Ref */
			1);			/* Force English */

	myMenu = XPLMCreateMenu(
			"Arduino Broker", 
			XPLMFindPluginsMenu(), 
			mySubMenuItem, 			/* Menu Item to attach to. */
			menuHandlerCallback,		/* The handler */
			0);				/* Handler Ref */

	// Menu Lines
	XPLMAppendMenuItem(myMenu, "Pause XP", (void *) ARD_CMD_PAUSE, 1);
	XPLMAppendMenuItem(myMenu, "Pause Plugin", (void *) ARD_CMD_PAUSEPI, 1);
	XPLMAppendMenuItem(myMenu, "Ping Arduino", (void *) ARD_CMD_PING, 1);
	XPLMAppendMenuItem(myMenu, "Register Arduino Config", (void *) ARD_CMD_REGISTER, 1);

	// main loop
	XPLMRegisterFlightLoopCallback(
			flightLoopCallback,
			REFRESH_RATE_SECS,
			NULL);


	addLogMessage("Staring Plugin", "");
	active = true;
	pingArduino();
	return 1;
}


PLUGIN_API void	XPluginStop(void) {
	XPLMUnregisterFlightLoopCallback(flightLoopCallback, NULL);
	closeSocket(sock);
}
PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage( XPLMPluginID inFromWho, int inMessage, void * inParam) { }



void menuHandlerCallback(void * inMenuRef, void * inItemRef)
{
	if (inItemRef == (void*) ARD_CMD_PING) {
		pingArduino();
	} else if (inItemRef == (void*) ARD_CMD_REGISTER) {
		readArduinoConf();
	} else if (inItemRef == (void*) ARD_CMD_PAUSEPI) {
		active = ! active;
	} else if (inItemRef == (void*) ARD_CMD_PAUSE) {
		XPLMCommandKeyStroke(xplm_key_pause);
		
	} else {
	}
}

int c = 0;
float	flightLoopCallback(
		float inElapsedSinceLastCall,
		float inElapsedTimeSinceLastFlightLoop,
		int   inCounter,
		void *inRefcon)
{
	char msg[1024];
	int vali;
	int valvi[8];
	float valf;
	float valvf[8];
	if (sendIdx >= activeDataIdx) {
		sendIdx = 0;
	}
	if (activeDataIdx == 0) {
		if ((c++ % 100) == 0) {
			addLogMessagei("read conf:", c);
			readArduinoConf();
		}
	} else if (sendData) {
		char* propType = activeDataTypes[sendIdx];
		char getOrSet =	activeDataActions[sendIdx];
		XPLMDataRef dRef = activeDataRefs[sendIdx];
		if (strncmp(propType, "i", 2) == 0) {
			vali = XPLMGetDatai(dRef);
			sprintf(msg, "%c:%i:%i", getOrSet, sendIdx, vali);
		} else if (strncmp(propType, "f", 2) == 0) {
			valf = XPLMGetDataf(dRef);
			sprintf(msg, "%c:%i:%f", getOrSet, sendIdx, valf);
		} else if (strncmp(propType, "vi", 2) == 0) {
			XPLMGetDatavi(dRef, valvi, 0, 8);
			sprintf(msg, "%c:%i:[%i, %i, %i, %i, %i, %i, %i, %i]", getOrSet, sendIdx, valvi[0],valvi[1],valvi[2],valvi[3],valvi[4],valvi[5],valvi[6],valvi[7] );
		} else if (strncmp(propType, "vf", 2) == 0) {
			XPLMGetDatavf(dRef, valvf, 0, 8);
			sprintf(msg, "%c:%i:[%f, %f, %f, %f, %f, %f, %f, %f]", getOrSet, sendIdx, valvf[0],valvf[1],valvf[2],valvf[3],valvf[4],valvf[5],valvf[6],valvf[7] );
		} else {
			snprintf(msg, sizeof(msg)-1, "%c:%i:%s:cannot handle type: %s", getOrSet, sendIdx, activeDataPaths[sendIdx],  propType);
			addLogMessage(msg, " 1");
			snprintf(msg, sizeof(msg)-1, "%i:%s:cannot handle type", sendIdx, activeDataPaths[sendIdx]);
		}
		// this slow...
		sendArduino(msg, true);
		sendIdx++;
	} else {
		if (sock != -1) {
			getReply(sock);
		}
	}
	return REFRESH_RATE_SECS; // seconds before next call
}

void addDataRef(char getOrSet, char* propType, char* propPath) {
	if (activeDataIdx < 32) {
		activeDataActions[activeDataIdx] = getOrSet;
		strncpy(activeDataTypes[activeDataIdx], propType, sizeof(activeDataTypes[activeDataIdx]) - 1);
		activeDataRefs[activeDataIdx] = XPLMFindDataRef(propPath);
		strncpy(activeDataPaths[activeDataIdx], propPath, sizeof(activeDataPaths[activeDataIdx]) - 1);
		addLogMessage(propPath, " - registering...");
		if (activeDataRefs[activeDataIdx] == NULL) {
			addLogMessage(" - !! CANNOT register the property:", propPath);
		} else {
			addLogMessage(" - registered:", propPath);
			activeDataIdx++;
		}
	} else {
		addLogMessage("too many properties to register", " 2");
	}
}

void registerProperty(char *request) {
	activeDataIdx = 0;
	char  buff[1024];
	strncpy(buff, request, sizeof(buff) - 1);
	addLogMessage("register request: ", buff);
	char delim[] = ":\n\r\t";

	XPLMDataRef prop;
	char propPath[64] = "";
	char propType[4] = "";
	char getOrSet[8] = "";
	bool found = false;
	char *ptr = strtok(buff, delim);
	while (ptr != NULL) {
		// discard "R"
		if (strncmp(ptr, "R", 1) == 0) {
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(propPath, ptr, sizeof(propPath) - 1);
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					strncpy(propType, ptr, sizeof(propType) - 1);
					ptr = strtok(NULL, delim);
					// pointer will have RW flag if it exists
					if (ptr != NULL) {
						strncpy(getOrSet, ptr, sizeof(getOrSet) - 1);
						found = true;
						addDataRef(toupper(getOrSet[0]), propType, propPath);
					} else {
						addLogMessage("invalid register string, no get or set??", " 3");
					}
				} else {
					addLogMessage("invalid register string, no type??", " 4");
				}
			} else {
				addLogMessage("invalid register string, no path??", " 5");
			}
		} else if (strncmp(ptr, "O", 1) == 0) {
			addLogMessage("Recv'd OK:", ptr);
		} else {
			addLogMessage("invalid register string, no 'R' prefix??", ptr);
		}
		ptr = strtok(NULL, delim);
	}
}

void translateIntArrayString(int* iVector, const char *val) {
	// have string like: [2,0,0,0,0,0,0]
	char buff[32];
	strncpy(buff, val, sizeof(buff) - 1);
	char delim[] = ",";
	char *ptr = strtok(buff+1, delim);
	int idx = 0;
	while (ptr != NULL) {
		iVector[idx++] = atoi(ptr);
		ptr = strtok(NULL, delim);
	}
}

void setXPData(const char *msg) {
	char  buff[32];
	strncpy(buff, msg, sizeof(buff) - 1);
	char delim[] = ":";
	char idxSt[4];
	char val[32];
	char *ptr = strtok(buff, delim);
	if (ptr != NULL) {
		ptr = strtok(NULL, delim);
		if (ptr != NULL) {
			strncpy(idxSt, ptr, sizeof(idxSt) - 1);
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(val, ptr, sizeof(val) - 1);
				int idx = atoi(idxSt);
				if (idx < activeDataIdx) {
					if (idx == 5) {
						addLogMessage("v:",  msg);
					}
					char* propType = activeDataTypes[idx];
					if (strncmp(propType, "i", 2) == 0) {
						int vali = atoi(val);
						XPLMDataRef dRef = activeDataRefs[idx];
						XPLMSetDatai(dRef, vali);
					} else if (strncmp(propType, "f", 2) == 0) {
						float valf = atof(val);
						XPLMDataRef dRef = activeDataRefs[idx];
						XPLMSetDataf(dRef, valf);
					} else if (strncmp(propType, "vi", 3) == 0) {
						int iVector[8] = {0, 0, 0, 0, 0, 0, 0, 0};
						translateIntArrayString(iVector, val);
						XPLMDataRef dRef = activeDataRefs[idx];
						XPLMSetDatavi(dRef, iVector, 0, 8);
					} else {
						addLogMessage("Cannot set type yet!",  propType);
					}
				} else {
					addLogMessagei("idx err?",  idx);
				}
			} else {
				addLogMessage("no val?",  " 6");
			}
		} else {
			addLogMessage("no S?",  " 7");
		}
	}
}

void actOnMessage(char* msg) {
	//addLogMessage("acting on msg:", msg);
	if (strlen(msg) == 0) {
		return;
	} else if (strncmp(msg, "R:", 2) == 0) {
		sendData = false;
		addLogMessage("got register response", " 8");
		registerProperty(msg);
		sendData = true;
	} else if (strncmp(msg, "W:", 2) == 0) {
		//addLogMessage("got write response", " 8.1");
		setXPData(msg); // got a 'write' response
	} else if (strncmp(msg, "S:", 2) == 0) {
		//addLogMessage("got set response", " 8.1a");
		setXPData(msg); // got a 'write' response
	} else if (strncmp(msg, "O", 1) == 0) {
		//addLogMessage("got O response", " 8.2");
		return; // just acknowledgement
	} else if (strncmp(msg, "Yes Hello!", 10) == 0) {
		addLogMessage("got ping response", " 9");
	} else {
		addLogMessage("got unknown response: ",  msg);
	}
}

int initialiseSocket() {
	udpOK = false;
	char errMsg[256];
	unsigned short port = htons(ARD_PORT);
	//int sock = socket(AF_INET, SOCK_DGRAM, 0);
	sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (sock < 0)
	{
		strncpy(errMsg, "Error: Socket?", sizeof(errMsg) - 1);
		addLogMessage(errMsg, " 10");
		return sock;
	}

	/* Set up the server name */
	server.sin_family      = AF_INET;
	server.sin_port        = port;
	server.sin_addr.s_addr = inet_addr(ARD_IP);
	udpOK = true;
	return sock;
}

void closeSocket(int sock) {
	if (udpOK) {
		close(sock);
	}
}

bool getReply(int sock) {
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	char rxBuf[2048];
	int result = recvfrom(
			sock,
			(char *)rxBuf,
			(sizeof(rxBuf)+1),
			MSG_DONTWAIT,
			(struct sockaddr *) &cliaddr,
			&len);
	if (result > 0) {
		rxBuf[result] = '\0';
		//addLogMessage(rxBuf, " 11a");
		actOnMessage(rxBuf);
		return true;
	} else {
		addLogMessage("no response?", "");
		return false;
	}
}

int sendArduino(char* msg, bool waitForReply) {

	if (! active) {
		addLogMessage("plugin paused", "");
		return 0;
	}

	if (! udpOK) {
		sock = initialiseSocket();
		if (sock < 0) {
			return sock;
		}
	}
	char sendBuf[128];

	// send to Arduino
	strncpy(sendBuf, msg, sizeof(sendBuf) - 1);
	//addLogMessage("sending:", msg);
	int result = sendto(
				sock,
 				sendBuf,
 				(strlen(sendBuf)+1),
 				0,
 				(struct sockaddr *)&server,
 				sizeof(server));
	if (result < 0)
	{
		closeSocket(sock);
		strncpy(sendBuf, "Error: Send?", sizeof(sendBuf) - 1);
		addLogMessage(sendBuf, " 11");
		return result;
	}
	//addLogMessage("sent:", msg);

	if (waitForReply) {
		int i = 0;
		if (! getReply(sock) && (i < 10)) {
			getReply(sock);
			i++;
		}
	}
	return 0;
}

int pingArduino() {
	int res = sendArduino((char*) "Hello", true);
	return res;
}

int readArduinoConf() {
	int res = sendArduino((char*) "Register", true);
	return res;
}


void addLogMessage(const char* msg, const char* extra) {
	char buf[512];
	sprintf(buf, "[xpduino] '%s%s'\n", msg, extra);
	XPLMDebugString(buf);
}

void addLogMessagei(const char* msg, const int extra) {
	char buf[512];
	sprintf(buf, "[xpduino](i)  %s%i\n", msg, extra);
	XPLMDebugString(buf);
}

