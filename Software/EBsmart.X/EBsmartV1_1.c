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
 * 1.1 - Retirada do enchimento de volume de sparging automatico, correção
 * de bug modo manual não estava ligando resistencia da central, implementado
 * erro fim curso valvulas motorizadas, criada tela com senha para parametros
 * PID, offset, etc
 *
 * A fazer:
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
//#define KPRESS 100                                                              //valores para converter a leitura analogica
//#define OFFSETPRESS 200                                                         //em mmCA
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
#define GIRAPARA FOUT0
#define GIRAPARAON OUT0ON
#define GIRAPARAOFF OUT0OFF

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
#define TCTRLTRASFEGA 15                                                        //intervalo em seg que corrige rotação da bomba
                                                                                //durante trasfega
#define LIMITEMINERRO -2                                                        //limite erro temperatura
#define LIMITEMAXERRO 2                                                         //ajustado para 0,2oC
#define LIMITEMAXSOMAERRO 50                                                    //usado no PID (parte da integral)
#define LIMITEMINSOMAERRO -50
//Cabeçalho do frame do display DWIN
#define CABECALHOHIGH 0x5A
#define CABECALHOLOW 0xA5
//Definicoes dos enderecos das variaveis no display DWIN
//Mais detalhes sobre o display consultar:www.dwin.com.cn
//consultar aplicação criada com o software DGUS SDK V4.9
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
#define DWINadrEtapasBrassagem1a3 0x31
#define DWINadrEtapasBrassagem4a5 0x66
#define DWINadrEtapasBrassagem6a8 0x67
#define DWINadrEtapasBrassagem9a10 0x68
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
#define DWINadrIconeBomba 0x72
#define DWINadrPorcentagemMinBombaTrafega 0x73
#define DWINadrIconeResistenciaPanela 0x49
#define DWINadrIconeResistenciaReservatorio 0x50
#define DWINadrSetPointTemperatura 0x52
#define DWINadrSetPointVesselManual 0x64
#define DWINadrSetPointCentralManual 0x63
#define DWINadrVolumeLitros 0x65
#define DWINadrSenha 0x69
#define NUMTELAMENUPRINCIPAL 1
#define NUMTELABRASSAGEM 6
#define NUMTELARECEITA 4
#define NUMTELACONFIGURACAO 3
#define NUMTELAMANUAL 2
#define NUMTELAPARAM 36
#define SENHA 2707
#define NUMPARAMCONFIG 16
#define ADRPARAMEEPROM 240
#define TEMPOSUBIRAUTOMATICO 150                                                //tempo x10ms que o guincho sobe para iniciar o sparging
#define ROTACAOMINIMABOMBA 5

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
unsigned char HorasRegressivo;                                                  //e o tempo previsto para concluir
unsigned char SegundosRegressivo;
unsigned char MinutosRegressivo;
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
    unsigned char TempoLupulo5;                                                 //endereco na tela DGUS SDK 0x24
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
unsigned char OffsetPressao;                                                    //correção do transmissor de pressão
unsigned char KPressao;
//maquina de estados das etapas de brassagem
enum EstadosBrassagem{ENCHERMINRESERVATORIO,ENCHERRESERVATORIO,ENCHERMAXRESERVATORIO,CONFIRMACENTRAL,DOSARAGUAPRIMARIA,CICLABOMBA,MASHIN,ADICIONARMALTE,DESCANSOMASHIN,PREFASE1,FASE1,PREFASE2,FASE2,PREFASE3,FASE3,PREFASE4,FASE4,PREFASE5,FASE5,TESTEIODO,PREMASHOUT,MASHOUT,VERIFICATEMPRESERVATORIO,RETIRARCESTO,SPARGINGMANUAL,PREFERVURA,FERVURA,ADICAOLUPULO,FINALIZARFERVURA,WHIRLPOOL,FINALIZAWHIRLPOOL,CONFIRMATRANSFERIR,TRANSFERENCIA};
//maquina de estados da bomba
enum EstadosBomba{CENTRAL,EXTERNO,NIVEL};
unsigned char EstadoBrassagem = ENCHERMINRESERVATORIO;                          //estado inicial da maq. estado da brassagem
unsigned char EstadoBomba = CENTRAL;                                            //estado inicial da maq. estado da bomba
enum EstadosMenuPrincipal{MENUPRINCIPAL,BRASSAGEM,EDICAORECEITA,MANUAL,CONFIGURACAO};
unsigned char EstadoPrincipal;
unsigned char RotacaoBombaAtual;

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
    static unsigned int ValoresADanteriores1[4] = {0,0,0,0};                    //na verdade é feito uma média
    static unsigned int ValoresADanteriores2[4] = {0,0,0,0};                    //das ultimas 4 médias (tamanho definido em uma constante)
    static unsigned int ValoresADanteriores3[4] = {0,0,0,0};                    //para as leituras de temperatura
    static unsigned char CanalAD = 0;                                           //numero do canal A/D atual para leitura
    signed long Erro;                                                           //erro de temperatura (PID)
    static signed long SomaErro = 0;                                            //somatoria do erro de temperatura (PID)
    static signed long ErroAnterior = 0;                                        //erro de temperatura anterior ao calculo presente
    
    /*  interrupcao timer 0 a cada 5ms.
        seta flag que será usada como base de tempo
        no laço principal para verificar o tempo de ciclo
    */
    if(INTCONbits.T0IE && INTCONbits.T0IF){
        TMR0L = 60;
        INTCONbits.T0IF = 0;

//** atualiza entradas e saidas digitais lendo/escrevendo CI's 74541 e 74574 ***
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
//******* calcula porcentagem de acionamento da resistencia da panela **********
        if(OnOffControleTemp){
            if(ContDecSeg >= TAmostragemPID){
                ContDecSeg = 0;
                Erro = (long)SetPointTempPanela*10 - ValoresAD[0];              //resolução erro 0,1oC
                if((Erro >= LIMITEMINERRO) && (Erro <= LIMITEMAXERRO))
                    TempPanelaOK = 1;                                           //indica que temperatura atingiu setpoint
                else
                    TempPanelaOK = 0;                                           //indica que a temperatura está fora dos limites
                SomaErro += Erro;
                if( (SomaErro < LIMITEMINSOMAERRO) || (SomaErro > LIMITEMAXSOMAERRO) )
                    SomaErro = 0;                                               //reinicializa a somatoria de erro de temperatura
//****************** CALCULO DO ERRO DE TEMPERATURA ****************************
                Erro = (long)(Erro*KP)/10 + (long)(SomaErro*KI)/10 + (long)(KD/TAmostragemPID)*(Erro-ErroAnterior);
                ErroAnterior = Erro;                                            //salva valor de erro atual para uso no próximo claculo
                                
                if( (Erro < 0) || (ValoresAD[0]>=SetPointTempPanela*10) )       //temperatura acima do setpoint sempre desliga a resistencia
                    PWMpanela = PWMmin;
                else{
                    if(Erro < PWMmax)                                           //limita o PWM que controla a resistencia a um vaor maximo
                        PWMpanela = Erro;
                    else
                        PWMpanela = PWMmax;
                }
            }
       }
//**************** contador indica um segundo decorrido ************************
       if(FreeRun5ms >= 200){
            Segundos++;                                                         //incrementa contador de segundos
            MudancaSegundos = 1;                                                //usado durante a etapa de trasfega do mosto
            TotalSegundos++;                                                    //totalizador de segundos (uso na tela brassagem)
            FreeRun5ms = 0;                                                     //reinicializa contador para próximo segundo
            if(Segundos > 59){                                                  //verifica de decorreu um minuto
                Segundos = 0;                                                   //reinicializa contador de segundos
                Minutos++;                                                      //usado na determinação do tempo das etapas
            }
            if(AtualizaRegressivo){
                SegundosRegressivo--;
                if(SegundosRegressivo == 0){
                    SegundosRegressivo = 59;
                    if( (HorasRegressivo > 0) || ( (HorasRegressivo == 0) && (MinutosRegressivo > 0) ) )
                        MinutosRegressivo--;
                    if( (HorasRegressivo > 0) && (MinutosRegressivo == 0)){                                    //contadores usados para mostrar
                        MinutosRegressivo = 59;                                    //na tela de brassagem uma estimativa de quanto
                        if(HorasRegressivo > 0)                                    //falta para terminar a brassagem
                            HorasRegressivo--;
                    }
                }
            }
            if(TotalSegundos > 59){                                             //um minuto do totalizador decorrido
                TotalSegundos = 0;                                              //reiniciliza contador de segundos do totalizador
                MinutosBomba++;                                                 //usado no ciclo on/off da bomba de mosto
                EnviaDadoTrend = 1;                                             //flag para indicar momento de envio da temperatura para o grafico de tendencia no display DWIN
                TotalMinutos++;                                                 //totalizador de minutos desde o inicio da brassagem, para mostra na tela
            }
            if(TotalMinutos > 59){                                              //totalizador de minutos atingiu uma hora
                TotalMinutos = 0;                                               //reinicializa
                TotalHoras++;                                                   //e incrementa totalizador de horas
            }
        }
//***************** acionamento da resistencia da panela ***********************
        if(OnOffControleTemp){                                                  //verifica se controle está ligado
            if( (PWMpanela > PWMmin) && (ValoresAD[0]<SetPointTempPanela*10) ){ //só liga a resistencia se temparatura menor que setpoint e PWM maior que o mínimo (calculo PID)
                if(FreeRun5ms < PWMpanela)                                      //usa contador de 5ms para criar um PWM
                    RESISTENCIAPANELAON;
                else
                    RESISTENCIAPANELAOFF;
            }
            else
                RESISTENCIAPANELAOFF;                                           //temperatura > setpoint ou PWM=min, desliga resistencia
        }
        
        if(ControlePotenciaFervura){                                            //durante a fervura não precisa desligar quanto ao limite de setpoint
            if(PWMpanela > PWMmin){                                             //é apenas controle direto do PWM
                if(FreeRun5ms < PWMpanela)
                    RESISTENCIAPANELAON;
                else
                    RESISTENCIAPANELAOFF;
            }
            else
                RESISTENCIAPANELAOFF;
        }
//***************** acionamento da resistencia da central **********************
        /*  Para o controle de temperatura da central de água quente é apenas
         *  um controle on/off simples
         */
        if(OnOffControleTempCentral){
            if((ValoresAD[1] <= TemperaturaSpargingManual*10) && BOMBACIRC && (!NB_BOILER) )
                RESISTENCIARESERVATORIOON;
            else
                RESISTENCIARESERVATORIOOFF;
        }
    }

//************** Conversao de 4 sinais analógicos ******************************
    //Tempo de atualização: n.o canais(4) * 15ms * n.o amostras
    //há uma repetiçao a cada 5ms dos estados: setar canal, habilitar conversão, ler resultado da conversão
    //apenas qdo atinge o numero de amostras da media há um acrescimo de 5ms
    switch(EstadoAD){
        case SETACANAL:
            ADCON0 &= 0b11000011;                                               //limpa bits selecao canal
            if(CanalAD < 3)
                ADCON0 |= CanalAD << 2;                                         //seta canal analogico
            else
                ADCON0 |= (CanalAD+1) << 2;                                     //seta canal pulando canal +vref
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
                CanalAD++;                                                      //passa para o próximo canal A/D
                if(CanalAD > 3){                                                //converteu os 4 canais
                    CanalAD = 0;                                                //retorna para primeiro canal
                    ContMedia++;                                                //incrementa contador amostra
                }
                EstadoAD = SETACANAL;
                if(ContMedia == MEDIA_AD)                                       //atingiu amostras para tirar media?
                    EstadoAD = CALCULAMEDIA;                                    //se sim, vai para o estado de calcular a media
            }
            break;
        case CALCULAMEDIA:
            for(Temp8=0;Temp8<4;Temp8++){                                       //calcula a media dos 4 canais
                ValoresAD[Temp8] = AccValoresAD[Temp8] / MEDIA_AD;
                AccValoresAD[Temp8] = 0;                                        //reinicializa contador para prox. media
                if(Temp8<3){                                                    //para leituras de temperatura tira média da média
                    switch(Temp8){                                              //a leitura de pressao é apenas média simples
                        case 0: ValoresAD[0] += OffsetAD0;break;                //corrigi erro de offset dos sensores
                        case 1: ValoresAD[1] += OffsetAD1;break;                //de temperatura
                        case 2: ValoresAD[2] += OffsetAD2;break;                //valores ajustados atarvés do display DWIN
                        default:break;
                    }
                    ValoresAD[Temp8] += ValoresADanteriores1[Temp8];            //faz um media das 4 medias anteriores
                    ValoresAD[Temp8] += ValoresADanteriores2[Temp8];
                    ValoresAD[Temp8] += ValoresADanteriores3[Temp8];
                    ValoresAD[Temp8] /= 4;
                    ValoresADanteriores3[Temp8] = ValoresADanteriores2[Temp8];  //descarta valores da média mais antiga
                    ValoresADanteriores2[Temp8] = ValoresADanteriores1[Temp8];
                    ValoresADanteriores1[Temp8] = ValoresAD[Temp8];             //salva o valor mais atual da media para uso no próximo ciclo
                }
                
            }
            ContMedia = 0;                                                      //reinicia contador de amostra
            PressaommCA = ValoresAD[3];                                         //transmissor de pressão ligado ao quarto canal
            if(PressaommCA >= OffsetPressao){                                   //o projeto de hardware já garante que o resultado
                PressaommCA -= OffsetPressao;                                   //da conversão A/D estará em mmCA
                PressaommCA *= KPressao;                                        //aqui é apenas para pequenas correções
                PressaommCA /= 100;
            }
            else
                PressaommCA = 0;                                                //pressao menor que offset (considera que offset sempre positivo, pois o sensor está abaixo do fundo da panela)
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

/*******************************************************************************
 * Espera de n segundos
 ******************************************************************************/
void DelaySeg(unsigned char tempo)
{
    unsigned int cont;

    do{
        for(cont=0;cont<1000;cont++)                                            //loop com duração de 1seg
            __delay_ms(1);
        tempo--;
    }while(tempo>0);                                                            //fica em loop por n segundos
}

/*******************************************************************************
 * atraso em milisegundos para aguardar tempo de processamento do display DWIN
 ******************************************************************************/
void DelayDWIN(void){
    unsigned char cont;

    for(cont=0;cont<DELAYDWIN;cont++)                                           //fica em loop por n milisegundos
         __delay_ms(1);
}

/******************************************************************************
 * Inicialização do microcontrolador
 *
 * Configuração dos pinos como entrada ou saída
 * Configuração dos periféricos internos do micro
 * Configuração das interrupções e suas prioridades (alta ou baixa)
 ******************************************************************************/
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
    
    //habiltaçao das interrupções
    RCONbits.IPEN = 1;                                                          //habilita 2 niveis de prioridade para as interrupções
    INTCONbits.GIEH = 1;                                                        //habilita interrupções com prioridade alta
    INTCONbits.GIEL = 1;                                                        //habilita interrupções com prioridade baixa
}

/******************************************************************************
 * Escrever na memória RAM do display DWIN                                    *
 ******************************************************************************/
void EscreverDWINram(unsigned int endereco,int valor){

    unsigned char cont;

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 5;                                                           //bytes msg + endereco(2bytes) + valor(2bytes)
    SerialOut[3] = 0x82;                                                        //indica escrita na RAM
    SerialOut[4] = endereco >> 8;                                               //consultar documentação do display
    SerialOut[5] = endereco & 0x00FF;                                           //para maiores detalhes
    SerialOut[6] = valor >> 8;
    SerialOut[7] = valor & 0x00FF;
    PIR1bits.TXIF = 0;
    for(cont=0;cont<8;cont++){                                                  //loop para enviar todos os caracteres da matriz
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);                                                  //aguarda módulo do microcontrolador indicar que enviou o caracter
        PIR1bits.TXIF = 0;
    }
    DelayDWIN();                                                                //delay para display processar informações
}

/******************************************************************************
 * Escrever em um registrador do display DWIN                                 *
 ******************************************************************************/
void EscreverDWINreg(unsigned char endereco,int valor){

    unsigned char cont;

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;                                                           //bytes msg + endereco(1byte) + valor(2bytes)
    SerialOut[3] = 0x80;                                                        //indica escrita no registrador
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
}

/******************************************************************************
 * Realiza leitura em um endereço da memoria RAM do display DWIN              *
 ******************************************************************************/
int LerDWINram(unsigned int endereco)
{
    unsigned char cont;
    unsigned int VPrecebido;

    //envia comando para o display solicitando dados
    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;                                                           //bytes msg + endereco(2bytes) + tamanho(1byte)
    SerialOut[3] = 0x83;                                                        //indica leitura da RAM do display DWIN
    SerialOut[4] = endereco >> 8;
    SerialOut[5] = endereco & 0x00FF;
    SerialOut[6] = 1;                                                           //esta função só lê 1 palavra (word)
    PIR1bits.TXIF = 0;
    for(cont=0;cont<7;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    PIR1bits.RCIF = 0;                                                          //limpa flag que indica caracter recebido pela serial

    //recebe os dados do display
    for(cont=0;cont<9;cont++){
        while(!PIR1bits.RCIF);
        PIR1bits.RCIF = 0;
        SerialIn[cont] = RCREG;
    }
    VPrecebido = SerialIn[7] << 8;                                              //nos interessa apenas os 2 ultimos bytes
    VPrecebido += SerialIn[8];
    
    DelayDWIN();                                                                //delay para display processar informações
    return(VPrecebido);
}

/******************************************************************************
 * Realiza leitura em um registrador do display DWIN                          *
 ******************************************************************************/
int LerDWINreg(unsigned char endereco)
{
    unsigned char cont;
    unsigned char recebido;

    //envia comando para o display solicitando dados
    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 3;                                                           //bytes msg (1byte) + endereco(1byte) + n.o reg a ler(1byte)
    SerialOut[3] = 0x81;                                                        //indica leitura registrador do display DWIN
    SerialOut[4] = endereco;
    SerialOut[5] = 2;                                                           //esta função só lê 1 palavra (word)
    PIR1bits.TXIF = 0;
    for(cont=0;cont<6;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    PIR1bits.RCIF = 0;                                                          //limpa flag que indica caracter recebido pela serial

    //recebe os dados do display
    for(cont=0;cont<8;cont++){
        while(!PIR1bits.RCIF);
        PIR1bits.RCIF = 0;
        SerialIn[cont] = RCREG;
    }
    recebido = SerialIn[6] >> 8;
    recebido += SerialIn[7];

    DelayDWIN();                                                                //delay para display processar informações
    return(recebido);
}

/******************************************************************************
 * Escreve temperatura no grafico de tendencia - recurso do display DWIN      *
 ******************************************************************************/
void EscreverDWINtrend(int valor){

    unsigned char cont;

    SerialOut[0] = CABECALHOHIGH;
    SerialOut[1] = CABECALHOLOW;
    SerialOut[2] = 4;                                                           //bytes msg + canal 0 + valor
    SerialOut[3] = 0x84;                                                        //indica escrita no registrador
    SerialOut[4] = 1;                                                           //canal 0, ver doc. DWIN
    SerialOut[5] = valor >> 8;
    SerialOut[6] = valor & 0x00FF;
    PIR1bits.TXIF = 0;
    for(cont=0;cont<7;cont++){
        TXREG = SerialOut[cont];
        while(!PIR1bits.TXIF);
        PIR1bits.TXIF = 0;
    }
    DelayDWIN();                                                                //delay para display processar informações
}

/******************************************************************************
 *  Leitura dos valores de configuracao do equipamento
 *  que podem ser editados na tela do display DWIN especifica para isto
 ******************************************************************************/
void LeParamConfig(void){
    KP = LerDWINram(0x27);                                                      //constante proporcional do controle PID da temp. panela
    KI = LerDWINram(0x28);                                                      //constante integral do controle PID da temp. panela
    KD = LerDWINram(0x29);                                                      //constante derivativa do controle PID da temp. panela
    TAmostragemPID = LerDWINram(0x2A);                                          //tempo de amostragem do calculo PID
    TonBomba = LerDWINram(0x57);                                                //tempo ligado no ciclo da bomba de mosto
    ToffBomba = LerDWINram(0x58);                                               //tempo desligado no ciclo da bomba de mosto
    TemperaturaSparging = LerDWINram(0x2C);                                     //temperatura da agua da central
    PorcentagemBombaSparging = LerDWINram(0x2D);                                //vazao da bomba na lavagem dos grãos
    TemperaturaMaxTrasfega = LerDWINram(0x55);                                  //temperatura maxima de saida do mosto no trocador
    TempoWhirlpool = LerDWINram(0x56);                                          //tempo de whirlpool
    IntensidadeFervura = LerDWINram(0x2B);                                      //intensidade da fervura (1 a 100%)
    OffsetAD0 = LerDWINram(0x2E);                                               //correção de leitura do sensor de temperatura da panela
    OffsetAD1 = LerDWINram(0x2F);                                               //correção de leitura do sensor de temperatura da central
    OffsetAD2 = LerDWINram(0x30);                                               //correção de leitura do sensor de temperatura da saida do trocador
    OffsetPressao = LerDWINram(0x70);
    KPressao = LerDWINram(0x71);
}

/******************************************************************************
 * Grava os valores dos parametros na memoria EEPROM do microcontrolador      *
 ******************************************************************************/
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
            case 14: eeprom_write(ADRPARAMEEPROM+utemp8,OffsetPressao);break;
            case 15: eeprom_write(ADRPARAMEEPROM+utemp8,KPressao);break;
        }
        __delay_ms(10);
    }
    
}

/******************************************************************************
 * Envia para o display DWIN os valores dos parametros
 * salvos na memoria EEPROM do microcontrolador
 ******************************************************************************/
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
            case 14: EscreverDWINram(0x70,eeprom_read(ADRPARAMEEPROM+utemp8));break;
            case 15: EscreverDWINram(0x71,eeprom_read(ADRPARAMEEPROM+utemp8));break;
        }
    }
    //limpa (zera) cronometros na tela de brassagem
    EscreverDWINram(DWINadrMinutosParaFim,0);
    EscreverDWINram(DWINadrSegundosParaFim,0);
    EscreverDWINram(DWINadrMinutosDoInicio,0);
    EscreverDWINram(DWINadrSegundosDoInicio,0);
}

/******************************************************************************
 * Apaga área entre 0 e 227
 * da memoria EEPROM do microcontrolador
 * Cada receita ocupa 38bytes, então cabem 6 receitas, para deixar espaço para
 * os parametros
 ******************************************************************************/
void ApagaReceitasDaEEPROM()
{
    NOP();
    unsigned char utemp8;

    for(utemp8 = 0;utemp8 < 228;utemp8++)
    {
        eeprom_write(utemp8,0);
        __delay_ms(10);
    }
}

/******************************************************************************
 *  Leitura dos valores da receita armazanados no display touch
 *  Retorna 1 se dados tiverem consistencia ou 0 se houver algum problema com os valores
 *  Por exemplo: 1a temperatura da rampa < temperatura mash in
 ******************************************************************************/
unsigned char LeDadosReceita(void)
{
    unsigned char utemp8;
    unsigned int buffer;

    //faixa de endereços dos valores na tela de receita do display DWIN
    //está entre 0x10 e 0x25
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
            case 0x24: ReceitaAtual.TempoLupulo5 = LerDWINram(utemp8);break;
            case 0x25: ReceitaAtual.TempoMashIn = LerDWINram(0x51);             //tempo Mash In foi colocado depois
        }
    }

    //conversâo do volume para altura de agua primaria no vessel
    mmAguaPrimaria = (long)ReceitaAtual.VolumeInicial*10000;
    mmAguaPrimaria /= 6082;

    //leitura do nome da receita
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
    if( (ReceitaAtual.TempoLupulo1 == 0) && ( (ReceitaAtual.TempoLupulo2 != 0) || (ReceitaAtual.TempoLupulo3 != 0) || (ReceitaAtual.TempoLupulo4 != 0) || (ReceitaAtual.TempoLupulo5 != 0) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo2 != 0) && (ReceitaAtual.TempoLupulo2 > ReceitaAtual.TempoLupulo1) )
        return(0);
    if( (ReceitaAtual.TempoLupulo2 == 0) && ( (ReceitaAtual.TempoLupulo3 != 0) || (ReceitaAtual.TempoLupulo4 != 0) || (ReceitaAtual.TempoLupulo5 != 0) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo3 != 0) && (ReceitaAtual.TempoLupulo3 > ReceitaAtual.TempoLupulo2) )
        return(0);
    if( (ReceitaAtual.TempoLupulo3 == 0) && ( (ReceitaAtual.TempoLupulo4 != 0) || (ReceitaAtual.TempoLupulo4 != 0) ) )
        return(0);
    if( (ReceitaAtual.TempoLupulo4 != 0) && (ReceitaAtual.TempoLupulo4 > ReceitaAtual.TempoLupulo3) )
        return(0);
    if( (ReceitaAtual.TempoLupulo4 == 0) && (ReceitaAtual.TempoLupulo5 != 0)  )
        return(0);
    if( (ReceitaAtual.TempoLupulo5 != 0) && (ReceitaAtual.TempoLupulo5 > ReceitaAtual.TempoLupulo4) )
        return(0);

    return(1);
}

/******************************************************************************
 *  Grava dados de uma receita na memoria EEPROM do microcontrolador
 ******************************************************************************/
void GravarDadosNaEEPROM(unsigned char receita)
{
    unsigned char cont,temp;

    receita--;
    if(receita>0)
        receita *= 38;                                                          //cada receita ocupa 38 bytes

    //grava valores da receita na memoria EEPROM do microcontrolador
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
            case 20: temp = ReceitaAtual.TempoLupulo5;break;
            case 21: temp = ReceitaAtual.TempoMashIn;break;
        }
        eeprom_write((receita+cont),temp);
        NOP();
        __delay_ms(10);
    }
    //grava nome da receita na EEPROM
    for(cont=22;cont<38;cont++){
        temp = ReceitaAtual.Nome[cont-22];
        eeprom_write((receita+cont),temp);
    }
    NOP();
}

/******************************************************************************
 *  Envia dados de uma receita na memoria EEPROM do microcontrolador
 *  para a tela do display DWIN
 ******************************************************************************/
void AtualizaReceitaNoDisplay(unsigned char receita){

    unsigned char cont,contdest,temp;
    unsigned int buffer;

    receita--;                                                                  //na tela do display não existe receita zero
    if(receita>0)
        receita *= 38;                                                          //cada receita ocupa 38bytes na EEPROM
    contdest = 0;
    //escreve valores da receita na tela do display DWIN
    for(cont=receita;cont<receita+22;cont++){
        temp = eeprom_read(cont);
        if(contdest<21)
            EscreverDWINram(contdest+0x10,temp);                                //faixa de endereço dos valores da receita
        if(contdest == 21)                                                      //vai de 0x10 a 0x24 mais o 0x51
            EscreverDWINram(0x51,temp);                                         //fora da sequencia pq foi inserido depois
        contdest++;
    }
    contdest = 0;
    //escreve o nome da receita na tela do display DWIN
    for(cont=receita+22;cont<receita+38;cont+=2){
        buffer = eeprom_read(cont);
        buffer = buffer << 8;
        buffer|= eeprom_read(cont+1);
        EscreverDWINram(contdest+0x100,buffer);                                 //endereço do nome na tela começa em 0x100
        contdest++;
    }
}

/******************************************************************************
 * Abre/fecha as valvulas motorizadas
 * Retorna
 ******************************************************************************/
unsigned char AcionaValvulaMotorizada(unsigned char sentido,unsigned char valvula){

    unsigned char Timeout,NumValvulaErro,TelaAtual;

    if(sentido == ABRIR)                                                        //rele inversor existente na placa
        SENTIDOVALVULA = 0;                                                     //determina a polaridade da tensão 12Vcc
    else                                                                        //que energizará a respectiva válvula
        SENTIDOVALVULA = 1;

    DelayDWIN();                                                                //delay entre a comutação do rele de sentido
                                                                                //e o rele da vávula

    NumValvulaErro = 0;                                                                //zero indica que não houve erro ao acionar a valvula
    //seleciona qual das válvulas será energizada
    //apenas uma valvula pode ser energizada por vez
    switch(valvula){
        case MASKY2:VALVULASAIDARESERVATORIOON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){                                      //tempo para valv. acionar é de 4seg
                            NumValvulaErro = 2;                                 //indica erro no acionamento da valvula
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULASAIDARESERVATORIOFECHADA)) || ((sentido == ABRIR) && (VALVULASAIDARESERVATORIOABERTA)) );
                    break;
        case MASKY3:VALVULASAIDAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 3;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULASAIDAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULASAIDAPANELAABERTA)) );
                    break;
        case MASKY4:VALVULALAVAGEMGRAOSON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 4;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULALAVAGEMGRAOSFECHADA)) || ((sentido == ABRIR) && (VALVULALAVAGEMGRAOSABERTA)) );
                    break;
        case MASKY5:VALVULAENTRADAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 5;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAPANELAABERTA)) );
                    break;
        case MASKY6:VALVULAWHIRLPOOLON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 6;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULAWHIRLPOOLFECHADA)) || ((sentido == ABRIR) && (VALVULAWHIRLPOOLABERTA)) );
                    break;
        case MASKY7:VALVULAENTRADAMOSTOTROCADORON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 7;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAMOSTOTROCADORFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAMOSTOTROCADORABERTA)) );
                    break;
        case MASKY8:VALVULAENTRADAAGUATROCADORON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        NOP();
                        if(Timeout > 200){
                            NumValvulaErro = 8;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAAGUATROCADORFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAAGUATROCADORABERTA)) );
                    break;
        case MASKY9:VALVULAENTRADAAGUAPANELAON;
                    Timeout = 0;
                    do{
                        Timeout++;
                        __delay_ms(20);
                        if(Timeout > 200){
                            NumValvulaErro = 9;
                            break;
                        }
                    }while( ((sentido == FECHAR) && (VALVULAENTRADAAGUAPANELAFECHADA)) || ((sentido == ABRIR) && (VALVULAENTRADAAGUAPANELAABERTA)) );
                    break;
    }
    //desliga rele da respectiva valvula
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
    if(NumValvulaErro != 0){
        TelaAtual = LerDWINreg(03);                                             //salva numero tela atual
        NOP();
        EscreverDWINreg(03,37);                                                 //mostra tela reconhecimento erro valvula motorizada
        SIRENEON;
        EscreverDWINram(0x72,NumValvulaErro);                                   //mostra o numero da valvula que falhou
        while(!BOTAO_START);                                                    //aguarda pressionar botao reconhecendo erro
        SIRENEOFF;
        EscreverDWINreg(03,TelaAtual);                                          //retorna para tela anterior ao erro
        return(0);
    }
    else
        return(1);                                                              //valvula acionada com sucesso
}

/******************************************************************************
 * Aciona buzzer por 1 seg
 ******************************************************************************/
void Bipar(void)
{
    SIRENEON;
    DelaySeg(1);
    SIRENEOFF;
}

/******************************************************************************
 * Desliga todas as saidas de hardware, fecha valvulas motorizadas
 ******************************************************************************/
void ResetaSaidas(void)
{
    EscreverDWINreg(3,29);                                                      //exibe tela de fechamento das valvulas
    OnOffControleTemp = 0;                                                      //desabilita PID dentro da interrupção
    ControlePotenciaFervura = 0;
    OnOffControleTempCentral = 0;
    CCPR2L = 0;                                                                 //zera saida 0-10V
    GIRAPARAOFF;
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
    EscreverDWINram(DWINadrIconeBomba,0);
    EscreverDWINram(DWINadrIconeResistenciaPanela,0);
    EscreverDWINram(DWINadrIconeResistenciaReservatorio,0);
    EscreverDWINram(DWINadrPorcentagemBomba,0);
}

/******************************************************************************
 * Verifica se pressionou botão de emergencia ou
 * houve falha no inversor de frequencia
 * retorna 0 se ok e 1 se falha
 ******************************************************************************/
unsigned char VerificaFalha(void)
{
    //verifica se pressionou emergencia
    if(BOTAO_EMERGENCIA){
        EscreverDWINreg(3,10);                                              //exibe emergencia pressionada
        SIRENEON;
        while(BOTAO_EMERGENCIA);                                            //aguarda soltar botão emergencia
        SIRENEOFF;
        ResetaSaidas();
        EstadoBrassagem = ENCHERMINRESERVATORIO;
        EstadoBomba = CENTRAL;
        EscreverDWINreg(3,31);                                              //solicita pressionar botão para energizar saidas
        while(!BOTAO_START);
        DelaySeg(1);                                                        //tempo de energização do inversor de frequencia
        if(INVERSOROK){
            RESETINVERSORON;                                                //sinal para resetar erro do inversor
            DelaySeg(5);
            RESETINVERSOROFF;
            if(INVERSOROK){
                EscreverDWINreg(3,30);                                      //exibe tela falha inversor
                while(LerDWINreg(3) != 1);
            }
        }
        EscreverDWINreg(3,1);                                               //exibe menu principal
        EstadoPrincipal = MENUPRINCIPAL;
        return(1);
    }
    //verifica falha inversor
    if(INVERSOROK){
        ResetaSaidas();
        EscreverDWINreg(3,30);                                              //falha inversor
        SIRENEON;
        while(LerDWINreg(3) != 1);                                          //aguarda reconhecer falha
        SIRENEOFF;
        while(INVERSOROK);                                                  //maquina travada enquanto falha no inversor
        EscreverDWINreg(3,1);                                               //exibe menu principal
        EstadoBrassagem = ENCHERMINRESERVATORIO;
        EstadoBomba = CENTRAL;
        EstadoPrincipal = MENUPRINCIPAL;
        return(1);
    }
    return(0);
}

/******************************************************************************
 * Realiza a brassagem de forma automatica, orientando o operador qdo
 * necessário
 ******************************************************************************/
void ExecutaBrassagem(void)
{
    unsigned int temp;
    unsigned char MinutosEtapa;
    unsigned char SegundosControleTrasfega;

    //verifica se pressionou emergencia ou falha no inversor
    if(VerificaFalha())
        return;
    
    if(AtualizaValoresTela){                                                    //flag que controla se valores serão atualizados no display DWIN durante a brassagem
        EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);                  //temperarura da panela
        EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);           //temperatura da central de agua quente (boiler)
        EscreverDWINram(DWINadrSetPointTemperatura,SetPointTempPanela);         //setpoint atual de temperatura, conforme receita
        EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);           //volume de agua (ou mosto) dentro da panela
    
        if((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= WHIRLPOOL)){
            EscreverDWINram(DWINadrMinutosDoInicio,TotalHoras);                 //atualiza cronometro totalizador do tempo
            EscreverDWINram(DWINadrSegundosDoInicio,TotalMinutos);              //desde o inicio da brassagem
            if(AtualizaRegressivo){                                             //flag para cronometro regressivo do tempo que falta para concluir a etapa
                EscreverDWINram(DWINadrMinutosParaFim,HorasRegressivo);
                EscreverDWINram(DWINadrSegundosParaFim,MinutosRegressivo);
            }
            else
            {
                EscreverDWINram(DWINadrMinutosParaFim,0);                       //se flag desligado, zera cronometro regressivo
                EscreverDWINram(DWINadrSegundosParaFim,0);
            }
        }
    }

    //controle da rotação da bomba, saida 0-10V
    if( ((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= MASHOUT) && (EstadoBomba == CENTRAL)) || (EstadoBrassagem == SPARGINGMANUAL) || ((EstadoBrassagem >= PREFERVURA) && (EstadoBrassagem <= FINALIZARFERVURA)) ){
        temp = LerDWINram(DWINadrPorcentagemBomba);                             //permite operador alterar o valor da rotação
        if(temp >= ROTACAOMINIMABOMBA){                                         //verifica se valor acima do minimo
            temp = temp * 255;
            CCPR2L = temp / 100;
            GIRAPARAON;                                                         //liga saida inversor gira para
        }
        else                                                                    //operador ajustou um valor abaixo do minimo
        {
            GIRAPARAOFF;                                                        //para a bomba e zera tensão
            CCPR2L = 0;
        }
    }

    if( (EstadoBrassagem == FERVURA) || (EstadoBrassagem == FINALIZARFERVURA) ){//durante a fervura permite ao operador
        PWMpanela = LerDWINram(DWINadrIntensidadeFervura)*2 - 1;                //ajustar a potencia da resistencia da panela
    }
    else{
        EscreverDWINram(DWINadrIntensidadeFervura,PWMpanela/2);
    }

    //de tempos em tempos desliga a bomba, para decantar cama de grãos
    //enquanto isso, circula mosto fora do cesto pelo whirlpool para
    //homogeinização
    if((EstadoBrassagem >= PREFASE1) && (EstadoBrassagem <= PREMASHOUT)){
        switch(EstadoBomba){
            case CENTRAL:
                if(MinutosBomba >= TonBomba){                                   //verifica tempo bomba ligada
                    RotacaoBombaAtual = LerDWINram(DWINadrPorcentagemBomba);    //salva valor da rotação da bomba que está na tela
                    MinutosBomba = 0;
                    CCPR2L = 0;                                                 //desliga bomba pra equalizar nivel fora cesto
                    EscreverDWINram(DWINadrPorcentagemBomba,0);
                    GIRAPARAOFF;
                    EstadoBomba = NIVEL;
                }
                break;
            case EXTERNO:
                EscreverDWINram(DWINadrPorcentagemBomba,33);                    //não deixa operador alterar rotação bomba principal
                if(MinutosBomba >= ToffBomba){                                  //verifica tempo bomba desligada
                    AcionaValvulaMotorizada(ABRIR,MASKY5);                      //entrada do vessel
                    AcionaValvulaMotorizada(FECHAR,MASKY6);                     //whirlpool
                    MinutosBomba = 0;
                    EscreverDWINram(DWINadrPorcentagemBomba,RotacaoBombaAtual); //volta para a tela o valor antes de circular mosto por fora
                    temp = RotacaoBombaAtual * 255;
                    CCPR2L = temp / 100;
                    EstadoBomba = CENTRAL;
                }
                break;
            case NIVEL:                                                         //bomba fica desligada por um minuto para equalizar nivel mosto dentro e fora do cesto
                EscreverDWINram(DWINadrPorcentagemBomba,0);                     //não deixa operador alterar rotação bomba principal
                if(MinutosBomba >= 1){
                    AcionaValvulaMotorizada(ABRIR,MASKY6);                      //whirlpool
                    AcionaValvulaMotorizada(FECHAR,MASKY5);                     //entrada do vessel
                    CCPR2L = 85;                                                //bomba a 33%
                    EscreverDWINram(DWINadrPorcentagemBomba,33);
                    GIRAPARAON;
                    MinutosBomba = 0;
                    EstadoBomba = EXTERNO;
                }
                break;
        }
    }

    //envia dados da temperatura para mostrar o chart da temperatura durante toda a brassagem
    if(EnviaDadoTrend){
        EnviaDadoTrend = 0;                                                     //limpa flag para proximo envio
        if((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= WHIRLPOOL)){
            if(ValoresAD[0]>400)                                                //começa a enviar valores apenas qdo temperatura ultrapassar 40oC
                EscreverDWINtrend(ValoresAD[0]);
        }
    }

    //bomba da central de agua quente fica ligada entre o momento em que há nivel minimo dentro da central
    //até o momento de iniciar a lavagem
    if( (EstadoBrassagem > ENCHERMINRESERVATORIO) && (EstadoBrassagem <= VERIFICATEMPRESERVATORIO) )
        BOMBACIRCON;
    else
        BOMBACIRCOFF;

    //a temperatura da central é controlada do modo simples, on/off
    //pela própria inercia do sistema não há determinação nem controle de histerese
    if( (EstadoBrassagem >= DOSARAGUAPRIMARIA) && (EstadoBrassagem <= VERIFICATEMPRESERVATORIO) ){
        if(ValoresAD[1] < TemperaturaSparging*10)                               //temperatura da central de agua quente (boiler)
            RESISTENCIARESERVATORIOON;
        else
            RESISTENCIARESERVATORIOOFF;
    }
    else{
        RESISTENCIARESERVATORIOOFF;
    }

    /***************************************************************************
     * INICIO MAQUINA DE ESTADOS CONTROLE BRASSAGEM AUTOMATICA
     **************************************************************************/
    switch(EstadoBrassagem){
        //enche até altura dos furos de tubulações do reservatório
        case ENCHERMINRESERVATORIO:
            VALVULAENTRADARESERVATORIOON;
            if(!NB_BOILER)                                                      //verifica se sensor nivel minimo central (boiler) está acionado
                EstadoBrassagem = ENCHERRESERVATORIO;
            break;
        //verifica status nivel agua na central de agua quente (boiler)
        case ENCHERRESERVATORIO:
            EscreverDWINram(DWINadrPorcentagemBomba,0);                         //bomba parada
            EscreverDWINram(DWINadrEtapasBrassagem1a3,1);                       //preenche amarelo etapa encher com agua
            EscreverDWINram(DWINadrEtapasBrassagem4a5,0);  
            EscreverDWINram(DWINadrEtapasBrassagem6a8,0);  
            EscreverDWINram(DWINadrEtapasBrassagem9a10,0);  
            SetPointTempPanela = 0;
            if(!NA_BOILER)                                                      //se não tem nivel maximo, passa para
                EstadoBrassagem = ENCHERMAXRESERVATORIO;                        //encher central
            else
            {
                if(PressaommCA <= mmAguaPrimaria)                               //verifica se já tem agua na panela
                    AcionaValvulaMotorizada(ABRIR,MASKY9);                      //entrada agua vessel
                EscreverDWINreg(3,35);                                          //exibe tela confirmar aquecer sparging
                SIRENEON;
                EstadoBrassagem = CONFIRMACENTRAL;
            }
            break;
        //enche cenral agua quente até atingir nivel maximo
        case ENCHERMAXRESERVATORIO:
            VALVULAENTRADARESERVATORIOON;                                       //Aciona valvula para comecar a encher o reservatorio
            if(NA_BOILER){                                                      //verifica se atingiu nivel maximo central (boiler)
                VALVULAENTRADARESERVATORIOOFF;
                AcionaValvulaMotorizada(ABRIR,MASKY9);                          //entrada agua vessel
                EscreverDWINreg(3,35);                                          //exibe tela confirmar aquecer sparging
                SIRENEON;
                EstadoBrassagem = CONFIRMACENTRAL;
            }
            break;
        //aguarda operador confirmar que pode ligar a resistencia da central
        //de agua quente (boiler) esta segurança é devido a um problema de
        //posicionamento da bomba na primeira EB250 construida
        //a resistencia é ligada no condicional antes da maquina de estados
        case CONFIRMACENTRAL:
            if(BOTAO_START){                                                    //botão usuario pressionado?
                SIRENEOFF;
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                EstadoBrassagem = DOSARAGUAPRIMARIA;
            }
        break;
        //enche a panela com agua até nivel programado na receita
        case DOSARAGUAPRIMARIA:
            if(PressaommCA >= mmAguaPrimaria){                                  //verifica se atingiu nivel programado
                AcionaValvulaMotorizada(FECHAR,MASKY9);                         //entrada agua vessel
                AcionaValvulaMotorizada(ABRIR,MASKY3);                          //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);                          //entrada do vessel
                EstadoBrassagem = CICLABOMBA;
            }
            NOP();
            break;
        //cicla bomba principal para retirar bolhas ar do circuito
        case CICLABOMBA:    
            for(temp=0;temp<CICLOSBOMBA;temp++){
                //verifica se pressionou emergencia ou saiu da brassagem
                //para não ficar preso no laço até concluir ciclos
                if( (!BOTAO_EMERGENCIA) && (LerDWINreg(03) == NUMTELAMENUPRINCIPAL) ) break;
                GIRAPARAON;                                                     //liga sinal gira do inversor
                CCPR2L = 255;
                EscreverDWINram(DWINadrPorcentagemBomba,100);                   //bomba a 100%
                DelaySeg(2);
                CCPR2L = 25;
                EscreverDWINram(DWINadrPorcentagemBomba,10);                    //bomba a 10%
                DelaySeg(2);
            }
            EscreverDWINram(DWINadrEtapasBrassagem1a3,2);                       //elevar temperatura antes malte
            EscreverDWINram(DWINadrEtapasBrassagem4a5,0);  
            EscreverDWINram(DWINadrEtapasBrassagem6a8,0);  
            EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
            CCPR2L = 85;
            EscreverDWINram(DWINadrPorcentagemBomba,33);                        //bomba a 33%
            RESISTENCIAPANELAON;                                                //liga resistencia da panela
            OnOffControleTemp = 1;                                              //liga controle PID
            Minutos = 0;
            Segundos = 0;
            SetPointTempPanela = ReceitaAtual.TemperaturaMashIn;                //carrega o primeiro setpoint (mash in)
            TempPanelaOK = 0;
            Segundos = 0;
            TotalHoras = 0;
            TotalSegundos = 0;
            TotalMinutos = 0;
            EstadoBrassagem = MASHIN;
            break;
        //aguarda atingir temperatura mash in (1a rampa)
        case MASHIN:
            NOP();
            if(TempPanelaOK){                                                   //verifica se atingiu setpoint
                Bipar();
                Minutos = 0;
                Segundos = 0;
                EscreverDWINram(DWINadrEtapasBrassagem1a3,3);                   //preenche amarelo adicionar malte
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,0);  
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);  
                EscreverDWINram(DWINadrPorcentagemBomba,0);                     //isso já para a bomba principal
                EscreverDWINreg(3,7);                                           //exibe tela para adicionar malte
                SIRENEON;
                EstadoBrassagem = ADICIONARMALTE;
            }
        break;
        case ADICIONARMALTE:
            if(LerDWINreg(3) == 6){                                             //voltou para tela de brassagem?
                GIRAPARAOFF;                                                    //ou seja, operador reconheceu alerta de malte?
                CCPR2L = 0;                                                     //bomba parada
                OnOffControleTemp = 0;                                          //como nao havera circulaçao, desliga resistência
                RESISTENCIAPANELAOFF;
                EscreverDWINreg(3,13);                                          //exibe tela confirmar adicao malte
                SIRENEOFF;                                                      //desliga sirene
            }
            if( (BOTAO_START) && (!SIRENE) ){                                   //so aceita botão se já reconheceu a tela, ou seja, desligou sirene
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoMashIn;                        //carrega tempo mash in (1o patamar)
                HorasRegressivo = MinutosEtapa/60;                              //carrega cronometro regressivo com
                MinutosRegressivo = MinutosEtapa%60;                            //tempo da etapa
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                RESISTENCIAPANELAON;
                OnOffControleTemp = 1;                                          //liga novamente a resistencia da panela
                TempPanelaOK = 0;
                GIRAPARAON;
                CCPR2L = 64;
                EscreverDWINram(DWINadrPorcentagemBomba,25);                    //bomba a 25%
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,1);                   //preenche amarelo etapa rampas temperatura
                EscreverDWINram(DWINadrEtapasBrassagem6a8,0);
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                EstadoBrassagem = DESCANSOMASHIN;
             }
        break;
        //aguarda tempo de mash in
        case DESCANSOMASHIN:
            if(Minutos>=MinutosEtapa){                                          //verifica se já decorreu tempo de mash in
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase1;             //carrega segundo setpoint (2o patamar)
                TempPanelaOK = 0;
                MinutosBomba = 0;
                AtualizaRegressivo = 0;
                EstadoBrassagem = PREFASE1;
            }
            break;
        //aguarda atingir temperatura setpoint
        case PREFASE1:
            if(TempPanelaOK){                                                   //atingiu setpoint?
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase1;                         //carrega tempo fase 1 (2o patamar)
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = FASE1;
            }
        break;
        //aguarda tempo fase 1 (2o patamar)
        case FASE1:
            if(Minutos>=MinutosEtapa){                                          //verifica se já decorreu tempo fase 1
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase2;             //carrega setpoint fase 2
                TempPanelaOK = 0;
                AtualizaRegressivo = 0;
                EstadoBrassagem = PREFASE2;
            }
        break;
        //aguarda atingir temperatura setponit 3o patamar
        case PREFASE2:
            if(ReceitaAtual.TempoFase2 == 0){                                   //se tempo igual a zero, não há mais
                SIRENEON;                                                       //patamares de mosturação, pula para teste iodo
                EscreverDWINreg(3,16);                                          //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
                break;
            }
            if(TempPanelaOK){                                                   //atingiu setpoint?
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase2;
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = FASE2;
            }
            break;
        //aguarda tempo fase 2 (3o patamar)
        case FASE2:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase3;
                TempPanelaOK = 0;
                AtualizaRegressivo = 0;
                EstadoBrassagem = PREFASE3;
            }
        break;
        case PREFASE3:
            if(ReceitaAtual.TempoFase3 == 0){
                SIRENEON;
                EscreverDWINreg(3,16);                                          //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase3;
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = FASE3;
            }
            break;
        //aguarda tempo fase 3 (4o patamar)
        case FASE3:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase4;
                TempPanelaOK = 0;
                EstadoBrassagem = PREFASE4;
                AtualizaRegressivo = 0;
            }
        break;
        case PREFASE4:
            if(ReceitaAtual.TempoFase4 == 0){
                SIRENEON;
                EscreverDWINreg(3,16);  //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
                break;
            }
            if(TempPanelaOK){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFase4;
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = FASE4;
            }
            break;
        //aguarda tempo fase 4 (5o patamar)
        case FASE4:
            if(Minutos>=MinutosEtapa){
                Bipar();
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFase5;
                TempPanelaOK = 0;
                AtualizaRegressivo = 0;
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
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = FASE5;
            }
            break;
        //aguarda tempo fase 5 (6o patamar)
        case FASE5:
            if(Minutos>=MinutosEtapa){
                SIRENEON;
                EscreverDWINreg(3,16);  //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
                AtualizaRegressivo = 0;
            }
        break;
        //avisa operador sobre a necessidade de fazer teste iodo
        case TESTEIODO:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,17);                                          //exibe tela confirmar MASH OUT
                SIRENEOFF;
            }
            if( (BOTAO_START) && (SIRENE == 0) ){                               //so aceita botão se já reconheceu a tela, ou seja, desligou sirene
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                SetPointTempPanela = ReceitaAtual.TemperaturaMashOut;           //carrega setpoint mash out
                Minutos = 0;
                Segundos = 0;
                TempPanelaOK = 0;
                EstadoBrassagem = PREMASHOUT;
            }
        break;
        //aguarda atingir temperatura de mash out
        case PREMASHOUT:
            if(TempPanelaOK){                                                   //verifica se atingiu a temperatura
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoMashOut;                       //carrega tempo de mash out
                HorasRegressivo = MinutosEtapa/60;
                MinutosRegressivo = MinutosEtapa%60;
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                AcionaValvulaMotorizada(ABRIR,MASKY5);                          //entrada do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY6);                         //whirlpool
                CCPR2L = 85;
                EscreverDWINram(DWINadrPorcentagemBomba,33);                    //bomba a 33%
                EstadoBomba = CENTRAL;
                EstadoBrassagem = MASHOUT;
            }
            break;
        //aguarda tempo de mash out (7o patamar de temperatura)
        case MASHOUT:
            if(Minutos>=MinutosEtapa){
                Minutos = 0;
                Segundos = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,2);                   //preenche amarelo etapa retirar cesto
                EscreverDWINram(DWINadrEtapasBrassagem6a8,0);
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                GIRAPARAOFF;
                CCPR2L = 0;                                                     //desliga bomba
                AcionaValvulaMotorizada(ABRIR,MASKY3);                          //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);                          //entrada do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY6);                         //whirlpool
                OnOffControleTemp = 0;
                RESISTENCIAPANELAOFF;                                           //alt. em 27set16, passou a desligar a resistencia, pois se operador demora pra retirar o cesto, dá problema
                RESISTENCIARESERVATORIOOFF;
                EstadoBrassagem = VERIFICATEMPRESERVATORIO;
            }
        break;
        //verifica se a temperatura da agua da central (boiler) atingiu setponit da receita
        //pressionar botão de usuario tambem avança para próximo estado
        case VERIFICATEMPRESERVATORIO:
            if( (ValoresAD[1] >= (TemperaturaSparging-2)*10) || BOTAO_START){
                EscreverDWINreg(3,9);                                           //exibe tela alerta para operador elevar cesto
                SIRENEON;
                EstadoBrassagem = RETIRARCESTO;
            }    
            else
                EscreverDWINreg(3,33);                                          //exibe tela aguardar terminar aquecer sparging
        break;
        //aguarda operador confirmar retirada do cesto
        case RETIRARCESTO:
            if(LerDWINreg(3) == 6){                                             //alerta reconhecido
                EscreverDWINreg(3,14);                                          //exibe tela confirmar retirada cesto
                SIRENEOFF;
            }
            
            if( (BOTAO_START) && (SIRENE == 0) ){                               //so aceita botao se reconheceu a tela
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,1);                   //preenche amarelo etapa sparging
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                while(BOTAO_START);                                             //aguarda soltar botao para não dar problema adiante
                Segundos = 0;
                EscreverDWINreg(3,18);                                          //exibe tela sparging manual
                AcionaValvulaMotorizada(FECHAR,MASKY3);                         //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY5);                         //entrada do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY4);                          //sparging
                AcionaValvulaMotorizada(ABRIR,MASKY2);                          //saida boiler
                EstadoBrassagem = SPARGINGMANUAL;
            }
        break;
        //neste estado operador irá controlar manualmente a rotação da bomba
        //para o sparging (lavagem dos grãos com a água da central)
        case SPARGINGMANUAL:
            if(BOTAO_START){                                                    //operador pressionou botão para encerrar sparging
                GIRAPARAOFF;                                                    //e passar para próximo estado
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);                     //bomba parada
                DelaySeg(1);
                AcionaValvulaMotorizada(FECHAR,MASKY4);                         //sparging
                AcionaValvulaMotorizada(FECHAR,MASKY2);                         //saida boiler
                AcionaValvulaMotorizada(ABRIR,MASKY3);                          //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);                          //entrada do vessel
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFervura;           //carrega temperatura de fervura
                TempPanelaOK = 0;
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,2);                   //preenche amarelo etapa fervura
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                OnOffControleTemp = 0;
                RESISTENCIAPANELAON;
                RESISTENCIARESERVATORIOOFF;
                PWMpanela = PWMmax;
                EscreverDWINram(DWINadrIntensidadeFervura,100);                 //resistencia panela 100%
                AtualizaValoresTela = 1;
                EstadoBrassagem = PREFERVURA;
            }
            break;
         //aguarda atingir setponit fervura
         //o operador deve programar o setponit de fervura para próximo do ponto
         //de ebulição no local, mas um pouco abaixo, por ex. 94oC
         case PREFERVURA:
            if(ValoresAD[0] >= SetPointTempPanela*10){                          //atingiu temperatura setponit fervura
                Bipar();
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoFervura;                       //carrega tempo de fervura
                HorasRegressivo = MinutosEtapa/60;                              //carrega cronometro regressivo
                MinutosRegressivo = MinutosEtapa%60;                            //com o tempo da etapa
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                PWMpanela = PWMmax;
                EscreverDWINram(DWINadrIntensidadeFervura,100);
                ControlePotenciaFervura = 1;                                    //para permitir que o operador ajuste a potencia
                OnOffControleTemp = 0;                                          //de fervura apos atingir o setpoint
                EstadoBrassagem = FERVURA;
            }
            break;
        //aguarda o tempo de fervura, enquanto alerta operador sobre a
        //adição de lúpulos
        case FERVURA:
            if(ReceitaAtual.TempoLupulo1 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo1){        //tempo de adição do 1o lúpulo?
                    ReceitaAtual.TempoLupulo1 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                    EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                    EscreverDWINram(DWINadrEtapasBrassagem6a8,3);               //preenche amarelo etapa adicao lupulo
                    EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                    EscreverDWINreg(3,8);                                       //exibe tela para adicionar lupulo
                    SIRENEON;                                                   //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo2 != 0){                                 //tempo de adição do 2o lúpulo?
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo2){
                    ReceitaAtual.TempoLupulo2 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                    EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                    EscreverDWINram(DWINadrEtapasBrassagem6a8,3);               //preenche amarelo etapa adicao lupulo
                    EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                    EscreverDWINreg(3,8);                                       //exibe tela para adicionar lupulo
                    SIRENEON;                                                   //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo3 != 0){                                 //tempo de adição do 3o lúpulo?
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo3){
                    ReceitaAtual.TempoLupulo3 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                    EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                    EscreverDWINram(DWINadrEtapasBrassagem6a8,3);               //preenche amarelo etapa adicao lupulo
                    EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                    EscreverDWINreg(3,8);                                       //exibe tela para adicionar lupulo
                    SIRENEON;                                                   //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo4 != 0){                                 //tempo de adição do 4o lúpulo?
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo4){
                    ReceitaAtual.TempoLupulo4 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                    EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                    EscreverDWINram(DWINadrEtapasBrassagem6a8,3);               //preenche amarelo etapa adicao lupulo
                    EscreverDWINram(DWINadrEtapasBrassagem9a10,0);

                    EscreverDWINreg(3,8);                                       //exibe tela para adicionar lupulo
                    SIRENEON;                                                   //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo5 != 0){                                 //tempo de adição do 5o lúpulo?
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo5){
                    ReceitaAtual.TempoLupulo5 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                    EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                    EscreverDWINram(DWINadrEtapasBrassagem6a8,3);               //preenche amarelo etapa adicao lupulo
                    EscreverDWINram(DWINadrEtapasBrassagem9a10,0);

                    EscreverDWINreg(3,8);                                       //exibe tela para adicionar lupulo
                    SIRENEON;                                                   //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            //verifica se decorreu o tempo de fervura
            if(Minutos>=MinutosEtapa){
                EscreverDWINreg(3,21);                                          //exibe tela confirmar fim fervura
                SIRENEON;
                AtualizaRegressivo = 0;
                EstadoBrassagem = FINALIZARFERVURA;
            }
        break;
        //aguarda operador confirmar adição de lúpulo
        case ADICAOLUPULO:
            if(LerDWINreg(3) == 6)
            {
                EscreverDWINreg(3,15);                                          //exibe tela confirmar adição lupulo
                SIRENEOFF;
            }
            if( (BOTAO_START) && (SIRENE == 0) ){
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,2);                   //preenche amarelo etapa fervura
                EscreverDWINram(DWINadrEtapasBrassagem9a10,0);
                EstadoBrassagem = FERVURA;                                      //retorna para estado de fervura
            }
        break;
        //aguarda operador confirmar que quer encerrar a fervura
        case FINALIZARFERVURA:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,19);                                          //exibe tela confirmar fim fervura
                SIRENEOFF;
            }
            EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);       //mostra o volume de mosto na panela
            if( (BOTAO_START) && (SIRENE == 0) ){                               //operador pressionou botão encerrando a fervura
                Minutos = 0;
                Segundos = 0;
                OnOffControleTemp = 0;
                ControlePotenciaFervura = 0;
                RESISTENCIAPANELAOFF;
                RESISTENCIARESERVATORIOOFF;
                EscreverDWINram(DWINadrIntensidadeFervura,0);                   //mostra resistencia panela desligada
                EscreverDWINreg(3,6);                                           //exibe tela brassagem
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,0);
                EscreverDWINram(DWINadrEtapasBrassagem9a10,1);                  //preenche amarelo etapa whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY6);                          //whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY3);                          //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY5);                         //entrada vessel
                GIRAPARAON;
                CCPR2L = 255;
                EscreverDWINram(DWINadrPorcentagemBomba,100);                   //bomba principal na rotação máxima
                MinutosEtapa = TempoWhirlpool;                                  //para o whirlpool
                HorasRegressivo = MinutosEtapa/60;                              //carrega cronometro regressivo
                MinutosRegressivo = MinutosEtapa%60;                            //com tempo da etapa
                SegundosRegressivo = 59;
                AtualizaRegressivo = 1;
                EstadoBrassagem = WHIRLPOOL;
            }
        break;
        //aguarda tempo de whirlpool
        case WHIRLPOOL:
            PWMpanela = 0;
            EscreverDWINram(DWINadrPorcentagemBomba,100);                       //nao deixa usuario alterar rotação bomba
            if(Minutos >= MinutosEtapa){
                SIRENEON;
                EscreverDWINreg(3,32);                                          //exibe tela fim do whirlpool
                AtualizaRegressivo = 0;
                EstadoBrassagem = FINALIZAWHIRLPOOL;
            }
        break;
        //aguarda operador confirmar que quer encerrar o whirlpool
        case FINALIZAWHIRLPOOL:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,25);                                          //exibe tela confirmaçao transfega manual
                SIRENEOFF;
                EscreverDWINram(DWINadrPorcentagemBomba,0);                     //bomba parada
                CCPR2L = 0;
                GIRAPARAOFF;
                AcionaValvulaMotorizada(FECHAR,MASKY6);                         //whirlpool
                EstadoBrassagem = CONFIRMATRANSFERIR;
            }
        break;
        //aguarda operador confirmar que quer começar a trasfega do mosto para o fermentador
        case CONFIRMATRANSFERIR:
            if( (BOTAO_START) && (SIRENE == 0) ){
                SIRENEOFF;
                EscreverDWINreg(3,20);                                          //exibe tela transfega manual
                EscreverDWINram(DWINadrEtapasBrassagem1a3,0);
                EscreverDWINram(DWINadrEtapasBrassagem4a5,0);
                EscreverDWINram(DWINadrEtapasBrassagem6a8,0);
                EscreverDWINram(DWINadrEtapasBrassagem9a10,2);                  //preenche amarelo etapa resfriamento
                AcionaValvulaMotorizada(ABRIR,MASKY8);                          //entrada agua fria trocador
                AcionaValvulaMotorizada(ABRIR,MASKY3);                          //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY7);                          //saida trocador
                TempUInt = 25;                                                  //inicia trasfega com bomba rotação a 25%
                GIRAPARAON;
                SegundosControleTrasfega  = 0;
                EscreverDWINram(DWINadrPorcentagemMinBombaTrafega,25);          //inicia minimo transferencia em 25%
                EstadoBrassagem = TRANSFERENCIA;
            }
        break;
        /*  trasfega automatica do mosto para o fermentador
            a rotaçao da bomba é controlada automaticamente para que a
            temperatura na saída do trocador não ultrapasse o valor do parametro
            mas isso pode ocorrer caso a água para resfriamento não esteja fria o bastante
         daí a bomba vai ficar na rotação mínima
         */
        case TRANSFERENCIA:
            EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);              //exibe temperatura de saida do trocador de calor
            EscreverDWINram(DWINadrPorcentagemBomba,TempUInt);                  //exibe rotação da bomba %
            if(RotacaoBombaAtual != LerDWINram(DWINadrPorcentagemMinBombaTrafega)){        //permite ajuste manual da rotacao
                RotacaoBombaAtual = LerDWINram(DWINadrPorcentagemMinBombaTrafega);        //minima da bomba
                TempUInt = RotacaoBombaAtual;                                   //passa rotação atual para valor ajustado
                                                                                //p.ex. caso ao iniciar com 25% naõ esteja transferindo
            }
            if(MudancaSegundos){
                SegundosControleTrasfega++;
                MudancaSegundos = 0;
            }
            if(SegundosControleTrasfega >= TCTRLTRASFEGA){                      //dado um intervalo de tempo, é analisada
                SegundosControleTrasfega = 0;                                   //a temperatura na saida do trocador
                if(ValoresAD[2] > TemperaturaMaxTrasfega*10){                   //caso a temperatura esteja acima do permitido
                    if(TempUInt > RotacaoBombaAtual)                            //diminui a rotação em passos de 1%, minimo 20%
                        TempUInt -= 1;
                }
                if(ValoresAD[2] < TemperaturaMaxTrasfega*10){                   //se a temperatura estiver abaixo do permitido
                    if(TempUInt < 75)                                           //aumenta a rotação em passos de 1%, maximo 75%
                        TempUInt += 1;
                }
            }
            CCPR2L = (TempUInt * 255) / 100;                                    //ajusta rotação da bomba (tensão 0-10V)
            if(BOTAO_START){                                                    //operador encerrou a brassagem, fim da trasfega
                CCPR2L = 0;
                GIRAPARAOFF;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                AcionaValvulaMotorizada(FECHAR,MASKY8);                         //entrada agua fria trocador
                AcionaValvulaMotorizada(FECHAR,MASKY7);                         //saida trocador
                AcionaValvulaMotorizada(FECHAR,MASKY3);                         //saida vessel
                while(BOTAO_START);                                             //aguarda soltar o botão
                EscreverDWINreg(3,1);                                           //exibe menu principal
            }
        break;
    }
}

/******************************************************************************
 * Esta função aciona manualmente as 8 válvulas, a bomba e as 2 resistencias eletricas
 * Existe uma tela no display DWIN apenas para este modo manual
 ******************************************************************************/
void ModoManual(void)
{
    static unsigned int temp;
    
    //atualiza os valores de temperatura da panela, do reservatorio,da saida do trocador e do volume da panela
    EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);
    EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
    EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);
    EscreverDWINram(DWINadrVolumeLitros,PressaommCA*100/KVOLUME);

    //setpoint manual da temperatura da panela
    SetPointTempPanela = LerDWINram(DWINadrSetPointVesselManual);
    //setpoint manual da temperatura da central de agua quente (boiler)
    TemperaturaSpargingManual = LerDWINram(DWINadrSetPointCentralManual);
    //ajuste da rotação da bomba principal
    temp = LerDWINram(DWINadrPorcentagemBomba);
    if(temp >= ROTACAOMINIMABOMBA){
        temp = temp * 255;
        CCPR2L = temp / 100;                                                    //saida 0-10Vcc
        GIRAPARAON;                                                             //liga sinal gira para do inversor
    }
    else
    {
        CCPR2L = 0;
        GIRAPARAOFF;                                                            //desliga saida gira para do inversor
    }

    //lê bits de acionamento (variavel display DWIN) das valvulas
    TempUInt = LerDWINram(DWINadrBitsModoManual);
    if(TempUInt != 0){
        switch(TempUInt){
            //Valvula diafragma que enche o reservatorio de agua de sparging
            case MASKY1:
                if(VALVULAENTRADARESERVATORIO)
                    VALVULAENTRADARESERVATORIOOFF;                              //valvula solenoide
                else
                    VALVULAENTRADARESERVATORIOON;
                if(VALVULAENTRADARESERVATORIO)
                    EscreverDWINram(DWINadrIconeY1,1);                          //muda cor icone na tela para verde
                else
                    EscreverDWINram(DWINadrIconeY1,0);                          //muda cor icone na tela para vermelho
                break;
            //Valvula motorizada de esfera na saida do reservatorio de agua de sparging
            case MASKY2:
                StatusY2 = !StatusY2;
                AcionaValvulaMotorizada(StatusY2,MASKY2);                       //valvula motorizada
                if(StatusY2)
                    EscreverDWINram(DWINadrIconeY2,1);                          //muda cor icone na tela para verde
                else
                    EscreverDWINram(DWINadrIconeY2,0);                          //muda cor icone na tela para vermelho
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
            //bomba da central de agua quente (boiler)
            case MASKBOMBACIRC:
                if(BOMBACIRC){
                    EscreverDWINram(DWINadrIconeBomba,0);
                    BOMBACIRCOFF;
                }
                else{
                    EscreverDWINram(DWINadrIconeBomba,1);
                    BOMBACIRCON;
                }
                break;
        }
        NOP();
        //limpa bits da variavel no display DWIN para permitir novas mudanças de estado
        EscreverDWINram(DWINadrBitsModoManual,0);
        DelayDWIN();
    }
    //Le variavel display DWIN para controle das resistencias
    TempUInt = LerDWINram(DWINadrBitsReceitaBrassagem);
    if(TempUInt != 0){
        NOP();
        switch(TempUInt){
            case MASKRESISTENCIAPANELA:
                if(OnOffControleTemp){
                    OnOffControleTemp = 0;
                    RESISTENCIAPANELAOFF;
                    EscreverDWINram(DWINadrIconeResistenciaPanela,0);           //muda cor do icone para cinza
                }
                else{
                    OnOffControleTemp = 1;                                      //liga resistencia e controle PID da panela
                    EscreverDWINram(DWINadrIconeResistenciaPanela,1);           //muda cor do icone para vermelho
                }
                break;
            case MASKRESISTENCIARESERVATORIO:
                if(OnOffControleTempCentral){
                    OnOffControleTempCentral = 0;
                    RESISTENCIARESERVATORIOOFF;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,0);     //muda cor do icone para cinza
                }
                else{
                    OnOffControleTempCentral = 1;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,1);     //muda cor do icone para vermelho
                }
                break;
        }
        //limpa bits da variavel no display DWIN para permitir novas mudanças de estado
        EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
    }
}

/******************************************************************************
 *
 *              FUNÇÃO MAIN, PONTO DE ENTRADA DO PROGRAMA
 *
 ******************************************************************************/
void main(void){

    unsigned char SegundoAtual;
    unsigned char ReceitaAtual;
    
    //Inicialização de variáveis
    Inicializacao();                                                            //configuração do microcontrolador
    TransmiteSerial = 0;
    fSerialInEnd = 0;
    SegundoAtual = Segundos;
    PWMpanela = PWMmin;
    OnOffControleTemp = 0;
    OnOffControleTempCentral = 0;
    ControlePotenciaFervura = 0;
    PORTD = 0;
    CCPR2L = 0;                                                                 //Bomba (inversor) parada saida 0-10Vcc
    EnviaDadoTrend  = 0;
    ReceitaAtual = 1;

    DelaySeg(2);                                                                //tempo exibição tela de abertura

    EscreverDWINram(DWINadrBitsModoManual,0);                                   //limpa bits modo manual
    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);                             //limpa bits aqui pra verificar apagar receitas

    //Ligando com o botão de start pressionado zera os valores das receitas
    //e carrega valores default nos parametros (PID,etc...)
    if(BOTAO_START){
        EscreverDWINreg(3,34);                                                  // exibe tela apagar todas as receitas
        while(BOTAO_START);
        while(LerDWINreg(3) == 34){                                             //aguarda sair da tela (escolher sim ou nao)
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

    EscreverDWINreg(3,31);                                                      // solicita pressionar botão para energizar saidas
    while(!BOTAO_START);                                                        // aguarda pressionar o botão

    //Inicia fechando todas as válvulas
    ResetaSaidas();                                                             //desliga todas as saidas e fecha valvulas motorizadas
    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
    ReceitaCarregada = 0;
    AtualizaValoresTela = 1;
    DelaySeg(1);                                                                //tempo de energização do inversor de frequencia

    // verifica se inversor está ok
    if(INVERSOROK){
        RESETINVERSORON;                                                        //liga rele para resetar inversor por hardware
        DelaySeg(5);
        RESETINVERSOROFF;
        if(INVERSOROK){
            EscreverDWINreg(3,30);                                              // falha inversor
            while(LerDWINreg(3) != 1);                                          //aguarda reconhecer falha na tela
        }
    }

    EstadoPrincipal = MENUPRINCIPAL;                                            //reseta maquina de estado principal
    AtualizaReceitaNoDisplay(ReceitaAtual);                                     //carrega campos da tela receita com valores da receita 1 (valores da eeprom)
    EscreverDWINram(0x25,1);                                                    //seta numero da receita para 1 no display
    LeParamConfig();                                                            //lê parametros de configuração (PID, etc)
    AtualizaParamNoDisplay();                                                   //carrega campos da tela parametros com valores da eeprom
    EscreverDWINreg(3,1);                                                       //exibe menu principal
    Bipar();
    EscreverDWINram(DWINadrSenha,0);                                            //limpa senha da memoria

    /***************************************************************************
     *                  LACO PRINCIPAL DO PROGRAMA
     **************************************************************************/
    while(1){
        NOP();
        VerificaFalha();                                                        //verifica se pressionou emerg. ou falha no inversor
        /***********************************************************************
        *                MAQUINA DE ESTADOS PRINCIPAL DO PROGRAMA
        ************************************************************************/
        switch(EstadoPrincipal){
            //tela de menu principal no display DWIN
            case MENUPRINCIPAL:
                switch(LerDWINreg(03)){                                         //verifica para que tela passou no display DWIN
                    //tela de brassagem
                    case NUMTELABRASSAGEM: if(ReceitaCarregada)                 //flag para verificar se já existe uma receita carregada
                                                EstadoPrincipal = BRASSAGEM;
                                           else
                                                EscreverDWINreg(3,26);          //exibe tela de erro na receita
                    break;
                    //tela de receita
                    case NUMTELARECEITA: AtualizaReceitaNoDisplay(1);
                                        EstadoPrincipal = EDICAORECEITA;
                                        break;
                    //tela modo manual
                    case NUMTELAMANUAL: EstadoPrincipal = MANUAL;break;
                    //tela configuração parametros
                    case NUMTELACONFIGURACAO: EscreverDWINram(DWINadrSenha,0);
                                        EstadoPrincipal = CONFIGURACAO;break;
                }
                break;
            //execução de brassagem em modo automatico
            case BRASSAGEM:
                ExecutaBrassagem();                                             //funcao que executa brassagem automatica
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){                     //operador decidiu encerrar a brassagem no meio
                    ReceitaCarregada = 0;
                    EstadoBrassagem = ENCHERMINRESERVATORIO;
                    EstadoBomba = CENTRAL;
                    ResetaSaidas();
                    EscreverDWINreg(3,1);                                       //exibe menu principal
                    DelaySeg(2);
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            //edição de receita
            case EDICAORECEITA:
                if(LerDWINram(0x25) != ReceitaAtual){                           //operador está mudando n.o receita na tela
                    ReceitaAtual = LerDWINram(0x25);                            //atualiza numero da receita atual
                    AtualizaReceitaNoDisplay(ReceitaAtual);                     //atualiza na tela os valores para a receita atual
                }
                if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKDOWNLOADRECEITA){ //pressionado icone para salvar receita
                    if(LeDadosReceita()){                                       //verifica se dados estão consistentes
                        GravarDadosNaEEPROM(ReceitaAtual);                      //salva dados da receita na EEPROM do microcontrolador
                    }
                    else                                                        //dados da receita inconsistentes
                        EscreverDWINreg(3,27);                                  //exibe tela de erro na receita
                    EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                }
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){                     //saiu da edição da receita sem salvar
                    if(LeDadosReceita()){                                       //verifica se dados estão consistentes
                        ReceitaCarregada = 1;
                        EstadoPrincipal = MENUPRINCIPAL;
                    }
                    else                                                        //dados da receita inconsistentes
                        EscreverDWINreg(3,27);                                  //exibe tela de erro na receita
                }
                break;
            //operação em modo manual
            case MANUAL:
                ModoManual();                                                   //funcao para operação em modo manual
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){
                    ResetaSaidas();                                             //ao sair do manual, desliga todas as saidas e fecha valvulas motorizadas
                    EscreverDWINreg(3,1);                                       //exibe menu principal
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                break;
            //configuração da EBsmart, parametros de trabalho, offset de sensores, etc
            case CONFIGURACAO:
                if(LerDWINreg(03) == NUMTELAMENUPRINCIPAL){                     //voltou para menu principal
                    LeParamConfig();                                            //salva os parametros
                    GravaParamConfig();                                         //na memoria EEPROM do microcontrolador
                    EstadoPrincipal = MENUPRINCIPAL;
                }
                //parametros protegidos
                if(LerDWINram(DWINadrSenha) == SENHA){                          //lê senha de administrador
                      EscreverDWINram(DWINadrSenha,0);                          //volta a desabilitar senha
                      EscreverDWINreg(3,NUMTELAPARAM);                          //vai para tela de parametros protegidos
                }
                break;
        }
    }
    return;
}
