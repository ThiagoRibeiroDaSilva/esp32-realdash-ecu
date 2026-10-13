# 📦 BOM FÍSICO DEFINITIVO — ECU CHEVETTE (V22.0 / Matrix V8.0)

Lista consolidada de materiais (Bill of Materials) validada para a matriz real de **55 linhas x 37 colunas (Colunas A a AK)**, incluindo o sensor capacitivo do radiador e o subsistema de voz/alertas via barramento digital I2S.

---

## [A] MÓDULOS, CÉREBRO E TRANSDUTORES EXTERNOS (Proibido substituir modelos)
* **1x Placa ESP32-S3 DevKitC-1** (Versão WROOM-1, 44 pinos, pinos pré-soldados).
* **1x Módulo Display TFT 4.0" SPI ST7796S** (OBRIGATÓRIO: Pino BLK Lógico com driver interno. Proibido LED direto).
* **1x Módulo GPS NEO-M8N** (OBRIGATÓRIO: Versão operando estritamente em 3.3V no pino VCC).
* **1x Módulo A/D ADS1115** (Com pull-ups SMD de 10k integrados. Endereço fixo `0x48`).
* **1x Módulo Level Shifter I2C BSS138** (4 canais, com pull-ups integrados).
* **1x Módulo Step-Down LM2596** (Buck Converter DC-DC ajustado para 5.0V).
* **1x Módulo DAC e Amplificador I2S MAX98357A** (Saída Classe D de 3W p/ síntese de voz e alertas sonoros).
* **1x Alto-Falante Passivo 4 Ohms / 3W** (Dimensões compactas 35x25mm — cortar conector JST 1.25mm para solda direta no MAX98357A).
* **1x Sensor Capacitivo de Nível Sem Contato XKC-Y25-NPN** (Operação em 5V via barramento `5V_D`, saída NPN p/ nível do radiador).

---

## [B] SEMICONDUTORES (PTH / DIP)
* **1x CI Op-Amp MCP6002-I/P** (DIP-8, Dual Rail-to-Rail p/ Buffer de Combustível e Referência do Painel).
* **1x LDO Regulador MCP1700-3302** (TO-92, saída 3.3V exclusiva p/ o GPS).
* **1x Optoacoplador Schmitt-Trigger H11L1M** (DIP-6, captura digital rápida de RPM).
* **3x Optoacopladores PC817** (DIP-4: `IC1` Alerta Temperatura Água, `IC2` Alerta Freio e `IC3` Nível do Radiador).
* **1x P-MOSFET IRF9540N** (TO-220, -100V, proteção contra inversão de polaridade na entrada 12V).
* **1x N-MOSFET Logic-Level IRLZ44N** (TO-220, chaveamento PWM do Backlight do Display).

---

## [C] DIODOS E PROTEÇÕES
* **1x Diodo TVS Unidirecional TP6KE22A** (DO-201, AEC-Q101, proteção contra Load Dump na linha 12V).
* **10x Diodos Schottky SD103B ou SD103C** (DO-35 Axial — 2x para cada uma das 5 linhas analógicas: A0, A1, A3, Boia A e Painel B).
* **1x Diodo Zener 15V / 1W** (`1N4744A` — Proteção VGS do Gate do IRF9540N).
* **1x Diodo Zener 5.1V / 0.5W** (`1N5231B` — Ceifador de tensão da malha de RPM).
* **1x Diodo de Sinal Rápido 1N4148** (DO-35 — Proteção antiparalela do LED interno do H11L1M).

---

## [D] CAPACITORES
* **1x Eletrolítico 1000µF / 50V** *(Instalado em substituição direta ao de 470µF / 50V no barramento `VBAT_PROT`)*.
* **1x Eletrolítico 47µF / 16V (ou 25V)** (Filtro passa-baixa da avenida analógica `5V_A`).
* **1x Cerâmico 1µF / 50V (Código 105)** (Desacoplamento de média frequência no barramento `VBAT_PROT`).
* **1x Cerâmico 1µF / 16V (Código 105 — OBRIGATÓRIO: Dielétrico X7R)** (Filtro térmico estável do sensor NTC da água em A1).
* **2x Cerâmicos 1µF / 16V (Código 105 — Comuns)** (Estabilização de entrada e saída do regulador LDO MCP1700).
* **2x Filme de Poliéster / Cerâmicos 470nF (Código 474 — ex: 474K100V)** (Filtro de amortecimento de 0.34Hz p/ Boia e Painel no Bairro 6).
* **10x Cerâmicos 100nF / 50V (Código 104):**
  * `1x` Barramento `VBAT_PROT` (`M13-M16`)
  * `1x` Barramento `5V_A` (`R52-R54`)
  * `1x` Desacoplamento VCC do `H11L1M` (`I32-I33`)
  * `1x` Filtro Canal A0 Óleo (`B44-B54`)
  * `1x` Filtro Canal A2 Monitor 5V_D (`K44-K54`)
  * `1x` Filtro Canal A3 Alternador (`M44-M54`)
  * `1x` Desacoplamento VCC do `MCP6002` (`M32-M35`)
  * `1x` Filtro de Saída ADC Boia — Circuito A (`K32-K35`)
  * `1x` Filtro de Saída ADC Painel — Circuito B (`N36-N38`)
  * `1x` Reserva técnica na bancada.

---

## [E] RESISTORES (1/4W PTH comum, EXCETO onde especificado)
* **4x 6.8k Ohms / 0.5W (OBRIGATÓRIO: Filme Metálico / Metal Film)** (Muralha de dissipação em série p/ pulsos de alta tensão da bobina de ignição).
* **1x 10 Ohms** (Elo resistivo do filtro RC da linha `5V_A`).
* **3x 1M Ohm** (`1x` Pull-up Gate-Source do IRF9540N; `2x` Entradas de alta impedância dos Circuitos A e B do MCP6002).
* **2x 220k Ohms** (Divisores de referência p/ LGND dos Circuitos A e B do MCP6002).
* **2x 100k Ohms** (`1x` Pull-down do Gate do IRF9540N; `1x` Pull-down do Gate do IRLZ44N).
* **1x 33k Ohms (OBRIGATÓRIO: 1% de tolerância)** (Braço superior do divisor de tensão do Alternador em A3).
* **2x 10k Ohms (OBRIGATÓRIO: 1% de tolerância)** (Divisor simétrico de monitoramento do barramento `5V_D` em A2).
* **3x 10k Ohms (Comuns)** (Pull-ups lógicos de 3.3V nas saídas dos três optoacopladores `PC817`: `IC1`, `IC2` e `IC3`).
* **1x 4.7k Ohms (OBRIGATÓRIO: 1% de tolerância)** (Braço inferior do divisor de tensão do Alternador em A3).
* **1x 4.7k Ohms (Comum)** (Pull-up lógico de 3.3V na saída `VO` do optoacoplador `H11L1M` no GPIO 4).
* **1x 2.2k Ohms (OBRIGATÓRIO: 1% de tolerância)** (Pull-up de precisão ligado ao `5V_A` para o termistor NTC da água em A1).
* **5x 2.2k Ohms (Comuns)** (`1x` Filtro de entrada A0 Óleo; `1x` Filtro de entrada A1 Água; `3x` Limitadores de corrente dos LEDs dos optoacopladores `IC1`, `IC2` e `IC3`).
* **2x 1k Ohm** (Resistores de proteção em série p/ as portas A2 e A3 do ADS1115).
* **1x 330 Ohms** (Limitador de corrente de entrada do LED do optoacoplador `H11L1M`).
* **3x 100 Ohms** (`2x` Isolamento capacitivo nas saídas dos buffers A e B do MCP6002; `1x` Amortecimento de Gate do MOSFET IRLZ44N).

---

## [F] MECÂNICA, CHICOTE E CONSUMÍVEIS
* **1x Placa Perfurada Universal Dupla Face 10x15cm** (Matriz física real de **55 linhas x 37 colunas**, de `A` até `AK`).
* **1x Caixa Patola PB-114** (ou equivalente para alojamento em cabine).
* **1x Keystone RJ45 Fêmea** (Cat5e ou Cat6 para conexão dedicada do módulo GPS).
* **1x Kit Conector Automotivo Selado 16 Vias** (1x Carcaça Macho, 1x Carcaça Fêmea, 32x Terminais de crimpagem, 32x Vedações).
* **1x Porta-fusível aéreo c/ Fusível Lâmina 2A**.
* **4x Espaçadores de Nylon M3 Macho-Fêmea** (10mm corpo + 6mm rosca) + **4x Parafusos M3x6mm** + **4x Porcas M3**.
* **1x Metro de Cabo Blindado (1 via + malha) ou Par Trançado** (Exclusivo p/ captação do sinal de RPM).
* **1x Rolo (5m) de Fio AWG 18 (0.75mm²)** para barramentos de potência e entradas principais.
* **1x Rolo (10m) de Fio AWG 22 (0.30mm²)** rígido/flexível para avenidas e jumpers internos na placa.
* **1x Tubo de Cola PU (Poliuretano) ou Resina Epóxi** (Calçamento antivibração obrigatório de módulos e capacitores altos).
* **1x Lata de Spray Verniz Conformal Coating** (Selagem tropicalizada da placa após validação final).

---

## 🛒 STATUS DE AQUISIÇÃO (PENDÊNCIAS DE COMPRAS)
Todos os componentes de telemetria, condicionamento analógico, proteção e potência (incluindo os resistores de 33k 1% e os 4x 6.8k 0.5W de Filme Metálico) **já foram adquiridos**.

**Única pendência atual (Subsistema de Áudio I2S — Passo 11):**
* [ ] **1x Módulo DAC/Amplificador I2S MAX98357A**
* [ ] **1x Alto-Falante Passivo 4 Ohms / 3W** (35x25mm)
