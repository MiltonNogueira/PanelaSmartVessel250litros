/*******************************************************************************
 * www.neuheitbier.com.br
 * Controle para panela cervejeira smart vessel
 * Autor: Milton Nogueira
 *
 * Criado em agosto de 2017
 * MPLAB X v2.35, XC8 v1.34
 *
 * Revisoes:
 * 1.0 - instalacao na cervejaria Iperoig em Ubatuba
 *
 * Melhorias detectadas na versão 1.0 na instalação em Ubatuba:
 * - verificar controle temperatura da central de lavagem em modos manual
 * e brassagem, é possivel otimizar o código
 * - implementar falha momentanea (queda) de energia
 ******************************************************************************/

#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CpuConfigWords.h"

/*******************************************************************************
 * Definições do programa
 ******************************************************************************/
#define _XTAL_FREQ 20000000                                                     // oscilador interno em valor default
#define FECHAR 0
#define ABRIR 1
#define SUBIR RELESENTIDOGUINCHOOFF
#define DESCER RELESENTIDOGUINCHOON
#define PWMmin 1
#define PWMmax 199
#define CICLOSBOMBA 5                                                           // numero vezes que a bomba liga/desliga
                                                                                //antes de começar a aquecer a água para
                                                                                //eliminar bolhas do sistema

//entradas e saidas digitais
unsigned char Inputs2to9;                                                       //10 entradas PNP 24Vcc
unsigned char Inputs10to17;                                                     //8 entradas NPN - fim de curso válv. motorizadas
unsigned char Inputs18to25;                                                     //8 entradas NPN - fim de curso válv. motorizadas
unsigned char Outputs2to9;                                                      //10 saidas NPN coletor aberto
unsigned char Outputs10to17;                                                    //8 reles para acionam. valv. motorizadas
unsigned int ValoresAD[4];                                                      //valores 4 entradas analógicas
unsigned long PressaommCA;                                                      //Valor da pressão na panela em mmCA
/* inicio das definicoes das entradas e saídas digitais*/
#define IN0 PORTAbits.RA4                                                       //as duas primeiras entradas da placa estão
#define IN1 PORTCbits.RC2                                                       //aos pinos do microcontrolador
#define IN2 ( (Inputs2to9 & 1) ? (1) : (0) )                                    //as demais entradas estao ligadas aos latches
#define IN3 ( (Inputs2to9 & 2) ? (1) : (0) )                                    //74541
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
#define FOUT0 PORTBbits.RB2                                                     //as duas primeiras saidas da placa estao
#define OUT0ON PORTBbits.RB2 = 1                                                //ligadas aos pinos do microcontrolador
#define OUT0OFF PORTBbits.RB2 = 0
#define FOUT1 PORTBbits.RB4
#define OUT1ON PORTBbits.RB4 = 1
#define OUT1OFF PORTBbits.RB4 = 0
#define FOUT2 ( (Outputs2to9 & 1) ? (1) : (0) )                                 //as demais saidas estao ligadas aos
#define OUT2ON Outputs2to9 |= 1                                                 //latches 74574
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
/* fim das definicoes das entradas e saídas digitais*/
#define KPRESS 100                                                              //valores para converter a leitura analogica
#define OFFSETPRESS 200                                                         //em mmCA
#define MINIMOMMCAENCHERRESERVATORIO 200                                        //altura minima de agua na panela p/ cobrir a resistencia
                                                                                //diferenca de altura em mm entre o ponto em que
                                                                                //está instalado o transmissor de pressao e o fundo
                                                                                //da panela

#define SENTIDOVALVULA PORTBbits.RB5                                            //saida que aciona o rele que inverte 12V para abrir
                                                                                //ou fecha as valv. motorizadas

#define SIRENE FOUT9                                                            //sinal sonoro para o operador
#define SIRENEON OUT9ON
#define SIRENEOFF OUT9OFF
#define RESETINVERSORON OUT1ON                                                  //sinal de hardware para tentar
#define RESETINVERSOROFF OUT1OFF                                                //"limpar" o erro do inversor
#define RESETINVERSOR FOUT1

//VALVULA DIAFRAGMA Y1                                                          //uma vávula diafragma
#define VALVULAENTRADARESERVATORIOON OUT2ON                                     //e oito motorizadas
#define VALVULAENTRADARESERVATORIOOFF OUT2OFF                                   //as válvulas motorizadas
#define VALVULAENTRADARESERVATORIO FOUT2                                        //sao acionadas atraves dos
//VALVULA MOTORIZADA Y2                                                         //dos reles existentes
#define VALVULASAIDARESERVATORIOON OUT10ON                                      //na placa de controle
#define VALVULASAIDARESERVATORIOOFF OUT10OFF
#define VALVULASAIDARESERVATORIOFECHADA IN10
#define VALVULASAIDARESERVATORIOABERTA IN11
//VALVULA MOTORIZADA Y3
#define VALVULASAIDAPANELAON OUT11ON
#define VALVULASAIDAPANELAOFF OUT11OFF
#define VALVULASAIDAPANELAFECHADA IN12
#define VALVULASAIDAPANELAABERTA IN13
//VALVULA MOTORIZADA Y4
#define VALVULALAVAGEMGRAOSON OUT12ON
#define VALVULALAVAGEMGRAOSOFF OUT12OFF
#define VALVULALAVAGEMGRAOSFECHADA IN14
#define VALVULALAVAGEMGRAOSABERTA IN15
//VALVULA MOTORIZADA Y5
#define VALVULAENTRADAPANELAON OUT13ON
#define VALVULAENTRADAPANELAOFF OUT13OFF
#define VALVULAENTRADAPANELAFECHADA IN16
#define VALVULAENTRADAPANELAABERTA IN17
//VALVULA MOTORIZADA Y6
#define VALVULAWHIRLPOOLON OUT14ON
#define VALVULAWHIRLPOOLOFF OUT14OFF
#define VALVULAWHIRLPOOLFECHADA IN18
#define VALVULAWHIRLPOOLABERTA IN19
//VALVULA MOTORIZADA Y7
#define VALVULAENTRADAMOSTOTROCADORON OUT15ON
#define VALVULAENTRADAMOSTOTROCADOROFF OUT15OFF
#define VALVULAENTRADAMOSTOTROCADORFECHADA IN20
#define VALVULAENTRADAMOSTOTROCADORABERTA IN21
//VALVULA MOTORIZADA Y8
#define VALVULAENTRADAAGUATROCADORON OUT16ON
#define VALVULAENTRADAAGUATROCADOROFF OUT16OFF
#define VALVULAENTRADAAGUATROCADORFECHADA IN22
#define VALVULAENTRADAAGUATROCADORABERTA IN23
//VALVULA MOTORIZADA Y9
#define VALVULAENTRADAAGUAPANELAON OUT17ON
#define VALVULAENTRADAAGUAPANELAOFF OUT17OFF
#define VALVULAENTRADAAGUAPANELAFECHADA IN24
#define VALVULAENTRADAAGUAPANELAABERTA IN25
//
#define RESISTENCIAPANELA FOUT4
#define RESISTENCIAPANELAON OUT4ON
#define RESISTENCIAPANELAOFF OUT4OFF
#define RESISTENCIARESERVATORIO FOUT5
#define RESISTENCIARESERVATORIOON OUT5ON
#define RESISTENCIARESERVATORIOOFF OUT5OFF
//ELETROBOMBA DE RECIRCULACAO DE AGUA NO RESERVATORIO AGUA SPARGING
#define BOMBACIRC FOUT3
#define BOMBACIRCON OUT3ON
#define BOMBACIRCOFF OUT3OFF
#define RELESENTIDOGUINCHOON OUT7ON
#define RELESENTIDOGUINCHOOFF OUT7OFF
#define RELEGUINCHO FOUT6
#define RELEGUINCHOON OUT6ON
#define RELEGUINCHOOFF OUT6OFF
#define BOTAO_EMERGENCIA IN2
#define BOTAO_START IN3
#define NB_BOILER IN4
#define NA_BOILER IN5
#define INVERSOROK IN6
#define FIMCURSOGUINCHO IN7
#define SUBIRGUINCHO IN8
#define DESCERGUINCHO IN9
#define MEDIA_AD 25                                                             //numero de leituras A/D para calcular a media e só
                                                                                //salvar na matriz de cada canal analógico
#define NUMBYTESRECEITA 38
#define KVOLUME 172
#define TCTRLTRASFEGA 2                                                         //intervalo em seg que corrige rotação da bomba
                                                                                //durante trasfega
#define LIMITEMINERRO -2                                                        //limite erro temperatura
#define LIMITEMAXERRO 2                                                         //ajustado para 0,2oC
#define LIMITEMAXSOMAERRO 50                                                    //usado no PID (parte da integral)
#define LIMITEMINSOMAERRO -50
//Cabeçalho do frame do display DWIN
#define CABECALHOHIGH 0x5A
#define CABECALHOLOW 0xA5
//Definicoes dos enderecos das variaveis no display DWIN
//consultar aplicação criada com o software DGUS SDK
#define DELAYDWIN 80
#define DWINadrTemperaturaMashIn 0x10
#define DWINadrTempoMashIn 0x51
#define DWINadrTemperaturaFase1 0x11
#define DWINadrTempoFase1 0x12
#define DWINadrTemperaturaFase2 0x13
#define DWINadrTempoFase2 0x14
#define DWINadrTemperaturaFase3 0x15
#define DWINadrTempoFase3 0x16
#define DWINadrTemperaturaFase4 0x17
#define DWINadrTempoFase4 0x18
#define DWINadrTemperaturaFase5 0x19
#define DWINadrTempoFase5 0x1A
#define DWINadrTemperaturaMashOut 0x1B
#define DWINadrTempoMashOut 0x1C
#define DWINadrTemperaturaFervura 0x1D
#define DWINadrTempoFervura 0x1E
#define DWINadrTempoLupulo1 0x1F
#define DWINadrTempoLupulo2 0x20
#define DWINadrTempoLupulo3 0x21
#define DWINadrTempoLupulo4 0x22
#define DWINadrVolumeAgua 0x23
#define DWINadrTempoSparging 0x24
#define DWINadrNumReceita 0x25
//variavel tratada bit a bit pela função incremental adjustment do display DWIN
//cada area touch seta um dos bits da variavel e o software do microcontrolador
//reseta toda a variavel após detectar e tratar o bit
#define DWINadrBitsReceitaBrassagem 0x26
#define MASKDELRECEITA 0b00000001
#define MASKDOWNLOADRECEITA 0b00000010
#define MASKENCHEUMANUAL 0b00000100
#define MASKENCHERAUTOMATICO 0b00001000
#define MASKBRASSAGEMPAUSE 0b00010000
#define MASKBRASSAGEMMANUAL 0b00100000
#define MASKRESISTENCIAPANELA 0b01000000
#define MASKRESISTENCIARESERVATORIO 0b10000000
//variavel tratada bit a bit pela função incremental adjustment do display DWIN
//cada area touch seta um dos bits da variavel e o software do microcontrolador
//reseta toda a variavel após detectar e tratar o bit
#define DWINadrBitsModoManual 0x40
#define MASKY1 0b00000001
#define MASKY2 0b00000010
#define MASKY3 0b00000100
#define MASKY4 0b00001000
#define MASKY5 0b00010000
#define MASKY6 0b00100000
#define MASKY7 0b01000000
#define MASKY8 0b10000000
#define MASKY9 0b100000000
#define MASKBOMBACIRC 0b1000000000
#define DWINadrKP 0x27
#define DWINadrKI 0x28
#define DWINadrKD 0x29
#define DWINadrTSample 0x2A
#define DWINadrIntensidadeFervura 0x2B
#define DWINadrTemperaturaSparging 0x2C
#define DWINadrVazaoSparging 0x2D
#define DWINadrOffsetSensorMosto 0x2E
#define DWINadrOffsetSensorBoiler 0x2F
#define DWINadrOffsetSensorTrocador 0x30
#define DWINadrEtapasBrassagem 0x31
#define DWINadrTemperaturaMosto 0x32
#define DWINadrMinutosDoInicio 0x33
#define DWINadrSegundosDoInicio 0x34
#define DWINadrMinutosParaFim 0x35
#define DWINadrSegundosParaFim 0x36
#define DWINadrTemperaturaReservatorio 0x37
#define DWINadrTemperaturaSaida 0x38
#define DWINadrPressaommCA 0x0062
#define DWINadrPorcentagemBomba 0x39
#define DWINadrTempoWhirlpool 0x56
#define DWINadrIconeY1 0x41
#define DWINadrIconeY2 0x42
#define DWINadrIconeY3 0x43
#define DWINadrIconeY4 0x44
#define DWINadrIconeY5 0x45
#define DWINadrIconeY6 0x46
#define DWINadrIconeY7 0x47
#define DWINadrIconeY8 0x48
#define DWINadrIconeY9 0x59
#define DWINadrIconeResistenciaPanela 0x49
#define DWINadrIconeResistenciaReservatorio 0x50
#define DWINadrSetPointTemperatura 0x52
#define DWINadrSetPointVesselManual 0x64
#define DWINadrSetPointCentralManual 0x63
#define DWINadrVolumeLitros 0x65
#define NUMTELAMENUPRINCIPAL 1
#define NUMTELABRASSAGEM 6
#define NUMTELARECEITA 4
#define NUMTELACONFIGURACAO 3
#define NUMTELAMANUAL 2
#define NUMPARAMCONFIG 14
#define ADRPARAMEEPROM 240
#define TEMPOSUBIRAUTOMATICO 150                                                //tempo x10ms que o guincho sobe para iniciar o sparging

/*******************************************************************************
 * Variaveis globais
 ******************************************************************************/
bit TransmiteSerial;
unsigned char SerialIn[40];                                                     //buffer entrada dados canal serial - display DWIN
bit fSerialInEnd;
unsigned char SerialOut[40];                                                    //buffer saida dados canal serial - display DWIN
unsigned int ValoresAD[4];                                                      //leituras analogicas
                                                                                //indice 0: temperatura vessel
                                                                                //indice 1: temperatura da central de agua quente
                                                                                //indice 2: temperatura saida trocador de calor
                                                                                //indice 3: pressao mmCA do vessel
unsigned char Minutos;                                                          //contador minutos decorridos na brassagem
unsigned char Segundos;                                                         //contador segundos decorridos na brassagem
unsigned char TotalHoras;                                                       //totalizadores de tempo utilizados
unsigned char TotalMinutos;                                                     //para mostrar na tela tempo decorrido
unsigned char TotalSegundos;                                                    //desde o inicio da brassagem
unsigned char TotalHorasRegressivo;                                             //e o tempo previsto para concluir
unsigned char TotalSegundosRegressivo;
unsigned char TotalMinutosRegressivo;
bit MudancaSegundos;                                                            //usada na trasfega para o fermentador
struct ValoresReceita{
    unsigned char TemperaturaMashIn;                                            //endereco na tela DGUS SDK 0x10
    unsigned char TemperaturaFase1;                                             //endereco na tela DGUS SDK 0x11
    unsigned char TempoFase1;                                                   //endereco na tela DGUS SDK 0x12
    unsigned char TemperaturaFase2;                                             //endereco na tela DGUS SDK 0x13
    unsigned char TempoFase2;                                                   //endereco na tela DGUS SDK 0x14
    unsigned char TemperaturaFase3;                                             //endereco na tela DGUS SDK 0x15
    unsigned char TempoFase3;                                                   //endereco na tela DGUS SDK 0x16
    unsigned char TemperaturaFase4;                                             //endereco na tela DGUS SDK 0x17
    unsigned char TempoFase4;                                                   //endereco na tela DGUS SDK 0x18
    unsigned char TemperaturaFase5;                                             //endereco na tela DGUS SDK 0x19
    unsigned char TempoFase5;                                                   //endereco na tela DGUS SDK 0x1A
    unsigned char TemperaturaMashOut;                                           //endereco na tela DGUS SDK 0x1B
    unsigned char TempoMashOut;                                                 //endereco na tela DGUS SDK 0x1C
    unsigned char TemperaturaFervura;                                           //endereco na tela DGUS SDK 0x1D
    unsigned char TempoFervura;                                                 //endereco na tela DGUS SDK 0x1E
    unsigned char TempoLupulo1;                                                 //endereco na tela DGUS SDK 0x1F
    unsigned char TempoLupulo2;                                                 //endereco na tela DGUS SDK 0x20
    unsigned char TempoLupulo3;                                                 //endereco na tela DGUS SDK 0x21
    unsigned char TempoLupulo4;                                                 //endereco na tela DGUS SDK 0x22
    unsigned char VolumeInicial;                                                //endereco na tela DGUS SDK 0x23
    unsigned char TempoLavagem;                                                 //endereco na tela DGUS SDK 0x24
    unsigned char TempoMashIn;                                                  //endereco na tela DGUS SDK 0x51
    unsigned char Nome[17];
};
struct ValoresReceita ReceitaAtual;
unsigned long mmAguaPrimaria;                                                   //altura da coluna d´água dentro do vessel
unsigned char PWMpanela;                                                        //potencia da resistencia
unsigned char ContDecSeg;                                                       //contador de décimos de segundo
bit OnOffControleTemp;                                                          //hab-desab controle SSR resistencia vessel
bit OnOffControleTempCentral;                                                   //hab-desab controle SSR resistencia central agua quente
bit ControlePotenciaFervura;                                                    //hab-desab controle SSR resistencia vessel durante a fervura
unsigned char SetPointTempPanela;                                               //temperatura alvo da panela
bit TempPanelaOK;                                                               //indica que erro < limite, temperatura vessel ok
unsigned int TempUInt;
bit StatusY2 = 0;                                                               //varieveis usadas na rotina modo
bit StatusY3 = 0;                                                               //manual para armazenar estado valvulas
bit StatusY4 = 0;                                                               //(aberta ou fechada)
bit StatusY5 = 0;
bit StatusY6 = 0;
bit StatusY7 = 0;
bit StatusY8 = 0;
bit StatusY9 = 0;
char KP,KI,KD;                                                                  //cosntantes PID
int TAmostragemPID;                                                             //max 25 seg
unsigned char TonBomba,ToffBomba;                                               //1 a 99 minutos
unsigned char TemperaturaSparging;                                              //tempetura agua lavagem (sparge)
unsigned char TemperaturaSpargingManual;                                        //tempetura agua lavagem (sparge)
unsigned char PorcentagemBombaSparging;                                         //20 a 100%
unsigned char TemperaturaMaxTrasfega;                                           //temperatura max. trasfega para fermentador
unsigned char TempoWhirlpool;                                                   //1 a 30 minutos
unsigned char IntensidadeFervura;                                               //0 (resistencia vessel desligada) a 100%
bit EnviaDadoTrend;                                                             //envia dados curva temperatura vessel para memoria display DWIN
unsigned char MinutosBomba;                                                     //controle da bomba (inversor) lig/desl durante a recirculação (brassagem)
bit ReceitaCarregada;                                                           //para indicar que o operador visualizou a receita antes de iniciar a brassagem
unsigned long PressaommCA;                                                      //conversao leitura A/D 0-10V em pressao mmCA
bit AtualizaValoresTela;                                                        //em determinados momentos não atualiza a tela para otimizar tempo de execução
bit AtualizaRegressivo;                                                         //desabilita decremento tempo durante as etapas da brassagem que aguardam comandos do operador
signed char OffsetAD0,OffsetAD1,OffsetAD2;                                      //correção das leituras de temperatura
//maquina de estados das etapas de brassagem
enum EstadosBrassagem{FALHA,ENCHERMINRESERVATORIO,ENCHERRESERVATORIO,ENCHERMAXRESERVATORIO,ESVAZIARRESERVATORIO,ENCHERVESSELSPARGING,DOSARAGUASPARGING,DOSARAGUAPRIMARIA,CONFIRMACENTRAL,CICLABOMBA,MASHIN,ADICIONARMALTE,DESCANSOMASHIN,PREFASE1,FASE1,PREFASE2,FASE2,PREFASE3,FASE3,PREFASE4,FASE4,PREFASE5,FASE5,TESTEIODO,PREMASHOUT,MASHOUT,VERIFICATEMPRESERVATORIO,RETIRARCESTO,ELEVARCESTOAUTOMATICO,SPARGINGAUTOMATICO,SPARGINGMANUAL,ENCHENDOBOILER,AQUECENDOBOILER,SPARGING,DESCANSOSPARGING,SOBEGUINCHOTOTAL,PREFERVURA,FERVURA,ADICAOLUPULO,FINALIZARFERVURA,WHIRLPOOL,FINALIZAWHIRLPOOL,CONFIRMATRANSFERIR,TRANSFERENCIA};
//maquina de estados da bomba
enum EstadosBomba{CENTRAL,EXTERNO,NIVEL};
unsigned char EstadoBrassagem = ENCHERMINRESERVATORIO;                          //estado inicial da maq. estado da brassagem
unsigned char EstadoBomba = CENTRAL;                                            //estado inicial da maq. estado da bomba

/*******************************************************************************
 * Tratamento interrupções com prioridade alta
 ******************************************************************************/
void interrupt HighISR(void){

    unsigned int Temp16;
    unsigned char Temp8;
    enum EstadosAD{SETACANAL,CONVERSAO,AGUARDACONVERSAO,CALCULAMEDIA};
    static unsigned char EstadoAD = SETACANAL;                                  //maquina de estado leitura analogica
    static unsigned char FreeRun5ms = 0;                                        //contador que incrementa a cada 5ms
    static unsigned char ContMedia = 0;                                         //contador numero de leituras para calcular a media das leituras A/D
    static unsigned int AccValoresAD[4];                                        //acumulador das leituras A/D para calculo da media
    static unsigned int ValoresADanteriores1[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores2[4] = {0,0,0,0};
    static unsigned int ValoresADanteriores3[4] = {0,0,0,0};
    static unsigned char CanalAD = 0;
    signed long Erro;
    static signed long SomaErro = 0;
    static signed long ErroAnterior = 0;
    
    /*  interrupcao timer 0 a cada 5ms.
        seta flag que será usada como base de tempo
        no laço principal para verificar o tempo de ciclo
     */
    if(INTCONbits.T0IE && INTCONbits.T0IF){
        TMR0L = 60;
        INTCONbits.T0IF = 0;

        //atualiza entradas e saidas digitais lendo/escrevendo CI's 74541 e 74574
        TRISD = 0xFF;                                                           //port D como entrada p/ ler CI's 74541
        PORTE = 1;                                                              //chip select entradas digitais
        NOP();
        NOP();
        NOP();
        Inputs2to9 = PORTD;
        PORTE = 3;                                                              //chip select entradas digitais PNP 24V
        NOP();
        NOP();
        NOP();
        Inputs18to25 = PORTD;
        PORTE = 4;                                                              //chip select entradas digitais NPN - fim curso valv. mot.
        NOP();
        NOP();
        NOP();
        Inputs10to17 = PORTD;                                                   //chip select entradas digitais NPN - fim curso valv. mot.
        PORTE = 2;
        NOP();
        TRISD = 0;                                                              //port D como saida p/ ler CI's 74574
        NOP();
        NOP();
        NOP();
        PORTD = Outputs2to9;                                                    //chip select saida coletor aberto
        NOP();
        PORTE = 5;
        NOP();
        NOP();
        NOP();
        PORTD = Outputs10to17;                                                  //chip select reles valv. motorizadas ULN2803
        NOP();
        PORTE = 0;
        NOP();
        TRISD = 0xFF;

        FreeRun5ms++;                                                           //contador que incrementa a cada 5ms (timer 0)
        if((FreeRun5ms % 20) == 0)
            ContDecSeg++;                                                       //contador que incrementa a cada 0,1 seg
        //calcula porcentagem de acionamento da resistencia da panela
        if(OnOffControleTemp){
            if(ContDecSeg >= TAmostragemPID){
                ContDecSeg = 0;
                Erro = (long)SetPointTempPanela*10 - ValoresAD[0];                    //resolução erro 0,1oC
                if((Erro >= LIMITEMINERRO) && (Erro <= LIMITEMAXERRO))
                    TempPanelaOK = 1;
                else
                    TempPanelaOK = 0;
                SomaErro += Erro;
                if( (SomaErro < LIMITEMINSOMAERRO) || (SomaErro > LIMITEMAXSOMAERRO) )
                    SomaErro = 0;
                Erro = (long)(Erro*KP)/10 + (long)(SomaErro*KI)/10 + (long)(KD/TAmostragemPID)*(Erro-ErroAnterior);
                ErroAnterior = Erro;
                                
                if( (Erro < 0) || (ValoresAD[0]>=SetPointTempPanela*10) )
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
            MudancaSegundos = 1;
            TotalSegundos++;
            FreeRun5ms = 0;
            if(Segundos > 59){
                Segundos = 0;
                Minutos++;
            }
            if(TotalMinutosRegressivo == 0){
                TotalMinutosRegressivo = 59;
                if(TotalHorasRegressivo > 0)
                    TotalHorasRegressivo--;
            }
            if(TotalSegundos > 59){
                TotalSegundos = 0;
                MinutosBomba++;
                EnviaDadoTrend = 1;
                TotalMinutos++;
                if(AtualizaRegressivo)
                    TotalMinutosRegressivo--;
                if( (TotalHorasRegressivo == 0) && (TotalMinutosRegressivo == 0) )
                    TotalMinutosRegressivo = 1;
            }
            if(TotalMinutos > 59){
                TotalMinutos = 0;
                TotalHoras++;
            }
        }
        
        if(OnOffControleTemp){
            if( (PWMpanela > PWMmin) && (ValoresAD[0]<SetPointTempPanela*10) ){
                if(FreeRun5ms < PWMpanela)
                    RESISTENCIAPANELAON;
                else
                    RESISTENCIAPANELAOFF;
            }
            else
                RESISTENCIAPANELAOFF;
        }
        
        if(ControlePotenciaFervura){
            if(PWMpanela > PWMmin){
                if(FreeRun5ms < PWMpanela)
                    RESISTENCIAPANELAON;
                else
                    RESISTENCIAPANELAOFF;
            }
            else
                RESISTENCIAPANELAOFF;
        }

        if(OnOffControleTempCentral){
            if(ValoresAD[1] <= TemperaturaSpargingManual )
                RESISTENCIARESERVATORIOON;
            else
                RESISTENCIARESERVATORIOOFF;
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
            ADCON0bits.GO = 1;                                                  //inicia conversao A/D
            EstadoAD = AGUARDACONVERSAO;
            break;
        case AGUARDACONVERSAO:
            if(!ADCON0bits.GO){                                                 //verifica se acabou conversão
                Temp16 = ADRESH;
                Temp16 = Temp16 << 8;
                Temp16 |= ADRESL;
                AccValoresAD[CanalAD] += Temp16;                                //salva no acumulador
                CanalAD++;
                if(CanalAD > 3){                                                //converteu os 4 canais
                    CanalAD = 0;
                    ContMedia++;                                                //incrementa contador amostra
                }
                EstadoAD = SETACANAL;
                if(ContMedia == MEDIA_AD)                                       //atingiu amostras para tirar media?
                    EstadoAD = CALCULAMEDIA;
            }
            break;
        case CALCULAMEDIA:
            for(Temp8=0;Temp8<4;Temp8++){   //calcula a media dos 4 canais
                ValoresAD[Temp8] = AccValoresAD[Temp8] / MEDIA_AD;
                if(Temp8<3){
                    switch(Temp8){
                        case 0: ValoresAD[0] += OffsetAD0;break;
                        case 1: ValoresAD[1] += OffsetAD1;break;
                        case 2: ValoresAD[2] += OffsetAD2;break;
                        default:break;
                    }
                    //faz um media das 4 medias anteriores
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
            PressaommCA = ValoresAD[3];
            if(PressaommCA >= OFFSETPRESS){
                PressaommCA -= OFFSETPRESS;
                PressaommCA *= KPRESS;
                PressaommCA /= 100;
            }
            else
                PressaommCA = 0;
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

void DelayDWIN(void){
    unsigned char cont;

    for(cont=0;cont<DELAYDWIN;cont++)
         __delay_ms(1);
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
    //Configuração dos ports de A a E
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

    //geracao do sinal D/A para controle da rotação da bomba - sinal analógico para a entrada do inversor de frequencia
    PR2 = 0xFF;
    CCPR2L = 0x7F;
    T2CON = 0x04;                                                               //pre scaler 4
    CCP2CON = 0x0F;                                                             //PWM

    //configura timer 3 - será usado com CCP1 pra medir fluxo entrada agua
    //T3CON = 0b11111001;
    //PIE2bits.TMR3IE = 1;
    //IPR2bits.TMR3IP = 0;        //baixa prioridade int timer 3

    CCP1CON = 0b00000101;
    IPR1bits.CCP1IP = 0;        //baixa prioridade captura fluxo agua
    PIE1bits.CCP1IE = 1;        //habilita interrupção captura

    //habiltaçao das interrupções
    RCONbits.IPEN = 1;                                                          //habilita 2 niveis de prioridade para as interrupções
    INTCONbits.GIEH = 1;                                                        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 1;                                                        //habilita interrupções com prioridade baixa
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

/*  Leitura dos valores de configuracao do equipamento
 *  que podem ser editados na tela do display DWIN especifica para isto
 */
void LeParamConfig(void){
    KP = LerDWINram(0x27);                                                 //constante proporcional do controle PID da temp. panela
    KI = LerDWINram(0x28);                                                 //constante integral do controle PID da temp. panela
    KD = LerDWINram(0x29);                                                 //constante derivativa do controle PID da temp. panela
    TAmostragemPID = LerDWINram(0x2A);
    TonBomba = LerDWINram(0x57);
    ToffBomba = LerDWINram(0x58);
    TemperaturaSparging = LerDWINram(0x2C);
    PorcentagemBombaSparging = LerDWINram(0x2D);
    TemperaturaMaxTrasfega = LerDWINram(0x55);
    TempoWhirlpool = LerDWINram(0x56);
    IntensidadeFervura = LerDWINram(0x2B);
    OffsetAD0 = LerDWINram(0x2E);
    OffsetAD1 = LerDWINram(0x2F);
    OffsetAD2 = LerDWINram(0x30);
}

void GravaParamConfig(void)
{
    unsigned char utemp8;

    for(utemp8 = 0;utemp8 < NUMPARAMCONFIG;utemp8++)
    {
        switch(utemp8){
            case 0: eeprom_write(ADRPARAMEEPROM+utemp8,KP);break;
            case 1: eeprom_write(ADRPARAMEEPROM+utemp8,KI);break;
            case 2: eeprom_write(ADRPARAMEEPROM+utemp8,KD);break;
            case 3: eeprom_write(ADRPARAMEEPROM+utemp8,TAmostragemPID);break;
            case 4: eeprom_write(ADRPARAMEEPROM+utemp8,TonBomba);break;
            case 5: eeprom_write(ADRPARAMEEPROM+utemp8,ToffBomba);break;
            case 6: eeprom_write(ADRPARAMEEPROM+utemp8,TemperaturaSparging);break;
            case 7: eeprom_write(ADRPARAMEEPROM+utemp8,PorcentagemBombaSparging);break;
            case 8: eeprom_write(ADRPARAMEEPROM+utemp8,TemperaturaMaxTrasfega);break;
            case 9: eeprom_write(ADRPARAMEEPROM+utemp8,TempoWhirlpool);break;
            case 10: eeprom_write(ADRPARAMEEPROM+utemp8,IntensidadeFervura);break;
            case 11: eeprom_write(ADRPARAMEEPROM+utemp8,OffsetAD0);break;
            case 12: eeprom_write(ADRPARAMEEPROM+utemp8,OffsetAD1);break;
            case 13: eeprom_write(ADRPARAMEEPROM+utemp8,OffsetAD2);break;
        }
        __delay_ms(10);
    }
    
}

void AtualizaParamNoDisplay(void){
    unsigned char utemp8;

    for(utemp8 = 0;utemp8 < NUMPARAMCONFIG;utemp8++)
    {
        switch(utemp8){
            case 0: EscreverDWINram(0x27,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 1: EscreverDWINram(0x28,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 2: EscreverDWINram(0x29,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 3: EscreverDWINram(0x2A,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 4: EscreverDWINram(0x57,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 5: EscreverDWINram(0x58,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 6: EscreverDWINram(0x2C,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 7: EscreverDWINram(0x2D,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 8: EscreverDWINram(0x55,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 9: EscreverDWINram(0x56,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 10: EscreverDWINram(0x2B,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 11: EscreverDWINram(0x2E,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 12: EscreverDWINram(0x2F,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 13: EscreverDWINram(0x30,eeprom_read(ADRPARAMEEPROM+utemp8));break;
        }
    }
}

void ApagaReceitasDaEEPROM()
{
    NOP();
    unsigned char utemp8;

    for(utemp8 = 0;utemp8 < 222;utemp8++)
    {
        eeprom_write(utemp8,0);
        __delay_ms(10);
    }
}

/*  Leitura dos valores da receita armazanados no display touch
 *  Retorna 1 se dados tiverem consistencia ou 0 se houver algum problema com os valores
 *  Por exemplo: 1a temperatura da rampa < temperatura mash in
 */
unsigned char LeDadosReceita(void)
{
    unsigned char utemp8;
    unsigned int buffer;

    for(utemp8=0x10;utemp8<0x26;utemp8++){
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
            case 0x25: ReceitaAtual.TempoMashIn = LerDWINram(0x51);    //tempo Mash In foi colocado depois
        }
    }

    //conversâo do volume para altura de agua primaria no vessel
    mmAguaPrimaria = (long)ReceitaAtual.VolumeInicial*10000;
    mmAguaPrimaria /= 6082;
    
    for(utemp8=0;utemp8<8;utemp8++){
        buffer = LerDWINram(0x100+utemp8);
        ReceitaAtual.Nome[utemp8*2+1] = buffer;
        ReceitaAtual.Nome[utemp8*2] = buffer>>8;
    }

    //Verificacao da consistencia dos valores da receita
    if(ReceitaAtual.TemperaturaFase1 < ReceitaAtual.TemperaturaMashIn)
        return(0);
    if( (ReceitaAtual.TemperaturaFase2 == 0) && ( (ReceitaAtual.TemperaturaFase3 != 0) || (ReceitaAtual.TemperaturaFase4 != 0) || (ReceitaAtual.TemperaturaFase5 != 0) ) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase2 != 0) && (ReceitaAtual.TemperaturaFase2 < ReceitaAtual.TemperaturaFase1) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase2 != 0) && (ReceitaAtual.TempoFase2 == 0) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase2 != 0) && (ReceitaAtual.TemperaturaFase5 == 0) && (ReceitaAtual.TemperaturaFase4 == 0) && (ReceitaAtual.TemperaturaFase3 == 0) && (ReceitaAtual.TemperaturaMashOut < ReceitaAtual.TemperaturaFase3) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase3 == 0) && ( (ReceitaAtual.TemperaturaFase4 != 0) || (ReceitaAtual.TemperaturaFase5 != 0) ) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase3 != 0) && (ReceitaAtual.TemperaturaFase3 < ReceitaAtual.TemperaturaFase2) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase3 != 0) && (ReceitaAtual.TempoFase3 == 0) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase3 != 0) && (ReceitaAtual.TemperaturaFase5 == 0) && (ReceitaAtual.TemperaturaFase4 == 0) && (ReceitaAtual.TemperaturaMashOut < ReceitaAtual.TemperaturaFase3) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase4 == 0) && (ReceitaAtual.TemperaturaFase5 != 0) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase4 != 0) && (ReceitaAtual.TemperaturaFase5 == 0) && (ReceitaAtual.TemperaturaMashOut < ReceitaAtual.TemperaturaFase4) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase4 != 0) && (ReceitaAtual.TemperaturaFase4 < ReceitaAtual.TemperaturaFase3) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase4 != 0) && (ReceitaAtual.TempoFase4 == 0) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase5 != 0) && (ReceitaAtual.TemperaturaFase5 < ReceitaAtual.TemperaturaFase4) )
        return(0);
    if( (ReceitaAtual.TemperaturaFase5 != 0) && ( (ReceitaAtual.TempoFase5 == 0) || (ReceitaAtual.TemperaturaMashOut < ReceitaAtual.TemperaturaFase5) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo1 == 0) && ( (ReceitaAtual.TempoLupulo2 != 0) || (ReceitaAtual.TempoLupulo3 != 0) || (ReceitaAtual.TempoLupulo4 != 0) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo2 != 0) && (ReceitaAtual.TempoLupulo2 > ReceitaAtual.TempoLupulo1) )
        return(0);
    if( (ReceitaAtual.TempoLupulo2 == 0) && ( (ReceitaAtual.TempoLupulo3 != 0) || (ReceitaAtual.TempoLupulo4 != 0) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo3 != 0) && (ReceitaAtual.TempoLupulo3 > ReceitaAtual.TempoLupulo2) )
        return(0);
    if( (ReceitaAtual.TempoLupulo3 == 0) && (ReceitaAtual.TempoLupulo4 != 0) )
        return(0);
    if( (ReceitaAtual.TempoLupulo4 != 0) && (ReceitaAtual.TempoLupulo4 > ReceitaAtual.TempoLupulo3) )
        return(0);

    //os valores abaixo são os tempos dos patamares de temperatura
    buffer = ReceitaAtual.TemperaturaMashIn - 25;   //valor da temperatura da água logo após enchimento
    buffer+= ReceitaAtual.TempoMashIn;
    buffer+= ReceitaAtual.TempoFase1;
    buffer+= ReceitaAtual.TempoFase2;
    buffer+= ReceitaAtual.TempoFase3;
    buffer+= ReceitaAtual.TempoFase4;
    buffer+= ReceitaAtual.TempoFase5;
    buffer+= ReceitaAtual.TempoMashOut;
    buffer+= ReceitaAtual.TempoFervura;
    buffer+= ReceitaAtual.TemperaturaMashIn;                                    //somando esta temperatura corrigi alguns erros de operações manuais, o correto seria considerar a temperatura ambiente

    //os  valores abaixo são os tempos das rampas de temperatura
    buffer+= ReceitaAtual.TemperaturaFase1 - ReceitaAtual.TemperaturaMashIn;
    
    if(ReceitaAtual.TempoFase2>0){
        buffer+= ReceitaAtual.TemperaturaFase2 - ReceitaAtual.TemperaturaFase1;
    }
    if(ReceitaAtual.TempoFase3>0){
        buffer+= ReceitaAtual.TemperaturaFase3 - ReceitaAtual.TemperaturaFase2;
    }
    if(ReceitaAtual.TempoFase4>0){
        buffer+= ReceitaAtual.TemperaturaFase4 - ReceitaAtual.TemperaturaFase3;
    }
    if(ReceitaAtual.TempoFase5>0){
        buffer+= ReceitaAtual.TemperaturaFase5 - ReceitaAtual.TemperaturaFase4;
    }

    if(buffer>60){
        TotalHorasRegressivo = buffer/60;
        TotalMinutosRegressivo = buffer%60;
    }
    else{
        TotalHorasRegressivo = 0;
        TotalMinutosRegressivo = buffer;
    }

    /*
    for(utemp8=0;utemp8<15;utemp8++){
        buffer = LerDWINram(0x100+utemp8);
        ReceitaAtual.Nome[utemp8] = buffer;
        //ReceitaAtual.Nome[utemp8*2+1] = buffer;
    }
     */
    NOP();

    return(1);
}

void GravarDadosNaEEPROM(unsigned char receita)
{
    unsigned char cont,temp;

    receita--;
    if(receita>0)
        receita *= 38;

    for(cont=0;cont<22;cont++){
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
            case 21: temp = ReceitaAtual.TempoMashIn;break;
        }
        eeprom_write((receita+cont),temp);
        NOP();
        __delay_ms(10);
    }
    for(cont=22;cont<38;cont++){
        temp = ReceitaAtual.Nome[cont-22];
        eeprom_write((receita+cont),temp);
    }
    NOP();
}

void AtualizaReceitaNoDisplay(unsigned char receita){

    unsigned char cont,contdest,temp;
    unsigned int buffer;

    receita--;
    if(receita>0)
        receita *= 38;
    contdest = 0;
    for(cont=receita;cont<receita+37;cont++){
        temp = eeprom_read(cont);
        if(contdest<21)
            EscreverDWINram(contdest+0x10,temp);
        if(contdest == 21)
            EscreverDWINram(0x51,temp); //fora da sequencia pq foi inserido depois
        contdest++;
    }
    contdest = 0;
    for(cont=receita+22;cont<receita+38;cont+=2){
        buffer = eeprom_read(cont);
        buffer = buffer << 8;
        buffer|= eeprom_read(cont+1);
        EscreverDWINram(contdest+0x100,buffer);
        contdest++;
    }
}

unsigned char AcionaValvulaMotorizada(unsigned char sentido,unsigned char valvula){

    unsigned char Timeout;

    if(sentido == ABRIR)
        SENTIDOVALVULA = 0;
    else
        SENTIDOVALVULA = 1;

    DelayDWIN();

    switch(valvula){
        case MASKY2:VALVULASAIDARESERVATORIOON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULASAIDARESERVATORIOFECHADA)) || ((sentido == ABRIR) && (VALVULASAIDARESERVATORIOABERTA)) );
                    break;
        case MASKY3:VALVULASAIDAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULASAIDAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULASAIDAPANELAABERTA)) );
                    break;
        case MASKY4:VALVULALAVAGEMGRAOSON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULALAVAGEMGRAOSFECHADA)) || ((sentido == ABRIR) && (VALVULALAVAGEMGRAOSABERTA)) );
                    break;
        case MASKY5:VALVULAENTRADAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAPANELAABERTA)) );
                    break;
        case MASKY6:VALVULAWHIRLPOOLON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULAWHIRLPOOLFECHADA)) || ((sentido == ABRIR) && (VALVULAWHIRLPOOLABERTA)) );
                    break;
        case MASKY7:VALVULAENTRADAMOSTOTROCADORON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAMOSTOTROCADORFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAMOSTOTROCADORABERTA)) );
                    break;
        case MASKY8:VALVULAENTRADAAGUATROCADORON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        NOP();
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAAGUATROCADORFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAAGUATROCADORABERTA)) );
                    break;
        case MASKY9:VALVULAENTRADAAGUAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200)
                            return(0);
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAAGUAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAAGUAPANELAABERTA)) );
                    break;
    }
    //DelaySeg(3);
    switch(valvula){
        case MASKY2: VALVULASAIDARESERVATORIOOFF;break;
        case MASKY3: VALVULASAIDAPANELAOFF;break;
        case MASKY4: VALVULALAVAGEMGRAOSOFF;break;
        case MASKY5: VALVULAENTRADAPANELAOFF;break;
        case MASKY6: VALVULAWHIRLPOOLOFF;break;
        case MASKY7: VALVULAENTRADAMOSTOTROCADOROFF;break;
        case MASKY8: VALVULAENTRADAAGUATROCADOROFF;break;
        case MASKY9: VALVULAENTRADAAGUAPANELAOFF;break;

    }
    DelayDWIN();
}

void Bipar(void)
{
    SIRENEON;
    DelaySeg(1);
    SIRENEOFF;
}

void ResetaSaidas(void)
{
    EscreverDWINreg(3,29);  //exibe tela de fechamento das valvulas
    OnOffControleTemp = 0;
    ControlePotenciaFervura = 0;
    OnOffControleTempCentral = 0;
    CCPR2L = 0;
    SIRENEOFF;
    VALVULAENTRADARESERVATORIOOFF;
    RELEGUINCHOOFF;
    RESISTENCIAPANELAOFF;
    RESISTENCIARESERVATORIOOFF;
    BOMBACIRCOFF;
    AcionaValvulaMotorizada(FECHAR,MASKY2);
    AcionaValvulaMotorizada(FECHAR,MASKY3);
    AcionaValvulaMotorizada(FECHAR,MASKY4);
    AcionaValvulaMotorizada(FECHAR,MASKY5);
    AcionaValvulaMotorizada(FECHAR,MASKY6);
    AcionaValvulaMotorizada(FECHAR,MASKY7);
    AcionaValvulaMotorizada(FECHAR,MASKY8);
    AcionaValvulaMotorizada(FECHAR,MASKY9);
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
    StatusY8 = 0;
    EscreverDWINram(DWINadrIconeY8,0);
    StatusY9 = 0;
    EscreverDWINram(DWINadrIconeY9,0);
    EscreverDWINram(DWINadrIconeResistenciaPanela,0);
    EscreverDWINram(DWINadrIconeResistenciaReservatorio,0);
    EscreverDWINram(DWINadrPorcentagemBomba,0);
    //EscreverDWINreg(3,1);  //exibe tela menu principal
}

void ExecutaBrassagem(void)
{
    unsigned int temp;
    unsigned char MinutosEtapa;
    unsigned int PressaoMaxSparging;
    unsigned char SegundosControleTrasfega;
    unsigned char DelaySobeGuincho;

    //******** FALHA !!! ********
    if(BOTAO_EMERGENCIA){
        EscreverDWINreg(3,10);  // exibe emergencia pressionada
        while(BOTAO_EMERGENCIA);    //aguarda soltar botão emergencia
        EscreverDWINreg(3,31);  // solicita pressionar botão para energizar saidas
        while(!BOTAO_START);
        EstadoBrassagem = FALHA;
    }
    
    if(AtualizaValoresTela){
        EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);
        EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
        EscreverDWINram(DWINadrSetPointTemperatura,SetPointTempPanela);
        EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);
    
        if((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= WHIRLPOOL)){
            EscreverDWINram(DWINadrMinutosDoInicio,TotalHoras);
            EscreverDWINram(DWINadrSegundosDoInicio,TotalMinutos);
            EscreverDWINram(DWINadrMinutosParaFim,TotalHorasRegressivo);
            EscreverDWINram(DWINadrSegundosParaFim,TotalMinutosRegressivo);
        }

    }
    if( (EstadoBrassagem > CICLABOMBA) && (EstadoBrassagem <= WHIRLPOOL) && (EstadoBrassagem != ELEVARCESTOAUTOMATICO) && (EstadoBrassagem != SPARGINGAUTOMATICO) && (EstadoBrassagem != SOBEGUINCHOTOTAL) )
        AtualizaRegressivo = 1;
    else
        AtualizaRegressivo = 0;

    if( ((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= MASHOUT) && (EstadoBomba == CENTRAL)) || (EstadoBrassagem == SPARGINGMANUAL) || ((EstadoBrassagem >= PREFERVURA) && (EstadoBrassagem <= FINALIZARFERVURA)) ){
        temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
        CCPR2L = temp / 100;
    }

    if( (EstadoBrassagem == FERVURA) || (EstadoBrassagem == FINALIZARFERVURA) ){
        PWMpanela = LerDWINram(DWINadrIntensidadeFervura)*2 - 1;
    }
    else{
        EscreverDWINram(DWINadrIntensidadeFervura,PWMpanela/2);
    }

    if((EstadoBrassagem >= PREFASE1) && (EstadoBrassagem <= MASHOUT)){
        switch(EstadoBomba){
            case CENTRAL:
                if(MinutosBomba >= TonBomba){
                    MinutosBomba = 0;
                    CCPR2L = 0;    //desliga bomba pra equalizar nivel fora cesto
                    EstadoBomba = NIVEL;
                }
                break;
            case EXTERNO:
                if(MinutosBomba >= ToffBomba){
                    AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                    AcionaValvulaMotorizada(FECHAR,MASKY6);  //whirlpool
                    MinutosBomba = 0;
                    EstadoBomba = CENTRAL;
                }
                break;
            case NIVEL:
                if(MinutosBomba >= 1){
                    AcionaValvulaMotorizada(ABRIR,MASKY6);  //whirlpool
                    AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada do vessel
                    CCPR2L = 85;    //bomba a 33%
                    EstadoBomba = EXTERNO;
                }
                break;
        }
    }

    if(EnviaDadoTrend){
        EnviaDadoTrend = 0;
        if((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= WHIRLPOOL)){
            if(ValoresAD[0]>400)
                EscreverDWINtrend(ValoresAD[0]);
        }
    }

    if( (EstadoBrassagem >= ENCHERMINRESERVATORIO) && (EstadoBrassagem <= VERIFICATEMPRESERVATORIO) )
        BOMBACIRCON;
    else
        BOMBACIRCOFF;

    if( (EstadoBrassagem >= CICLABOMBA) && (EstadoBrassagem <= VERIFICATEMPRESERVATORIO) ){
        if(ValoresAD[1] < TemperaturaSparging*10)
            RESISTENCIARESERVATORIOON;
        else
            RESISTENCIARESERVATORIOOFF;
    }
    else{
        RESISTENCIARESERVATORIOOFF;
    }

    if( (EstadoBrassagem != ELEVARCESTOAUTOMATICO) && (EstadoBrassagem != SPARGINGAUTOMATICO) /*&& (EstadoBrassagem != SOBEGUINCHOTOTAL)*/ ){
        if(!DESCERGUINCHO){
            if(SUBIRGUINCHO){
                SUBIR;
                RELEGUINCHOON;
            }
            else
                RELEGUINCHOOFF;
        }
        if(!SUBIRGUINCHO){
            if(DESCERGUINCHO){
                DESCER;
                RELEGUINCHOON;
            }
            else
                RELEGUINCHOOFF;
        }
    }

    switch(EstadoBrassagem){
        case FALHA:
            //ResetaSaidas();
            EstadoBrassagem = ENCHERMINRESERVATORIO;//ENCHERRESERVATORIO;
            EstadoBomba = CENTRAL;
            DelaySeg(1);    //tempo de energização do inversor de frequencia
            if(INVERSOROK){
                RESETINVERSORON;
                DelaySeg(5);
                RESETINVERSOROFF;
                if(INVERSOROK){
                    EscreverDWINreg(3,30);                                                 // falha inversor
                    while(LerDWINreg(3) != 1);
                }
            }
            EscreverDWINreg(3,1);  //retorna para menu principal, encerrando a brassagem                                        // falha inversor
                            
            break;
        //enche até altura dos furos de tubulações do reservatório
        case ENCHERMINRESERVATORIO:
            if(!NB_BOILER){
                VALVULAENTRADARESERVATORIOOFF;
                EstadoBrassagem = ENCHERRESERVATORIO;
            }
            else
                VALVULAENTRADARESERVATORIOON;
            break;
        case ENCHERRESERVATORIO:
            EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
            EscreverDWINram(DWINadrEtapasBrassagem,1);  //preenche amarelo etapa encher com agua
            SetPointTempPanela = 0;
            //o volume de lavagem zerado, o usuario vai realizar o sparging
            if(ReceitaAtual.TempoLavagem == 0){
                if(!NA_BOILER)
                    EstadoBrassagem = ENCHERMAXRESERVATORIO;
                else
                {
                    if(PressaommCA <= mmAguaPrimaria)
                        AcionaValvulaMotorizada(ABRIR,MASKY9);  //entrada agua vessel
                    EstadoBrassagem = DOSARAGUAPRIMARIA;
                }
            }
            //já está programada a quantidade de água
            //vai encher o reservtório através do enchimento da vessel,
            //que tem o transmissor de pressão
            else{
                //reservatorio está vazio, vai para enchimento do vessel com
                //o volume de sparging
                if(NB_BOILER){
                    if(PressaommCA <= (MINIMOMMCAENCHERRESERVATORIO+ReceitaAtual.TempoLavagem*10)){
                        AcionaValvulaMotorizada(ABRIR,MASKY9);  //entrada agua vessel
                        EstadoBrassagem = ENCHERVESSELSPARGING;
                    }
                    else{
                        PressaoMaxSparging = PressaommCA;
                        AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida do reservatorio
                        AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                        EstadoBrassagem = DOSARAGUASPARGING;
                    }
                }
                //há agua no reservatorio, primeiro passa para o estado
                //de esvaziar para depois poder saber o volume exato
                //que sairá do vessel para o reservatório
                else{
                    EscreverDWINram(DWINadrPorcentagemBomba,50); //bomba a 50%
                    AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida do reservatorio
                    AcionaValvulaMotorizada(ABRIR,MASKY6);  //entrada whirlpool
                    CCPR2L = 127;
                    EstadoBrassagem = ESVAZIARRESERVATORIO;
                }
            }
            break;
        case ENCHERMAXRESERVATORIO:
            VALVULAENTRADARESERVATORIOON;                                       //Aciona valvula para comecar a encher o reservatorio
            if(NA_BOILER){
                VALVULAENTRADARESERVATORIOOFF;
                AcionaValvulaMotorizada(ABRIR,MASKY9);  //entrada agua vessel
                EstadoBrassagem = DOSARAGUAPRIMARIA;
            }
            break;
        case ESVAZIARRESERVATORIO:
            if(NB_BOILER){
                CCPR2L = 0;
                AcionaValvulaMotorizada(FECHAR,MASKY6);  //entrada do whirlpool
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida do reservatorio
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba a 0%
                if(PressaommCA <= (MINIMOMMCAENCHERRESERVATORIO+ReceitaAtual.TempoLavagem*10))
                    AcionaValvulaMotorizada(ABRIR,MASKY9);  //entrada agua vessel
                EstadoBrassagem = ENCHERVESSELSPARGING;
            }
            break;
        case ENCHERVESSELSPARGING:
            if(PressaommCA > (MINIMOMMCAENCHERRESERVATORIO+ReceitaAtual.TempoLavagem*10)){
                PressaoMaxSparging = PressaommCA;
                AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida do reservatorio
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY9);  //entrada agua vessel
                EstadoBrassagem = DOSARAGUASPARGING;
            }
            break;
        case DOSARAGUASPARGING:
            if( (PressaommCA <= (PressaoMaxSparging-ReceitaAtual.TempoLavagem*10) ) || BOTAO_START){
                AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida do reservatorio
                if(PressaommCA <= (ReceitaAtual.VolumeInicial*10))
                    AcionaValvulaMotorizada(ABRIR,MASKY9);  //entrada agua vessel
                EstadoBrassagem = DOSARAGUAPRIMARIA;
            }
            break;
        case DOSARAGUAPRIMARIA:
            if(PressaommCA >= mmAguaPrimaria){
                EscreverDWINreg(3,35);  //exibe tela aguardar terminar aquecer sparging
                SIRENEON;
                AcionaValvulaMotorizada(FECHAR,MASKY9);  //entrada agua vessel
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                EstadoBrassagem = CONFIRMACENTRAL;
            }
            NOP();
            break;
        case CONFIRMACENTRAL:
            if(BOTAO_START){
                SIRENEOFF;
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EstadoBrassagem = CICLABOMBA;
            }
        break;
        case CICLABOMBA:    //cicla para retirar bolhas ar do circuito
            for(temp=0;temp<CICLOSBOMBA;temp++){
                if( (!BOTAO_EMERGENCIA) && (LerDWINreg(03) == NUMTELAMENUPRINCIPAL) ) break;
                CCPR2L = 255;
                EscreverDWINram(DWINadrPorcentagemBomba,100); //bomba a 100%
                DelaySeg(2);
                CCPR2L = 25;
                EscreverDWINram(DWINadrPorcentagemBomba,10); //bomba a 10%
                DelaySeg(2);
            }
            EscreverDWINram(DWINadrEtapasBrassagem,2);  //preenche amarelo etapa encher com agua
            CCPR2L = 85;
            EscreverDWINram(DWINadrPorcentagemBomba,33); //bomba a 33%
            RESISTENCIARESERVATORIOOFF;    //seleciona resistencia vessel
            RESISTENCIAPANELAON;
            OnOffControleTemp = 1;
            Minutos = 0;
            Segundos = 0;
            SetPointTempPanela = ReceitaAtual.TemperaturaMashIn;
            TempPanelaOK = 0;
            Segundos = 0;
            Minutos = 0;
            TotalHoras = 0;
            TotalSegundos = 0;
            TotalMinutos = 0;
            EstadoBrassagem = MASHIN;
            break;
        case MASHIN:
            NOP();
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                EscreverDWINram(DWINadrEtapasBrassagem,3);  //preenche amarelo etapa adicionar malte
                EstadoBrassagem = ADICIONARMALTE;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                EscreverDWINreg(3,7);   //exibe tela para adicionar malte
                SIRENEON;
                //BOMBADAGUA = 0;
            }
        break;
        case ADICIONARMALTE:
            if(LerDWINreg(3) == 6){
                CCPR2L = 0;     //bomba parada
                OnOffControleTemp = 0;  //como nao havera circulaçao, desliga resistência
                RESISTENCIAPANELAOFF;
                EscreverDWINreg(3,13);  //exibe tela confirmar adicao malte
                SIRENEOFF;
            }
            //if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKBRASSAGEMPLAY){
            if( (BOTAO_START) && (!SIRENE) ){       //so aceita botão se já reconheceu a tela, ou seja, desligou sirene
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoMashIn;
                RESISTENCIAPANELAON;
                OnOffControleTemp = 1;  //liga novamente a resistencia
                TempPanelaOK = 0;   
                CCPR2L = 64;
                EscreverDWINram(DWINadrPorcentagemBomba,25); //bomba a 25%
                EscreverDWINram(DWINadrEtapasBrassagem,4);  //preenche amarelo etapa rampas temperatura
                EstadoBrassagem = DESCANSOMASHIN;
                //BOMBADAGUA = 1;
             }
        break;
        case DESCANSOMASHIN:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase1;
                TempPanelaOK = 0;
                MinutosBomba = 0;
                EstadoBrassagem = PREFASE1;
            }
            break;
        case PREFASE1:
            if(ReceitaAtual.TempoFase1 == 0){
                SetPointTempPanela = ReceitaAtual.TemperaturaFase2;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE2;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase1;
                EstadoBrassagem = FASE1;
            }
        break;
        case FASE1:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase2;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE2;
            }
        break;
        case PREFASE2:
            if(ReceitaAtual.TempoFase2 == 0){
                SetPointTempPanela = ReceitaAtual.TemperaturaFase3;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE3;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase2;
                EstadoBrassagem = FASE2;
            }
            break;
        case FASE2:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase3;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE3;
            }
        break;
        case PREFASE3:
            if(ReceitaAtual.TempoFase3 == 0){
                EstadoBrassagem = PREFASE4;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase4;
                TempPanelaOK = 0;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase3;
                EstadoBrassagem = FASE3;
            }
            break;
        case FASE3:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase4;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE4;
            }
        break;
        case PREFASE4:
            if(ReceitaAtual.TempoFase4 == 0){
                SetPointTempPanela = ReceitaAtual.TemperaturaFase5;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE5;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase4;
                EstadoBrassagem = FASE4;
            }
            break;
        case FASE4:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase5;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE5;
            }
        break;
        case PREFASE5:
            if(ReceitaAtual.TempoFase5 == 0){
                SIRENEON;
                EscreverDWINreg(3,16);  //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
            break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase5;
                EstadoBrassagem = FASE5;
            }
            break;
        case FASE5:
            if(Minutos>=MinutosEtapa){
                SIRENEON;
                EscreverDWINreg(3,16);  //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
            }
        break;
        case TESTEIODO:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,17);  //exibe tela confirmar MASH OUT
                SIRENEOFF;
            }
            if( (BOTAO_START) && (SIRENE == 0) ){       //so aceita botão se já reconheceu a tela, ou seja, desligou sirene
                EscreverDWINreg(3,6);  //exibe tela brassagem
                SetPointTempPanela = ReceitaAtual.TemperaturaMashOut;
                Minutos = 0;
                Segundos = 0;
                TempPanelaOK = 0;
                EstadoBrassagem = PREMASHOUT;
            }
        break;
        case PREMASHOUT:
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoMashOut;
                EstadoBrassagem = MASHOUT;
            }
            break;
        case MASHOUT:
            if(Minutos>=MinutosEtapa){
                Minutos = 0;
                Segundos = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                EscreverDWINram(DWINadrEtapasBrassagem,5);  //preenche amarelo etapa retirar cesto
                //EscreverDWINreg(3,9);   //exibe tela para retirar cesto
                CCPR2L = 0;     //desliga bomba
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY6);  //whirlpool
                OnOffControleTemp = 0;
                RESISTENCIAPANELAOFF;  //alt. em 27set16, passou a desligar a resistencia, pois se operador demora pra retirar o cesto, dá problema
                RESISTENCIARESERVATORIOOFF;
                SIRENEON;
                EstadoBrassagem = VERIFICATEMPRESERVATORIO;
            }
        break;
        case VERIFICATEMPRESERVATORIO:
            if( (ValoresAD[1] >= (TemperaturaSparging-2)*10) || BOTAO_START){
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EstadoBrassagem = RETIRARCESTO;
            }    
            else
                EscreverDWINreg(3,33);  //exibe tela aguardar terminar aquecer sparging
        break;
        case RETIRARCESTO:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,14);  //exibe tela confirmar retirada cesto
                SIRENEOFF;
            }
            
            //if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKBRASSAGEMPLAY){
            if( (BOTAO_START) && (SIRENE == 0) ){   //so aceita botao se reconheceu a tela
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                EscreverDWINram(DWINadrEtapasBrassagem,7);  //preenche amarelo etapa sparging
                while(BOTAO_START);
                //MinutosEtapa = TempoAdicionarAguaSparging;
                Segundos = 0;
                if(ReceitaAtual.TempoLavagem > 0){
                    //EscreverDWINreg(3,28);  //exibe tela de elevacao automatica do guincho
                    EstadoBrassagem = SPARGINGMANUAL;//ELEVARCESTOAUTOMATICO;
                }
                else{
                    EscreverDWINreg(3,18);  //exibe tela sparging manual
                    EstadoBrassagem = SPARGINGMANUAL;
                }
                AcionaValvulaMotorizada(FECHAR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY4);  //sparging
                AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida boiler
            }
        break;
        case SPARGINGMANUAL:
            if(BOTAO_START){
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
                DelaySeg(1);
                AcionaValvulaMotorizada(FECHAR,MASKY4);  //sparging
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida boiler
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                EscreverDWINreg(3,6);  //exibe tela brassagem
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFervura;
                TempPanelaOK = 0;
                EscreverDWINram(DWINadrEtapasBrassagem,8);  //preenche amarelo etapa fervura
                OnOffControleTemp = 0;
                RESISTENCIAPANELAON;
                RESISTENCIARESERVATORIOOFF;
                //RELESSRON;
                EscreverDWINram(DWINadrIntensidadeFervura,100); //resistencia panela 100%
                AtualizaValoresTela = 1;
                EstadoBrassagem = PREFERVURA;
            }
            break;
        case  ELEVARCESTOAUTOMATICO:
            if(!FIMCURSOGUINCHO){
                SUBIR;
                RELEGUINCHOON;
                for(DelaySobeGuincho=0;DelaySobeGuincho<TEMPOSUBIRAUTOMATICO;DelaySobeGuincho++)
                    __delay_ms(10);
                RELEGUINCHOOFF;
                EstadoBrassagem = SPARGINGAUTOMATICO;
            }
        break;
        case SPARGINGAUTOMATICO:
            temp =  PorcentagemBombaSparging * 255;
            CCPR2L = temp / 100;
            if(NB_BOILER){
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
                //RELESUBIRGUINCHOON;
                EstadoBrassagem = SOBEGUINCHOTOTAL;
            }
        break;
        case SOBEGUINCHOTOTAL:
            //if(FIMCURSOGUINCHO){
                RELEGUINCHOOFF;
                EscreverDWINreg(3,6);  //exibe tela brassagem
                AcionaValvulaMotorizada(FECHAR,MASKY4);  //sparging
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida boiler
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFervura;
                TempPanelaOK = 0;
                EscreverDWINram(DWINadrEtapasBrassagem,8);  //preenche amarelo etapa fervura
                OnOffControleTemp = 0;
                RESISTENCIAPANELAON;
                RESISTENCIARESERVATORIOOFF;
                //RELESSRON;
                EscreverDWINram(DWINadrIntensidadeFervura,100); //resistencia panela 100%
                AtualizaValoresTela = 1;
                EstadoBrassagem = PREFERVURA;
            //}
            break;
         case PREFERVURA:
            PWMpanela = 200;
            EscreverDWINram(DWINadrIntensidadeFervura,100); //aquece a 100% até atingir inicio fervura
            if(ValoresAD[0] >= SetPointTempPanela*10){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFervura;
                EstadoBrassagem = FERVURA;
                PWMpanela = PWMmax;
                EscreverDWINram(DWINadrIntensidadeFervura,100);
                ControlePotenciaFervura = 1;
                OnOffControleTemp = 0;
                //aqui ajustamos o possível erro na previsão mostrada no display de quanto tempo
                //ainda falta para acabar a brassagem
                TotalHorasRegressivo = (ReceitaAtual.TempoFervura+TempoWhirlpool)/60;
                TotalMinutosRegressivo = (ReceitaAtual.TempoFervura+TempoWhirlpool)%60;
            }
            break;
        case FERVURA:
            if(ReceitaAtual.TempoLupulo1 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo1){
                    ReceitaAtual.TempoLupulo1 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENEON;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo2 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo2){
                    ReceitaAtual.TempoLupulo2 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENEON;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo3 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo3){
                    ReceitaAtual.TempoLupulo3 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENEON;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo4 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo4){
                    ReceitaAtual.TempoLupulo4 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENEON;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(Minutos>=MinutosEtapa){
                EscreverDWINreg(3,21);  //exibe tela confirmar fim fervura
                SIRENEON;
                EstadoBrassagem = FINALIZARFERVURA;
            }
        break;
        case ADICAOLUPULO:
            if(LerDWINreg(3) == 6)
            {
                EscreverDWINreg(3,15);  //exibe tela confirmar adição lupulo
                SIRENEOFF;
            }
            //if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKBRASSAGEMPLAY){
            if(BOTAO_START){
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                EscreverDWINram(DWINadrEtapasBrassagem,8);  //preenche amarelo etapa fervura
                EstadoBrassagem = FERVURA;
            }
        break;
        case FINALIZARFERVURA:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,19);  //exibe tela confirmar fim fervura
                SIRENEOFF;
            }
            EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);
            if( (BOTAO_START) && (SIRENE == 0) ){
                Minutos = 0;
                Segundos = 0;
                OnOffControleTemp = 0;
                ControlePotenciaFervura = 0;
                RESISTENCIAPANELAOFF;
                RESISTENCIARESERVATORIOOFF;
                //RELESSROFF;
                EscreverDWINram(DWINadrIntensidadeFervura,0); //mostra resistencia panela desligada
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrEtapasBrassagem,10);  //preenche amarelo etapa whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY6);  //whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada vessel
                CCPR2L = 255;
                EscreverDWINram(DWINadrPorcentagemBomba,100);
                MinutosEtapa = TempoWhirlpool;
                EstadoBrassagem = WHIRLPOOL;
            }
        break;
        case WHIRLPOOL:
            PWMpanela = 0;
            EscreverDWINram(DWINadrPorcentagemBomba,100);   //nao deixa usuario alterar rotação bomba
            if(Minutos >= MinutosEtapa){
                SIRENEON;
                EscreverDWINreg(3,32);  //exibe tela fim do whirlpool
                EstadoBrassagem = FINALIZAWHIRLPOOL;
            }
        break;
        case FINALIZAWHIRLPOOL:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,25);  //exibe tela confirmaçao transfega manual
                SIRENEOFF;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                CCPR2L = 0;
                EstadoBrassagem = CONFIRMATRANSFERIR;
            }
        break;
        case CONFIRMATRANSFERIR:
            if( (BOTAO_START) && (SIRENE == 0) ){
                SIRENEOFF;
                EscreverDWINreg(3,20);  //exibe tela transfega manual
                EscreverDWINram(DWINadrEtapasBrassagem,11);  //preenche amarelo etapa resfriamento
                AcionaValvulaMotorizada(FECHAR,MASKY6);  //whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY7);  //saida trocador
                VALVULAENTRADAAGUATROCADORON;
                TempUInt = 25;
                SegundosControleTrasfega  = 0;
                EstadoBrassagem = TRANSFERENCIA;
            }
        break;
        case TRANSFERENCIA:
            EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);
            EscreverDWINram(DWINadrPorcentagemBomba,TempUInt);
            if(MudancaSegundos){
                SegundosControleTrasfega++;
                MudancaSegundos = 0;
            }
            if(SegundosControleTrasfega >= TCTRLTRASFEGA){
                SegundosControleTrasfega = 0;
                if(ValoresAD[2] > TemperaturaMaxTrasfega*10){
                    if(TempUInt >= 30)
                        TempUInt -= 5;
                }
                if(ValoresAD[2] < TemperaturaMaxTrasfega*10){
                    if(TempUInt <= 90)
                        TempUInt += 5;
                }
            }
            //temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
            //temp *= 255;
            CCPR2L = (TempUInt * 255) / 100;
            if(BOTAO_START){
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                VALVULAENTRADAAGUATROCADOROFF;
                EscreverDWINreg(3,1);  //exibe menu principal
                AcionaValvulaMotorizada(FECHAR,MASKY7);  //saida trocador
                AcionaValvulaMotorizada(FECHAR,MASKY3);  //saida vessel
                while(BOTAO_START);
                EscreverDWINreg(3,1);  //retorna para menu principal, encerrando a brassagem
                //EstadoBrassagem = AGUARDASTART;
            }
        break;
    }
}

/*
 * Esta função aciona manualmente as 8 válvulas, a bomba e as 2 resistencias eletricas
 * Existe uma tela no display DWIN apenas para este modo manual
 */
void ModoManual(void)
{
    static unsigned int temp;
    
    if(FIMCURSOGUINCHO){
        if(!DESCERGUINCHO){
            if(SUBIRGUINCHO){
                SUBIR;
                RELEGUINCHOON;
            }
            else
                RELEGUINCHOOFF;
        }
        if(!SUBIRGUINCHO){
            if(DESCERGUINCHO){
                DESCER;
                RELEGUINCHOON;
            }
            else
                RELEGUINCHOOFF;
        }
    }

    if(RELEGUINCHO) return;

    //atualiza os valores de temperatura da panela, do reservatorio e da saida do trocador
    EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);
    EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
    EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);
    EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);
    //EscreverDWINram(DWINadrVolumeLitros,PressaommCA);

    SetPointTempPanela = LerDWINram(DWINadrSetPointVesselManual);
    TemperaturaSpargingManual = LerDWINram(DWINadrSetPointCentralManual)*10;

    temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
    CCPR2L = temp / 100;

    TempUInt = LerDWINram(DWINadrBitsModoManual);
    if(TempUInt != 0){
        switch(TempUInt){
            //Valvula diafragma que enche o reservatorio de agua de sparging
            case MASKY1:
                if(VALVULAENTRADARESERVATORIO)
                    VALVULAENTRADARESERVATORIOOFF;
                else
                    VALVULAENTRADARESERVATORIOON;
                if(VALVULAENTRADARESERVATORIO)
                    EscreverDWINram(DWINadrIconeY1,1);
                else
                    EscreverDWINram(DWINadrIconeY1,0);
                break;
            //Valvula motorizada de esfera na saida do reservatorio de agua de sparging
            case MASKY2:
                StatusY2 = !StatusY2;
                AcionaValvulaMotorizada(StatusY2,MASKY2);
                if(StatusY2)
                    EscreverDWINram(DWINadrIconeY2,1);
                else
                    EscreverDWINram(DWINadrIconeY2,0);
                break;
            //Valvula motorizada de esfera na saida da panela vessel
            case MASKY3:
                StatusY3 = !StatusY3;
                AcionaValvulaMotorizada(StatusY3,MASKY3);
                if(StatusY3)
                    EscreverDWINram(DWINadrIconeY3,1);
                else
                    EscreverDWINram(DWINadrIconeY3,0);
                break;
            //Valvula motorizada de esfera que injeta a agua de sparging no chuveiro
            case MASKY4:
                StatusY4 = !StatusY4;
                AcionaValvulaMotorizada(StatusY4,MASKY4);
                if(StatusY4)
                    EscreverDWINram(DWINadrIconeY4,1);
                else
                    EscreverDWINram(DWINadrIconeY4,0);
                break;
            //Valvula motorizada de esfera na entrada da panela vessel, injeta agua no fundo do cesto
            case MASKY5:
                StatusY5 = !StatusY5;
                AcionaValvulaMotorizada(StatusY5,MASKY5);
                if(StatusY5)
                    EscreverDWINram(DWINadrIconeY5,1);
                else
                    EscreverDWINram(DWINadrIconeY5,0);
                break;
            //Valvula motorizada de esfera para criar o redemoinho no mosto (whirlpool)
            case MASKY6:
                StatusY6 = !StatusY6;
                AcionaValvulaMotorizada(StatusY6,MASKY6);
                if(StatusY6)
                    EscreverDWINram(DWINadrIconeY6,1);
                else
                    EscreverDWINram(DWINadrIconeY6,0);
                break;
            //Valvula motorizada de esfera de saida do mosto da panela indo para o trocador de calor
            case MASKY7:
                StatusY7 = !StatusY7;
                AcionaValvulaMotorizada(StatusY7,MASKY7);
                if(StatusY7)
                    EscreverDWINram(DWINadrIconeY7,1);
                else
                    EscreverDWINram(DWINadrIconeY7,0);
                break;
            //Valvula diafragma da agua que entra no trocador para resfriar o mosto que está passando
            case MASKY8:
                StatusY8 = !StatusY8;
                AcionaValvulaMotorizada(StatusY8,MASKY8);
                if(StatusY8)
                    EscreverDWINram(DWINadrIconeY8,1);
                else
                    EscreverDWINram(DWINadrIconeY8,0);
                break;
            //Valvula motorizada de esfera de entrada direta de agua na panela
            case MASKY9:
                StatusY9 = !StatusY9;
                AcionaValvulaMotorizada(StatusY9,MASKY9);
                if(StatusY9)
                    EscreverDWINram(DWINadrIconeY9,1);
                else
                    EscreverDWINram(DWINadrIconeY9,0);
                break;
            case MASKBOMBACIRC:
                if(BOMBACIRC)
                    BOMBACIRCOFF;
                else
                    BOMBACIRCON;
                break;
        }
        NOP();
        EscreverDWINram(DWINadrBitsModoManual,0);
        DelayDWIN();
    }
    TempUInt = LerDWINram(DWINadrBitsReceitaBrassagem);
    if(TempUInt != 0){
        NOP();
        switch(TempUInt){
            case MASKRESISTENCIAPANELA:
                if(OnOffControleTemp){
                    OnOffControleTemp = 0;//RESISTENCIAPANELAOFF;
                    RESISTENCIAPANELAOFF;
                    EscreverDWINram(DWINadrIconeResistenciaPanela,0);
                }
                else{
                    OnOffControleTemp = 1;//RESISTENCIAPANELAON;
                    EscreverDWINram(DWINadrIconeResistenciaPanela,1);
                }
                break;
            case MASKRESISTENCIARESERVATORIO:
                if(OnOffControleTempCentral){
                    OnOffControleTempCentral = 0;
                    RESISTENCIARESERVATORIOOFF;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,0);
                }
                else{
                    OnOffControleTempCentral = 1;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,1);
                    
                }
                break;
        }
        EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
        //DelaySeg(1);
    }
}

void main(void){

    enum EstadosMenuPrincipal{MENUPRINCIPAL,BRASSAGEM,EDICAORECEITA,MANUAL,CONFIGURACAO};
    unsigned char EstadoPrincipal;
    unsigned char SegundoAtual;
    unsigned char ReceitaAtual;
    
    //Inicialização de variáveis
    Inicializacao();
    TransmiteSerial = 0;
    fSerialInEnd = 0;
    SegundoAtual = Segundos;
    PWMpanela = PWMmin;
    OnOffControleTemp = 0;
    OnOffControleTempCentral = 0;
    ControlePotenciaFervura = 0;
    PORTD = 0;
    CCPR2L = 0;                                                                 //Bomba (inversor) parada
    EnviaDadoTrend  = 0;
    ReceitaAtual = 1;

    DelaySeg(2);    //tempo exibição tela de abertura

    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);     //limpa bits aqui pra verificar apagar receitas

    //Ligando com o botão de start pressionado zera os valores das receitas
    //e carrega valores default nos parametros (PID,etc...)
    if(BOTAO_START){
        EscreverDWINreg(3,34);  // exibe tela apagar todas as receitas
        while(BOTAO_START);
        while(LerDWINreg(3) == 34){ //aguarda sair da tela (escolher sim ou nao)
            if(LerDWINram(DWINadrBitsReceitaBrassagem) == MASKDELRECEITA){
                ApagaReceitasDaEEPROM();
                KP = 30;
                KI = 30;
                KD = 30;
                TAmostragemPID = 30;
                TonBomba = 20;
                ToffBomba = 2;
                TemperaturaSparging = 78;
                PorcentagemBombaSparging = 50;
                TemperaturaMaxTrasfega = 30;
                TempoWhirlpool = 5;
                IntensidadeFervura = 100;
                GravaParamConfig();
                AtualizaParamNoDisplay();                                       //carrega campos da tela parametros com valores da eeprom
                break;
            }
        }
    }

    //Se o botao de emergência estiver pressionado mostra tela de aviso
    if(BOTAO_EMERGENCIA){
        EscreverDWINreg(3,10);                                                  // exibe emergencia pressionada
        while(BOTAO_EMERGENCIA);                                                //aguarda soltar botão emergencia
    }
  //Para debugar a placa sem a EBsmart esta parte foi comentada
    EscreverDWINreg(3,31);  // solicita pressionar botão para energizar saidas
    while(!BOTAO_START);    // aguarda pressionar o botão

    //Inicia fechando todas as válvulas
    ResetaSaidas();     //desliga todas as saidas e fecha valvulas motorizadas
 
    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
    ReceitaCarregada = 0;
    AtualizaValoresTela = 1;
    DelaySeg(1);    //tempo de energização do inversor de frequencia

    // verifica se inversor está ok
    if(INVERSOROK){
        RESETINVERSORON;
        DelaySeg(5);
        RESETINVERSOROFF;
        if(INVERSOROK){
            EscreverDWINreg(3,30);                                                 // falha inversor
            while(LerDWINreg(3) != 1);                                             //aguarda reconhecer falha na tela
        }
    }

    EstadoPrincipal = MENUPRINCIPAL;
    AtualizaReceitaNoDisplay(ReceitaAtual);                                     //carrega campos da tela receita com valores da receita 1 (valores da eeprom)
    EscreverDWINram(0x25,1);                                                    //seta numero da receita para 1 no display
    LeParamConfig();
    AtualizaParamNoDisplay();                                                   //carrega campos da tela parametros com valores da eeprom
    EscreverDWINreg(3,1);                                                       //exibe menu principal
    Bipar();
    
    while(1){
        NOP();
        if(EstadoPrincipal != BRASSAGEM){
            if(BOTAO_EMERGENCIA){
                EscreverDWINreg(3,10);  // exibe emergencia pressionada
                while(BOTAO_EMERGENCIA);    //aguarda soltar botão emergencia
                EscreverDWINreg(3,31);  // solicita pressionar botão para energizar saidas
                while(!BOTAO_START);
                ResetaSaidas();
                DelaySeg(1);    //tempo de energização do inversor de frequencia
                if(INVERSOROK){
                    RESETINVERSORON;
                    DelaySeg(5);
                    RESETINVERSOROFF;
                        if(INVERSOROK){
                            EscreverDWINreg(3,30);                                                 // falha inversor
                            while(LerDWINreg(3) != 1);
                        }
                }
                EscreverDWINreg(3,1);  //exibe menu principal
                EstadoPrincipal = MENUPRINCIPAL;
            }
        }
        switch(EstadoPrincipal){
            case MENUPRINCIPAL:
                switch(LerDWINreg(03)){
                    case NUMTELABRASSAGEM: if(ReceitaCarregada)
                                                EstadoPrincipal = BRASSAGEM;
                                           else
                                                EscreverDWINreg(3,26);  //exibe tela de erro na receita
                    break;
                    case NUMTELARECEITA: AtualizaReceitaNoDisplay(1);
                                        EstadoPrincipal = EDICAORECEITA;
                                        break;
                    case NUMTELAMANUAL: EstadoPrincipal = MANUAL;break;
                    case NUMTELACONFIGURACAO: EstadoPrincipal = CONFIGURACAO;break;
                }
                break;
            case BRASSAGEM:
                ExecutaBrassagem();
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    ReceitaCarregada = 0;
                    EstadoBrassagem = ENCHERMINRESERVATORIO;//ENCHERRESERVATORIO;
                    EstadoBomba = CENTRAL;
                    ResetaSaidas();
                    EscreverDWINreg(3,1);  //exibe menu principal
                    DelaySeg(2);
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            case EDICAORECEITA:
                if(LerDWINram(0x25) != ReceitaAtual){
                    ReceitaAtual = LerDWINram(0x25);
                    AtualizaReceitaNoDisplay(ReceitaAtual);
                }
                if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKDOWNLOADRECEITA){
                    if(LeDadosReceita()){
                        GravarDadosNaEEPROM(ReceitaAtual);
                    }
                    else
                        EscreverDWINreg(3,27);  //exibe tela de erro na receita
                    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                }
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    if(LeDadosReceita()){
                        ReceitaCarregada = 1;
                        EstadoPrincipal = MENUPRINCIPAL;
                    }
                    else
                        EscreverDWINreg(3,27);  //exibe tela de erro na receita
                }
                break;
            case MANUAL:
                ModoManual();
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    ResetaSaidas();                                           //comentado para testar sem a EBsmart
                    EscreverDWINreg(3,1);  //exibe menu principal
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            case CONFIGURACAO:
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    LeParamConfig();
                    GravaParamConfig();
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
        }
    }
    return;
}
