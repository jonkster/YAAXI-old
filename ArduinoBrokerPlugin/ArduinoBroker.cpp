/*
 * 
 */

#include <arpa/inet.h>
#include <netdb.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


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

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"

#define ARD_PORT 8888
#define ARD_IP "192.168.0.177"
#define ARD_CMD_PING +9999999
#define ARD_CMD_REGISTER +9999998
#define ARD_CMD_PAUSE 0

static		XPLMWindowID	gWindow;
void		setUpWindow();
char		sendMsg[256];
char		rxMsg[256];
int 		pingArduino();

static XPLMDataRef		gPlaneLat;
static XPLMDataRef		gPlaneLon;
static XPLMDataRef		gPlaneEl;

static void	menuHandlerCallback(void*, void*); 
float		flightLoopCallback(float, float, int, void*);
int		readArduinoConf();
int 		sendArduino(char*);

PLUGIN_API int XPluginStart( char *  outName, char *  outSig, char *  outDesc)
{
	XPLMMenuID	myMenu;
	int		mySubMenuItem;

	strcpy(outName, "ArduinoBroker");
	strcpy(outSig,  "motioncapture.interface.arduinobroker");
	strcpy(outDesc, "A plugin that talks to an Arduino.");

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
	XPLMAppendMenuItem(myMenu, "Register Arduino Config", (void *) ARD_CMD_REGISTER, 1);
	

	// get data refs
	gPlaneLat = XPLMFindDataRef("sim/flightmodel/position/latitude");
	gPlaneLon = XPLMFindDataRef("sim/flightmodel/position/longitude");
	gPlaneEl = XPLMFindDataRef("sim/flightmodel/position/elevation");

	// main loop
	XPLMRegisterFlightLoopCallback(
			flightLoopCallback,	/* Callback */
			1.0,			/* Interval */
			NULL);			/* refcon not used. */


	return 1;
}

PLUGIN_API void	XPluginStop(void) {
	XPLMUnregisterFlightLoopCallback(flightLoopCallback, NULL);
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
	/*float	elapsed = XPLMGetElapsedTime();
	float	lat = XPLMGetDataf(gPlaneLat);
	float	lon = XPLMGetDataf(gPlaneLon);
	float	el = XPLMGetDataf(gPlaneEl);

    	sprintf(sendMsg, "Time=%f, lat=%f,lon=%f,el=%f.\n",elapsed, lat, lon, el);
	sendArduino(sendMsg);*/
	// GetData:code:arduino source
	sendArduino((char*)"GetData:switch1:d5");
	sendArduino((char*)"GetData:switch2:d6");
	sendArduino((char*)"GetData:trim:a2");
	// SetData:value:arduino source
	sendArduino((char*)"SetData:1:d7");
	sendArduino((char*)"SetData:0:d8");

	/* Return 1.0 to indicate that we want to be called again in 1 second. */
	return 1.0;
}

int sendArduino(char* msg) {
	unsigned short port = htons(ARD_PORT);
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
	{
		strcpy(sendMsg, "Socket?");
		return 1;
	}

	/* Set up the server name */
	struct sockaddr_in server;
	server.sin_family      = AF_INET;
	server.sin_port        = port;
	server.sin_addr.s_addr = inet_addr(ARD_IP);

	char sendbuf[128];
	char rxbuf[128];
	// send to Arduino
	strcpy(sendbuf, msg);
	int result = sendto(
				sock,
 				sendbuf,
 				(strlen(sendbuf)+1),
 				0,
 				(struct sockaddr *)&server,
 				sizeof(server));
	if (result < 0)
	{
		strcpy(sendMsg, "Send?");
		return 2;
	}

	// receive from Arduino
	struct sockaddr_in cliaddr;
	socklen_t len = sizeof(cliaddr);
	result = recvfrom(
			sock,
		       	(char *)rxbuf, 1024,
                	MSG_WAITALL,
		       	(struct sockaddr *) &cliaddr,
                	&len);
    	rxbuf[result] = '\0';
	close(sock);
    	sprintf(sendMsg, "You: %s", sendbuf);
    	sprintf(rxMsg, "Arduino: %s", rxbuf);
	return 0;
}

int pingArduino() {
	int res = sendArduino((char*) "Hello");
	setUpWindow();
	return res;
}

int readArduinoConf() {
	int res = sendArduino((char*) "Register");
	return res;
}

void	drawMsgBox(XPLMWindowID in_window_id, void * in_refcon) {
	XPLMSetGraphicsState(
						 0 /* no fog */,
						 0 /* 0 texture units */,
						 0 /* no lighting */,
						 0 /* no alpha testing */,
						 1 /* do alpha blend */,
						 1 /* do depth testing */,
						 0 /* no depth writing */
						 );

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);
	float col_white[] = {1.0, 1.0, 1.0}; // red, green, blue
	XPLMDrawString(col_white, l + 10, t - 20, sendMsg, NULL, xplmFont_Proportional);
	XPLMDrawString(col_white, l + 10, t - 40, rxMsg, NULL, xplmFont_Proportional);
}

int     	 dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int     	 dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void    	 dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }

void	setUpWindow()
{
	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	params.visible = 1;
	params.drawWindowFunc = drawMsgBox;
	params.handleMouseClickFunc = dummy_mouse_handler;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	params.left = left + 50;
	params.bottom = bottom + 150;
	params.right = params.left + 200;
	params.top = params.bottom + 200;
	gWindow = XPLMCreateWindowEx(&params);
}

