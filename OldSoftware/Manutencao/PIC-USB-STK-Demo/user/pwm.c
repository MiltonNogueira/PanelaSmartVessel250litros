#include "pwm.h"
#include "adc.h"
#include <p18cxxx.h>


#define SIN_SAPMLE 48
#define MIN_SAPMLE 24
#define FIN_SAPMLE 12

char 	sample=0;
/*

// sinus 1khz - variable
int sin[] = { 	512, 	579, 	645, 	708, 	768, 	824, 	874, 	918, 	955, 	985, 
			   1007,   1020,   1023,   1020,   1007,    985,    955,    918,    874,    824, 
				768,    708,    645,    579,    512,    445,	379, 	316, 	256, 	200, 
				150, 	106, 	 69, 	39, 	 17, 	  4, 	  0, 	  4, 	 17, 	 39, 
				 69,	106, 	150, 	200, 	256, 	316, 	379, 	445 
};

int min[] = { 	32,  	40,  	48,  	54,  	60,  
			    63,     64,     63,     60,     54,  
				48,     40,     32, 	24,  	16,  
				 9,  	 4, 	 2,  	 0,  	 2,  
				 4, 	 9,  	16,  	24, 
};

int fin[] = { 	32,    	48,    	60,     64,     60,    
				48,     32,   	16,   	 4,	  	 0,    
				 4,   	16,     
};

int fin_[] = { 	128,    178,   230,    255,    230,    
				178,    128,   	78,   	18,	  	 0,    
				18,   	 78,     
};
*/

////////////////////////////////////////////////////////
//// Interrupt
////////////////////////////////////////////////////////
// #pragma interrupt_level 1
// static void interrupt ISR(void)
//
// #pragma code low_vector=0x18
// void interrupt_at_low_vector(void)
//
void low_isr(void);
void high_isr(void);
/*
* For PIC18 devices the low interrupt vector is found at
* 00000018h. The following code will branch to the
* low_interrupt_service_routine function to handle
* interrupts that occur at the low vector.
*/
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
_asm GOTO low_isr _endasm
}
#pragma code /* return to the default code section */
#pragma interruptlow low_isr
void low_isr (void)
{
	//timer1 interupt
    if((PIR1bits.TMR1IF)&&(PIE1bits.TMR1IE))
    {
		// Stop Timer		
		T1CONbits.TMR1ON = 0;
		
		// Clear Timer1 counter
		TMR1L = 0x00;
		TMR1H = 0xFF;
		
		// Start Timer	
		T1CONbits.TMR1ON = 1;

		// Clear interupt flag
        PIR1bits. TMR1IF = 0;

		// CCPR2L = (fin_[(sample++)]);
		// if(sample==FIN_SAPMLE)
		//	sample=0;

		CCPR2L = (GetMicValue());

    }

}

void InitTimer1(void) {

	// 16-bit, 1:1 Prescale value, Fosc/4
	// Enables Timer1
	T1CON = 0x01;

	// Clear overflow flag
	// PIR1 = 0;
	PIR1bits.TMR1IF = 0;
	
	// Enable TIMER1 interrupts		
	// PIE1 = 1;
	PIE1bits.TMR1IE = 1;
	
	// Load initial value to TIMER1
	TMR1L = 0x00;
	TMR1H = 0xFF;

	// High priority
	// IPR1bits.TMR1IP = 1;

	// Low priority
	IPR1bits.TMR1IP = 0;

	//enable interrupts
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;          

}

void InitPWM(void) {

	// Set PC1 as input 
	TRISCbits.TRISC1=1;
	
	// Set period for PWM
	PR2 = 0xFF;

	// Set duty period for PWM
	CCPR2L = 0x0;
	
	// Set PWM mode - CCP Chanel 2 PWM
	CCP2CON = 0xFC;
	
	// I don't 
	// CMCON = 0x06;
	
	// RC1 as pwm output
	// CONFIG3H = 0x01;

	// Enable Timer 2
	T2CON = 0x04;
	
	// Enable 
	TRISCbits.TRISC1=0;
	//ECCP1AS = 0x00;
}


