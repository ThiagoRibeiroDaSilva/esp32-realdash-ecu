# 🗺️ CÓDIGO MATRIX (V8.0 - Real Board 55x37 & Audio I2S Edition)

Esta Netlist reflete o mapeamento exato furo a furo da placa física definitiva, com todas as colisões espaciais eliminadas e separação estrita entre domínios de potência (PGND / 5V_D) e domínios de instrumentação limpa (LGND / 5V_A / 3.3V_ESP).

## LEGENDA E LIMITES FÍSICOS
* **Matriz da Placa:** 55 Linhas (1 a 55) x 37 Colunas (A a Z, seguidas de AA a AK).
* **Orientação:** Lado da solda/trilhas como referência de interconexão.
* **Polaridade de Diodos:** `[LISTRA]` = Cátodo | `[LISO]` = Ânodo.
* **[PONTE]:** Dobrar a própria perna do componente por baixo da placa até encostar no furo vizinho e unir com estanho.
* **[JUMPER]:** Fio flexível isolado (encapado) conectando dois furos distantes.

---

## PASSO 1: AS AVENIDAS DE ENERGIA E O NÓ ESTRELA
* **PGND (Terra de Potência / Sujo):** Fio de cobre nu unindo a **Linha 2** (`A2` ao `AJ2`).
* **3.3V_ESP (Barramento Lógico):** Fio de cobre nu unindo a **Linha 50** (`A50` ao `AB50`). *(Já soldado de A a V; estender até AB)*.
* **5V_A (Barramento Analógico Limpo):** Fio de cobre nu unindo a **Linha 52** (`A52` ao `AB52`). *(Já soldado de A a V; estender até AB)*.
* **LGND (Terra Lógico/Analógico Limpo):** Fio de cobre nu unindo a **Linha 54** (`A54` ao `AB54`). *(Já soldado de A a V; estender até AB)*.
* **NÓ ESTRELA (União Única de Terras):** `[JUMPER]` de fio AWG 18/22 unindo a extremidade do `PGND` no furo **`AB2`** até a extremidade do `LGND` no furo **`AB54`**.

---

## PASSO 2: FRONT-END 12V E PROTEÇÃO REVERSA (CONCLUÍDO NA PLACA)
* **Entrada +12V (Nó FRONT):** Fio vindo do Porta-Fusível (2A) entra no **`A4`**. Trilha de estanho/cobre nu unindo de **`A4` até `F4`**.
* **TVS TP6KE22A:** `[LISTRA]` no **`C4`** `[PONTE p/ trilha FRONT]`. `[LISO]` no **`C7`**. `[JUMPER]` do **`C7`** p/ `PGND` (**`C2`**).
* **P-MOSFET IRF9540N (TO-220):** Gate no **`E9`**, Drain no **`F9`**, Source no **`G9`**.
* **Alimentação Source:** `[JUMPER]` flexível do **`G9`** (Source) p/ trilha FRONT (**`F4`**).
* **Resistor 1M Ohm (Gate-Source):** Pernas no **`D9`** e **`D12`**. `[PONTE de D9 p/ E9 (Gate)]`. `[JUMPER flexível do D12 p/ G9 (Source)]`.
* **Resistor 100k Ohm (Gate-PGND):** Pernas no **`E10`** e **`E12`**. `[PONTE de E10 p/ E9 (Gate)]`. `[JUMPER flexível do E12 p/ PGND (E2)]`.
* **Diodo Zener 15V (1N4744A):** `[LISTRA]` no **`H9`** `[PONTE p/ G9 (Source)]`. `[LISO]` no **`H12`** `[PONTE ou JUMPER curto p/ E12 (Gate)]`.
* **Saída Protegida:** `[JUMPER]` flexível do **`F9`** (Drain) p/ furo **`A13`**.
* **Nó VBAT_PROT:** Trilha de estanho/cobre nu unindo de **`A13` até `M13`**.
* **Filtros do Barramento VBAT_PROT:**
  * **Cap Eletrolítico 1000µF/50V (substituto do 470µF):** Positivo no **`I13`**, Negativo no **`I16`** `[JUMPER p/ PGND (I2)]`.
  * **Cap Cerâmico 1µF/50V (Código 105):** Pernas no **`L13`** e **`L16`** `[JUMPER de L16 p/ PGND (L2)]`.
  * **Cap Cerâmico 100nF/50V (Código 104):** Pernas no **`M13`** e **`M16`** `[JUMPER de M16 p/ PGND (M2)]`.

---

## PASSO 3: MÓDULOS DE ENERGIA, FILTRO 5V_A E ESP32-S3
* **Módulo Step-Down LM2596:** Posicionado nos furos **`P2`** (IN-), **`P9`** (IN+), **`AF2`** (OUT-) e **`AF9`** (OUT+). *(Já soldado)*.
  * **Entrada Positiva (IN+):** `[JUMPER 2]` de **`M14`** `[PONTE p/ M13 (VBAT_PROT)]` até **`P10`** `[PONTE p/ P9 (IN+)]`. *(Já soldado)*.
  * **Entrada Negativa (IN-):** `[PONTE]` direta do **`P2`** na trilha `PGND` (Linha 2).
  * **Saída Negativa (OUT-):** `[PONTE]` direta do **`AF2`** na trilha `PGND` (Linha 2).
  * **Saída Positiva (OUT+ / 5V_D):** `[JUMPER 1]` de **`AF10`** `[PONTE p/ AF9 (OUT+)]` até **`A16`**. *(Já soldado)*.
* **Nó 5V_D (Carga Pesada Digital):** Trilha de estanho/cobre nu unindo de **`A16` até `V16`**.
* **Filtro Passa-Baixa do Barramento 5V_A (CONCLUÍDO NA PLACA):**
  * `[JUMPER 3]` de **`O16`** (`5V_D`) até **`O51`**.
  * **Resistor 10 Ohms:** Pernas em **`O51`** e **`O52`** (Avenida `5V_A`).
  * **Cap Eletrolítico 47µF:** Positivo em **`Q52`** (`5V_A`), Negativo em **`Q54`** (`LGND`).
  * **Cap Cerâmico 100nF (104):** Pernas em **`R52`** (`5V_A`) e **`R54`** (`LGND`).
* **LDO MCP1700-3302 (3.3V Exclusivo GPS — CONCLUÍDO NA PLACA):**
  * **Pinos do MCP1700:** Pino 1 (GND) em **`U40`**, Pino 2 (VIN) em **`U41`**, Pino 3 (VOUT) em **`U42`**.
  * **Cap Cerâmico 1µF (105) de Entrada:** Pernas em **`T39`** e **`T41`**. `[PONTE de T41 p/ U41 (VIN)]` e `[PONTE de T39 p/ U40 (GND)]`.
  * **Cap Cerâmico 1µF (105) de Saída:** Pernas em **`V40`** e **`V42`**. `[PONTE de V42 p/ U42 (VOUT)]` e `[PONTE de V40 p/ U40 (GND)]`.
  * **Alimentação e Terra do LDO:** `[JUMPER]` de **`U40`** (GND) p/ `LGND` (**`U54`**). `[JUMPER]` de **`U41`** (VIN) p/ `5V_D` (**`U16`**).
  * **Nó 3.3V_GPS:** O furo **`V42`** (já unido em ponte ao **`U42`**) é o ponto de saída para alimentar o GPS.
* **ESP32-S3 DevKitC-1 (44 Pinos — 22 por fileira):**
  * **Fileira Esquerda na Coluna P (`P20` a `P41`):**
    * `P20`=3V3 | `P21`=3V3 | `P22`=RST | `P23`=GPIO4 | `P24`=GPIO5 | `P25`=GPIO6 | `P26`=GPIO7 | `P27`=GPIO15 | `P28`=GPIO16 | `P29`=GPIO17 | `P30`=GPIO18 | `P31`=GPIO8 | `P32`=GPIO3 | `P33`=GPIO46 | `P34`=GPIO9 | `P35`=GPIO10 | `P36`=GPIO11 | `P37`=GPIO12 | `P38`=GPIO13 | `P39`=GPIO14 | `P40`=5V(VIN) | `P41`=GND.
  * **Fileira Direita na Coluna Z (`Z20` a `Z41`):**
    * `Z20`=GND | `Z21`=TX | `Z22`=RX | `Z23`=GPIO1 | `Z24`=GPIO2 | `Z25`=GPIO42 | `Z26`=GPIO41 | `Z27`=GPIO40 | `Z28`=GPIO39 | `Z29`=GPIO38 | `Z30`=GPIO37 | `Z31`=GPIO36 | `Z32`=GPIO35 | `Z33`=GPIO0 | `Z34`=GPIO45 | `Z35`=GPIO48 | `Z36`=GPIO47 | `Z37`=GPIO21 | `Z38`=GPIO20 | `Z39`=GPIO19 | `Z40`=GND | `Z41`=GND.
  * **Ligações de Energia do ESP32-S3:**
    * `[JUMPER]` flexível de **`V16`** (`5V_D`) p/ Pino 5V do ESP (**`P40`**).
    * `[JUMPER]` flexível de **`P54`** (`LGND`) p/ Pino GND do ESP (**`P41`**).
    * `[JUMPER]` flexível do Pino 3V3 do ESP (**`P20`**) p/ Avenida `3.3V_ESP` (**`P50`**).

---

## PASSO 4: BARRAMENTO I2C (ADS1115 E LEVEL SHIFTER BSS138)
* **Módulo ADS1115:** Pinos na **Coluna AB (`AB30` a `AB39`)**.
  * `AB30` (VDD): `[JUMPER]` p/ Avenida `5V_A` (**`AB52`**).
  * `AB31` (GND): `[JUMPER]` p/ Avenida `LGND` (**`AB54`**).
  * `AB32` (SCL) e `AB33` (SDA).
  * `AB34` (ADDR): `[JUMPER curto ou PONTE]` p/ **`AB31`** (GND — fixa endereço I2C em `0x48`).
  * `AB35` (ALRT): Deixar sem conexão.
  * Entradas Analógicas: `AB36` = **A0** | `AB37` = **A1** | `AB38` = **A2** | `AB39` = **A3**.
* **Módulo Level Shifter BSS138:** Posicionado ao lado do ADS1115 (**Coluna AA**, linhas 30 a 38).
  * **Lado HV (5V):** Pino `HV` (**`AA30`**) `[JUMPER p/ 5V_A (AA52)]`. Pino `GND-HV` `[JUMPER p/ LGND (AA54)]`. Pinos `HV1` e `HV2` unidos por `[PONTE]` direta aos pinos `SCL` (**`AB32`**) e `SDA` (**`AB33`**) do ADS1115.
  * **Lado LV (3.3V):** Pino `LV` (**`AA35`**) `[JUMPER p/ 3.3V_ESP (AA50)]`. Pino `GND-LV` `[JUMPER p/ LGND (AA54)]`.
  * **Linhas I2C p/ ESP32-S3:**
    * `LV1` (SCL 3.3V): `[JUMPER]` flexível p/ **GPIO 9** do ESP (**`P34`**).
    * `LV2` (SDA 3.3V): `[JUMPER]` flexível p/ **GPIO 8** do ESP (**`P31`**).

---

## PASSO 5: CONDICIONAMENTO DE RPM (MURALHA 6.8K E OPTO H11L1M)
* **Muralha de Resistores (4x 6.8k Ohms / 0.5W Filme Metálico em Zig-Zag):**
  * Fio do Negativo da Bobina entra no furo **`A20`**.
  * **R1 (6.8k):** Pernas em **`A20`** e **`A23`**. `[PONTE de A23 p/ B23]`.
  * **R2 (6.8k):** Pernas em **`B23`** e **`B26`**. `[PONTE de B26 p/ C26]`.
  * **R3 (6.8k):** Pernas em **`C26`** e **`C29`**. `[PONTE de C29 p/ D29]`.
  * **R4 (6.8k):** Pernas em **`D29`** e **`D32`** (Nó `RPM_IN`).
* **Diodo Zener 5.1V (1N5231B):** `[LISTRA]` em **`D32`** `[PONTE c/ R4]`. `[LISO]` em **`D30`**. `[JUMPER]` de **`D30`** p/ `PGND` (**`D2`**).
* **Resistor 330 Ohms:** Pernas em **`D32`** `[PONTE c/ R4 e Zener]` e **`F32`**.
* **Optoacoplador Schmitt-Trigger H11L1M (DIP-6):**
  * Pinos 1, 2, 3 na **Coluna G** (`G32`, `G33`, `G34`). Pinos 6, 5, 4 na **Coluna J** (`J32`, `J33`, `J34`).
  * **Pino 1 (Ânodo — `G32`):** `[PONTE]` p/ **`F32`** (Resistor de 330R).
  * **Pino 2 (Cátodo — `G33`):** `[JUMPER]` p/ `PGND` (**`G2`**).
  * **Diodo 1N4148 (Antiparalelo):** `[LISTRA]` em **`H32`** `[PONTE p/ G32]`. `[LISO]` em **`H33`** `[PONTE p/ G33]`.
  * **Pino 6 (VCC — `J32`):** `[JUMPER]` p/ Avenida `3.3V_ESP` (**`J50`**).
  * **Pino 5 (GND — `J33`):** `[JUMPER]` p/ Avenida `LGND` (**`J54`**).
  * **Cap 100nF (Desacoplamento):** Pernas em **`I32`** e **`I33`** (`[PONTE de I32 p/ J32]` e `[PONTE de I33 p/ J33]`). *(Mantém o furo K32 livre para o Passo 8)*.
  * **Pino 4 (VO Saída — `J34`):** `[JUMPER]` flexível p/ **GPIO 4** do ESP (**`P23`**).
  * **Resistor 4.7k Ohms (Pull-up RPM):** Pernas em **`I34`** `[PONTE p/ J34]` e **`I31`**. `[JUMPER]` de **`I31`** p/ `3.3V_ESP` (**`I50`**).

---

## PASSO 6: SENSORES ANALÓGICOS DE BLOCO (A0 ÓLEO E A1 ÁGUA NTC)
* **Canal A0 (Pressão de Óleo — Linha 44, Colunas A a D):**
  * Alimentação do Transdutor: Positivo no barramento **`5V_D`** (`A16`); GND retorna por fio dedicado direto ao **`LGND`** (`A54`).
  * Fio de Sinal do Óleo entra no furo **`A44`**.
  * **Resistor 2.2k Ohms (comum):** Pernas em **`A44`** e **`B44`** (Nó `A0_FILT`).
  * **Cap Cerâmico 100nF (104):** Pernas em **`B44`** `[PONTE c/ R 2.2k]` e **`B54`** (Avenida `LGND`).
  * **Diodo SD103 Alto:** `[LISO]` em **`C44`** `[PONTE p/ B44]`. `[LISTRA]` em **`C47`** `[JUMPER p/ 5V_D (C16)]`.
  * **Diodo SD103 Baixo:** `[LISTRA]` em **`D44`** `[PONTE p/ C44/B44]`. `[LISO]` em **`D54`** (Avenida `LGND`).
  * **Saída p/ ADC:** `[JUMPER]` flexível de **`B44`** p/ Pino **A0** do ADS1115 (**`AB36`**).
* **Canal A1 (Temperatura da Água NTC — Linha 44, Colunas F a I):**
  * **Resistor 2.2k Ohms (OBRIGATÓRIO 1% — Pull-up):** Pernas na Avenida `5V_A` (**`F52`**) e no furo **`F44`** (Nó `NTC_RAW`).
  * Fio de Sinal do Sensor NTC entra em **`F44`** (Fio de retorno do sensor vai direto ao `LGND` em **`F54`**).
  * **Resistor 2.2k Ohms (comum — Filtro):** Pernas em **`F44`** e **`G44`** (Nó `A1_FILT`).
  * **Cap Cerâmico 1µF X7R (Código 105):** Pernas em **`G44`** e **`G54`** (Avenida `LGND`).
  * **Diodo SD103 Alto:** `[LISO]` em **`H44`** `[PONTE p/ G44]`. `[LISTRA]` em **`H52`** (Avenida `5V_A`).
  * **Diodo SD103 Baixo:** `[LISTRA]` em **`I44`** `[PONTE p/ H44/G44]`. `[LISO]` em **`I54`** (Avenida `LGND`).
  * **Saída p/ ADC:** `[JUMPER]` flexível de **`G44`** p/ Pino **A1** do ADS1115 (**`AB37`**).

---

## PASSO 7: DIVISORES DE MONITORAMENTO INTERNO (A2 5V_D E A3 ALTERNADOR)
* **Canal A2 (Monitor do Barramento 5V_D — Colunas J e K):**
  * **Resistor 10k Ohms (1% — Superior):** Pernas em **`J41`** `[JUMPER p/ 5V_D (J16)]` e **`J44`** (Nó `A2_DIV`).
  * **Resistor 10k Ohms (1% — Inferior):** Pernas em **`J44`** e **`J54`** (Avenida `LGND`).
  * **Cap Cerâmico 100nF (104):** Pernas em **`K44`** `[PONTE p/ J44]` e **`K54`** (Avenida `LGND`).
  * **Resistor 1k Ohm:** Pernas em **`K44`** e **`K41`**. `[JUMPER]` flexível de **`K41`** p/ Pino **A2** do ADS1115 (**`AB38`**).
* **Canal A3 (Monitor do Alternador / Bateria — Colunas L, M e N):**
  * **Resistor 33k Ohms (1% — Superior):** Pernas em **`L41`** `[JUMPER p/ VBAT_PROT (B13)]` e **`L44`** (Nó `A3_DIV`).
  * **Resistor 4.7k Ohms (1% — Inferior):** Pernas em **`L44`** e **`L54`** (Avenida `LGND`).
  * **Diodo SD103 Alto:** `[LISO]` em **`M44`** `[PONTE p/ L44]`. `[LISTRA]` em **`M52`** (Avenida `5V_A`).
  * **Cap Cerâmico 100nF (104):** Pernas em **`M44`** e **`M54`** (Avenida `LGND`).
  * **Diodo SD103 Baixo:** `[LISTRA]` em **`N44`** `[PONTE p/ M44/L44]`. `[LISO]` em **`N54`** (Avenida `LGND`).
  * **Resistor 1k Ohm:** Pernas em **`N44`** e **`N41`**. `[JUMPER]` flexível de **`N41`** p/ Pino **A3** do ADS1115 (**`AB39`**).

---

## PASSO 8: COMBUSTÍVEL E REFERÊNCIA DE PAINEL (BUFFER OP-AMP MCP6002)
* **CI Op-Amp MCP6002-I/P (DIP-8):**
  * Pinos 1 a 4 na **Coluna L** (`L32`=Pino 1, `L33`=Pino 2, `L34`=Pino 3, `L35`=Pino 4).
  * Pinos 8 a 5 na **Coluna O** (`O32`=Pino 8, `O33`=Pino 7, `O34`=Pino 6, `O35`=Pino 5).
  * **Alimentação:** Pino 8 (**`O32`**) `[JUMPER p/ 3.3V_ESP (O50)]`. Pino 4 (**`L35`**) `[JUMPER p/ LGND (L54)]`.
  * **Desacoplamento:** Cap 100nF (104) entre **`M32`** `[PONTE p/ O32]` e **`M35`** `[PONTE p/ L35]`.
* **Circuito A (Sinal da Boia — Linha 34):**
  * Fio da Boia entra em **`A34`**.
  * **Resistor 1M Ohm:** Pernas em **`A34`** e **`B34`** (Nó `DIV_A`). `[JUMPER]` de **`B34`** p/ Pino 3 do MCP6002 (**`L34`**).
  * **Resistor 220k Ohms:** Pernas em **`B34`** e **`B37`** `[JUMPER de B37 p/ LGND (B54)]`.
  * **Cap Poliéster/Cerâmico 470nF (474):** Pernas em **`C34`** `[PONTE p/ B34]` e **`C37`** `[PONTE p/ B37 (LGND)]`.
  * **Diodo SD103 Alto:** `[LISO]` em **`D34`** `[PONTE p/ C34/B34]`. `[LISTRA]` em **`D37`** `[JUMPER p/ 3.3V_ESP (D50)]`.
  * **Diodo SD103 Baixo:** `[LISTRA]` em **`E34`** `[PONTE p/ D34/B34]`. `[LISO]` em **`E37`** `[PONTE p/ C37/B37 (LGND)]`.
  * **Malha Buffer A e Saída:** `[PONTE]` unindo Pino 1 (**`L32`**) ao Pino 2 (**`L33`**).
  * **Resistor 100 Ohms:** Pernas em **`L32`** e **`K32`**.
  * **Cap 100nF (104):** Pernas em **`K32`** e **`K35`** `[JUMPER de K35 p/ LGND (K54)]`.
  * `[JUMPER]` flexível de **`K32`** p/ **GPIO 5** do ESP (**`P24`**).
* **Circuito B (Feed Positivo do Relógio Original do Painel — Linha 39):**
  * Fio Positivo do Relógio do Painel entra em **`A39`**.
  * **Resistor 1M Ohm:** Pernas em **`A39`** e **`B39`** (Nó `DIV_B`). `[JUMPER]` de **`B39`** p/ Pino 5 do MCP6002 (**`O35`**).
  * **Resistor 220k Ohms:** Pernas em **`B39`** e **`B42`** `[JUMPER de B42 p/ LGND (B54)]`.
  * **Cap Poliéster/Cerâmico 470nF (474):** Pernas em **`C39`** `[PONTE p/ B39]` e **`C42`** `[PONTE p/ B42 (LGND)]`.
  * **Diodo SD103 Alto:** `[LISO]` em **`D39`** `[PONTE p/ C39/B39]`. `[LISTRA]` em **`D42`** `[JUMPER p/ 3.3V_ESP (D50)]`.
  * **Diodo SD103 Baixo:** `[LISTRA]` em **`E39`** `[PONTE p/ D39/B39]`. `[LISO]` em **`E42`** `[PONTE p/ C42/B42 (LGND)]`.
  * **Malha Buffer B e Saída (Vertical na Coluna N para não colidir com a Coluna P):**
    * `[PONTE]` unindo Pino 7 (**`O33`**) ao Pino 6 (**`O34`**).
    * **Resistor 100 Ohms:** Pernas em **`N33`** `[PONTE p/ O33]` e **`N36`**.
    * **Cap 100nF (104):** Pernas em **`N36`** e **`N38`** `[JUMPER de N38 p/ LGND (N54)]`.
    * `[JUMPER]` flexível de **`N36`** p/ **GPIO 6** do ESP (**`P25`**).

---

## PASSO 9: ALERTAS ISOLADOS POR OPTOACOPLADOR (3x PC817)
*(Posicionados nas Colunas K e N, Linhas 4 a 11 — totalmente fora da Linha 13 VBAT_PROT)*

* **IC1 — Alerta de Temperatura Alta / Interruptor Água (Linhas 4 e 5):**
  * Fio `12V Pós-Chave (ACC)` entra em **`I4`**. **Resistor 2.2k Ohms** de **`I4`** p/ Pino 1 (**`K4`**).
  * Fio do Interruptor Térmico (Terra no motor) entra no Pino 2 (**`K5`**).
  * Pino 3 (Emissor — **`N5`**): `[JUMPER]` p/ Avenida `LGND` (**`N54`**).
  * Pino 4 (Coletor — **`N4`**): `[JUMPER]` p/ **GPIO 15** do ESP (**`P27`**).
  * **Resistor 10k Ohms (Pull-up):** Pernas em **`N4`** e **`O4`** `[JUMPER de O4 p/ 3.3V_ESP (P50)]`.
* **IC2 — Alerta de Freio de Mão / Fluido (Linhas 7 e 8):**
  * `[PONTE]` de `12V ACC` de **`I4`** p/ **`I7`**. **Resistor 2.2k Ohms** de **`I7`** p/ Pino 1 (**`K7`**).
  * Fio do Interruptor de Freio entra no Pino 2 (**`K8`**).
  * Pino 3 (Emissor — **`N8`**): `[JUMPER]` p/ Avenida `LGND` (**`N54`**).
  * Pino 4 (Coletor — **`N7`**): `[JUMPER]` p/ **GPIO 16** do ESP (**`P28`**).
  * **Resistor 10k Ohms (Pull-up):** Pernas em **`N7`** e **`O7`** `[PONTE p/ O4 (3.3V_ESP)]`.
* **IC3 — Sensor Capacitivo de Nível do Radiador XKC-Y25-NPN (Linhas 10 e 11):**
  * **Pads do Chicote do Sensor:**
    * **Fio Marrom (VCC 5V):** Soldar direto na trilha `5V_D` no furo **`H16`**.
    * **Fio Azul (GND) + Fio Preto (Modo):** Soldar juntos no furo **`I11`** `[JUMPER p/ PGND (I2)]`.
    * **Fio Amarelo (Sinal OUT):** Soldar no furo **`I10`**.
  * **Resistor 2.2k Ohms:** Pernas em **`I10`** (Sinal) e Pino 1 do `IC3` (**`K10`**).
  * Pino 2 (Cátodo — **`K11`**): `[JUMPER]` p/ `PGND` (**`K2`**).
  * Pino 3 (Emissor — **`N11`**): `[JUMPER]` p/ Avenida `LGND` (**`N54`**).
  * Pino 4 (Coletor — **`N10`**): `[JUMPER]` p/ **GPIO 7** do ESP (**`P26`**).
  * **Resistor 10k Ohms (Pull-up):** Pernas em **`N10`** e **`O10`** `[PONTE p/ O7/O4 (3.3V_ESP)]`.

---

## PASSO 10: DISPLAY TFT, BACKLIGHT (IRLZ44N) E PADS DO GPS (KEYSTONE RJ45)
* **Driver de Backlight IRLZ44N (Colunas AH, AI e AJ — Livre do LM2596):**
  * Pinos do IRLZ44N (TO-220): Gate em **`AH5`**, Drain em **`AI5`**, Source em **`AJ5`**.
  * **Source (`AJ5`):** `[JUMPER]` p/ `PGND` (**`AJ2`**).
  * **Resistor 100k Ohms (Pull-down):** Pernas em **`AH5`** (Gate) e **`AH2`** (`PGND`).
  * **Resistor 100 Ohms (Gate Drive):** Pernas em **`AH5`** e **`AH8`**. `[JUMPER]` flexível de **`AH8`** p/ **GPIO 13** do ESP (**`P38`**).
  * **Drain (`AI5`):** Soldar o fio `BLK` do Display TFT.
* **Barramento SPI do Display TFT 4.0" ST7796S:**
  * `VCC` no **`5V_D`** (`V16`) | `GND` no **`LGND`** (`V54`).
  * `CS` -> **GPIO 10** (**`P35`**) | `MOSI` -> **GPIO 11** (**`P36`**) | `SCK` -> **GPIO 12** (**`P37`**) | `DC` -> **GPIO 14** (**`P39`**) | `RST` -> **GPIO 21** (**`Z37`**).
* **Pads do GPS NEO-M8N (Chicote Keystone RJ45 — Linha 18):**
  * **`P18` (VCC RJ45 Pino 1/2):** `[JUMPER]` flexível p/ Nó `3.3V_GPS` (**`V42`**).
  * **`Q18` (GND RJ45 Pino 3/6):** `[JUMPER]` flexível p/ Avenida `LGND` (**`Q54`**).
  * **`R18` (TX ESP -> RX GPS Pino 4):** `[JUMPER]` p/ **GPIO 17** do ESP (**`P29`**).
  * **`S18` (RX ESP <- TX GPS Pino 5):** `[JUMPER]` p/ **GPIO 18** do ESP (**`P30`**).

---

## PASSO 11: SÍNTESE DE VOZ E ALERTAS SONOROS (DAC/AMP I2S MAX98357A)
*(Posicionado na Coluna AE, Linhas 20 a 26 — Isolado no domínio de potência 5V_D / PGND)*

* **Módulo MAX98357A (7 Pinos na Coluna AE):**
  * **`AE20` (LRC / WS):** `[JUMPER]` flexível p/ **GPIO 41** do ESP (**`Z26`**).
  * **`AE21` (BCLK):** `[JUMPER]` flexível p/ **GPIO 40** do ESP (**`Z27`**).
  * **`AE22` (DIN):** `[JUMPER]` flexível p/ **GPIO 42** do ESP (**`Z25`**).
  * **`AE23` (GAIN):** Deixar desconectado (Ganho padrão de 9dB) ou unir via `[PONTE]` ao `GND` (`AE25`) para 12dB.
  * **`AE24` (SD / Shutdown):** Deixar desconectado (Pull-up interno mantém o amplificador ativo).
  * **`AE25` (GND):** `[JUMPER]` exclusivo p/ Avenida **`PGND`** (**`AE2`**). *(PROIBIDO ligar ao LGND)*.
  * **`AE26` (VIN):** `[JUMPER]` exclusivo p/ o barramento **`5V_D`** (**`V16`** ou **`AF10`**). *(PROIBIDO ligar ao 5V_A)*.
* **Alto-Falante Passivo (4 Ohms / 3W):**
  * Cortar o conector JST de 1.25mm, decapar os fios vermelho (`+`) e preto (`-`) e soldá-los diretamente nos dois furos de saída de áudio (`Speaker +` e `Speaker -`) no topo do módulo `MAX98357A`.
