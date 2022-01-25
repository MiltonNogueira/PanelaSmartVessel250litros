/*******************************************************************************
 * Definições do programa
 ******************************************************************************/
#define _XTAL_FREQ      20000000                // oscilador interno em valor default
#define FECHAR 0
#define ABRIR 1
#define PWMmin 1
#define PWMmax 199
#define CICLOSBOMBA 5
//Funcoes dos pinos do microcontrolador
#define SENTIDOVALVULA PORTBbits.RB5
#define VALVULAENTRADARESERVATORIO PORTDbits.RD0
#define VALVULASAIDARESERVATORIO PORTDbits.RD5
#define VALVULASAIDAPANELA PORTDbits.RD6
#define VALVULALAVAGEMGRAOS PORTDbits.RD7
#define VALVULAENTRADAPANELA PORTEbits.RE0
#define VALVULAWHIRLPOOL PORTEbits.RE1
#define VALVULAENTRADAMOSTOTROCADOR PORTEbits.RE2
#define VALVULAENTRADAAGUATROCADOR PORTDbits.RD1
#define RESISTENCIAPANELA PORTDbits.RD2
#define RESISTENCIARESERVATORIO PORTDbits.RD3
#define RELESSR PORTBbits.RB1
#define SIRENE PORTDbits.RD4
#define BOTAO_EMERGENCIA PORTAbits.RA4
#define BOTAO_START PORTCbits.RC2
#define NB_BOILER PORTBbits.RB2
#define NA_BOILER PORTBbits.RB4
//#define ED4 PORTBbits.RB5
#define TEMPERATURAPANELA
#define TEMPERATURARESERVATORIO
#define TEMPERATURASAIDATROCADOR
#define TEMPERATURAPANELA
#define MEDIA_AD 25
#define NUMBYTESRECEITA 38
#define KVOLUME 22
//#define TAMOSTRAGEMCONTROLEPI 30
//#define KP 4
//#define KI 4
#define LIMITEMINERRO -2        //limite erro temperatura
#define LIMITEMAXERRO 2         //ajustado para 0,5oC
#define LIMITEMAXSOMAERRO 50
#define LIMITEMINSOMAERRO -50
#define NUMTELAMENUPRINCIPAL 1
#define NUMTELABRASSAGEM 6
#define NUMTELARECEITA 4
#define NUMTELACONFIGURACAO 3
#define NUMTELAMANUAL 2
