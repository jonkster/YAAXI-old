# YAAXI

A simple Arduino/X-Plane interface system for adding physical cockpit controls to
X-Plane.

(NB simple means simple configuration and set up if you are comfortable with
Arduino programming in C and interfacing of devices to Arduino.  It doesn't
mean gui configuration screens and/or drag and drop customisation etc).

## Current Status

Very RAW.  IP address hardcoded.

### Philosophy

The Arduino is where things are configured, not X-Plane.  The configuration is
done via a set of X-Plane datarefs and associated callback functions.

Mostly in the Arduino code you just need to specify a set of datarefs and write
an associated callback 'handler' to deal with each particular dataref.

## Overview

The system allows you to set up physical hardware (switches, lights, motors,
displays etc) driven by an Arduino, to interact with X-Plane.

It assumes you are comfortable interfacing devices to an Arduino and with
programing an Arduino.

It requires an Arduino with an Ethernet shield (or on board ethernet - eg like
the Freetronics Ether\* Arduino clones).

All configuration of the system is done in the Arduino via code you write.

A simple default example of the Arduino code can be used as the basis of your
customised system.


### It consists of 2 components:

*xpDuinoBox* is an Arduino program that communicates with the X-Plane Arduino
Broker Plugin.

*ArduinoBroker* is an X-Plane plugin that communicates with an Arduino that is
running the xpDuinoBox program.

### out of the box

The default Arduino program supplied is set up to respond to a switch on pin 8
and a led on pin 7 that allow you to test the system works (the switch should
set the pitot heat switch in X-Plane and the LED should reflect the position of
the Nav Light switch in X-Plane).

Once you confirm this works you can modify the Arduino code (and add
whatever appropriate physical devices to it) to match the cockpit devices you
wish to use.

## How it works

The Arduino is programmed with code you write, based on the supplied default C
code.  The Arduino should be connected via ethernet to the same network as the computer
running X-Plane.

X-Plane needs to have the ArduinoBroker plugin installed.

In X-Plane the Arduino Broker plugin will initially ask the Arduino for a list
of XP datarefs that the Arduino system wishes to interact with.  Once it
receives that list, the broker will periodically send the requested X-Plane
dataref values to the Arduino via ethernet UDP/IP messages.

When the Arduino receives data from the plugin, it can act on it to do things
like set leds, run motors, draw display etc.  It can also reply to the Broker
with new values XP should set for the datarefs (eg in response to the position
of physical switches, it may update XP systems etc).

### Simple example

The Arduino may have 2 switches connected that represent the fuel pump and fuel selector.

It notifies the broker it wants to interact with the 2 following X-Plane
datarefs:

```
sim/cockpit/engine/fuel_pump_on
```
and
```
sim/cockpit/engine/fuel_tank_selector
```

After being notified, the broker will periodically send the values of these
datarefs to the Arduino, which in return may respond with new values based on
the connected switches physical positions.

If the broker receives new values from the Arduino it attempts to update the
values in X-Plane to reflect the state of the switches.

## Installation

1. Set up the Arduino initially with a switch connected to data pin 8 and a led
   to pin 7 (this just for testing purposes)
2. Compile and Upload the default C program to the Arduino
3. Connect the Arduino via ethernet to the same network as the X-Plane box
4. Copy the plugin directory to X-Plane
5. Run X-Plane with standard C172
6. Confirm that the physical switch connected to data pin 8 will set the
   'virtual' Pitot Heat Switch in X-Plane
7. Confirm that the 'virtual' Nav Light Switch changed in X-Plane will light a
   LED connected to Arduino pin 7

*THEN*

8. Set up your hardware (switches, lights, displays, motors etc) to the Arduino
   and modify the Arduino code to match your needs.

## Configuration and Setting up Your Specific Arduino Code

This assumes you have set up your hardware to the Arduino and know what
datarefs you want to read and or write to via the Arduino.

Modify the default Arduino program's registerXP function to match your Arduino
hardware setup.

The Arduino program calls the XPRegister function in setup.  You edit this to
supply your list of configuration values that list datarefs and your callback
functions for dealing with these datarefs.

You include a callback function that will be called whenever the Arduino
receives info on this dataref from X-Plane.


#### Example Configuration
```C
void registerXP() {
	newEntry((char*)"R:sim/cockpit/engine/fuel_pump_on:vi:w", &setXPFuelPump);
	newEntry((char*)"R:sim/cockpit/engine/fuel_tank_selector:i:w", &setXPFuelTank);
	newEntry((char*)"R:sim/cockpit/warnings/annunciators/gear_unsafe:i:r", &getXPGearUnsafe);
	newEntry((char*)"R:sim/aircraft/parts/acf_gear_deploy:vf:r", &getXPGearLocked);
	newEntry((char*)"R:sim/cockpit/autopilot/heading:f:w", &setXPHeadingBug);
	newEntry((char*)"R:sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot:f:w", &setXPOBS);
	newEntry((char*)"R:sim/cockpit/radios/nav1_freq_hz:i:w", &getXPNav1ActiveFreq);
	newEntry((char*)"R:sim/cockpit/radios/nav1_stdby_freq_hz:i:w", &getXPNav1StandbyFreq);
}
```

In the above case you will write callback functions such as ```setXPFuelPump``` etc

Callback functions are of type ```xpResponseHandler```.  
They return a void and will have 2 arguments set, an integer (the index of the
dataref) and a char* (the dataref value from X-Plane)
ie:
```C
typedef void (*xpResponseHandler)(const int, const char*);
```


### Very simple callback that just prints details on what it has received from X-Plane and doesn't respond
```C
// an example handler callback function
void printIdxValue(const int idx, const char* value) {
        Serial.print("dummy handler - idx:");
        Serial.print(idx);
        Serial.print(", value:");
        Serial.println(value);
}
```

If you wish to reply to X-Plane (eg with new values for the dataref) create a
suitable response string and pass it to the ```queueUdp``` function.

### examples of callbacks:
```C
void setXPFuelPump(const int idx, const char *value) {
	if (hasSwitchChanged(TOGGLE0) || hasSwitchChanged(TOGGLE1)) {
		int pumpState = 0;
		if (switchValue(TOGGLE0)) {
			pumpState = 2;
		} else if (switchValue(TOGGLE1)) {
			pumpState = 1;
		}
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:[%i, 0, 0, 0, 0, 0, 0]", idx, pumpState);
		queueUdp(buf);
	}
}

void setXPFuelTank(const int idx, const char *value) {
	if (hasSwitchChanged(TWIST0) || hasSwitchChanged(TWIST1)) {
		int twistState0 = switchValue(TWIST0);
		int twistState1 = switchValue(TWIST1);
		int s = 0;
		if (twistState0 == 0) {
			s = 1;
		} else if (twistState1 == 0) {
			s = 3;
		}
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "S:%i:%i", idx, s);
		queueUdp(buf);
	}
}

int gearUnsafeState = -1;
void getXPGearUnsafe(const int idx, const char* value) {
	int v = atoi(value);
	if (v != gearUnsafeState) {
		if (v == 0) { 
			lightSet(1, LOW);
		} else {
			lightSet(1, HIGH);
		}
	}
	gearUnsafeState = v;
}
```

## General Comm protocols

To interact with the broker you should be aware of how communication between
the 2 components operate.

You will need to know this when constructing responses from your code back to
the broker.

Generally, broker in X-Plane initiates communications, Arduino acts on these
and may or may not reply back to the broker.  Communication is plain text,
UDP/IP.



| Arduino Broker    |           xpDuinoBox              |     Arduino           |
|:-----------------:|:---------------------------------:|:---------------------:|
|  send msg         | &rarr; process and respond        | &larr; &rarr; read/set arduino pins |
|                   |                    &darr;         |                       |
|  recv msg  &larr; |        (optional)                 |                       |
|     &darr;        |                                   |                       |
| set XP DataRefs   |                                   |                       |


### Typical messages

*PING* (confirm system working)

```
Broker sends ping -----> "Hello" ------> Arduino -----> flash Leds
	                                     |			
recv ping <------------- "Hello There!" <----+
```

*READ CONFIGURATION* (XP asks Arduino to tell it what values it wants to read and/or set)

```
Broker sends Register request --- "Register" ------> Arduino
                                                       |
Broker registers <------------------- [data] <---------+
XP datarefs
```

*DATA MESSAGES*

If no response required,
```
Broker sends XP dataref  ---- "N:0:1" -----> Arduino -------> set pins

```

If response required,
```
Broker sends XP dataref  ---- "R:0:1" ----→ Arduino <------- get pins
                                               |
Broker sets  <-------------"V:0:3” <-----------+
XP datarefs
```



## Detailed Message Descriptions

|Message              |   Description               |  Sent From   |
|:--------------------|:---------------------------:|:------------:|
| Hello               |   ping request              | X-Plane      |
| Hello There!        |   ping response             | Arduino      |
| Register            |   data ref config request   | X-Plane      |
| R:dr:type:respond   |   data ref configuration    | Arduino      |
| [N\|R]:n:v           |   send dataref value        | X-Plane      |
| V:n:v               |   send dataref value        | Arduino      |
| O                   |   OK response               | Arduino      |


### Message: DataRef Configuration from Arduino:

Response from a "Register" request from X-Plane.  Arduino returns details of
DataRefs the Arduino system wants to read or set.
The XP plugin will send Register requests periodically until it gets
this response.

format:
```
	 R:data ref:data type:respond
	 R:data ref:data type:respond
	 R:data ref:data type:respond
	 ... etc ...
```

Where:
```
dataref = X-Plane dataref eg: sim/cockpit/engine/fuel_pump_on
data type = [i|vi|f|vf] 
respond = R (respond back to message) or N (no response needed)
```

eg:
```
R:sim/cockpit/engine/fuel_pump_on:vi:R
R:sim/cockpit/engine/fuel_tank_selector:i:R
R:sim/cockpit/warnings/annunciators/gear_unsafe:i:r
R:sim/aircraft/parts/acf_gear_deploy:vf:N
R:sim/cockpit/autopilot/heading:f:R
R:sim/cockpit2/radios/actuators/hsi_obs_deg_mag_pilot:f:R
R:sim/cockpit/radios/nav1_freq_hz:i:R
R:sim/cockpit/radios/nav1_stdby_freq_hz:i:R
```


### Message: DataRef Value from X-Plane


The XP plugin will periodically send dataref values (from the set of
registered datarefs sent by the Arduino).

If the value message is prefixed by "N" the plugin will not expect a response
from the Arduino.

If the value message is prefixed by "R" the plugin will expect a value or OK
response from the Arduino.


format:
```
 	[N|R]:n:v
```
where:
```
	n = index number (corresponds to the position in the configuration)
	v = dataref value
```

eg assume item #4 in the configuration response was:
```
	R:sim/cockpit/engine/fuel_pump_on:vi:R
```

the plugin will periodically send dataref value message like:
```
	R:4:[1, 0, 0, 0, 0, 0, 0, O] or
	R:4:[0, 0, 0, 0, 0, 0, 0, O] or
	R:4:[0, 1, 0, 0, 0, 0, 0, O] etc

```
(NB The index number of the datarefs starts from 0 and is in the order of the
configuration response items)

### Message: DataRef value to X-Plane

The Arduino can respond to a dataref value message from XP with a value
response indicating what the Arduino wants the XP dataref to be set to in
X-Plane.

format:
```
	V:n:v
```

where:
```
	n = dataref index number
	v = value
```

eg assume item #4 in the configuration is:
```
	R:sim/cockpit/engine/fuel_pump_on:vi:R
```

the plugin will periodically send a dataref message like:
```
	R:4:[1, 0, 0, 0, 0, 0, 0, O]
```

the Arduino may respond with responses like
```
	V:4:[2, 0, 0, 0, 0, 0, 0, O]
```
indicating it wants XP to set the sim/cockpit/engine/fuel_pump_on dataref in
XP to be [2, 0, 0, 0, 0, 0, 0, O]

(NB The index number of the datarefs starts from 0 and is in the order of the
configuration response items)

### Message: OK response from Arduino

If the Arduino does not want to set values in XP but should send a response to
a request, it will send an "OK" response.

format:
```
 	O:n
```
where:
```
	n = dataref index number
```
eg:
```
	O:4
```
