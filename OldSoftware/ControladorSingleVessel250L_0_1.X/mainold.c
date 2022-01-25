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

bit IN0;
bit IN1;
bit OUT0;
bit OUT1;
unsigned char Inputs2to9;
unsigned char Inputs10to17;
unsigned char Inputss18to25;
unsigned char Outputs2to9;
unsigned char Outputs10to17;

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
        Inputss18to25 = PORTD;
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
    SPBRGH = 2;                                                                 //configura comunicação serial
    SPBRG = 8;                                                                  //para 9600bps
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH = 1;
    TXSTAbits.TXEN = 1;                                                         //transmissao habilitada
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    IPR1bits.RCIP = 0;                                                          //recepção serial gera interrupção de baixa prioridade
    PIE1bits.RCIE = 0;                                                          //desabilita interrupção recepção serial

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

    //configura timer 3 - será usado com CCP1 pra medir fluxo entrada agua
    T3CON = 0b11111001;
    PIE2bits.TMR3IE = 1;
    IPR2bits.TMR3IP = 0;                                                        //baixa prioridade int timer 3

    CCP1CON = 0b00000101;
    IPR1bits.CCP1IP = 0;                                                        //baixa prioridade captura fluxo agua
    PIE1bits.CCP1IE = 1;                                                        //habilita interrupção captura

    //habiltaçao das interrupções
    RCONbits.IPEN = 1;                                                          //habilita 2 niveis de prioridade para as interrupções
    INTCONbits.GIEH = 1;                                                        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 0;                                                        //habilita interrupções com prioridade baixa
}

/*
 * 
 */
void main(void) {
    unsigned int cont;
    unsigned char imagem = 1;
    Inicializacao();
    CCPR2L = 50;
    while(1){
        for(cont=0;cont<50;cont++)
            __delay_ms(10);
        OUT0 = IN0;
        OUT1 = IN1;
        PORTBbits.RB5 = IN0;
        Outputs10to17 = imagem;
        Outputs2to9 = Inputss18to25;
        imagem = imagem << 1;
        if(imagem == 0) imagem = 1;
    }
    return;
}

