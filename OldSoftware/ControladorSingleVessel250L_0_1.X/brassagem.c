/*******************************************************************************
 * www.neuheitbier.com.br
 * Funçao de execução da brassagem automatica panela cervejeira smart vessel
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

extern unsigned int ValoresAD[4];
unsigned char SetPointTempPanela;
extern unsigned char TotalHoras;
extern unsigned char TotalMinutos;
extern unsigned char TotalSegundos;
extern unsigned char PWMpanela;
extern unsigned char TonBomba,ToffBomba;           //1 a 99 minutos
extern unsigned char MinutosBomba;
extern bit EnviaDadoTrend;
unsigned char CanalADAtivo;                 //99 = todos canais
extern unsigned int TempoEncherVessel;
extern unsigned int FreeRunSegundos;
extern unsigned char Minutos;
extern unsigned char Segundos;
extern bit OnOffControleTemp;
extern bit TempPanelaOK;
extern struct ValoresReceita{
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
extern struct ValoresReceita ReceitaAtual;
extern unsigned char TempoAdicionarAguaSparging;   //5 a 60 segundos
extern unsigned char TemperaturaSparging;          //60 a 90
extern bit ControlePotenciaFervura;

extern void EscreverDWINram(unsigned int endereco,int valor);
extern void EscreverDWINreg(unsigned char endereco,int valor);
extern int LerDWINram(unsigned int endereco);
extern int LerDWINreg(unsigned char endereco);
extern void EscreverDWINtrend(int valor);
extern void AcionaValvulaMotorizada(unsigned char sentido,unsigned char valvula);
extern void DelaySeg(unsigned char tempo);

void ExecutaBrassagem(void)
{
    enum EstadosBrassagem{AGUARDASTART,MODOENCHIMENTO,ENCHERBOILERAGUAPRIMARIA,DOSARAGUAPRIMARIA,CICLABOMBA,MASHIN,ADICIONARMALTE,DESCANSOMASHIN,PREFASE1,FASE1,PREFASE2,FASE2,PREFASE3,FASE3,PREFASE4,FASE4,PREFASE5,FASE5,TESTEIODO,PREMASHOUT,MASHOUT,RETIRARCESTO,ENCHENDOBOILER,AQUECENDOBOILER,SPARGING,DESCANSOSPARGING,PREFERVURA,FERVURA,ADICAOLUPULO,FINALIZARFERVURA,WHIRLPOOL,CONFIRMATRANSFERIR,TRANSFERENCIA};
    enum EstadosBomba{CENTRAL,EXTERNO,NIVEL};
    static unsigned char EstadoBrassagem = AGUARDASTART;
    static unsigned char EstadoBomba = CENTRAL;
    unsigned int temp;
    unsigned char MinutosEtapa;

    EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);
    EscreverDWINram(DWINadrSetPointTemperatura,SetPointTempPanela);

    if((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= WHIRLPOOL)){
        EscreverDWINram(DWINadrMinutosDoInicio,TotalHoras);
        EscreverDWINram(DWINadrSegundosDoInicio,TotalMinutos);
    }

    if( ((EstadoBrassagem >= MASHIN) && (EstadoBrassagem <= MASHOUT) && (EstadoBomba == CENTRAL)) || ((EstadoBrassagem >= PREFERVURA) && (EstadoBrassagem <= FERVURA)) ){
        temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
        CCPR2L = temp / 100;
    }

    if(EstadoBrassagem == FERVURA){
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

    switch(EstadoBrassagem){
        case AGUARDASTART:
            CanalADAtivo = 0;
            EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
            EscreverDWINram(DWINadrEtapasBrassagem,0);  //limpa icones etapas
            SetPointTempPanela = 0;
            if(BOTAO_START){
                EscreverDWINreg(3,24);  //exibe tela modo enchimento
                EstadoBomba = CENTRAL;
                EstadoBrassagem = MODOENCHIMENTO;
            }
            break;
        case MODOENCHIMENTO:
            temp = LerDWINram(DWINadrBitsReceitaBrassagem);
            if(temp == MASKENCHEUMANUAL){
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0); //limpa flags - não está sendo utilizado, retirado botões touch da tela de brassagem (play,etc)
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba a 100%
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                EstadoBrassagem = CICLABOMBA;
            }
            if(temp == MASKENCHERAUTOMATICO){
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrEtapasBrassagem,1);  //preenche amarelo etapa encher com agua
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0); //limpa flags - não está sendo utilizado, retirado botões touch da tela de brassagem (play,etc)
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                TempoEncherVessel = 0;
                FreeRunSegundos = 0;
                EstadoBrassagem = ENCHERBOILERAGUAPRIMARIA;//RETIRARCESTO;//
            }
            break;
        case ENCHERBOILERAGUAPRIMARIA:
            if(NA_BOILER){
                VALVULAENTRADARESERVATORIO = 0;
                FreeRunSegundos = 0;
                CCPR2L = 127;
                EscreverDWINram(DWINadrPorcentagemBomba,50); //bomba a 50%
                AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida boiler
                EstadoBrassagem = DOSARAGUAPRIMARIA;
            }
            else
                VALVULAENTRADARESERVATORIO = 1;
            break;
        case DOSARAGUAPRIMARIA:
            if((TempoEncherVessel + FreeRunSegundos) >= ReceitaAtual.VolumeInicial*1.5)
            {
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida boiler
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba a 100%
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                EstadoBrassagem = CICLABOMBA;
            }
            if(NB_BOILER){
                TempoEncherVessel += FreeRunSegundos;
                AcionaValvulaMotorizada(FECHAR,MASKY2);  //saida boiler
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba a 100%
                EstadoBrassagem = ENCHERBOILERAGUAPRIMARIA;
            }
            break;
        case CICLABOMBA:    //cicla para retirar bolhas ar do circuito
            for(temp=0;temp<CICLOSBOMBA;temp++){
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
            RESISTENCIARESERVATORIO = 0;    //seleciona resistencia vessel
            RESISTENCIAPANELA = 1;
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
                SIRENE = 1;
                //BOMBADAGUA = 0;
            }
        break;
        case ADICIONARMALTE:
            if(LerDWINreg(3) == 6){
                CCPR2L = 0;     //bomba parada
                OnOffControleTemp = 0;  //como nao havera circulaçao, desliga resistência
                RESISTENCIAPANELA = 0;
                EscreverDWINreg(3,13);  //exibe tela confirmar adicao malte
                SIRENE = 0;
            }
            //if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKBRASSAGEMPLAY){
            if( (BOTAO_START) && (SIRENE == 0) ){       //so aceita botão se já reconheceu a tela, ou seja, desligou sirene
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                Minutos = 0;
                Segundos = 0;
                MinutosEtapa = ReceitaAtual.TempoMashIn;
                RESISTENCIAPANELA = 1;
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
                SIRENE = 1;
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
                SIRENE = 1;
                EscreverDWINreg(3,16);  //exibe tela teste iodo
                EstadoBrassagem = TESTEIODO;
            }
        break;
        case TESTEIODO:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,17);  //exibe tela confirmar MASH OUT
                SIRENE = 0;
            }
            if(BOTAO_START){
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
                EscreverDWINreg(3,9);   //exibe tela para retirar cesto
                CCPR2L = 0;     //desliga bomba
                AcionaValvulaMotorizada(ABRIR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY6);  //whirlpool
                OnOffControleTemp = 0;
                RESISTENCIAPANELA = 0;  //alt. em 27set16, passou a desligar a resistencia, pois se operador demora pra retirar o cesto, dá problema
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 0;
                SIRENE = 1;
                EstadoBrassagem = RETIRARCESTO;
            }
        break;
        case RETIRARCESTO:
            if(LerDWINreg(3) == 6){
                EscreverDWINreg(3,14);  //exibe tela confirmar retirada cesto
                SIRENE = 0;
            }
            //if(LerDWINram(DWINadrBitsReceitaBrassagem) && MASKBRASSAGEMPLAY){
            if( (BOTAO_START) && (SIRENE == 0) ){   //so aceita botao se reconheceu a tela
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
                EscreverDWINram(DWINadrEtapasBrassagem,7);  //preenche amarelo etapa sparging
                while(BOTAO_START);
                MinutosEtapa = TempoAdicionarAguaSparging;
                Segundos = 0;
                EscreverDWINreg(3,18);  //exibe tela sparging manual
                AcionaValvulaMotorizada(FECHAR,MASKY3);  //saida do vessel
                AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada do vessel
                AcionaValvulaMotorizada(ABRIR,MASKY4);  //sparging
                AcionaValvulaMotorizada(ABRIR,MASKY2);  //saida boiler
                EstadoBrassagem = ENCHENDOBOILER;
            }
        break;
        case ENCHENDOBOILER:
            EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
            if(NA_BOILER){
                VALVULAENTRADARESERVATORIO = 0;
                RESISTENCIAPANELA = 0;
                RESISTENCIARESERVATORIO = 1;
                RELESSR = 1;
                EstadoBrassagem = AQUECENDOBOILER;
            }
            else{
                RESISTENCIAPANELA = 1;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 1;
                VALVULAENTRADARESERVATORIO = 1;
            }
            if(BOTAO_START){
                VALVULAENTRADARESERVATORIO = 0;
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
                RESISTENCIAPANELA = 1;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 1;
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
                EscreverDWINram(DWINadrIntensidadeFervura,100); //resistencia panela 100%
                EstadoBrassagem = PREFERVURA;
            }
            break;
        case AQUECENDOBOILER:
            EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
            if(ValoresAD[1] < ((TemperaturaSparging*10)-30)){
                RESISTENCIAPANELA = 0;
                RESISTENCIARESERVATORIO = 1;
                RELESSR = 1;
            }
            else{
                RESISTENCIAPANELA = 1;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 1;
                Bipar();
                EstadoBrassagem = SPARGING;
            }
            if(BOTAO_START){
                VALVULAENTRADARESERVATORIO = 0;
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
                RESISTENCIAPANELA = 1;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 1;
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
                EscreverDWINram(DWINadrIntensidadeFervura,100); //mostra resistencia panela 100%
                EstadoBrassagem = PREFERVURA;
            }
        break;
        case SPARGING:
            EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
            temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
            CCPR2L = temp / 100;
            if(NB_BOILER){
                CCPR2L = 0; //desliga bomba
                EstadoBrassagem = ENCHENDOBOILER;
            }
            if(BOTAO_START){
                EscreverDWINreg(3,6);  //exibe tela brassagem
                CCPR2L = 0; //desliga bomba
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
                RESISTENCIAPANELA = 1;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 1;
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0); //bomba parada
                EscreverDWINram(DWINadrIntensidadeFervura,100); //mostra resistencia panela 100%
                EstadoBrassagem = PREFERVURA;
            }
             /*
                Minutos = 0;
                MinutosEtapa = TempoDescansosparging;
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                EstadoBrassagem = DESCANSOSPARGING;
            }
             */
        break;
        case DESCANSOSPARGING:
            if(Minutos>=MinutosEtapa){
                Minutos = 0;
                Segundos = 0;
                SetPointTempPanela = ReceitaAtual.TemperaturaFervura;
                TempPanelaOK = 0;
                EscreverDWINram(DWINadrEtapasBrassagem,8);  //preenche amarelo etapa fervura
                EstadoBrassagem = PREFERVURA;
            }
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
            }
            break;
        case FERVURA:
            if(ReceitaAtual.TempoLupulo1 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo1){
                    ReceitaAtual.TempoLupulo1 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENE = 1;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo2 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo2){
                    ReceitaAtual.TempoLupulo2 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENE = 1;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo3 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo3){
                    ReceitaAtual.TempoLupulo3 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENE = 1;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(ReceitaAtual.TempoLupulo4 != 0){
                if((MinutosEtapa-Minutos) == ReceitaAtual.TempoLupulo4){
                    ReceitaAtual.TempoLupulo4 = 0;
                    EscreverDWINram(DWINadrEtapasBrassagem,9);  //preenche amarelo etapa adicao lupulo
                    EscreverDWINreg(3,8);   //exibe tela para adicionar lupulo
                    SIRENE = 1;             //aciona sinal sonoro
                    EstadoBrassagem = ADICAOLUPULO;
                }
            }
            if(Minutos>=MinutosEtapa){
                EscreverDWINreg(3,21);  //exibe tela confirmar fim fervura
                SIRENE = 1;
                EstadoBrassagem = FINALIZARFERVURA;
            }
        break;
        case ADICAOLUPULO:
            if(LerDWINreg(3) == 6)
            {
                EscreverDWINreg(3,15);  //exibe tela confirmar adição lupulo
                SIRENE = 0;
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
                SIRENE = 0;
            }
            if(BOTAO_START){
                Minutos = 0;
                Segundos = 0;
                OnOffControleTemp = 0;
                ControlePotenciaFervura = 0;
                RESISTENCIAPANELA = 0;
                RESISTENCIARESERVATORIO = 0;
                RELESSR = 0;
                EscreverDWINram(DWINadrIntensidadeFervura,0); //mostra resistencia panela desligada
                EscreverDWINreg(3,6);  //exibe tela brassagem
                EscreverDWINram(DWINadrEtapasBrassagem,10);  //preenche amarelo etapa whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY6);  //whirlpool
                AcionaValvulaMotorizada(FECHAR,MASKY5);  //entrada vessel
                CCPR2L = 255;
                EscreverDWINram(DWINadrPorcentagemBomba,100);
                MinutosEtapa = ReceitaAtual.TempoLavagem;
                EstadoBrassagem = WHIRLPOOL;
            }
        break;
        case WHIRLPOOL:
            PWMpanela = 0;
            EscreverDWINram(DWINadrPorcentagemBomba,100);   //nao deixa usuario alterar rotação bomba
            if(Minutos >= MinutosEtapa){
                EscreverDWINreg(3,25);  //exibe tela confirmaçao transfega manual
                SIRENE = 1;
                EstadoBrassagem = CONFIRMATRANSFERIR;
            }
        break;
        case CONFIRMATRANSFERIR:
            if(BOTAO_START){
                SIRENE = 0;
                EscreverDWINreg(3,20);  //exibe tela transfega manual
                EscreverDWINram(DWINadrEtapasBrassagem,11);  //preenche amarelo etapa resfriamento
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                CCPR2L = 0;
                AcionaValvulaMotorizada(FECHAR,MASKY6);  //whirlpool
                AcionaValvulaMotorizada(ABRIR,MASKY7);  //saida trocador
                VALVULAENTRADAAGUATROCADOR = 1;
                EstadoBrassagem = TRANSFERENCIA;
            }
        break;
        case TRANSFERENCIA:
            EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);
            temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
            CCPR2L = temp / 100;
            if(BOTAO_START){
                CCPR2L = 0;
                EscreverDWINram(DWINadrPorcentagemBomba,0);
                VALVULAENTRADAAGUATROCADOR = 0;
                EscreverDWINreg(3,1);  //exibe menu principal
                AcionaValvulaMotorizada(FECHAR,MASKY7);  //saida trocador
                AcionaValvulaMotorizada(FECHAR,MASKY3);  //saida vessel
                while(BOTAO_START);
                EstadoBrassagem = AGUARDASTART;
            }
        break;
    }
}