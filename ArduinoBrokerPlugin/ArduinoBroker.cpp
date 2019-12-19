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
#include "XPStandardWidgets.h"
#include "XPWidgets.h"


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
#define ARD_CMD_LOG +9999997
#define ARD_CMD_PAUSE 0

#define REFRESH_RATE_SECS  0.08

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
XPWidgetID	ardLogWidget = NULL;
XPWidgetID	ardLogTextWidget[50] = {NULL};

XPLMDataRef	activeDataRefs[32];
char		activeDataPaths[32][64];
char		activeDataTypes[32][4];
char		activeDataActions[32];
int		activeDataIdx = 0;
int		sendIdx = 0;
bool		sendData = false;
bool		udpOK = false;
int		sock;
struct		sockaddr_in server;

char		rxMsg[256];
int 		pingArduino();

static void	menuHandlerCallback(void*, void*); 
float		flightLoopCallback(float, float, int, void*);
int		readArduinoConf();
int 		sendArduino(char*);
void		createLogWidget(int, int, int, int);
int		ardLogHandler(XPWidgetMessage, XPWidgetID, long, long);
void		addLogMessage(const char*, const char*);
void		addLogMessagei(const char*, const int);
void		showLog();
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
	XPLMAppendMenuItem(myMenu, "Pause", (void *) ARD_CMD_PAUSE, 1);
	XPLMAppendMenuItem(myMenu, "Ping Arduino", (void *) ARD_CMD_PING, 1);
	XPLMAppendMenuItem(myMenu, "Show Log", (void *) ARD_CMD_LOG, 1);
	XPLMAppendMenuItem(myMenu, "Register Arduino Config", (void *) ARD_CMD_REGISTER, 1);

	

	// main loop
	XPLMRegisterFlightLoopCallback(
			flightLoopCallback,
			REFRESH_RATE_SECS,
			NULL);


	return 1;
}

PLUGIN_API void	XPluginStop(void) {
	XPLMUnregisterFlightLoopCallback(flightLoopCallback, NULL);
	XPDestroyWidget(ardLogWidget, 1);
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
	} else if (inItemRef == (void*) ARD_CMD_LOG) {
		showLog();
	} else if (inItemRef == (void*) ARD_CMD_PAUSE) {
		XPLMCommandKeyStroke(xplm_key_pause);
		
	} else {
	}
}

float	flightLoopCallback(
		float inElapsedSinceLastCall,
		float inElapsedTimeSinceLastFlightLoop,
		int   inCounter,
		void *inRefcon)
{
	char msg[512] = "";
	int vali;
	int valvi[8];
	float valf;
	float valvf[8];
	if (sendIdx >= activeDataIdx) {
		sendIdx = 0;
	}
	if (sendData) {
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
			sprintf(msg, "%c:%i:%s:cannot handle type: %s", getOrSet, sendIdx, activeDataPaths[sendIdx],  propType);
			addLogMessage(msg, "");
		}
		// this slow...
		sendArduino(msg);
		sendIdx++;
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
			addLogMessage(" - cannot register the property:", propPath);
		} else {
			addLogMessage(" - registered:", propPath);
			activeDataIdx++;
		}
	} else {
		addLogMessage("too many properties to register", "");
	}
}

void registerProperty(char *request) {
	activeDataIdx = 0;
	char  buff[1024];
	strncpy(buff, request, sizeof(buff) - 1);
	addLogMessage("parse request", "");
	char delim[] = ":\n\r\t";

	XPLMDataRef prop;
	char propPath[64] = "";
	char propType[4] = "";
	char getOrSet[8] = "";
	bool found = false;
	addLogMessage("start parsing", "");
	char *ptr = strtok(buff, delim);
	addLogMessage("got bit", "");
	while (ptr != NULL) {
		addLogMessage("is it R", "?");
		// discard "R"
		if (strncmp(ptr, "R", 1) == 0) {
			addLogMessagei("parsing entry #", activeDataIdx);
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				strncpy(propPath, ptr, sizeof(propPath) - 1);
				ptr = strtok(NULL, delim);
				if (ptr != NULL) {
					strncpy(propType, ptr, sizeof(propType) - 1);
					ptr = strtok(NULL, delim);
					if (ptr != NULL) {
						strncpy(getOrSet, ptr, sizeof(getOrSet) - 1);
						found = true;
						addDataRef(toupper(getOrSet[0]), propType, propPath);
					} else {
						addLogMessage("invalid register string, no get or set??", "");
					}
				} else {
					addLogMessage("invalid register string, no type??", "");
				}
			} else {
				addLogMessage("invalid register string, no path??", "");
			}
		} else {
			addLogMessage("invalid register string, no 'R' prefix??", "");
		}
		ptr = strtok(NULL, delim);
	}
}

void actOnMessage(char* msg) {
	if (strlen(msg) == 0) {
		return;
	}
	if (strncmp(msg, "R:", 2) == 0) {
		sendData = false;
		addLogMessage("got register response", "");
		registerProperty(msg);
		sendData = true;
	} else if (strncmp(msg, "Yes Hello!", 10) == 0) {
		addLogMessage("got ping response", "");
	} else {
		//addLogMessage("rx:",  msg);
	}
}

int initialiseSocket() {
	udpOK = false;
	char errMsg[256];
	unsigned short port = htons(ARD_PORT);
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		strncpy(errMsg, "Error: Socket?", sizeof(errMsg) - 1);
		addLogMessage(errMsg, "");
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

int sendArduino(char* msg) {

	if (! udpOK) {
		sock = initialiseSocket();
		if (sock < 0) {
			return sock;
		}
	}
	char sendBuf[1024];

	// send to Arduino
	strncpy(sendBuf, msg, sizeof(sendBuf) - 1);
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
		addLogMessage(sendBuf, "");
		return result;
	}

	// receive from Arduino
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	result = recvfrom(
			sock,
		       	(char *)sendBuf, 1024,
                	MSG_WAITALL,
		       	(struct sockaddr *) &cliaddr,
                	&len);
    	sendBuf[result] = '\0';
	actOnMessage(sendBuf);

	return 0;
}

int pingArduino() {
	int res = sendArduino((char*) "Hello");
	return res;
}

int readArduinoConf() {
	int res = sendArduino((char*) "Register");
	return res;
}

void	showLog() {
	createLogWidget(50, 712, 974, 662);	//left, top, right, bottom.
}

void addLogMessage(const char* msg, const char* extra) {
	char buf[256];
	sprintf(buf, "[xpduino] %s%s\n", msg, extra);
	XPLMDebugString(buf);
}

void addLogMessagei(const char* msg, const int extra) {
	char buf[256];
	sprintf(buf, "[xpduino] %s%i\n", msg, extra);
	XPLMDebugString(buf);
}

void addLogMessageWid(const char* msg) {

	char  buff[512];
	char delim[] = "\n";
	strncpy(buff, msg, sizeof(buff) - 1);
	char *ptr = strtok(buff, delim);
	while (ptr != NULL) {
		if (ardLogIdx >= 20) {
			for (int i = 0; i < ardLogIdx; i++) {
				strncpy(ardLog[i], ardLog[i+1], sizeof(ardLog[i]) - 1);
			}
			ardLogIdx--;
		}
		strncpy(ardLog[ardLogIdx++], ptr, sizeof(ardLog[ardLogIdx++]) - 1);
		strncpy(ardLog[ardLogIdx + 1], (char*) "end", 4);
		ptr = strtok(NULL, delim);
	}
}

void createLogWidget(int x, int y, int w, int h)
{
	int Index;
	int x2 = x + w;
	int y2 = y - h;
	// Create the Main Widget window.
	ardLogWidget = XPCreateWidget(
			x,
			y,
			x2,
			y2,
			1,		// Visible
			"Arduino Log",	// desc
			1,		// root
			NULL,		// no container
			xpWidgetClass_MainWindow);

	// Add Close Box to the Main Widget.  Other options are available.  See the SDK Documentation.
	XPSetWidgetProperty(ardLogWidget, xpProperty_MainWindowHasCloseBoxes, 1);
	// Print each line of instructions.
	for (Index=0; Index < 50; Index++)
	{
		if(strcmp(ardLog[Index], "end") == 0) {break;}
		// Create a text widget
		ardLogTextWidget[Index] = XPCreateWidget(x+10, y-(30+(Index*20)) , x2-10, y-(42+(Index*20)),
				1,	// Visible
				ardLog[Index],// desc
				0,		// root
				ardLogWidget,
				xpWidgetClass_Caption);
	}
	// Register our widget handler
	XPAddWidgetCallback(ardLogWidget, ardLogHandler);
	ardLogVisible = true;
}

int ardLogHandler(XPWidgetMessage  inMessage, XPWidgetID  inWidget, long  inParam1, long  inParam2) {
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (ardLogVisible)
		{
			XPHideWidget(ardLogWidget);
			ardLogVisible = false;
		}
		return 1;
	}
	return 0;
}
