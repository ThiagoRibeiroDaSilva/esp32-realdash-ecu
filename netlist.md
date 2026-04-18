DIRETRIZES DE ARQUITETURA E FIRMWARE
 * Aterramento (O Nó Estrela): Divisão estrita entre PGND (Terra Sujo p/ Potência/TVS/Buck/Backlight) e LGND/AGND (Terra Limpo p/ Lógica/Sensores/ADC). União em um ÚNICO ponto físico na placa.
 * Ambiente: Instalação OBRIGATÓRIA na cabine do veículo. Vetado uso no cofre do motor.
 * Regras de Firmware (C++):
   * Ratiometria: PROIBIDO dividir contagens 'raw' do ADS1115. Converter todos os canais para Volts reais antes das equações. (A0 e A3 em GAIN_TWOTHIRDS. A1 e A2 em GAIN_ONE).
   * Alternador (A3): Histerese com estado transitório. >13.5V por 2s = Alternador. <12.8V por 2s = Bateria. Entre 12.8V e 13.5V = Manter último estado válido.
   * Combustível: Filtro em HW (0.34Hz) exige ~3s para estabilizar o tanque.
   * RPM: Validar forma de onda real com osciloscópio (Bobina, RPM_IN, Saída Opto). Usar Glitch Filter por hardware no S3.
PARTE 3: O CÓDIGO MATRIX (NETLIST FÍSICO "FURO A FURO")
LEGENDA:
[Furo X00]: Coluna (A a AM), Linha (1 a 59). Lado da solda virado para CIMA.
[LISTRA]: Cátodo do Diodo. / [LISO]: Ânodo do Diodo.
[JUMPER]: Fio longo soldando os dois furos. / [PONTE]: Dobrar a perna do componente até o furo vizinho e soldar.
PASSO 1: AS AVENIDAS E O NÓ ESTRELA
 * PGND (Terra Sujo): Fio longo cruzando toda a Linha 2 (Do A2 ao AM2).
 * LGND (Terra Limpo): Fio longo cruzando toda a Linha 58 (Do A58 ao AM58).
 * 5V_A (Analógico): Fio cruzando a Linha 56 (Do A56 ao AM56).
 * 3.3V_ESP (Lógico): Fio cruzando a Linha 54 (Do A54 ao AM54).
 * NÓ ESTRELA: [JUMPER] fio grosso do AM2 (ponta do PGND) conectando direto ao AM58 (ponta do LGND). (Módulo ADS1115 ficará fisicamente ao lado).
PASSO 2: FRONT-END 12V E PROTEÇÃO
6. Entrada 12V: Fio do Fusível entra no A4. O Nó FRONT é a Linha 4 (A4 a G4). Una todos com solda.
7. TVS TP6KE22A: [LISTRA] no C4 [PONTE p/ B4 (Nó FRONT)]. [LISO] no C8. [JUMPER] do C8 p/ C2 (PGND).
8. IRF9540N (TO-220): Gate no E10, Drain no F10, Source no G10.
 * [JUMPER] de G10 p/ G4 (FRONT).
 * Drain (F10) p/ A15 [JUMPER]. A Linha 15 (A15 a M15) é o Nó VBAT_PROT. Una com solda.
 * R 100k: Espete no E11 e E14. [PONTE de E11 p/ Gate E10]. [JUMPER] do E14 p/ E2 (PGND).
 * R 1M: Espete no D10 e D14. [PONTE de D10 p/ Gate E10]. [JUMPER] do D14 p/ G11. [PONTE de G11 p/ Source G10].
 * Zener 15V: [LISTRA] no H10 [PONTE p/ Source G10]. [LISO] no H13. [JUMPER] do H13 p/ E12 [PONTE p/ Gate E10].
 * Filtros VBAT_PROT:
   * Cap 470µF: Positivo no I15. Negativo no I18 ([JUMPER] p/ I2 PGND).
   * Cap 1µF: Pernas no J15 e J18. [JUMPER] do J18 p/ J2 (PGND).
   * Cap 100nF: Pernas no K15 e K18. [JUMPER] do K18 p/ K2 (PGND).
PASSO 3: MÓDULOS DE ENERGIA E ESP32 (WROOM-1)
10. LM2596: IN+ no M15. IN- no M2 (PGND). OUT- no O2 (PGND). OUT+ no A18. A Linha 18 (A18 a Z18) é a trilha 5V_D (Una com solda).
11. Caminho do 5V_A: Resistor 10 Ohms do O18 (5V_D) ao O56 (5V_A). Cap 47µF (Positivo no P56, Negativo no P58/LGND). Cap 100nF do Q56 p/ Q58.
12. ESP32-S3 DevKit: Pinos esquerdos na Col W (W20 a W41). Direitos na Col AG (AG20 a AG41).
 * [JUMPER] do V18 (trilha 5V_D) p/ Pino 5V do ESP (W40).
 * [JUMPER] do W58 (trilha LGND) p/ UM ÚNICO GND do ESP (W41).
 * [JUMPER] do Pino 3.3V do ESP (W20) p/ trilha 3.3V_ESP (W54).
PASSO 4: GPS LDO E UART
13. MCP1700-3302 (TO-92): GND no AA48 ([JUMPER] p/ LGND AA58). VIN no AB48 ([JUMPER] p/ 5V_D AB18). VOUT no AC48.
14. Caps LDO: Cap 1µF IN (AB49 p/ AB52 -> Jumper LGND). [PONTE de AB49 p/ AB48]. Cap 1µF OUT (AC49 p/ AC52 -> Jumper LGND). [PONTE de AC49 p/ AC48].
15. Módulo GPS NEO-M8N: VCC no AC48. GND no LGND. Fio TX do GPS p/ RX do ESP (W30). Fio RX do GPS p/ TX do ESP (W29).
PASSO 5: I2C (ADS1115 E BSS138)
16. ADS1115: Col AK (40 a 49). VDD (AK40) p/ 5V_A. GND (AK41) p/ LGND. ADDR (AK44) p/ LGND. SCL em AK42. SDA em AK43.
17. BSS138: Col AI (40 a 48).
 * Lado HV: HV(AI40) p/ 5V_A. GND(AI41) p/ LGND. HV1(AI42) [PONTE p/ AK42]. HV2(AI43) [PONTE p/ AK43].
 * Lado LV: LV(AI45) p/ 3.3V_ESP. GND(AI46) p/ LGND. LV1(AI47) p/ W34 (SCL ESP). LV2(AI48) p/ W31 (SDA ESP).
PASSO 6: RPM (A MURALHA E O H11L1M)
18. Muralha 6.8k: Fio Bobina (Malha blindada no PGND) sinal entra no A20. R1 (A20 e A24). R2 (B24 e B28) [PONTE A24 p/ B24]. R3 (C28 e C32) [PONTE B28 p/ C28]. R4 (D32 e D36) [PONTE C32 p/ D32]. D36 é o RPM_IN.
19. Zener 5.1V: [LISTRA] no E36 [PONTE p/ D36]. [LISO] no E32 [JUMPER] p/ PGND.
20. Resistor 330 Ohms: F36 p/ F39 [PONTE de D36 p/ F36].
21. H11L1M: Col H (Pinos 1,2,3). Col K (Pinos 6,5,4).
 * Pino 1 no H39 [PONTE p/ F39]. Pino 2 no H40 [JUMPER] p/ PGND.
 * 1N4148: [LISTRA] no I39 [PONTE p/ H39]. [LISO] no I40 [PONTE p/ H40].
 * Pino 6 no K39 [JUMPER] p/ 3.3V_ESP. Pino 5 no K40 [JUMPER] p/ LGND.
 * Cap 100nF: J39 e J40 [PONTE p/ K39 e K40].
 * Pino 4 no K41 [JUMPER] p/ W23 (GPIO4 ESP).
 * R 4.7k (Pull-up): L41 e L45 [PONTE de K41 p/ L41]. [JUMPER] do L45 p/ 3.3V_ESP.
PASSO 7: SENSORES ANALÓGICOS DE BLOCO (Clamps Absolutos)
22. A0 (Óleo): VCC do Sensor p/ 5V_A. FIO GND DO SENSOR DIRETO P/ LGND. Fio Sinal no A45. R 2.2k do A45 p/ B45 (Nó A0_FILT).
 * Cap 100nF no B46 e B50. [PONTE de B45 p/ B46]. [JUMPER] de B50 p/ LGND.
 * SD103 Alto: [LISO] no C45 [PONTE p/ B45]. [LISTRA] no C56 (5V_A).
 * SD103 Baixo: [LISTRA] no D45 [PONTE p/ C45]. [LISO] no D58 (LGND).
 * [JUMPER] do B45 p/ Pino A0 do ADS (AK46).
 * A1 (NTC): R 2.2k do 5V_A p/ E45 (NTC_RAW). Fio de sinal do motor no E45. (OBRIGATÓRIO: Se 2 fios, retorno dedicado p/ LGND). R 2.2k do E45 p/ F45 (Nó A1_FILT).
 * Cap 1uF X7R no F46 e F50. [PONTE p/ F45 e LGND].
 * SD103 Alto: [LISO] G45 [PONTE p/ F45]. [LISTRA] G56 (5V_A).
 * SD103 Baixo: [LISTRA] H45 [PONTE p/ G45]. [LISO] H58 (LGND).
 * [JUMPER] do F45 p/ Pino A1 do ADS (AK47).
 * A2 (Monitor 5V_A): R 10k (1%) do 5V_A p/ I45 (A2_DIV). R 10k (1%) do I45 p/ I58 (LGND).
 * Cap 100nF no I46 e I50 [PONTE p/ I45 e LGND].
 * R 1k do I45 p/ J45. [JUMPER] de J45 p/ A2 do ADS (AK48).
 * A3 (Alternador): [JUMPER] do VBAT_PROT p/ K42. R 33k (1%) do K42 p/ K45 (Nó A3_DIV). R 4.7k (1%) do K45 p/ K58 (LGND).
 * Cap 100nF no K46 e K50 [PONTE p/ K45 e LGND].
 * SD103 Alto: [LISO] L45 [PONTE p/ K45]. [LISTRA] L56 (5V_A).
 * SD103 Baixo: [LISTRA] M45 [PONTE p/ L45]. [LISO] M58 (LGND).
 * R 1k do K45 p/ N45. [JUMPER] de N45 p/ A3 do ADS (AK49).
PASSO 8: COMBUSTÍVEL - MCP6002 BUFFER
26. MCP6002 (DIP-8): Pinos 1-4 Col Q (44 a 47). Pinos 8-5 Col T (44 a 47). Pino 8 (T44) [JUMPER] p/ 3.3V_ESP. Pino 4 (Q47) [JUMPER] p/ LGND. Cap 100nF de U44 p/ U47 [PONTE p/ T44 e Q47].
27. Circuito A (Boia): Sinal no N46. R 1M do N46 p/ O46 (DIV_A). [PONTE p/ Pino 3 (Q46)].
 * R 220k do O46 p/ O58 (LGND). Cap 470nF no P46 e P50 [PONTE p/ O46 e LGND].
 * SD103 Alto: [LISO] R46 [PONTE p/ P46]. [LISTRA] R54 (3.3V_ESP).
 * SD103 Baixo: [LISTRA] S46 [PONTE p/ R46]. [LISO] S58 (LGND).
 * Saída A: [PONTE UNINDO Pino 1 (Q44) ao Pino 2 (Q45)]. R 100 Ohms do Q44 p/ V44. [JUMPER] do V44 p/ W24 (GPIO5). Cap 100nF no V44 e V50 [JUMPER p/ LGND].
 * Circuito B (Relógio): FIO DE ALIMENTAÇÃO POSITIVO DO RELÓGIO no N47. R 1M do N47 p/ O47 (DIV_B). [PONTE p/ Pino 5 (T47)].
 * R 220k do O47 p/ O58 (LGND). Cap 470nF no P47 e P50 [PONTE p/ O47 e LGND].
 * SD103 Alto: [LISO] R47 [PONTE p/ P47]. [LISTRA] R54 (3.3V_ESP).
 * SD103 Baixo: [LISTRA] S47 [PONTE p/ R47]. [LISO] S58 (LGND).
 * Saída B: [PONTE UNINDO Pino 7 (T45) ao Pino 6 (T46)]. R 100 Ohms do T45 p/ V45. [JUMPER] do V45 p/ W25 (GPIO6). Cap 100nF no V45 e V50 [JUMPER p/ LGND].
PASSO 9: ALERTAS E TELA (Sinais e Backlight)
29. PC817 (DIP-4): IC1 e IC2.
 * IC1 (Água): 12V ACC p/ R 2.2k p/ Pino 1 (X44). Fio GND do Sensor no Pino 2 (X45). Pino 3 (AA45) p/ LGND. [JUMPER] Pino 4 (AA44) p/ W27 (GPIO15). Pull-up 10k do AA44 p/ 3.3V_ESP.
 * IC2 (Freio): 12V ACC p/ R 2.2k p/ Pino 1 (X50). Fio GND do Sensor no Pino 2 (X51). Pino 3 (AA51) p/ LGND. [JUMPER] Pino 4 (AA50) p/ W28 (GPIO16). Pull-up 10k do AA50 p/ 3.3V_ESP.
 * Backlight IRLZ44N: Gate R5, Drain S5, Source T5.
 * Source (T5) p/ PGND. R 100k do R5 p/ PGND.
 * R 100 Ohms do Gate (R5) cruzando p/ W38 (GPIO13).
 * Drain (S5) no pino BLK da Tela.
 * Módulo TFT SPI (ST7796S):
 * VCC da Tela no 5V_D.
 * GND da Tela no LGND (Obrigatório).
 * [JUMPER] CS p/ W35 (GPIO10).
 * [JUMPER] MOSI p/ W36 (GPIO11).
 * [JUMPER] SCK p/ W37 (GPIO12).
 * [JUMPER] DC p/ W39 (GPIO14).
 * [JUMPER] RST p/ AG37 (GPIO21).
