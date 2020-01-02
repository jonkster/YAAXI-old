#include "./XPlane.h"

#include "./Hardware.h"
#include "./Communication.h"

// ------------------ Configuration/Handlers --------------------------------

int fuelPumpState = 0;
bool toggleState0 = false;
bool toggleState1 = false;
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
		snprintf(buf, sizeof(buf)-1, "V:%i:[%i, 0, 0, 0, 0, 0, 0]", idx, pumpState);
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
		snprintf(buf, sizeof(buf)-1, "V:%i:%i", idx, s);
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

bool gearLockedDown = false;
void getXPGearLocked(const int idx, const char* value) {
	// value looks like: [0.5,0.5,0.5....]
	char delim[] = ", ";
	char arraySt[64];
	char *g[3];
	strncpy(arraySt, value + 1, sizeof(arraySt) - 1);
	char *ptr = strtok(arraySt, delim);
	if (ptr != NULL) {
		g[0] = ptr;
		ptr = strtok(NULL, delim);
		if (ptr != NULL) {
			g[1] = ptr;
			ptr = strtok(NULL, delim);
			if (ptr != NULL) {
				g[2] = ptr;
				if ((strncmp(g[0], "1.0", 3) == 0) && (strncmp(g[1], "1.0", 3) == 0) && (strncmp(g[2], "1.0", 3) == 0)) {
					gearLockedDown = true;
					lightSet(0, HIGH);
					return;
				} else {
					// gear not fully extended
				}
			}
		}
	}
	gearLockedDown = false;
	lightSet(0, LOW);
}

void setXPHeadingBug(const int idx, const char* value) {
	if (rotEncoderMoved(1)) {
		int v = getCounter(1);
		if (v < 0) {
			v += 360;
		}
		v = v % 360;

		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "V:%i:%i", idx, v);
		queueUdp(buf);
	}
}

void setXPOBS(const int idx, const char* value) {
	if (rotEncoderMoved(0)) {
		int v = getCounter(0);
		if (v < 0) {
			v += 360;
		}
		v = v % 360;
		char buf[32];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "V:%i:%i", idx, v);
		queueUdp(buf);
	}
}
void toggleXPComFreqs(const int activ, const int stdby) {
	char buf[32];
	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf)-1, "V:%i:%i", 9, activ);
	queueUdp(buf);
	Serial.print("change stdby to:"); Serial.println(buf);

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf)-1, "V:%i:%i", 8, stdby);
	queueUdp(buf);
	Serial.print("change active to:"); Serial.println(buf);
}

void toggleXPNavFreqs(const int activ, const int stdby) {
	char buf[32];
	memset(buf, 0, sizeof(buf));

	snprintf(buf, sizeof(buf)-1, "V:%i:%i", 7, activ);
	queueUdp(buf);
	Serial.print("change stdby to:"); Serial.println(buf);

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf)-1, "V:%i:%i", 6, stdby);
	queueUdp(buf);
	Serial.print("change active to:"); Serial.println(buf);
}

int com1AFreq = 12510;
int com1SFreq = 12190;
void getXPCom1Freq(const int idx, const char* value) {
	int f = atoi(value);
	if (idx == 8) {
		com1AFreq = f;
	} else if (idx == 9) {
		com1SFreq = f;
	} 
	if (hasSwitchChanged(PUSH0)) {
		if (switchValue(PUSH0) == 0) {
			toggleXPComFreqs(com1AFreq, com1SFreq);
		}
	}
}


int nav1AFreq = 10880;
int nav1SFreq = 11770;
void getXPNav1Freq(const int idx, const char* value) {
	int f = atoi(value);
	if (idx == 6) {
		nav1AFreq = f;
	} else if (idx == 7) {
		nav1SFreq = f;
	} 
	if (hasSwitchChanged(PUSH1)) {
		if (switchValue(PUSH1) == 0) {
			toggleXPNavFreqs(nav1AFreq, nav1SFreq);
		}
	}
}

void setHSISrc(const int idx, const char* value) {
	if (hasSwitchChanged(PUSH2)) {
		if (switchValue(PUSH2) == 0) {
			char buf[32];
			memset(buf, 0, sizeof(buf));
			int v = atoi(value);

			if (v == 0) {
				v = 2;
			} else if (v == 2) {
				v = 0;
			}
			snprintf(buf, sizeof(buf)-1, "V:%i:%i", idx, v);
			queueUdp(buf);
			
		}
	}
}



