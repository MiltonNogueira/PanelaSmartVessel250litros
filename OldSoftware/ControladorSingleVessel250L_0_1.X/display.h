//Definicoes dos enderecos das variaveis no display DWIN
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
#define MASKUPLOADRECEITA 0b00000001
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
#define DWINadrPorcentagemBomba 0x39
#define DWINadrIconeY1 0x41
#define DWINadrIconeY2 0x42
#define DWINadrIconeY3 0x43
#define DWINadrIconeY4 0x44
#define DWINadrIconeY5 0x45
#define DWINadrIconeY6 0x46
#define DWINadrIconeY7 0x47
#define DWINadrIconeY8 0x48
#define DWINadrIconeResistenciaPanela 0x49
#define DWINadrIconeResistenciaReservatorio 0x50
#define DWINadrSetPointTemperatura 0x52