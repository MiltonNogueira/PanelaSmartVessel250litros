/*******************************************************************************
 * www.neuheitbier.com.br
 * Arquivo principal de controle para panela cervejeira smart vessel
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
#include "CpuConfigWords.h"
#include "defines.h"
#include "display.h"


extern void EscreverDWINram(unsigned int endereco,int valor);
extern void EscreverDWINreg(unsigned char endereco,int valor);
extern int LerDWINram(unsigned int endereco);
extern int LerDWINreg(unsigned char endereco);
extern void EscreverDWINtrend(int valor);
extern void ExecutaBrassagem(void);

/*******************************************************************************
 * Variaveis globais
 ******************************************************************************/
bit IN0;
bit IN1;
bit OUT0;
bit OUT1;
unsigned char Inputs2to9;
unsigned char Inputs10to17;
unsigned char Inputs18to25;
unsigned char Outputs2to9;
unsigned char Outputs10to17;
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
#define BOTAO_EMERGENCIA IN2
#define BOTAO_USUARIO IN3
#define NIVEL_BAIXO_CENTRAL IN4
#define NIVEL_ALTO_CENTRAL IN5
#define FALHA_INVERSOR IN6
#define FIM_CURSO_GUINCHO IN7
#define BOTAO_SOBE_GUINCHO IN8
#define BOTAO_DESCE_GUINCHO IN9
#define OUT2 ( (1) ? (Outputs2to9 | 1) : (Outputs2to9 & 0xFE) )
#define OUT3 ( (1) ? (Outputs2to9 | 2) : (Outputs2to9 & 0xFD) )
#define OUT4 ( (1) ? (Outputs2to9 | 4) : (Outputs2to9 & 0xFB) )
#define OUT5 ( (1) ? (Outputs2to9 | 8) : (Outputs2to9 & 0xF7) )
#define OUT6 ( (1) ? (Outputs2to9 | 16) : (Outputs2to9 & 0xEF) )
#define OUT7 ( (1) ? (Outputs2to9 | 32) : (Outputs2to9 & 0xDF) )
#define OUT8 ( (1) ? (Outputs2to9 | 64) : (Outputs2to9 & 0xBF) )
#define OUT9 ( (1) ? (Outputs2to9 | 128) : (Outputs2to9 & 0x7F) )
#define OUT10 ( (1) ? (Outputs10to17 | 1) : (Outputs2to9 & 0xFE) )
#define OUT11 ( (1) ? (Outputs10to17 | 2) : (Outputs2to9 & 0xFD) )
#define OUT12 ( (1) ? (Outputs10to17 | 4) : (Outputs2to9 & 0xFB) )
#define OUT13 ( (1) ? (Outputs10to17 | 8) : (Outputs2to9 & 0xF7) )
#define OUT14 ( (1) ? (Outputs10to17 | 16) : (Outputs2to9 & 0xEF) )
#define OUT15 ( (1) ? (Outputs10to17 | 32) : (Outputs2to9 & 0xDF) )
#define OUT16 ( (1) ? (Outputs10to17 | 64) : (Outputs2to9 & 0xBF) )
#define OUT17 ( (1) ? (Outputs10to17 | 128) : (Outputs2to9 & 0x7F) )
bit TransmiteSerial;
unsigned char SerialIn[40];
bit fSerialInEnd;
unsigned char SerialOut[40];
unsigned int ValoresAD[4];
unsigned char Minutos;
unsigned char Segundos;
unsigned char TotalHoras;
unsigned char TotalMinutos;
unsigned char TotalSegundos;
bit MudancaSegundos;
struct ValoresReceita{
    unsigned char TemperaturaMashIn;    //0x10
    unsigned char TemperaturaFase1;     //0x11
    unsigned char TempoFase1;           //0x12
    unsigned char TemperaturaFase2;     //0x13
    unsigned char TempoFase2;           //0x14
    unsigned char TemperaturaFase3;     //0x15
    unsigned char TempoFase3;           //0x16
    unsigned char TemperaturaFase4;     //0x17
    unsigned char TempoFase4;           //0x18
    unsigned char TemperaturaFase5;     //0x19
    unsigned char TempoFase5;           //0x1A
    unsigned char TemperaturaMashOut;   //0x1B
    unsigned char TempoMashOut;         //0x1C
    unsigned char TemperaturaFervura;   //0x1D
    unsigned char TempoFervura;         //0x1E
    unsigned char TempoLupulo1;         //0x1F
    unsigned char TempoLupulo2;         //0x20
    unsigned char TempoLupulo3;         //0x21
    unsigned char TempoLupulo4;         //0x22
    unsigned char VolumeInicial;        //0x23
    unsigned char TempoLavagem;         //0x24
    unsigned char TempoMashIn;          //0x51
    unsigned char Nome[17];
};
struct ValoresReceita ReceitaAtual;
unsigned int t1captura,t2captura;
unsigned char periodocaptura;
unsigned char FrequenciaMedidorFluxo;
float VolumeDosado;
unsigned char ByteVolumeDosado;
unsigned char PWMpanela;
unsigned char ContDecSeg;
bit OnOffControleTemp;
bit ControlePotenciaFervura;
unsigned char SetPointTempPanela;
bit TempPanelaOK;
char TempChar;
unsigned char TempUChar;
int TempInt;
unsigned int TempUInt;
bit StatusY2 = 0;
bit StatusY3 = 0;
bit StatusY4 = 0;
bit StatusY5 = 0;
bit StatusY6 = 0;
bit StatusY7 = 0;
char KP,KI,KD;
int TAmostragemPID;
unsigned char TonBomba,ToffBomba;           //1 a 99 minutos
unsigned char TemperaturaSparging;          //60 a 90
unsigned char PorcentagemBombaSparging;     //20 a 100%
unsigned char TempoAdicionarAguaSparging;   //5 a 60 segundos
unsigned char TempoDescansosparging;        //1 a 30 minutos
unsigned char CanalADAtivo;                 //99 = todos canais
unsigned int TempoEncherVessel,TempoVolumeAguaPrimaria;
unsigned int FreeRunSegundos;
bit EnviaDadoTrend;
unsigned char MinutosBomba;

/*******************************************************************************
 * Tratamento interrupções com prioridade alta
 ******************************************************************************/
void interrupt HighISR(void){

    unsigned int Temp16;
    unsigned char Temp8;
    enum EstadosAD{SETACANAL,CONVERSAO,AGUARDACONVERSAO,CALCULAMEDIA};
    static unsigned char EstadoAD = SETACANAL;
    static unsigned char FreeRun5ms = 0;
    static unsigned char ContMedia = 0;
    static unsigned int AccValoresAD[4];
    static unsigned int ValoresADanteriores1[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores2[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores3[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores4[4] = {0,0,0,0};
    static unsigned char CanalAD = 0;
    int Erro;
    int SomaErro;
    
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
        FreeRun5ms++;
        MudancaSegundos = 0;
        if((FreeRun5ms % 20) == 0)
            ContDecSeg++;
        if(OnOffControleTemp){
            if(ContDecSeg >= TAmostragemPID){
                ContDecSeg = 0;
                Erro = SetPointTempPanela*10 - ValoresAD[0];    //resolução erro 0,1oC
                if((Erro >= LIMITEMINERRO) && (Erro <= LIMITEMAXERRO))
                    TempPanelaOK = 1;
                else
                    TempPanelaOK = 0;
                SomaErro += Erro;
                if( (SomaErro < LIMITEMINSOMAERRO) || (SomaErro > LIMITEMAXSOMAERRO) )
                    SomaErro = 0;
                Erro = Erro*KP + SomaErro*KI;
                                
                if(Erro < 0)
                    PWMpanela = PWMmin;
                else{
                    if(Erro < PWMmax)
                        PWMpanela = Erro;
                    else
                        PWMpanela = PWMmax;
                }
            }
       }
       if(FreeRun5ms >= 200){
            Segundos++;
            FreeRunSegundos++;
            MudancaSegundos = 1;
            TotalSegundos++;
            FreeRun5ms = 0;
            if(Segundos > 59){
                Segundos = 0;
                Minutos++;
            }
            if(TotalSegundos > 59){
                TotalSegundos = 0;
                MinutosBomba++;
                EnviaDadoTrend = 1;
                TotalMinutos++;
            }
            if(TotalMinutos > 59){
                TotalMinutos = 0;
                TotalHoras++;
            }
            
        }
        if( (OnOffControleTemp) || (ControlePotenciaFervura) ){
            if(PWMpanela > 0){
                if(FreeRun5ms < PWMpanela)
                    RELESSR = 1;
                else
                    RELESSR = 0;
            }
            else
                RELESSR = 0;
        }
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
                //}
                /*else{
                    CanalAD = CanalADAtivo;
                    ContMedia++;    //incrementa contador amostra
                }*/
                EstadoAD = SETACANAL;
                if(ContMedia == MEDIA_AD)   //atingiu amostras para tirar media?
                    EstadoAD = CALCULAMEDIA;
            }
            break;
        case CALCULAMEDIA:
            for(Temp8=0;Temp8<4;Temp8++){   //calcula a media dos 4 canais
                //if( (CanalADAtivo == 99) || (Temp8 == CanalADAtivo) ){
                    ValoresAD[Temp8] = AccValoresAD[Temp8] / MEDIA_AD;
                    ValoresAD[Temp8] += ValoresADanteriores1[Temp8];
                    ValoresAD[Temp8] += ValoresADanteriores2[Temp8];
                    ValoresAD[Temp8] += ValoresADanteriores3[Temp8];
                    //ValoresAD[Temp8] += ValoresADanteriores4[Temp8];
                    ValoresAD[Temp8] /= 4;
                    //ValoresADanteriores4[Temp8] = ValoresADanteriores3[Temp8];
                    ValoresADanteriores3[Temp8] = ValoresADanteriores2[Temp8];
                    ValoresADanteriores2[Temp8] = ValoresADanteriores1[Temp8];
                    ValoresADanteriores1[Temp8] = ValoresAD[Temp8];
                    AccValoresAD[Temp8] = 0;
                //}
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
            fSerialInEnd = 1;
        }
    }

    if(PIE2bits.TMR3IE && PIR2bits.TMR3IF){
        t1captura = 0;
        t2captura = 0;
        PIR2bits.TMR3IF = 0;
    }

    if(PIE1bits.CCP1IE && PIR1bits.CCP1IF){
        PIR1bits.CCP1IF = 0;
        if(t1captura == 0){
            t1captura = CCPR1H << 8;
            t1captura |= CCPR1L;
        }
        else{
            t2captura = CCPR1H << 8;
            t2captura |= CCPR1L;
            t2captura -= t1captura;
            periodocaptura = t2captura / 625;
            if(periodocaptura > 3)
                FrequenciaMedidorFluxo = 1000/periodocaptura;
            else
                FrequenciaMedidorFluxo = 0;
            t1captura = 0;
        }
    }
}

void DelaySeg(unsigned char tempo)
{
    unsigned int cont;

    do{
        for(cont=0;cont<1000;cont++)
            __delay_ms(1);
        tempo--;
    }while(tempo>0);
}

/*******************************************************************************
 * Inicialização do microcontrolador
 *
 * Configuração dos pinos como entrada ou saída
 * Configuração dos periféricos internos do micro
 * Configuração das interrupções e suas prioridades (alta ou baixa)
 ******************************************************************************/
void Inicializacao(void)
{
    //Configuração do port A - 4 canais analõgicos e +vref (tensão de referencia)
    //Pino RA4 entrada digital
    PORTA = 0;
    ADCON0 = 0;                 //seleciona canal 0
    ADCON1 = 0b00011010;        //RA0 a RA3 como analogicos, +vref externo
    ADCON2 = 0b10100101;
    CMCON = 7;                  //desliga comparadores
    TRISA = 0x3F;               //PORTA todos pinos entrada

    PORTB = 0;
    TRISB = 0b11010101;         //apenas RB1, RB3 e RB5 são saidas
    PORTC = 0;
    TRISC = 0xFF;               //Todos pinos port C como entrada
    PORTD = 0;
    TRISD = 0;                  //Todos pinos port D como saida
    TRISE = 0;
    PORTE = 0;                  //port E todo como saida
    
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

    ADCON0bits.ADON = 1;        //habilita modulo conversor A/D

    //configura timer 0 para gerar int a cada 5ms
    T0CON = 0b11000110;         //timer 0 ligado, com prescaler de 256
    TMR0L = 60;
    INTCON2bits.T0IP = 1;       //timer 0 gera int de alta prioridade
    INTCONbits.TMR0IE = 1;      //habilita interrupção timer 0

    PR2 = 0xFF;
    CCPR2L = 0x7F;
    T2CON = 0x04;               //pre scaler 4
    CCP2CON = 0x0F;             //PWM

    //configura timer 3 - será usado com CCP1 pra medir fluxo entrada agua
    T3CON = 0b11111001;
    PIE2bits.TMR3IE = 1;
    IPR2bits.TMR3IP = 0;        //baixa prioridade int timer 3

    CCP1CON = 0b00000101;
    IPR1bits.CCP1IP = 0;        //baixa prioridade captura fluxo agua
    PIE1bits.CCP1IE = 1;        //habilita interrupção captura

    //habiltaçao das interrupções
    RCONbits.IPEN = 1;          //habilita 2 niveis de prioridade para as interrupções
    INTCONbits.GIEH = 1;        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 1;        //habilita interrupções com prioridade baixa
}

void LeParamConfig(void){

    KP = LerDWINram(0x27) / 10;
    KI = LerDWINram(0x28) / 10;
    KD = LerDWINram(0x29) / 10;
    TAmostragemPID = LerDWINram(0x2A);
    TonBomba = LerDWINram(0x57);
    ToffBomba = LerDWINram(0x58);
    TemperaturaSparging = LerDWINram(0x2C);
    PorcentagemBombaSparging = LerDWINram(0x2D);
    TempoAdicionarAguaSparging = LerDWINram(0x55);
    TempoDescansosparging = LerDWINram(0x56);
}

void LeDadosReceita(void)
{
    unsigned char utemp8;
    unsigned int buffer;

    for(utemp8=0x10;utemp8<0x25;utemp8++){
        switch(utemp8){
            case 0x10: ReceitaAtual.TemperaturaMashIn = LerDWINram(utemp8);break;
            case 0x11: ReceitaAtual.TemperaturaFase1 = LerDWINram(utemp8);break;
            case 0x12: ReceitaAtual.TempoFase1 = LerDWINram(utemp8);break;
            case 0x13: ReceitaAtual.TemperaturaFase2 = LerDWINram(utemp8);break;
            case 0x14: ReceitaAtual.TempoFase2 = LerDWINram(utemp8);break;
            case 0x15: ReceitaAtual.TemperaturaFase3 = LerDWINram(utemp8);break;
            case 0x16: ReceitaAtual.TempoFase3 = LerDWINram(utemp8);break;
            case 0x17: ReceitaAtual.TemperaturaFase4 = LerDWINram(utemp8);break;
            case 0x18: ReceitaAtual.TempoFase4 = LerDWINram(utemp8);break;
            case 0x19: ReceitaAtual.TemperaturaFase5 = LerDWINram(utemp8);break;
            case 0x1A: ReceitaAtual.TempoFase5 = LerDWINram(utemp8);break;
            case 0x1B: ReceitaAtual.TemperaturaMashOut = LerDWINram(utemp8);break;
            case 0x1C: ReceitaAtual.TempoMashOut = LerDWINram(utemp8);break;
            case 0x1D: ReceitaAtual.TemperaturaFervura = LerDWINram(utemp8);break;
            case 0x1E: ReceitaAtual.TempoFervura = LerDWINram(utemp8);break;
            case 0x1F: ReceitaAtual.TempoLupulo1 = LerDWINram(utemp8);break;
            case 0x20: ReceitaAtual.TempoLupulo2 = LerDWINram(utemp8);break;
            case 0x21: ReceitaAtual.TempoLupulo3 = LerDWINram(utemp8);break;
            case 0x22: ReceitaAtual.TempoLupulo4 = LerDWINram(utemp8);break;
            case 0x23: ReceitaAtual.VolumeInicial = LerDWINram(utemp8);break;
            case 0x24: ReceitaAtual.TempoLavagem = LerDWINram(utemp8);break;
        }
    }
    ReceitaAtual.TempoMashIn = LerDWINram(0x51);    //tempo Mash In foi colocado depois
    for(utemp8=0;utemp8<8;utemp8++){
        buffer = LerDWINram(0x100+utemp8);
        ReceitaAtual.Nome[utemp8*2] = buffer>>8;
        ReceitaAtual.Nome[utemp8*2+1] = buffer;
    }
    NOP();
}

void GravarDadosNaEEPROM(unsigned char receita)
{
    unsigned char cont,temp;

    receita--;
    if(receita>0)
        receita *= 37;

    for(cont=0;cont<21;cont++){
        switch(cont){
            case 0: temp = ReceitaAtual.TemperaturaMashIn;break;
            case 1: temp = ReceitaAtual.TemperaturaFase1;break;
            case 2: temp = ReceitaAtual.TempoFase1;break;
            case 3: temp = ReceitaAtual.TemperaturaFase2;break;
            case 4: temp = ReceitaAtual.TempoFase2;break;
            case 5: temp = ReceitaAtual.TemperaturaFase3;break;
            case 6: temp = ReceitaAtual.TempoFase3;break;
            case 7: temp = ReceitaAtual.TemperaturaFase4;break;
            case 8: temp = ReceitaAtual.TempoFase4;break;
            case 9: temp = ReceitaAtual.TemperaturaFase5;break;
            case 10: temp = ReceitaAtual.TempoFase5;break;
            case 11: temp = ReceitaAtual.TemperaturaMashOut;break;
            case 12: temp = ReceitaAtual.TempoMashOut;break;
            case 13: temp = ReceitaAtual.TemperaturaFervura;break;
            case 14: temp = ReceitaAtual.TempoFervura;break;
            case 15: temp = ReceitaAtual.TempoLupulo1;break;
            case 16: temp = ReceitaAtual.TempoLupulo2;break;
            case 17: temp = ReceitaAtual.TempoLupulo3;break;
            case 18: temp = ReceitaAtual.TempoLupulo4;break;
            case 19: temp = ReceitaAtual.VolumeInicial;break;
            case 20: temp = ReceitaAtual.TempoLavagem;break;
        }
        eeprom_write((receita+cont),temp);
        __delay_ms(10);
    }
    NOP();
}

void AcionaValvulaMotorizada(unsigned char sentido,unsigned char valvula){

    if(sentido == ABRIR)
        SENTIDOVALVULA = 1;
    else
        SENTIDOVALVULA = 0;

    DelayDWIN();

    switch(valvula){
        case MASKY2: VALVULASAIDARESERVATORIO = 1;break;
        case MASKY3: VALVULASAIDAPANELA = 1;break;
        case MASKY4: VALVULALAVAGEMGRAOS = 1;break;
        case MASKY5: VALVULAENTRADAPANELA = 1;break;
        case MASKY6: VALVULAWHIRLPOOL = 1;break;
        case MASKY7: VALVULAENTRADAMOSTOTROCADOR = 1;break;
    }
    DelaySeg(3);
    switch(valvula){
        case MASKY2: VALVULASAIDARESERVATORIO = 0;break;
        case MASKY3: VALVULASAIDAPANELA = 0;break;
        case MASKY4: VALVULALAVAGEMGRAOS = 0;break;
        case MASKY5: VALVULAENTRADAPANELA = 0;break;
        case MASKY6: VALVULAWHIRLPOOL = 0;break;
        case MASKY7: VALVULAENTRADAMOSTOTROCADOR = 0;break;
    }
    DelayDWIN();
}

void Bipar(void)
{
    SIRENE = 1;
    DelaySeg(1);
    SIRENE = 0;
}

void ResetaSaidas(void)
{
    VALVULAENTRADARESERVATORIO = 0;
    VALVULAENTRADAAGUATROCADOR = 0;
    RESISTENCIAPANELA = 0;
    RESISTENCIARESERVATORIO = 0;
    RELESSR = 0;
    AcionaValvulaMotorizada(FECHAR,MASKY2);
    AcionaValvulaMotorizada(FECHAR,MASKY3);
    AcionaValvulaMotorizada(FECHAR,MASKY4);
    AcionaValvulaMotorizada(FECHAR,MASKY5);
    AcionaValvulaMotorizada(FECHAR,MASKY6);
    AcionaValvulaMotorizada(FECHAR,MASKY7);
    EscreverDWINram(DWINadrIconeY1,0);
    StatusY2 = 0;
    EscreverDWINram(DWINadrIconeY2,0);
    StatusY3 = 0;
    EscreverDWINram(DWINadrIconeY3,0);
    StatusY4 = 0;
    EscreverDWINram(DWINadrIconeY4,0);
    StatusY5 = 0;
    EscreverDWINram(DWINadrIconeY5,0);
    StatusY6 = 0;
    EscreverDWINram(DWINadrIconeY6,0);
    StatusY7 = 0;
    EscreverDWINram(DWINadrIconeY7,0);
    EscreverDWINram(DWINadrIconeY8,0);
}

void main(void){

    enum EstadosMenuPrincipal{MENUPRINCIPAL,BRASSAGEM,EDICAORECEITA,MANUAL,CONFIGURACAO};
    unsigned char EstadoPrincipal;
    unsigned char SegundoAtual;

    t1captura = 0;
    t2captura = 0;
    periodocaptura = 0;
    FrequenciaMedidorFluxo = 0;
    Inicializacao();

    TransmiteSerial = 0;
    fSerialInEnd = 0;
    SegundoAtual = Segundos;
    EstadoPrincipal = MENUPRINCIPAL;
    PWMpanela = PWMmin;
    OnOffControleTemp = 0;
    ControlePotenciaFervura = 0;
    PORTD = 0;
    CCPR2L = 0;     //Bomba parada
    CanalADAtivo = 99;
    EnviaDadoTrend  = 0;

    //Inicia fechando todas as válvulas
    ResetaSaidas();     //desliga todas as saidas e fecha valvulas motorizadas
    
    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
    
    LeParamConfig();
    LeDadosReceita();
    EscreverDWINreg(3,1);  //exibe menu principal
    Bipar();

    while(1){
        NOP();
        switch(EstadoPrincipal){
            case MENUPRINCIPAL:
                switch(LerDWINreg(03)){
                    case NUMTELABRASSAGEM: EstadoPrincipal = BRASSAGEM;EscreverDWINreg(3,12);  //exibe tela confirmar inicio
                    break;
                    case NUMTELARECEITA: EstadoPrincipal = EDICAORECEITA;break;
                    case NUMTELAMANUAL: EstadoPrincipal = MANUAL;break;
                    case NUMTELACONFIGURACAO: EstadoPrincipal = CONFIGURACAO;break;
                }
                break;
            case BRASSAGEM:
                ExecutaBrassagem();
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL)
                    EstadoPrincipal = MENUPRINCIPAL;
                break;
            case EDICAORECEITA:
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    LeDadosReceita();
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            case MANUAL:
                ModoManual();
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    ResetaSaidas();
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            case CONFIGURACAO:
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    LeParamConfig();
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
        }
    }
    return;
}
