# 📘 DOSSIÊ DE ENGENHARIA E GUIA DE MONTAGEM: ECU CHEVETTE — TELEMETRIA ESP32-S3
**Versão:** V22.0 (The Software & I2S Audio Bridge — Arquitetura Selada)  
**Status:** HARDWARE APROVADO P/ BANCADA E ROAD-TEST (MATRIZ REAL 55x37 AUDITADA)

---

## HISTÓRICO DE CONSOLIDAÇÃO ARQUITETURAL
* **A Fonte do Canal B (Combustível Ratiométrico):** O Circuito A lê o fio variável da boia do tanque; o Circuito B lê diretamente o fio positivo de alimentação do relógio original do painel, permitindo ao firmware cancelar matematicamente qualquer variação da tensão do chicote do carro.
* **O Limbo do Alternador (Zona Morta):** Se a tensão ficar entre `12.8V` e `13.5V`, entra em ação a regra estrita de histerese do firmware: *"Manter o último estado válido até que um novo limiar seja cruzado por pelo menos 2 segundos"*.
* **A Matemática do Óleo (Proteção de Firmware):** Como os canais do ADS1115 operam com ganhos (PGA) distintos, é proibido dividir contagens brutas (*raw counts*). O C++ converte obrigatoriamente todas as leituras para **Volts reais** antes de executar cálculos ratiométricos.
* **O Critério do Sensor de Óleo:** Alimentado pelo barramento `5V_D` (e nunca pelo `5V_A`), impedindo que o consumo ativo do transdutor provoque queda de tensão no resistor de 10 Ohms do filtro analógico.
* **A Lentidão Intencional da Boia (0.34Hz):** O filtro RC de entrada (`220k` + `470nF`) possui constante de tempo de ~3 segundos para atuar como amortecedor físico contra oscilações de combustível no tanque durante curvas e frenagens.
* **Isolamento do Subsistema de Voz I2S (Novo na V22.0):** Como o ESP32-S3 não possui DAC analógico nativo, a síntese de voz e os alertas sonoros utilizam o módulo digital I2S **MAX98357A** com alto-falante dedicado de **4Ω / 3W**, operando de forma 100% independente do rádio do veículo. Para evitar que picos de corrente de áudio contaminem as leituras analógicas, o módulo I2S é alimentado exclusivamente pela linha de potência **`5V_D`** e aterrado no **`PGND`**.

---

## 1. RESUMO EXECUTIVO DE HARDWARE
* **Matriz Física:** Placa Perfurada Dupla Face com **55 Linhas (1 a 55)** e **37 Colunas (`A` a `Z`, seguidas de `AA` a `AK`)**.
* **Cérebro:** ESP32-S3 DevKitC-1 (44 pinos: Coluna `P` linhas 20-41 e Coluna `Z` linhas 20-41). Fixação mecânica adicional com PU/Epóxi.
* **Tradutor Analógico:** ADS1115 (Endereço I2C `0x48`, Pull-ups internos de 10k mantidos. `GAIN_TWOTHIRDS` +/- 6.144V p/ A0 Óleo e A3 Alternador; `GAIN_ONE` +/- 4.096V p/ A1 Água e A2 Monitor 5V_D).
* **Ponte Lógica I2C (3.3V <-> 5V):** Módulo BSS138 (SCL no `GPIO 9` / `P34`; SDA no `GPIO 8` / `P31`).
* **Op-Amp Buffer (Combustível):** MCP6002-I/P (DIP-8).
* **Optoacoplador RPM:** H11L1M (DIP-6). Pinagem: `1` (Ânodo), `2` (Cátodo), `4` (VO Saída), `5` (GND), `6` (VCC).
* **Optoacopladores de Alerta:** 3x PC817 (DIP-4: Água no `GPIO 15` / `P27`, Freio no `GPIO 16` / `P28`, Nível do Radiador no `GPIO 7` / `P26`).
* **Áudio Digital I2S:** Módulo MAX98357A (`LRC` no `GPIO 41` / `Z26`, `BCLK` no `GPIO 40` / `Z27`, `DIN` no `GPIO 42` / `Z25`) + Alto-falante 4Ω 3W.
* **Proteção Reversa e Surtos:** IRF9540N (TO-220) + Zener 15V (`1N4744A`) + TVS Principal `TP6KE22A`.
* **Chaveamento de Backlight:** IRLZ44N (TO-220) acionado pelo `GPIO 13` (`P38`).
* **Clamps Analógicos:** 10x Diodos Schottky `SD103B/C` (DO-35 Axial).
* **Fontes Internas:** Buck `LM2596` (12V -> `5V_D`) e LDO `MCP1700-3302` (`5V_D` -> `3.3V_GPS`).

---

## 2. DIRETRIZES DE ATERRAMENTO E MONTAGEM
* **Domínios de Terra Separados (União única no Nó Estrela `AB2` <-> `AB54`):**
  * **`PGND` (Terra de Potência / Sujo — Avenida na Linha 2, de `A2` a `AJ2`):** Entrada 12V, Diodo TVS, Capacitores de `VBAT_PROT`, GND IN/OUT do LM2596, Source e Pull-down do Backlight (`IRLZ44N`), Pinos 2 (Cátodo) de todos os Optoacopladores (`H11L1M` e `3x PC817`), GND do sensor `XKC-Y25-NPN` e **GND do amplificador de áudio `MAX98357A`**.
  * **`LGND/AGND` (Terra Limpo de Instrumentação — Avenida na Linha 54, de `A54` a `AB54`):** Pinos GND do ESP32-S3, ADS1115, BSS138, MCP6002, Lado isolado dos Optoacopladores (Pino 5 do `H11L1M` e Pinos 3 dos `PC817`), Retorno GND dedicado dos sensores de bloco (Óleo e Água), GND do LDO `MCP1700` e GPS.
* **Avenidas Positivas Inferiores:**
  * **Linha 50 (`A50` a `AB50`):** Barramento lógico `3.3V_ESP`.
  * **Linha 52 (`A52` a `AB52`):** Barramento analógico limpo `5V_A`.
* **Ambiente Operacional:** VETADA a instalação no cofre do motor. Uso exclusivo no interior da cabine.

---

## 3. TOPOLOGIA FUNCIONAL POR BAIRROS (MATRIZ 55x37)

### BAIRRO 1: Front-End 12V (Linhas 2 a 16, Colunas A a M)
* `+12V` -> Fusível 2A -> Nó `FRONT` (`A4` a `F4`).
* TVS `TP6KE22A`: Cátodo no Nó `FRONT` (`C4`), Ânodo em `C7` com jumper p/ `PGND` (`C2`).
* Proteção Reversa: Nó `FRONT` -> Source `IRF9540N` (`G9`). Drain (`F9`) -> Nó `VBAT_PROT` (`A13` a `M13`).
* Malha de Gate `IRF9540N` (`E9`): Resistor `1M Ohm` (`D9-D12`, Gate p/ Source) + Resistor `100k` (`E10-E12`, Gate p/ `PGND`). Zener `15V` (`H9-H12`, Cátodo no Source, Ânodo no Gate).
* Filtragem `VBAT_PROT`: Eletrolítico `1000µF/50V` (`I13-I16`) + Cerâmico `1µF/50V` (`L13-L16`) + Cerâmico `100nF/50V` (`M13-M16`) referenciados ao `PGND`.

### BAIRRO 2: Distribuição de Energia
* **`5V_D` (Carga Pesada Digital — Trilha `A16` a `V16`):** Saída do `LM2596`. Alimenta `ESP32(VIN)`, `MCP1700(VIN)`, Transdutor de Óleo, Sensor do Radiador `XKC-Y25-NPN`, Display TFT (`VCC`) e Módulo de Áudio I2S `MAX98357A`.
* **`5V_A` (Analógico Limpo — Linha 52):** Saída `5V_D` (`O16`) -> Jumper p/ `O51` -> Resistor `10 Ohms` (`O51-O52`) -> Avenida `5V_A`. Filtrado por Eletrolítico `47µF` (`Q52-Q54`) + Cerâmico `100nF` (`R52-R54`) p/ `LGND`. Alimenta **exclusivamente** `ADS1115 (VDD)`, lado `HV` do `BSS138`, Pull-up de 1% do NTC e Clamps de A1/A3.
* **`3.3V_GPS` (LDO):** `MCP1700` (`U40`, `U41`, `U42`) alimentado pelo `5V_D` (`U16`), com capacitores de `1µF` na entrada (`T39-T41`) e na saída (`V40-V42`) p/ `LGND`. Alimenta exclusivamente o GPS (`V42` -> `P18`).
* **`3.3V_ESP` (Linha 50):** Proveniente do pino `3V3` do DevKit (`P20`). Alimenta lado `LV` do `BSS138`, `H11L1M (Pino 6)`, `MCP6002 (Pino 8)`, Clamps do Combustível e Pull-ups lógicos.

### BAIRRO 3: Condicionamento de RPM (Linhas 20 a 34, Colunas A a J)
* Entrada Negativa da Bobina (`A20`) -> 4x Resistores `6.8k Ohms / 0.5W` em série zig-zag (`A20-A23`, `B23-B26`, `C26-C29`, `D29-D32`) -> Nó `RPM_IN` (`D32`).
* Zener `5.1V` (`D32-D30`) ceifando para o `PGND` (`D2`).
* Nó `RPM_IN` (`D32`) -> Resistor `330 Ohms` (`D32-F32`) -> Pino 1 (Ânodo, `G32`) do `H11L1M`. Pino 2 (Cátodo, `G33`) -> `PGND` (`G2`).
* Diodo `1N4148` em antiparalelo (`H32-H33`): Cátodo no Pino 1 (`G32`), Ânodo no Pino 2 (`G33`).
* Lado Isolado (ESP): Pino 6 (`J32`) em `3.3V_ESP` (`J50`) + Cap `100nF` (`I32-I33`) p/ `LGND`. Pino 5 (`J33`) em `LGND` (`J54`). Pino 4 (`J34`) -> `GPIO 4` (`P23`) + Pull-up `4.7k` (`I34-I31`) p/ `3.3V_ESP` (`I50`).

### BAIRRO 4: Sensores Analógicos de Bloco (Linha 44, Colunas A a I)
* **A0 (Pressão de Óleo — Colunas A a D):** VCC no `5V_D`, GND dedicado no `LGND`. Sinal (`A44`) -> R `2.2k` (`A44-B44`) -> Nó `A0_FILT` (`B44`). Cap `100nF` (`B44-B54`) p/ `LGND`. Clamps `SD103`: Alto (`C44-C47` p/ `5V_D`), Baixo (`D44-D54` p/ `LGND`). Saída `B44` -> `ADS1115 A0` (`AB36`).
* **A1 (Temperatura da Água NTC — Colunas F a I):** `5V_A` (`F52`) -> R `2.2k (1%)` (`F52-F44`) -> Nó `NTC_RAW` (`F44`). Sinal -> R `2.2k` (`F44-G44`) -> Nó `A1_FILT` (`G44`). Cap `1µF X7R` (`G44-G54`) p/ `LGND`. Clamps `SD103`: Alto (`H44-H52` p/ `5V_A`), Baixo (`I44-I54` p/ `LGND`). Saída `G44` -> `ADS1115 A1` (`AB37`).

### BAIRRO 5: Monitoramento Interno e Alternador (Colunas J a N)
* **A2 (Monitor `5V_D` — Colunas J e K):** `5V_D` (`J16` -> `J41`) -> R `10k (1%)` (`J41-J44`) -> Nó `A2_DIV` (`J44`) -> R `10k (1%)` (`J44-J54`) -> `LGND`. Cap `100nF` (`K44-K54`) p/ `LGND`. R `1k` (`K44-K41`) -> `ADS1115 A2` (`AB38`).
* **A3 (Alternador — Colunas L a N):** `VBAT_PROT` (`B13` -> `L41`) -> R `33k (1%)` (`L41-L44`) -> Nó `A3_DIV` (`L44`) -> R `4.7k (1%)` (`L44-L54`) -> `LGND`. Cap `100nF` (`M44-M54`) p/ `LGND`. Clamps `SD103`: Alto (`M44-M52` p/ `5V_A`), Baixo (`N44-N54` p/ `LGND`). R `1k` (`N44-N41`) -> `ADS1115 A3` (`AB39`).

### BAIRRO 6: Combustível Ratiométrico (Buffer MCP6002 — Colunas A a O, Linhas 32 a 42)
* **CI `MCP6002` (Colunas L e O, Linhas 32 a 35):** Pino 8 (`O32`) em `3.3V_ESP` (`O50`), Pino 4 (`L35`) em `LGND` (`L54`), desacoplado por Cap `100nF` (`M32-M35`).
* **Circuito A (Boia — Linha 34):** Sinal (`A34`) -> R `1M` (`A34-B34`) -> Nó `DIV_A` (`B34`) -> Pino 3 (`L34`). De `DIV_A` p/ `LGND`: R `220k` (`B34-B37`) + Cap `470nF` (`C34-C37`). Clamps `SD103`: Alto (`D34-D37` p/ `3.3V_ESP`), Baixo (`E34-E37` p/ `LGND`). Buffer A: Pino 1 (`L32`) unido ao Pino 2 (`L33`) -> R `100 Ohms` (`L32-K32`) -> `GPIO 5` (`P24`), com Cap `100nF` (`K32-K35`) p/ `LGND`.
* **Circuito B (Alimentação do Relógio do Painel — Linha 39):** Sinal (`A39`) -> R `1M` (`A39-B39`) -> Nó `DIV_B` (`B39`) -> Pino 5 (`O35`). De `DIV_B` p/ `LGND`: R `220k` (`B39-B42`) + Cap `470nF` (`C39-C42`). Clamps `SD103`: Alto (`D39-D42` p/ `3.3V_ESP`), Baixo (`E39-E42` p/ `LGND`). Buffer B: Pino 7 (`O33`) unido ao Pino 6 (`O34`) -> R `100 Ohms` vertical (`N33-N36`) -> `GPIO 6` (`P25`), com Cap `100nF` (`N36-N38`) p/ `LGND`.

### BAIRRO 7: Display, GPS, Alertas Isolados e Áudio I2S
* **Display TFT SPI:** `CS` (`GPIO 10` / `P35`), `MOSI` (`GPIO 11` / `P36`), `SCK` (`GPIO 12` / `P37`), `DC` (`GPIO 14` / `P39`), `RST` (`GPIO 21` / `Z37`).
* **Backlight (`IRLZ44N` — Colunas AH, AI, AJ):** `GPIO 13` (`P38`) -> Jumper p/ `AH8` -> R `100 Ohms` (`AH8-AH5`) -> Gate (`AH5`). Pull-down R `100k` (`AH5-AH2`) p/ `PGND`. Source (`AJ5`) -> `PGND` (`AJ2`). Drain (`AI5`) -> Fio `BLK` do Display.
* **Alertas Optoacoplados (`3x PC817` — Colunas K e N, Linhas 4 a 11):**
  * `IC1` (Água — Linhas 4-5): `12V ACC` (`I4`) -> R `2.2k` -> Pino 1 (`K4`). Pino 2 (`K5`) -> Interruptor no motor. Pino 3 (`N5`) -> `LGND`. Pino 4 (`N4`) -> `GPIO 15` (`P27`) + Pull-up `10k` p/ `3.3V_ESP`.
  * `IC2` (Freio — Linhas 7-8): `12V ACC` (`I7`) -> R `2.2k` -> Pino 1 (`K7`). Pino 2 (`K8`) -> Interruptor do freio. Pino 3 (`N8`) -> `LGND`. Pino 4 (`N7`) -> `GPIO 16` (`P28`) + Pull-up `10k` p/ `3.3V_ESP`.
  * `IC3` (Nível do Radiador `XKC-Y25-NPN` — Linhas 10-11): Sensor alimentado em `5V_D` (`H16`) e `PGND` (`I11` -> `I2`). Fio Amarelo (`I10`) -> R `2.2k` (`I10-K10`) -> Pino 1 (`K10`). Pino 2 (`K11`) -> `PGND` (`K2`). Pino 3 (`N11`) -> `LGND`. Pino 4 (`N10`) -> `GPIO 7` (`P26`) + Pull-up `10k` p/ `3.3V_ESP`.
* **Áudio Digital I2S (`MAX98357A` — Coluna AE, Linhas 20 a 26):**
  * Dados: `LRC` (`AE20`) -> `GPIO 41` (`Z26`) | `BCLK` (`AE21`) -> `GPIO 40` (`Z27`) | `DIN` (`AE22`) -> `GPIO 42` (`Z25`).
  * Potência Isolada: `VIN` (`AE26`) em `5V_D` (`V16`) e `GND` (`AE25`) em `PGND` (`AE2`). Saída ligada diretamente ao Alto-Falante de `4Ω / 3W`.

---

## 4. GUIA PRÁTICO DE MONTAGEM NA BANCADA (CONCEITOS-CHAVE)
1. **Barramentos / Trilhas de Estanho (Ex: Nó `FRONT` de `A4` a `F4`):** Não use cabo flexível encapado. Utilize fio de cobre rígido nu (descascado) atravessando os furos e preencha com estanho, transformando todos os furos daquele segmento em um único barramento sólido.
2. **Diferença entre `[PONTE]` e `[JUMPER]`:**
   * **`[PONTE]`:** Quando dois terminais estão em furos vizinhos. Dobre a própria perna do componente por baixo da placa até encostar no terminal vizinho e solde ambos juntos.
   * **`[JUMPER]`:** Quando os pontos estão distantes. Use fio flexível **encapado** (AWG 22 ou AWG 18 conforme a corrente), decape apenas as pontas e solde no furo de origem e no furo de destino.
3. **Identificação do `IRF9540N` e do `IRLZ44N` (Encapsulamento TO-220):** Olhando o componente de frente (lendo a gravação no plástico preto) com as pernas apontando para baixo:
   * **Perna 1 (Esquerda):** Gate (Gatilho).
   * **Perna 2 (Meio):** Drain (Dreno — conectado também à aba metálica traseira).
   * **Perna 3 (Direita):** Source (Fonte).
4. **A Muralha de `6.8k` (Zig-Zag Anti-Arco):** O fio negativo da bobina traz picos indutivos de até 400V. Os 4 resistores de `6.8k / 0.5W` são soldados em zig-zag (`A20->A23`, ponte p/ `B23->B26`, ponte p/ `C26->C29`, ponte p/ `D29->D32`) para distribuir a queda de tensão física e impedir centelhamento entre furos adjacentes.
5. **Diodo `1N4148` em Antiparalelo no `H11L1M`:** Protege o LED infravermelho interno do optoacoplador contra tensões negativas de oscilação (*ringing*) da bobina. O Cátodo (listra) do `1N4148` fica em ponte com o Pino 1 (Ânodo) do `H11L1M`, e o Ânodo (liso) fica em ponte com o Pino 2 (Cátodo).
6. **Orientação dos Optoacopladores `PC817` (DIP-4):** Localize a marca circular (bolinha) escavada no topo do plástico:
   * **Pino 1 (Na bolinha):** Ânodo do LED interno.
   * **Pino 2 (Abaixo do Pino 1):** Cátodo do LED interno.
   * **Pino 3 (Do outro lado, embaixo):** Emissor do fototransistor (vai para o `LGND`).
   * **Pino 4 (Acima do Pino 3):** Coletor do fototransistor (vai para o GPIO do ESP32-S3 com pull-up para `3.3V_ESP`).

---

## 5. REQUISITOS OBRIGATÓRIOS DE FIRMWARE (C++)
1. **Ratiometria Absoluta em Volts:**
   * O código C++ **NUNCA** deve dividir os valores *raw* (contagens brutas de 16 bits) de canais configurados com ganhos PGA diferentes.
   * `A0` (Óleo) e `A3` (Alternador) operam em `GAIN_TWOTHIRDS` (`0.1875 mV/bit`). `A1` (Água) e `A2` (Monitor `5V_D`) operam em `GAIN_ONE` (`0.125 mV/bit`).
   * O firmware deve converter todas as leituras para **Volts reais** antes de aplicar qualquer equação ratiométrica.
2. **Histerese Temporal do Alternador (Canal A3):**
   * Estado `ALTERNADOR OK` se `V > 13.5V` ininterruptamente por `2 segundos`.
   * Estado `BATERIA / ALERTA` se `V < 12.8V` ininterruptamente por `2 segundos`.
   * **Zona Morta (`12.8V` a `13.5V`):** O firmware deve **manter o último estado válido** registrado até que um dos limiares seja cruzado pelo tempo mínimo exigido.
3. **Execução Não-Bloqueante do Áudio I2S:**
   * A reprodução de alertas falados ou tons via DMA I2S (`MAX98357A`) deve rodar em uma *Task* dedicada do FreeRTOS (alocada no Core secundário), garantindo latência zero na interrupção de captura de RPM (`GPIO 4`), na amostragem I2C do `ADS1115` e na atualização gráfica do Display SPI / RealDash.
