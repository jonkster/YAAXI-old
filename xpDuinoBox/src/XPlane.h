#ifndef __XPLANE_H__
#define __XPLANE_H__


void setXPFuelPump(const int idx, const char *value);

void setXPFuelTank(const int idx, const char *value);

void getXPGearUnsafe(const int idx, const char* value);
void getXPGearLocked(const int idx, const char* value);
void setXPHeadingBug(const int idx, const char* value);
void setXPOBS(const int idx, const char* value);
void toggleXPNavFreqs(const int sfreq, const int afreq);
void getXPNav1ActiveFreq(const int idx, const char* value);
void getXPNav1StandbyFreq(const int idx, const char* value);

#endif

