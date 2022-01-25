/*******************************************************************************
 * www.neuheitbier.com.br
 * Funçoes do display DWIN panela cervejeira smart vessel
 * Autor: Milton Nogueira
 *
 * Criado em fevereiro de 2017
 * MPLAB X v2.35, XC8 v1.34
 *
 ******************************************************************************/

#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "display.h"

#define _XTAL_FREQ      20000000                // oscilador interno em valor default
#define DELAYDWIN 80
//Cabeçalho do frame do display DWIN
#define CABECALHOHIGH 0x5A
#define CABECALHOLOW 0xA5

extern unsigned char SerialOut[40];
extern unsigned char SerialIn[40];

void DelayDWIN(void){
    unsigned char cont;

    for(cont=0;cont<DELAYDWIN;cont++)
        __delay_ms(1);
}

/*
 * Escrever na memória RAM do display DWIN
 */
void EscreverDWINram(unsigned int endereco,int valor){

    unsigned char cont;

    //PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 5;               //bytes msg + endereco(2bytes) + valor(2bytes)
    SerialOut[3] = 0x82;            //indica escrita na RAM
    SerialOut[4] = endereco >> 8;
    SerialOut[5] = endereco & 0x00FF;
    SerialOut[6] = valor >> 8;
    SerialOut[7] = valor & 0x00FF;
    PIR1bits.TXIF = 0;
    for(cont=0;cont<8;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    DelayDWIN();                    //delay para display processar informações

    //PIE1bits.RCIE = 1;          //reabilita interrupção recepção serial
}

/*
 * Escrever em um registrador do display DWIN
 */
void EscreverDWINreg(unsigned char endereco,int valor){

    unsigned char cont;

    //PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;               //bytes msg + endereco(1byte) + valor(2bytes)
    SerialOut[3] = 0x80;            //indica escrita no registrador
    SerialOut[4] = endereco;
    SerialOut[5] = valor >> 8;
    SerialOut[6] = valor & 0x00FF;
    PIR1bits.TXIF = 0;
    for(cont=0;cont<7;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    DelayDWIN();                    //delay para display processar informações
    //PIE1bits.RCIE = 1;          //reabilita interrupção recepção serial
}

/*
    Realiza leitura em um endereço da memoria RAM do display DWIN
 */
int LerDWINram(unsigned int endereco)
{
    unsigned char cont;
    unsigned int VPrecebido;

    //PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;               //bytes msg + endereco(2bytes) + tamanho(1byte)
    SerialOut[3] = 0x83;            //indica leitura da RAM do display DWIN
    SerialOut[4] = endereco >> 8;
    SerialOut[5] = endereco & 0x00FF;
    SerialOut[6] = 1;               //esta função só lê 1 palavra (word)
    PIR1bits.TXIF = 0;
    for(cont=0;cont<7;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    PIR1bits.RCIF = 0;

    for(cont=0;cont<9;cont++){
        while(!PIR1bits.RCIF);
        PIR1bits.RCIF = 0;
        SerialIn[cont] = RCREG;
    }
    VPrecebido = SerialIn[7] << 8;
    VPrecebido += SerialIn[8];

    //PIE1bits.RCIE = 1;          //reabilita interrupção recepção serial
    DelayDWIN();                    //delay para display processar informações
    return(VPrecebido);
}

/*
    Realiza leitura em um registrador do display DWIN
 */
int LerDWINreg(unsigned char endereco)
{
    unsigned char cont;
    unsigned char recebido;

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 3;               //bytes msg (1byte) + endereco(1byte) + n.o reg a ler(1byte)
    SerialOut[3] = 0x81;            //indica leitura registrador do display DWIN
    SerialOut[4] = endereco;
    SerialOut[5] = 2;               //esta função só lê 1 palavra (word)
    PIR1bits.TXIF = 0;
    for(cont=0;cont<6;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    PIR1bits.RCIF = 0;

    for(cont=0;cont<8;cont++){
        while(!PIR1bits.RCIF);
        PIR1bits.RCIF = 0;
        SerialIn[cont] = RCREG;
    }
    recebido = SerialIn[6] >> 8;
    recebido += SerialIn[7];

    //PIE1bits.RCIE = 1;          //reabilita interrupção recepção serial
    DelayDWIN();                    //delay para display processar informações
    return(recebido);
}

/*
 * Escrever em um registrador do display DWIN
 */
void EscreverDWINtrend(int valor){

    unsigned char cont;

    //PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;               //bytes msg + canal 0 + valor
    SerialOut[3] = 0x84;            //indica escrita no registrador
    SerialOut[4] = 1;               //canal 0, ver doc. DWIN
    SerialOut[5] = valor >> 8;
    SerialOut[6] = valor & 0x00FF;
    PIR1bits.TXIF = 0;
    for(cont=0;cont<7;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    DelayDWIN();                    //delay para display processar informações
    //PIE1bits.RCIE = 1;          //reabilita interrupção recepção serial
}