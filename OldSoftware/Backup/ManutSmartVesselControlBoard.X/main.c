/* 
 * File:   main.c
 * Author: Lenovo
 *
 * Created on 12 de Dezembro de 2016, 14:14
 */

#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "CpuConfigWords.h"

#define _XTAL_FREQ      20000000                // oscilador interno em valor default
//Cabeçalho do frame do display DWIN
#define CABECALHOHIGH 0x5A
#define CABECALHOLOW 0xA5
//Definicoes dos enderecos das variaveis no display DWIN
#define DELAYDWIN 80

bit IN0;
bit IN1;
bit OUT0;
bit OUT1;
unsigned char Inputs2to9;
unsigned char Inputs10to17;
unsigned char Inputs18to25;
unsigned char Outputs2to9;
unsigned char Outputs10to17;
unsigned char SerialIn[40];
unsigned char SerialOut[40];

/*******************************************************************************
 * Tratamento interrupções com prioridade alta
 ******************************************************************************/
void interrupt HighISR(void){
    //interrupcao timer 0 a cada 5ms.
    //seta flag que será usada como base de tempo
    //no laço principal para verificar o tempo de ciclo
    if(INTCONbits.T0IE && INTCONbits.T0IF){
        TMR0L = 60;
        INTCONbits.T0IF = 0;
        IN0 = PORTAbits.RA4;
        IN1 = PORTCbits.RC2;
        TRISD = 0xFF;
        PORTE = 1;                                                              //chip select entradas digitais
        NOP();
        NOP();
        NOP();
        Inputs2to9 = PORTD;
        PORTE = 3;                                                              //chip select entradas digitais
        NOP();
        NOP();
        NOP();
        Inputs10to17 = PORTD;
        PORTE = 4;                                                              //chip select entradas digitais
        NOP();
        NOP();
        NOP();
        Inputs18to25 = PORTD;
        PORTE = 2;
        NOP();
        TRISD = 0;
        NOP();
        NOP();
        NOP();
        PORTD = Outputs2to9;
        NOP();
        PORTE = 5;
        NOP();
        NOP();
        NOP();
        PORTD = Outputs10to17;
        NOP();
        PORTE = 0;
        NOP();
        TRISD = 0xFF;
        PORTBbits.RB2 = OUT0;
        PORTBbits.RB4 = OUT1;
    }
}

/*******************************************************************************
 * Tratamento de interrupções com prioridade baixa
 ******************************************************************************/
void interrupt low_priority LowISR(void){

    static unsigned char Indice;
    //recebe carateres pela serial e armazena em buffer até receber carriage return
    if(PIE1bits.RCIE && PIR1bits.RCIF){
        PIR1bits.RCIF = 0;  //habilita receber novos caracteres
        SerialIn[Indice] = RCREG;
        Indice++;
        if(RCREG == '\r'){  //recebeu carriage return
            Indice = 0;
        }
    }
}

/*
 * Inicialização do microcontrolador
 *
 * Configuração dos pinos como entrada ou saída
 * Configuração dos periféricos internos do micro
 * Configuração das interrupções e suas prioridades (alta ou baixa)
 */
void Inicializacao(void)
{
    //Configuração dos ports da A a E
    ADCON0 = 0;                                                                 //seleciona canal 0
    ADCON1 = 0b00011010;                                                        //RA0 a RA3 como analogicos, +vref externo
    ADCON2 = 0b10100101;
    CMCON = 7;                                                                  //desliga comparadores
    TRISA = 0x3F;                                                               //PORTA todos pinos entrada
    PORTA = 0;
    TRISB = 0b11000011;                                                         //RB0,RB1 comiun. I2C,RB2..RB5 saidas, RB6,RB7 ICD
    PORTB = 0;
    TRISC = 0xFF;                                                               //Todos pinos port C como entrada
    PORTC = 0;
    TRISD = 0xFF;                                                               //PORTD canal de 8 bits para os TTL 74 entradas e saidas
    PORTD = 0;
    TRISE = 0;                                                                  //RE0..RE2 ligado a 78138 para chip select
    PORTE = 0;                                                                  

    //configuração da comunicação serial
    SPBRGH = 2;                 //configura comunicação serial
    SPBRG = 8;                  //para 9600bps
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH = 1;
    TXSTAbits.TXEN = 1;         //transmissao habilitada
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    IPR1bits.RCIP = 0;          //recepção serial gera interrupção de baixa prioridade
    PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

    ADCON0bits.ADON = 1;                                                        //habilita modulo conversor A/D

    //configura timer 0 para gerar int a cada 5ms
    T0CON = 0b11000110;                                                         //timer 0 ligado, com prescaler de 256
    TMR0L = 60;
    INTCON2bits.T0IP = 1;                                                       //timer 0 gera int de alta prioridade
    INTCONbits.TMR0IE = 1;                                                      //habilita interrupção timer 0

    PR2 = 100;
    CCPR2L = 0x1F;
    T2CON = 0b00000100;                                                         //pre scaler 1
    CCP2CON = 0x0F;                                                             //PWM

    //habiltaçao das interrupções
    RCONbits.IPEN = 1;                                                          //habilita 2 niveis de prioridade para as interrupções
    INTCONbits.GIEH = 1;                                                        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 1;                                                        //habilita interrupções com prioridade baixa
}

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

    //PIE1bits.RCIE = 0;          //desabilita interrupção recepção serial

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
 * 
 */
void main(void) {
    unsigned int cont;
    unsigned int imagem;
    unsigned int temp;

    Inicializacao();
    temp = 0;
    imagem = 0;
    CCPR2L = 50;
    EscreverDWINram(0x1A,0);
    //EscreverDWINram(0x1B,0);
    Outputs2to9 = 0;
    while(1){
        
        temp = LerDWINram(0x001A);
        DelayDWIN();
        //imagem = LerDWINram(0x001B);
        switch(temp){
            case 1: OUT0 = !OUT0;break;                                         //SD0
            case 2: OUT1 = !OUT1;break;                                         //SD1
            case 4: if((Outputs2to9 && 1) == 1)                                 //SD2
                        Outputs2to9 = Outputs2to9 & 0b11111110;
                    else
                        Outputs2to9 = Outputs2to9 | 1;
                    break;
            case 8: if((Outputs2to9 && 2) == 2)                                 //SD3
                        Outputs2to9 = Outputs2to9 & 0b11111101;
                    else
                        Outputs2to9 = Outputs2to9 | 2;
                    break;
            case 16:if((Outputs2to9 && 4) == 4)                                 //SD4
                        Outputs2to9 = Outputs2to9 & 0b11111011;
                    else
                        Outputs2to9 = Outputs2to9 | 4;
                    break;
            case 32: if((Outputs2to9 && 8) == 8)                                //SD5
                        Outputs2to9 = Outputs2to9 & 0b11110111;
                    else
                        Outputs2to9 = Outputs2to9 | 8;
                    break;
            case 64:if((Outputs2to9 && 16) == 16)                               //SD6
                        Outputs2to9 = Outputs2to9 & 0b11101111;
                    else
                        Outputs2to9 = Outputs2to9 | 16;
                    break;
            case 128:if((Outputs2to9 && 32) == 32)                               //SD7
                        Outputs2to9 = Outputs2to9 & 0b11011111;
                    else
                        Outputs2to9 = Outputs2to9 | 32;
                    break;
            case 256:if((Outputs2to9 && 64) == 64)                              //SD8
                        Outputs2to9 = Outputs2to9 & 0b10111111;
                    else
                        Outputs2to9 = Outputs2to9 | 64;
                    break;
            case 512:if((Outputs2to9 && 128) == 128)                            //SD9
                        Outputs2to9 = Outputs2to9 & 0b01111111;
                    else
                        Outputs2to9 = Outputs2to9 | 128;
                    break;
            default: break;
        }
        if(temp > 0){
            EscreverDWINram(0x1A,0);
            for(cont=0;cont<50;cont++)
                __delay_ms(10);
        }

        //OUT0 = IN0;
        //OUT1 = IN1;
        //PORTBbits.RB5 = IN0;
        //Outputs2to9 = temp & 0x00FF;
        //Outputs10to17 = imagem & 0x00FF;
        //Outputs2to9 = Inputs10to17;
        //imagem = imagem << 1;
        //if(imagem == 0) imagem = 1;
    }
    return;
}

