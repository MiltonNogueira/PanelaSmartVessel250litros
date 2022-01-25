/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        user.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       11/19/04    Original.
 ********************************************************************/

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include <usart.h>
#include "system\typedefs.h"

#include "system\usb\usb.h"

#include "io_cfg.h"             // I/O pin mapping
#include "user\user.h"
#include "user\temperature.h"
#include "user\adc.h"
#include "user\pwm.h"

/** V A R I A B L E S ********************************************************/
#pragma udata
byte old_sw1, old_sw2, old_sw3, old_sw4;
byte counter;
byte trf_state;
byte temp_mode;

DATA_PACKET dataPacket;

byte pTemp;                     // Pointer to current logging position, will
                                // loop to zero once the max index is reached
byte valid_temp;                // Keeps count of the valid data points
word temp_data[30];             // 30 points of data

// Timer0 - 1 second interval setup.
// Fosc/4 = 12MHz
// Use /256 prescalar, this brings counter freq down to 46,875 Hz
// Timer0 should = 65536 - 46875 = 18661 or 0x48E5
#define TIMER0L_VAL         0xE5
#define TIMER0H_VAL         0x48

/** P R I V A T E  P R O T O T Y P E S ***************************************/

void BlinkUSBStatus(void);
BOOL Switch2IsPressed(void);
BOOL Switch3IsPressed(void);
void ResetTempLog(void);
void ReadPOT(void);
void ServiceRequests(void);

// For board testing purpose only
void PICDEMFSUSBDemoBoardTest(void);

/** D E C L A R A T I O N S **************************************************/
#pragma code
void UserInit(void)
{
    mInitAllLEDs();
    mInitAllSwitches();
    old_sw1 = sw1;
    old_sw2 = sw2;
    old_sw3 = sw3;
    old_sw4 = sw4;
    
    InitTempSensor();
    mInitPOT();
    ADCON2bits.ADFM = 1;   // ADC result right justified
  
    ResetTempLog();
    temp_mode = TEMP_REAL_TIME;
    
    /* Init Timer0 for data logging interval (every 1 second) */
    T0CON = 0b10010111;
    //T0CONbits.T08BIT = 0;       // 16-bit mode
    //T0CONbits.T0CS = 0;         // Select Fosc/4
    //T0CONbits.PSA = 0;          // Assign prescalar (default is /256)
    /* Timer0 is already enabled by default */
}//end UserInit

//// OLIMEX TEST FUNCTION ==============================================================

#define BIT2  0x04;

volatile unsigned char mask_port_a 	= 0x00; 
volatile unsigned char mask_port_b 	= 0x00;
volatile unsigned char mask_port_c 	= 0x05;
volatile unsigned char mask_port_d 	= 0xF0;
volatile unsigned char mask_port_e 	= 0x07;

volatile unsigned char temp_porta 	= 0x0;
volatile unsigned char temp_portb 	= 0x0;
volatile unsigned char temp_portc 	= 0x0;
volatile unsigned char temp_portd 	= 0x0; 
volatile unsigned char temp_porte 	= 0x0; 

char ERROR = 0;
char i=0;

void Delay(unsigned long a) { 
	 a *= 370;
	while (--a!=0); 
}

void GetPort_AND(void) {

	temp_porta 	= PORTA;
	temp_porta 	&= mask_port_a; 
	temp_portb 	= PORTB;
	temp_portb 	&= mask_port_b; 
	temp_portc 	= PORTC;
	temp_portc 	&= mask_port_c; 
	temp_portd 	= PORTD;
	temp_portd 	&= mask_port_d; 
	temp_porte 	= PORTE;
	temp_porte 	&= mask_port_e;
}

void GetPort_OR(void) {
	temp_porta 	= PORTA;
	temp_porta 	|= (~mask_port_a); 
	temp_portb 	= PORTB;
	temp_portb 	|= (~mask_port_b); 
	temp_portc 	= PORTC;
	temp_portc 	|= (~mask_port_c); 
	temp_portd 	= PORTD;
	temp_portd 	|= (~mask_port_d); 
	temp_porte 	= PORTE;
	temp_porte 	|= (~mask_port_e); 
}


void TestExtension(void) {

	ERROR=0;
	
	// TEST EXTENSION
	// PIN TO GND ====================================
	// all as input
	ADCON1 	= 0x0F;
	TRISA 	= 0xFF;
	TRISC 	= 0xFF;
	TRISB 	= 0xFF;
	TRISD 	= 0xFF;
	TRISE 	= 0xFF;

	//pull up - output
	TRISB &= ~BIT2;
	//pull up - high
	PORTB |= BIT2;

	Delay(10);

	GetPort_OR();

	if( ((temp_porta) != (0xFF)) ||  
		((temp_portb) != (0xFF)) ||  
		((temp_portc) != (0xFF)) ||  
		((temp_portd) != (0xFF)) ||  
		((temp_porte) != (0xFF)) )  {
		
		ERROR=1;
		return;
	}


	// PIN TO VCC ================================
	// all as input
	ADCON1 	= 0x0F;
	TRISA 	= 0xFF;
	TRISC 	= 0xFF;
	TRISB 	= 0xFF;
	TRISD 	= 0xFF;
	TRISE 	= 0xFF;
	
	//pull up - output
	TRISB &= ~BIT2;
	//pull up - low
	PORTB &= ~BIT2;

	Delay(10);

	GetPort_AND();
 

	if( ((temp_porta) != (0x0)) ||
		((temp_portb) != (0x0)) ||
		((temp_portc) != (0x0)) ||
		((temp_portd) != (0x0)) ||
		((temp_porte) != (0x0)) ) { 
		
		ERROR=2;
		return;
	}


	// Running zero =========================================
	
	// Port C
	// all as input
	ADCON1 	= 0x0F;
	TRISA 	= 0xFF;
	TRISC 	= 0xFF;
	TRISB 	= 0xFF;
	TRISD 	= 0xFF;
	TRISE 	= 0xFF;

	//pull up - output
	TRISB &= ~BIT2;
	//pull up - high
	PORTB |= BIT2;

	// loop
	for(i=0; i<8; i++) {
	
		// this port is not tested
		if(!((mask_port_c)&(1<<i))) continue;
		
		TRISC = ~(1<<i);
		PORTC = ~(1<<i);

		Delay(10);

		GetPort_OR();

		// check for other zero at PortC
		if((temp_portc) != (0xFF&(~(1<<i)))) {
			ERROR = 3;
		} 

		// check for other zero at PortD
		if((temp_portd) != (0xFF)) {
			ERROR = 3;
		} 

		// check for other zero at PortE
		if((temp_porte) != (0xFF)) {
			ERROR = 3;
		} 
	}

	// Port D
	// all as input
	ADCON1 	= 0x0F;
	TRISA 	= 0xFF;
	TRISC 	= 0xFF;
	TRISB 	= 0xFF;
	TRISD 	= 0xFF;
	TRISE 	= 0xFF;

	//pull up - output
	TRISB &= ~BIT2;
	//pull up - high
	PORTB |= BIT2;

	// loop
	for(i=0; i<8; i++) {
	
		// this port is not tested
		if(!((mask_port_d)&(1<<i))) continue;
		
		TRISD = ~(1<<i);
		PORTD = ~(1<<i);

		Delay(10);

		GetPort_OR();

		// check for other zero at PortC
		if((temp_portc) != (0xFF)) {
			ERROR = 3;
		} 

		// check for other zero at PortD
		if((temp_portd) != (0xFF&(~(1<<i)))) {
			ERROR = 3;
		} 

		// check for other zero at PortE
		if((temp_porte) != (0xFF)) {
			ERROR = 3;
		} 
		
	}

	// Port E
	// all as input
	ADCON1 	= 0x0F;
	TRISA 	= 0xFF;
	TRISC 	= 0xFF;
	TRISB 	= 0xFF;
	TRISD 	= 0xFF;
	TRISE 	= 0xFF;

	//pull up - output
	TRISB &= ~BIT2;
	//pull up - high
	PORTB |= BIT2;

	// loop
	for(i=0; i<8; i++) {
	
		// this port is not tested
		if(!((mask_port_e)&(1<<i))) continue;
		
		TRISE = ~(1<<i);
		PORTE = ~(1<<i);

		Delay(10);

		GetPort_OR();

		// check for other zero at PortC
		if((temp_portc) != (0xFF)) {
			ERROR = 3;
		} 

		// check for other zero at PortD
		if((temp_portd) != (0xFF)) {
			ERROR = 3;
		} 

		// check for other zero at PortE
		if((temp_porte) != (0xFF&(~(1<<i)))) {
			ERROR = 3;
		} 
		
	}

}

//// END OLIMEX TEST FUNCTION ==========================================================




/******************************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user routines.
 *                  It is a mixture of both USB and non-USB tasks.
 *
 * Note:            None
 *****************************************************************************/
void ProcessIO(void)
{   
    BlinkUSBStatus();
    // User Application USB tasks
    if((usb_device_state < CONFIGURED_STATE)||(UCONbits.SUSPND==1)) return;
    
    ServiceRequests();

    if(temp_mode == TEMP_LOGGING)
    {
        if(INTCONbits.TMR0IF == 1)
        {
            INTCONbits.TMR0IF = 0;          // Clear flag
            TMR0H = TIMER0H_VAL;
            TMR0L = TIMER0L_VAL;            // Reinit timer value;

            if(AcquireTemperature())
            {
                temp_data[pTemp] = temperature._word;
                
                // First update valid_temp
                if(valid_temp < 30)         // 30 data points max
                    valid_temp++;
                    
                // Next update pTemp
                if(pTemp == 29)
                    pTemp = 0;
                else
                    pTemp++;
            }//end if
        }//end if
    }//end if
}//end ProcessIO

void ResetTempLog(void)
{
    pTemp = 0;
    valid_temp = 0;
}//end ResetLog

void ReadPOT(void)
{
    ADCON0bits.GO = 1;              // Start AD conversion
    while(ADCON0bits.NOT_DONE);     // Wait for conversion
    return;
}//end ReadPOT

void ServiceRequests(void)
{
    byte index;
    
    if(USBGenRead((byte*)&dataPacket,sizeof(dataPacket)))
    {
        counter = 0;
        switch(dataPacket.CMD)
        {
            case READ_VERSION:
                //dataPacket._byte[1] is len
                dataPacket._byte[2] = MINOR_VERSION;
                dataPacket._byte[3] = MAJOR_VERSION;
                counter=0x04;
                break;

            case ID_BOARD:
                counter = 0x01;
                if(dataPacket.ID == 0)
                {
                    mLED_3_Off();mLED_4_Off();
                }
                else if(dataPacket.ID == 1)
                {
                    mLED_3_Off();mLED_4_On();
                }
                else if(dataPacket.ID == 2)
                {
                    mLED_3_On();mLED_4_Off();
                }
                else if(dataPacket.ID == 3)
                {
                    mLED_3_On();mLED_4_On();
                }
                else
                    counter = 0x00;
                break;

            case UPDATE_LED:
                // LED1 & LED2 are used as USB event indicators.
                if(dataPacket.led_num == 3)
                {
                    mLED_3 = dataPacket.led_status;
                    counter = 0x01;
                }//end if
                else if(dataPacket.led_num == 4)
                {
                    mLED_4 = dataPacket.led_status;
                    counter = 0x01;
                }//end if else
                break;
                
            case SET_TEMP_REAL:
                temp_mode = TEMP_REAL_TIME;
                ResetTempLog();
                counter = 0x01;
                break;

            case RD_TEMP:
                if(AcquireTemperature())
                {
                    dataPacket.word_data = temperature._word;
                    counter=0x03;
                }//end if
                break;

            case SET_TEMP_LOGGING:
                temp_mode = TEMP_LOGGING;
                ResetTempLog();
                counter=0x01;
                break;

            case RD_TEMP_LOGGING:
                counter = (valid_temp<<1)+2;  // Update count in byte
                dataPacket.len = (valid_temp<<1);

                for(index = valid_temp; index > 0; index--)
                {
                    if(pTemp == 0)
                        pTemp = 29;
                    else
                        pTemp--;
                    dataPacket._word[index] = temp_data[pTemp];
                }//end for
                
                ResetTempLog();             // Once read, log will restart
                break;

            case RD_POT:
                ReadPOT();
                dataPacket._byte[1] = ADRESL;
                dataPacket._byte[2] = ADRESH;
                counter=0x03;
                break;
                
            case RESET:
                Reset();
                break;
                
            default:
                break;
        }//end switch()
        if(counter != 0)
        {
            if(!mUSBGenTxIsBusy())
                USBGenWrite((byte*)&dataPacket,counter);
        }//end if
    }//end if

}//end ServiceRequests

/******************************************************************************
 * Function:        void BlinkUSBStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        BlinkUSBStatus turns on and off LEDs corresponding to
 *                  the USB device state.
 *
 * Note:            mLED macros can be found in io_cfg.h
 *                  usb_device_state is declared in usbmmap.c and is modified
 *                  in usbdrv.c, usbctrltrf.c, and usb9.c
 *****************************************************************************/
void BlinkUSBStatus(void)
{
    static word led_count=0;
    
    if(led_count == 0)led_count = 10000U;
    led_count--;

    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

    if(UCONbits.SUSPND == 1)
    {
        if(led_count==0)
        {
            mLED_1_Toggle();
            mLED_2 = mLED_1;        // Both blink at the same time
        }//end if
    }
    else
    {
        if(usb_device_state == DETACHED_STATE)
        {
            mLED_Both_Off();
            
            PICDEMFSUSBDemoBoardTest();
        }
        else if(usb_device_state == ATTACHED_STATE)
        {
            mLED_Both_On();
        }
        else if(usb_device_state == POWERED_STATE)
        {
            mLED_Only_1_On();
        }
        else if(usb_device_state == DEFAULT_STATE)
        {
            mLED_Only_2_On();
        }
        else if(usb_device_state == ADDRESS_STATE)
        {
            if(led_count == 0)
            {
                mLED_1_Toggle();
                mLED_2_Off();
            }//end if
        }
        else if(usb_device_state == CONFIGURED_STATE)
        {
            if(led_count==0)
            {
                mLED_1_Toggle();
                mLED_2 = !mLED_1;       // Alternate blink                
            }//end if
        }//end if(...)
    }//end if(UCONbits.SUSPND...)

}//end BlinkUSBStatus

BOOL Switch1IsPressed(void)
{
    if(sw1 != old_sw1)
    {
        old_sw1 = sw1;                  // Save new value
        if(sw1 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch1IsPressed

BOOL Switch2IsPressed(void)
{
    if(sw2 != old_sw2)
    {
        old_sw2 = sw2;                  // Save new value
        if(sw2 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch2IsPressed

BOOL Switch3IsPressed(void)
{
    if(sw3 != old_sw3)
    {
        old_sw3 = sw3;                  // Save new value
        if(sw3 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch3IsPressed

BOOL Switch4IsPressed(void)
{
    if(sw4 != old_sw4)
    {
        old_sw4 = sw4;                  // Save new value
        if(sw4 == 0)                    // If pressed
            return TRUE;                // Was pressed
    }//end if
    return FALSE;                       // Was not pressed
}//end Switch4IsPressed

void TXbyte(byte data)
{
    while(TXSTAbits.TRMT==0);
    TXREG = data;
}//end TXbyte

void WriteString(unsigned char* str) {
	
	unsigned char i=0;
	i=0;

	while(str[i]!='\0') {
		TXbyte(str[i]);
		i++;
	}
}

void PICDEMFSUSBDemoBoardTest(void)
{
    byte temp;
    
    //PICDEM FS USB Demo Board Test Procedure:
    if(Switch2IsPressed())
    {
        //LEDs and push buttons testing
        mLED_1_On();
        while(!Switch1IsPressed());
        mLED_1_Off();
        mLED_2_On();
        while(!Switch2IsPressed());
        mLED_2_Off();
        mLED_3_On();
        while(!Switch3IsPressed());
        mLED_3_Off();
        mLED_4_On();
        while(!Switch4IsPressed());
        mLED_4_Off();
        
        //RS-232 Setup
        SSPCON1 = 0;        // Make sure SPI is disabled
        TRISCbits.TRISC7=1; // RX
        TRISCbits.TRISC6=0; // TX
        SPBRG = 0x71;
        SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud
        TXSTA = 0x24;       // TX enable BRGH=1
        RCSTA = 0x90;       // continuous RX
        BAUDCON = 0x08;     // BRG16 = 1
        temp = RCREG;       // Empty buffer
        temp = RCREG;       // Empty buffer
        
		
		// WriteString("\n\rPress B3 to continue test\0");
		TXbyte('\n'); 
		TXbyte('\r'); 
		TXbyte('T');	 
		TXbyte('e'); 
		TXbyte('s'); 
		TXbyte('t');
		TXbyte(' ');
		TXbyte('R');
		TXbyte('S');
		TXbyte('2');
		TXbyte('3');
		TXbyte('2');
		TXbyte(' ');
		TXbyte('T');
		TXbyte('X');
		TXbyte(' ');
		TXbyte('p'); 
		TXbyte('r'); 
		TXbyte('e'); 
		TXbyte('s'); 
		TXbyte('s'); 
		TXbyte(' '); 
		TXbyte('B'); 
		TXbyte('3'); 
		TXbyte(' '); 


        //RS-232 Tx & Rx Tests
        while(!Switch3IsPressed());
		TXbyte('\n'); 
		TXbyte('\r'); 
        TXbyte('T');
        TXbyte('X');
        TXbyte(' ');
        TXbyte('T');
        TXbyte('e');
        TXbyte('s');
        TXbyte('t');
        TXbyte(' ');
        TXbyte('O');
        TXbyte('K');
        TXbyte('!');
        TXbyte(' ');
        TXbyte('T');
        TXbyte('e');
        TXbyte('s');
        TXbyte('t');
        TXbyte(' ');
        TXbyte('R');
        TXbyte('S');
        TXbyte('-');
        TXbyte('2');
        TXbyte('3');
        TXbyte('2');
        TXbyte(' ');
        TXbyte('R');
        TXbyte('X');
        TXbyte(' ');
        TXbyte('P');
        TXbyte('r');
        TXbyte('e');
        TXbyte('s');
        TXbyte('s');
        TXbyte(' ');
        TXbyte('"');
        TXbyte('R');
        TXbyte('"');
        TXbyte(' ');
        while(PIR1bits.RCIF==0);        //Wait for data from RS232
        if(RCREG == 'R')
        {
            TXbyte(' ');
            TXbyte('R');
            TXbyte('X');
            TXbyte(' ');
            TXbyte('T');
            TXbyte('e');
            TXbyte('s');
            TXbyte('t');
            TXbyte(' ');
            TXbyte('O');
            TXbyte('K');
        }//end if


		TXbyte('\n');
	    TXbyte('\r');
        TXbyte('P');
        TXbyte('u');
        TXbyte('t');
        TXbyte(' ');
        TXbyte('e');
        TXbyte('x');
        TXbyte('t');
        TXbyte('e');
        TXbyte('n');
        TXbyte('s');
        TXbyte('i');
        TXbyte('o');
        TXbyte('n');
        TXbyte(' ');
        TXbyte('P');
        TXbyte('r');
        TXbyte('e');
        TXbyte('s');
        TXbyte('s');
        TXbyte(' ');
        TXbyte('"');
        TXbyte('T');
        TXbyte('"');
        TXbyte(' ');
        while(PIR1bits.RCIF==0);        //Wait for data from RS232
        if(RCREG == 'T')
        {
			// test extension
			TestExtension();

			//RS-232 Setup
			SSPCON1 = 0;        // Make sure SPI is disabled
			TRISCbits.TRISC7=1; // RX
			TRISCbits.TRISC6=0; // TX
			SPBRG = 0x71;
			SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud
			TXSTA = 0x24;       // TX enable BRGH=1
			RCSTA = 0x90;       // continuous RX
			BAUDCON = 0x08;     // BRG16 = 1
			temp = RCREG;       // Empty buffer
			temp = RCREG;       // Empty buffer


			if(ERROR==0) {
				TXbyte('\n');
				TXbyte('\r');
				TXbyte('T');
	            TXbyte('e');
	            TXbyte('s');
	            TXbyte('t');
	            TXbyte(' ');
	            TXbyte('O');
	            TXbyte('K');
				TXbyte('!');
			}
			else if(ERROR==1) {
				TXbyte('\n');
				TXbyte('\r');
				TXbyte('P');
	            TXbyte('i');
	            TXbyte('n');
	            TXbyte(' ');
	            TXbyte('t');
	            TXbyte('o');
	            TXbyte(' ');
	            TXbyte('G');
	            TXbyte('N');
				TXbyte('D');
	            TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
			}
			else if(ERROR==2) {
				TXbyte('\n');
				TXbyte('\r');
				TXbyte('P');
	            TXbyte('i');
	            TXbyte('n');
	            TXbyte(' ');
	            TXbyte('t');
	            TXbyte('o');
	            TXbyte(' ');
	            TXbyte('V');
	            TXbyte('C');
				TXbyte('C');
	            TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
			}
			else if(ERROR==3) {
				TXbyte('\n');
				TXbyte('\r');
				TXbyte('P');
	            TXbyte('i');
	            TXbyte('n');
	            TXbyte(' ');
	            TXbyte('s');
	            TXbyte('h');
	            TXbyte('o');
	            TXbyte('r');
	            TXbyte('t');
				TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
	            TXbyte('!');
			}
		}

        //RS-232 Setup
        SSPCON1 = 0;        // Make sure SPI is disabled
        TRISCbits.TRISC7=1; // RX
        TRISCbits.TRISC6=0; // TX
        SPBRG = 0x71;
        SPBRGH = 0x02;      // 0x0271 for 48MHz -> 19200 baud
        TXSTA = 0x24;       // TX enable BRGH=1
        RCSTA = 0x90;       // continuous RX
        BAUDCON = 0x08;     // BRG16 = 1
        temp = RCREG;       // Empty buffer
        temp = RCREG;       // Empty buffer


		TXbyte('\n');
	    TXbyte('\r');
        TXbyte('P');
        TXbyte('r');
        TXbyte('e');
        TXbyte('s');
        TXbyte('s');
        TXbyte(' ');
        TXbyte('"');
        TXbyte('A');
        TXbyte('"');
        TXbyte(' ');
        TXbyte('f');
        TXbyte('o');
        TXbyte('r');
        TXbyte(' ');
        TXbyte('a');
        TXbyte('u');
        TXbyte('d');
        TXbyte('i');
        TXbyte('o');
        TXbyte(' ');
        TXbyte('t');
        TXbyte('e');
        TXbyte('s');
        TXbyte('t');
        TXbyte(' ');
        while(PIR1bits.RCIF==0);        //Wait for data from RS232
        if(RCREG == 'A')
        {
			// audio extension
			
			InitADC();
			InitPWM();
			InitTimer1();

			while(1);
		}

        UserInit();                     //Re-initialize default user fw
        //Test phase 1 done
    }//end if
}//end PICDEMFSUSBDemoBoardTest()

/** EOF user.c ***************************************************************/
