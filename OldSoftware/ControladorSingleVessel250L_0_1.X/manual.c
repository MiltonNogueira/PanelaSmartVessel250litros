/*******************************************************************************
 * www.neuheitbier.com.br
 * Função para operação manual panela cervejeira smart vessel
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

extern void EscreverDWINram(unsigned int endereco,int valor);
extern void EscreverDWINreg(unsigned char endereco,int valor);
extern int LerDWINram(unsigned int endereco);
extern int LerDWINreg(unsigned char endereco);
extern void EscreverDWINtrend(int valor);
extern void AcionaValvulaMotorizada(unsigned char sentido,unsigned char valvula);
extern void DelaySeg(unsigned char tempo);

extern unsigned int ValoresAD[4];
extern unsigned char CanalADAtivo;                 //99 = todos canais
extern bit StatusY2 = 0;
extern bit StatusY3 = 0;
extern bit StatusY4 = 0;
extern bit StatusY5 = 0;
extern bit StatusY6 = 0;
extern bit StatusY7 = 0;
extern unsigned int TempUInt;

/*
 * Esta função aciona manualmente as 8 válvulas, a bomba e as 2 resistencias eletricas
 * Existe uma tela no display DWIN apenas para este modo manual
 */
void ModoManual(void)
{
    unsigned int temp;

    CanalADAtivo = 99;

    //atualiza os valores de temperatura da panela, do reservatorio e da saida do trocador
    EscreverDWINram(DWINadrTemperaturaMosto,ValoresAD[0]);
    EscreverDWINram(DWINadrTemperaturaReservatorio,ValoresAD[1]);
    EscreverDWINram(DWINadrTemperaturaSaida,ValoresAD[2]);

    temp = LerDWINram(DWINadrPorcentagemBomba) * 255;
    CCPR2L = temp / 100;

    TempUInt = LerDWINram(DWINadrBitsModoManual);
    if(TempUInt != 0){
        switch(TempUInt){
            //Valvula diafragma que enche o reservatorio de agua de sparging
            case MASKY1:
                VALVULAENTRADARESERVATORIO = !VALVULAENTRADARESERVATORIO;
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
                VALVULAENTRADAAGUATROCADOR = !VALVULAENTRADAAGUATROCADOR;
                if(VALVULAENTRADAAGUATROCADOR)
                    EscreverDWINram(DWINadrIconeY8,1);
                else
                    EscreverDWINram(DWINadrIconeY8,0);
                break;
        }
        NOP();
        EscreverDWINram(DWINadrBitsModoManual,0);
        DelaySeg(1);
    }
    TempUInt = LerDWINram(DWINadrBitsReceitaBrassagem);
    if(TempUInt != 0){
        NOP();
        switch(TempUInt){
            case MASKRESISTENCIAPANELA:
                RESISTENCIAPANELA = !RESISTENCIAPANELA;
                if(RESISTENCIAPANELA){
                    RELESSR = 1;
                    EscreverDWINram(DWINadrIconeResistenciaPanela,1);
                }
                else{
                    RELESSR = 0;
                    EscreverDWINram(DWINadrIconeResistenciaPanela,0);
                }
                break;
            case MASKRESISTENCIARESERVATORIO:
                RESISTENCIARESERVATORIO = !RESISTENCIARESERVATORIO;
                if(RESISTENCIARESERVATORIO){
                    RELESSR = 1;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,1);
                }
                else{
                    RELESSR = 0;
                    EscreverDWINram(DWINadrIconeResistenciaReservatorio,0);
                }
                break;
        }
        EscreverDWINram(DWINadrBitsReceitaBrassagem,0);
        DelaySeg(1);
    }
}
