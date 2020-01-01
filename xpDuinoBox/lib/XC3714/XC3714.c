#include <Arduino.h>
#include "XC3714.h"


void clockToggle() {
	digitalWrite(MAX7219CLK, HIGH);   
	digitalWrite(MAX7219CLK, LOW);   //CLK toggle    
}

void MAX7219senddata(byte reg, byte data){
	digitalWrite(MAX7219CS, LOW);   //CS on
	shiftOut(MAX7219DIN, MAX7219CLK, MSBFIRST, reg);
	shiftOut(MAX7219DIN, MAX7219CLK, MSBFIRST, data);
	digitalWrite(MAX7219CS, HIGH);   //CS off
}

void MAX7219brightness(byte b){  //0-15 is range high nybble is ignored
	MAX7219senddata(BRIGHTNESS_REG,b);        //intensity  
}

void blankDigit(const int d) {
	MAX7219senddata(d, 0);
}

void MAX7219init(){
	pinMode(MAX7219DIN, OUTPUT);
	pinMode(MAX7219CS, OUTPUT);
	pinMode(MAX7219CLK, OUTPUT);
	digitalWrite(MAX7219CS, HIGH); //CS off
	digitalWrite(MAX7219CLK, LOW); //CLK low
	MAX7219senddata(TESTMODE_REG, 0);        //test mode off
	MAX7219senddata(SHUTDOWN_REG, 0);        //display off
	MAX7219senddata(DECODE_REG, 255);       //decode all digits
	MAX7219senddata(SCAN_LIMIT_REG, 7);        //scan all
	for(int i = 1; i < 9; i++) {
		blankDigit(i);		//blank all
	}
	MAX7219senddata(SHUTDOWN_REG, 1);        //display on
}

void MAX7219shownum(unsigned long n, const int dpPos){
	unsigned long k = n;
	byte blank = 0;
	for(int i = 1; i < 9; i++){
		if (blank) {
			if (i <= dpPos) {
				MAX7219senddata(dpPos, 0);
			} else {
				MAX7219senddata(i, 15);        
			}
		} else {
			if ((i != 0) && (i == dpPos)) {
				MAX7219senddata(i, (k % 10) | DP);
			} else {
				MAX7219senddata(i, k % 10);
			}
		}
		k = k / 10;
		if (k == 0) {
			blank=1;
		}
	}
}

