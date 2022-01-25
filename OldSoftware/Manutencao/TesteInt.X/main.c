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

unsigned char contador;

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
        contador++;
        if(PORTBbits.RB4)PORTBbits.RB4 = 0;
        else PORTBbits.RB4 = 1;
    }
}

/*******************************************************************************
 * Tratamento de interrupções com prioridade baixa
 ******************************************************************************/
void interrupt low_priority LowISR(void){

    //interrupção timer 1 a cada 1 seg, pois há um cristal de 32768Hz ligado
    //nos T1OSI e T1OSO. Sendo o timer 1 de 16 bits, carregando TMR1H com 0x80
    //incrementará de 32768 a 65535, por isso uma int a cada 1 seg
    if(PIE1bits.TMR1IE && PIR1bits.TMR1IF){
        TMR1H = 0x80;
        TMR1L = 0;
        PIR1bits.TMR1IF = 0;             //limpa flag para novas interrupções do TMR1
        if(PORTBbits.RB2)PORTBbits.RB2 = 0;
        else PORTBbits.RB2 = 1;
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
    TRISD = 0;                                                               //PORTD canal de 8 bits para os TTL 74 entradas e saidas
    PORTD = 0;
    TRISE = 0;                                                                  //RE0..RE2 ligado a 78138 para chip select
    PORTE = 0;                                                                  

    RCONbits.IPEN = 1;                                                          //habilita 2 niveis de prioridade para as interrupções

    //configura timer 0 para gerar int a cada 5ms
    T0CON = 0b11000110;                                                         //timer 0 ligado, com prescaler de 256
    TMR0L = 60;
    INTCON2bits.T0IP = 1;                                                       //timer 0 gera int de alta prioridade
    INTCONbits.TMR0IE = 1;                                                      //habilita interrupção timer 0

    //configuração do timer 1 para interromper a cada 1seg
    IPR1bits.TMR1IP = 0;    //timer 1 (relogio) é uma interrupção de prioridade baixa
    T1CONbits.T1SYNC = 1;     //timer assincrono
    T1CONbits.TMR1CS = 1;     //fonte de clock externa
    T1CONbits.T1OSCEN = 1;    //oscilador externo
    PIE1bits.TMR1IE = 1;     //habilita interrupcao timer 1
    T1CONbits.T1CKPS0 = 0;    //prescaler zero, portanto
    T1CONbits.T1CKPS1 = 0;    //TMR1 gerará uma int a cada 2 seg
    T1CONbits.TMR1ON = 1;     //habilta TMR1

    //habiltaçao das interrupções
    INTCONbits.GIEH = 1;                                                        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 1;                                                        //habilita interrupções com prioridade baixa

}

/*
 * 
 */
void main(void) {

    unsigned char output;
    
    Inicializacao();
    PORTE = 2;
    NOP();
    TRISD = 0;
    NOP();
    NOP();
    NOP();
    PORTD = 0;
    PORTE = 0;

    output = 1;
    contador = 0;

    while(1){
        output = output << 1;
        if(output == 0)
            output = 1;
        NOP();
        NOP();
        PORTE = 2;
        NOP();
        NOP();
        PORTD = output;
        NOP();
        NOP();
        PORTE = 0;
        while(contador < 20);
        contador = 0;
    }
}