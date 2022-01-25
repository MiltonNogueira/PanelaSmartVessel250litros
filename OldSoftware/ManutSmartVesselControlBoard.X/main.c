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
#define MEDIA_AD 25

bit IN0;
bit IN1;
bit OUT0;
bit OUT1;
unsigned char Inputs2to9;
unsigned char Inputs10to17;
unsigned char Inputs18to25;
unsigned char Outputs2to9;
unsigned char Outputs10to17;
unsigned int ValoresAD[4];
unsigned long PressaommCA;
#define IN2 ( (Inputs2to9 & 1) ? (1) : (0) )
#define IN3 ( (Inputs2to9 & 2) ? (1) : (0) )
#define IN4 ( (Inputs2to9 & 4) ? (1) : (0) )
#define IN5 ( (Inputs2to9 & 8) ? (1) : (0) )
#define IN6 ( (Inputs2to9 & 16) ? (1) : (0) )
#define IN7 ( (Inputs2to9 & 32) ? (1) : (0) )
#define IN8 ( (Inputs2to9 & 64) ? (1) : (0) )
#define IN9 ( (Inputs2to9 & 128) ? (1) : (0) )
#define IN10 ( (Inputs10to17 & 1) ? (1) : (0) )
#define IN11 ( (Inputs10to17 & 2) ? (1) : (0) )
#define IN12 ( (Inputs10to17 & 4) ? (1) : (0) )
#define IN13 ( (Inputs10to17 & 8) ? (1) : (0) )
#define IN14 ( (Inputs10to17 & 16) ? (1) : (0) )
#define IN15 ( (Inputs10to17 & 32) ? (1) : (0) )
#define IN16 ( (Inputs10to17 & 64) ? (1) : (0) )
#define IN17 ( (Inputs10to17 & 128) ? (1) : (0) )
#define IN18 ( (Inputs18to25 & 1) ? (1) : (0) )
#define IN19 ( (Inputs18to25 & 2) ? (1) : (0) )
#define IN20 ( (Inputs18to25 & 4) ? (1) : (0) )
#define IN21 ( (Inputs18to25 & 8) ? (1) : (0) )
#define IN22 ( (Inputs18to25 & 16) ? (1) : (0) )
#define IN23 ( (Inputs18to25 & 32) ? (1) : (0) )
#define IN24 ( (Inputs18to25 & 64) ? (1) : (0) )
#define IN25 ( (Inputs18to25 & 128) ? (1) : (0) )
#define FOUT2 ( (Outputs2to9 & 1) ? (1) : (0) )
#define OUT2ON Outputs2to9 |= 1
#define OUT2OFF Outputs2to9 &= 0xFE
#define FOUT3 ( (Outputs2to9 & 2) ? (1) : (0) )
#define OUT3ON Outputs2to9 |= 2
#define OUT3OFF Outputs2to9 &= 0xFD
#define FOUT4 ( (Outputs2to9 & 4) ? (1) : (0) )
#define OUT4ON Outputs2to9 |= 4
#define OUT4OFF Outputs2to9 &= 0xFB
#define FOUT5 ( (Outputs2to9 & 8) ? (1) : (0) )
#define OUT5ON Outputs2to9 |= 8
#define OUT5OFF Outputs2to9 &= 0xF7
#define FOUT6 ( (Outputs2to9 & 16) ? (1) : (0) )
#define OUT6ON Outputs2to9 |= 16
#define OUT6OFF Outputs2to9 &= 0xEF
#define FOUT7 ( (Outputs2to9 & 32) ? (1) : (0) )
#define OUT7ON Outputs2to9 |= 32
#define OUT7OFF Outputs2to9 &= 0xDF
#define FOUT8 ( (Outputs2to9 & 64) ? (1) : (0) )
#define OUT8ON Outputs2to9 |= 64
#define OUT8OFF Outputs2to9 &= 0xBF
#define FOUT9 ( (Outputs2to9 & 128) ? (1) : (0) )
#define OUT9ON Outputs2to9 |= 128
#define OUT9OFF Outputs2to9 &= 0x7F

#define FOUT10 ( (Outputs10to17 & 1) ? (1) : (0) )
#define OUT10ON Outputs10to17 |= 1
#define OUT10OFF Outputs10to17 &= 0xFE
#define FOUT11 ( (Outputs10to17 & 2) ? (1) : (0) )
#define OUT11ON Outputs10to17 |= 2
#define OUT11OFF Outputs10to17 &= 0xFD
#define FOUT12 ( (Outputs10to17 & 4) ? (1) : (0) )
#define OUT12ON Outputs10to17 |= 4
#define OUT12OFF Outputs10to17 &= 0xFB
#define FOUT13 ( (Outputs10to17 & 8) ? (1) : (0) )
#define OUT13ON Outputs10to17 |= 8
#define OUT13OFF Outputs10to17 &= 0xF7
#define FOUT14 ( (Outputs10to17 & 16) ? (1) : (0) )
#define OUT14ON Outputs10to17 |= 16
#define OUT14OFF Outputs10to17 &= 0xEF
#define FOUT15 ( (Outputs10to17 & 32) ? (1) : (0) )
#define OUT15ON Outputs10to17 |= 32
#define OUT15OFF Outputs10to17 &= 0xDF
#define FOUT16 ( (Outputs10to17 & 64) ? (1) : (0) )
#define OUT16ON Outputs10to17 |= 64
#define OUT16OFF Outputs10to17 &= 0xBF
#define FOUT17 ( (Outputs10to17 & 128) ? (1) : (0) )
#define OUT17ON Outputs10to17 |= 128
#define OUT17OFF Outputs10to17 &= 0x7F

#define KPRESS 135
#define OFFSETPRESS 15

unsigned char SerialIn[40];
unsigned char SerialOut[40];

/*******************************************************************************
 * Tratamento interrupções com prioridade alta
 ******************************************************************************/
void interrupt HighISR(void){
    unsigned int Temp16;
    unsigned char Temp8;
    enum EstadosAD{SETACANAL,CONVERSAO,AGUARDACONVERSAO,CALCULAMEDIA};
    static unsigned char EstadoAD = SETACANAL;
    static unsigned char ContMedia = 0;
    static unsigned int AccValoresAD[4];
    static unsigned int ValoresADanteriores1[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores2[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores3[4] = {0,0,0,0};
    static unsigned char CanalAD = 0;
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
        Inputs18to25 = PORTD;
        PORTE = 4;                                                              //chip select entradas digitais
        NOP();
        NOP();
        NOP();
        Inputs10to17 = PORTD;
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
    //Conversao de 4 sinais analógicos
    //Tempo de atualização: n.o canais(4) * 15ms * n.o amostras
    switch(EstadoAD){
        case SETACANAL:
            ADCON0 &= 0b11000011;   //limpa bits selecao canal
            if(CanalAD < 3)
                ADCON0 |= CanalAD << 2; //seta canal analogico
            else
                ADCON0 |= (CanalAD+1) << 2; //seta canal pulando canal +vref
            EstadoAD = CONVERSAO;
            break;
        case CONVERSAO:
            ADCON0bits.GO = 1;  //inicia conversao
            EstadoAD = AGUARDACONVERSAO;
            break;
        case AGUARDACONVERSAO:
            if(!ADCON0bits.GO){ //verifica se acabou conversão
                Temp16 = ADRESH;
                Temp16 = Temp16 << 8;
                Temp16 |= ADRESL;
                AccValoresAD[CanalAD] += Temp16;  //salva no acumulador
                //if(CanalADAtivo == 99){
                    CanalAD++;
                    if(CanalAD > 3){    //converteu os 4 canais
                        CanalAD = 0;
                        ContMedia++;    //incrementa contador amostra
                    }
                EstadoAD = SETACANAL;
                if(ContMedia == MEDIA_AD)   //atingiu amostras para tirar media?
                    EstadoAD = CALCULAMEDIA;
            }
            break;
        case CALCULAMEDIA:
            for(Temp8=0;Temp8<4;Temp8++){   //calcula a media dos 4 canais
                ValoresAD[Temp8] = AccValoresAD[Temp8] / MEDIA_AD;
                if(Temp8<3){
                    ValoresAD[Temp8] += ValoresADanteriores1[Temp8];
                    ValoresAD[Temp8] += ValoresADanteriores2[Temp8];
                    ValoresAD[Temp8] += ValoresADanteriores3[Temp8];
                    ValoresAD[Temp8] /= 4;
                    ValoresADanteriores3[Temp8] = ValoresADanteriores2[Temp8];
                    ValoresADanteriores2[Temp8] = ValoresADanteriores1[Temp8];
                    ValoresADanteriores1[Temp8] = ValoresAD[Temp8];
                }
                AccValoresAD[Temp8] = 0;
            }
            ContMedia = 0;  //reinicia contador de amostra
            EstadoAD = SETACANAL;
            break;
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
    unsigned int diminui;
    unsigned int aumenta;

    Inicializacao();
    temp = 0;
    imagem = 0;
    CCPR2L = 51;
    NOP();
    EscreverDWINram(0x1A,0);
    EscreverDWINram(0x1B,0);
    
    PORTBbits.RB5 = 1;
    OUT10ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT10OFF;
    OUT11ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT11OFF;
    OUT12ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT12OFF;
    OUT13ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT13OFF;
    OUT14ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT14OFF;
    OUT15ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT15OFF;
    OUT16ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT16OFF;
    OUT17ON;
    for(cont=0;cont<500;cont++)
        __delay_ms(10);
    OUT17OFF;

    PORTBbits.RB5 = 0;
    Outputs2to9 = 0;
    Outputs10to17 = 0;

    while(1){
        /* Mostra no display valores das entradas analogicas */
        EscreverDWINram(0x1C,ValoresAD[0]);
        EscreverDWINram(0x1D,ValoresAD[1]);
        EscreverDWINram(0x1E,ValoresAD[2]);
        PressaommCA = ValoresAD[3]*KPRESS - OFFSETPRESS*KPRESS;
        PressaommCA /= 100;
        EscreverDWINram(0x1F,PressaommCA);
        /* Mostra no display valores da saida analogica */
        EscreverDWINram(0x20,CCPR2L-1);
        diminui = LerDWINram(0x0021);
        if(diminui > 0){
            if(CCPR2L>10){
                CCPR2L -= 10;
                EscreverDWINram(0x20,CCPR2L-1);
            }
            EscreverDWINram(0x21,0);
        }
        
        aumenta = LerDWINram(0x0022);
        if(aumenta > 0){
            if(CCPR2L<91){
                CCPR2L += 10;
                EscreverDWINram(0x20,CCPR2L-1);
            }
            EscreverDWINram(0x22,0);
        }
        /* Mostra no display estado das entradas digitais */
        if(IN0)
            EscreverDWINram(0x80,1);
        else
            EscreverDWINram(0x80,0);
        if(IN1)
            EscreverDWINram(0x81,1);
        else
            EscreverDWINram(0x81,0);
        if(IN2)
            EscreverDWINram(0x82,1);
        else
            EscreverDWINram(0x82,0);
        if(IN3)
            EscreverDWINram(0x83,1);
        else
            EscreverDWINram(0x83,0);
        if(IN4)
            EscreverDWINram(0x84,1);
        else
            EscreverDWINram(0x84,0);
        if(IN5)
            EscreverDWINram(0x85,1);
        else
            EscreverDWINram(0x85,0);
        if(IN6)
            EscreverDWINram(0x86,1);
        else
            EscreverDWINram(0x86,0);
        if(IN7)
            EscreverDWINram(0x87,1);
        else
            EscreverDWINram(0x87,0);
        if(IN8)
            EscreverDWINram(0x88,1);
        else
            EscreverDWINram(0x88,0);
        if(IN9)
            EscreverDWINram(0x89,1);
        else
            EscreverDWINram(0x89,0);
        if(IN10)
            EscreverDWINram(0x8A,1);
        else
            EscreverDWINram(0x8A,0);
        if(IN11)
            EscreverDWINram(0x8B,1);
        else
            EscreverDWINram(0x8B,0);
        if(IN12)
            EscreverDWINram(0x8C,1);
        else
            EscreverDWINram(0x8C,0);
        if(IN13)
            EscreverDWINram(0x8D,1);
        else
            EscreverDWINram(0x8D,0);
        if(IN14)
            EscreverDWINram(0x8E,1);
        else
            EscreverDWINram(0x8E,0);
        if(IN15)
            EscreverDWINram(0x8F,1);
        else
            EscreverDWINram(0x8F,0);
        if(IN16)
            EscreverDWINram(0x90,1);
        else
            EscreverDWINram(0x90,0);
        if(IN17)
            EscreverDWINram(0x91,1);
        else
            EscreverDWINram(0x91,0);
        if(IN18)
            EscreverDWINram(0x92,1);
        else
            EscreverDWINram(0x92,0);
        if(IN19)
            EscreverDWINram(0x93,1);
        else
            EscreverDWINram(0x93,0);
        if(IN20)
            EscreverDWINram(0x94,1);
        else
            EscreverDWINram(0x94,0);
        if(IN21)
            EscreverDWINram(0x95,1);
        else
            EscreverDWINram(0x95,0);
        if(IN22)
            EscreverDWINram(0x96,1);
        else
            EscreverDWINram(0x96,0);
        if(IN23)
            EscreverDWINram(0x97,1);
        else
            EscreverDWINram(0x97,0);
        if(IN24)
            EscreverDWINram(0x98,1);
        else
            EscreverDWINram(0x98,0);
        if(IN25)
            EscreverDWINram(0x99,1);
        else
            EscreverDWINram(0x99,0);
        /* Atua as saidas digitais conforme touch no display */
        temp = LerDWINram(0x001A);
        imagem = LerDWINram(0x001B);
        switch(temp){
            case 1: OUT0 = !OUT0;break;                                         //SD0
            case 2: OUT1 = !OUT1;break;                                         //SD1
            case 4: if(FOUT2) 
                        OUT2OFF;
                    else
                        OUT2ON;
                    break;
            case 8: if(FOUT3)
                        OUT3OFF;
                    else
                        OUT3ON;
                    break;
            case 16:if(FOUT4)                                 //SD4
                        OUT4OFF;
                    else
                        OUT4ON;
                    break;
            case 32: if(FOUT5)                                //SD5
                        OUT5OFF;
                    else
                        OUT5ON;
                    break;
            case 64:if(FOUT6)                               //SD6
                        OUT6OFF;
                    else
                        OUT6ON;
                    break;
            case 128:if(FOUT7)                               //SD7
                        OUT7OFF;
                    else
                        OUT7ON;
                    break;
            case 256:if(FOUT8)                              //SD8
                        OUT8OFF;
                    else
                        OUT8ON;
                    break;
            case 512:if(FOUT9)                            //SD9
                        OUT9OFF;
                    else
                        OUT9ON;
                    break;
            default: break;
        }
        if(temp > 0){
            EscreverDWINram(0x1A,0);
        }

        switch(imagem){
            case 1: if(FOUT10)                                 //SD2
                        OUT10OFF;
                    else
                        OUT10ON;
                    break;
            case 2: if(FOUT11)                                 //SD3
                        OUT11OFF;
                    else
                        OUT11ON;
                    break;
            case 4:if(FOUT12)                                 //SD4
                        OUT12OFF;
                    else
                        OUT12ON;
                    break;
            case 8: if(FOUT13)                                //SD5
                        OUT13OFF;
                    else
                        OUT13ON;
                    break;
            case 16:if(FOUT14)                               //SD6
                        OUT14OFF;
                    else
                        OUT14ON;
                    break;
            case 32:if(FOUT15)                               //SD7
                        OUT15OFF;
                    else
                        OUT15ON;
                    break;
            case 64:if(FOUT16)                              //SD8
                        OUT16OFF;
                    else
                        OUT16ON;
                    break;
            case 128:if(FOUT17)                            //SD9
                        OUT17OFF;
                    else
                        OUT17ON;
                    break;
            case 256:PORTBbits.RB5 = !PORTBbits.RB5;
                    break;
            default: break;
        }
        if(imagem > 0){
            EscreverDWINram(0x1B,0);
        }
    }
    return;
}

