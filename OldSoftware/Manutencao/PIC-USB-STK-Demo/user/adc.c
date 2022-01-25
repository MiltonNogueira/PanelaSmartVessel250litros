#include "adc.h"
#include "pwm.h"
#include <p18cxxx.h>

//#define SAMPLE 	255
#define SAMPLE 	1

void InitADC(void) {

	// Disable ADC
	ADCON0 = 0x0;	

	// All as analog input, AVSS and AVDD as referances
	ADCON1 = 0x00;

	// Fadc = Fosc/4, 20 TAD, Right justified
	ADCON2 = 0xBC;

	// Input
	TRISA = 0xFF;

}

unsigned int GetValueAN4(void) {

	unsigned int val = 0;

	// Select Chanel4, enable ADC
	ADCON0 = 0x11;

	// wait for end of conversion
    ADCON0bits.GO = 1;              // Start AD conversion
    while(ADCON0bits.NOT_DONE);     // Wait for conversion

	// get value
	val = ADRES;

	// Disable ADC
	ADCON0 = 0x01;

	return val;

}


// Get average value
unsigned int GetMicValue(void) {

	unsigned int 	j=0;
	
	unsigned int 	val=0;
	unsigned long 	tmp=0;

	for(j=0;j<SAMPLE;j++) {

		tmp += GetValueAN4();
	}
	
	val = tmp/SAMPLE;

	// return val>>3;
	return val>>2;
}
