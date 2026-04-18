# esp32-realdash-ecu
Dossie 2.0 - Chevette ESP32

🛠️ A LIGAÇÃO FINAL (V21.0 - The Software Bridge)
Nós vamos acatar essas 5 regras de ouro agora, porque elas formam a ponte perfeita entre a placa que você vai soldar e o código C++ que vamos escrever a seguir:
 * A Fonte do Canal B (Omissão Documental): O Cláudio notou que eu detalhei o Circuito A (Fio da Boia), mas disse apenas "repetir para o Circuito B", sem dizer de onde vinha o sinal B. Adicionada a instrução explícita de que o sinal B vem da alimentação do relógio original do painel.
 * O Limbo do Alternador (Zona Morta): Se a voltagem ficar presa entre 12.8V e 13.5V, o que a tela mostra? Vamos documentar a regra de histerese do firmware: "Manter o último estado válido até que um novo limiar seja cruzado".
 * A Matemática do Óleo (Risco de Firmware): O alerta de ouro do Cláudio. Se o código dividir os números "brutos" do ADS1115 (onde o canal do óleo tem um tamanho de "degrau" diferente do canal de referência), a conta dá errado. Regra cravada: O C++ tem que converter os sinais para Volts antes de fazer qualquer divisão.
 * O Critério de Bancada do Óleo: Adicionada a regra clara: Se a queda de tensão no resistor de 10 Ohms baixar o barramento 5V_A para menos de 4.75V, o sensor puxa corrente demais e deve voltar para o 5V_D.
 * A Lentidão da Boia (0.34Hz): O filtro analógico leva 3 segundos para atualizar. Isso foi documentado não como defeito, mas como recurso desejado para o ponteiro não ficar dançando nas curvas.
Aqui está a V21.0 (The Software Bridge). É o documento final da Engenharia Física, já contendo as diretrizes estritas para a programação.
Pode enviar ao Cláudio e ao Gepeto como a planta selada.
DOSSIE DE ENGENHARIA: ECU CHEVETTE - TELEMETRIA ESP32-S3
Versao: V21.0 (The Software Bridge - Arquitetura Selada)
Status: HARDWARE APROVADO P/ BANCADA E ROAD-TEST (ZERO FALHAS CRITICAS)
 * HARDWARE E LISTA DE MATERIAIS (PTH/DIP BOM)
 * Cerebro: ESP32-S3 DevKit. (Fixacao com PU/Epoxi obrigatoria na base).
 * Tradutor Analogico: ADS1115. (I2C: 0x48. Pull-ups internos 10k mantidos. FSR +/- 6.144V p/ Oleo/A3).
 * Ponte Logica I2C: BSS138. (Pull-ups internos mantidos).
 * Op-Amp Buffer (Combustivel): MCP6002-I/P (DIP-8).
 * Opto RPM: H11L1M (DIP-6). Pinagem: 1(Anodo), 2(Catodo), 4(VO), 5(GND), 6(VCC).
 * Optos Alerta: 2x PC817 (DIP-4).
 * Protecao Reversa: IRF9540N (TO-220, -100V) + Zener 15V.
 * Backlight: IRLZ44N (TO-220).
 * TVS Principal: TP6KE22A (AEC-Q101).
 * Clamps Analogicos: SD103B/C (DO-35 Axial).
 * Fontes: LM2596 (Buck) e MCP1700-3302 (LDO).
 * DIRETRIZES DE ATERRAMENTO E MONTAGEM
 * Dominios Fechados (Uniao unica proxima ao ADS1115):
   -- PGND (Potencia): Entrada 12V, TVS, LM2596, Source/Pulldown Backlight, Pinos 2(Catodo) dos Optos.
   -- LGND/AGND (Limpo): Pinos GND do S3, ADS1115, MCP6002, Lado isolado (Pino 5) dos Optos, Pinos GND dos sensores de bloco e GND do LDO/GPS.
 * Ambiente: VETADA a instalacao no cofre. Uso exclusivo em cabine.
 * Filtro Boia (0.34Hz): O circuito de combustivel possui resposta intencionalmente lenta (~3s) para atuar como amortecedor mecanico contra oscilacoes do tanque em curvas.
 * A BATALHA NAVAL 39x59 (TOPOLOGIA FINAL DETALHADA)
BAIRRO 1: Front-End 12V
 * +12V -> Fusivel 2A -> No FRONT.
 * TVS TP6KE22A: Catodo no No FRONT, Anodo no PGND. (Trilhas curtissimas).
 * Protecao Reversa: No FRONT -> Source IRF9540N. Drain -> No VBAT_PROT.
 * Gate IRF9540N: R 1M Ohm (Gate p/ Source) + R 100k (Gate p/ PGND). Zener 15V (Catodo no Source, Anodo no Gate).
 * VBAT_PROT: Eletrolitico 470uF/50V + Cap 1uF/50V + Cap 100nF/50V p/ PGND. (Alimenta LM2596).
BAIRRO 2: A Distribuicao de Energia
 * 5V_D (Carga Pesada): Saida LM2596. Alimenta ESP32(Vin) e Backlight.
 * 5V_A (Analogico Limpo): Saida 5V_D -> R 10 Ohms -> No 5V_A. Cap 47uF + Cap 100nF p/ LGND/AGND. (Alimenta EXCLUSIVAMENTE Sensor de Oleo, ADS1115, Lado HV do BSS138, Pull-up do NTC e Divisores).
 * 3.3V_GPS (LDO): MCP1700 alimentado pelo 5V_D. (Obrigatorio: Cap 1uF IN p/ LGND, Cap 1uF OUT p/ LGND). Alimenta apenas o GPS.
 * 3.3V_ESP: Pino 3.3V nativo do DevKit. Alimenta: Lado LV do BSS138, Pull-ups Logicos e VDD do MCP6002.
BAIRRO 3: Condicionamento de RPM (Captura por Flyback)
 * Entrada Negativa Bobina -> 4x Resistores 6.8k Ohms (Filme Metalico 0.5W, espacados contra arco) -> No RPM_IN.
 * Zener 5.1V do No RPM_IN para PGND.
 * No RPM_IN -> R 330 Ohms -> Pino 1 (Anodo) H11L1M. Pino 2 (Catodo) -> PGND.
 * 1N4148 em antiparalelo: Anodo no Pino 2, Catodo no Pino 1 (Apos o R 330).
 * Lado ESP: Pino 6(VCC) em 3.3V_ESP + Cap 100nF p/ LGND. Pino 5(GND) no LGND/AGND. Pino 4(VO) -> GPIO 4 + Pull-up 4.7k p/ 3.3V_ESP.
BAIRRO 4: Analogicos de Bloco
 * A0 (Oleo): VCC no 5V_A. GND DEVE retornar via fio dedicado ao LGND/AGND. Sinal -> R 2.2k -> No A0_FILT. Diodo Alto (Anodo No, Catodo 5V_A). Diodo Baixo (Anodo LGND, Catodo No). Cap 100nF p/ LGND. No A0_FILT -> Canal A0.
 * A1 (Agua NTC): 5V_A -> R 2.2k (1%) -> No NTC_RAW. (Retorno 2 fios via LGND. Grounded-body assume offset). No NTC_RAW -> R 2.2k -> No A1_FILT. Cap 1uF X7R p/ LGND + Clamps SD103. No A1_FILT -> Canal A1.
BAIRRO 5: Monitoramento Interno e Alternador (Resistores 1%)
 * A2 (Monitor 5V_A): 5V_A -> R 10k(1%) -> No A2_DIV -> R 10k(1%) -> LGND. Cap 100nF p/ LGND. R 1k p/ Canal A2.
 * A3 (Alternador): VBAT_PROT -> R 33k(1%) -> No A3_DIV -> R 4.7k(1%) -> LGND. Cap 100nF p/ LGND. Clamps SD103 no A3_DIV (Diodo Alto no 5V_A). R 1k do No A3_DIV p/ Canal A3.
BAIRRO 6: Combustivel (Buffer MCP6002 - Conexoes Explicitas)
 * Pino 8 no 3.3V_ESP. Pino 4 no LGND/AGND. Cap 100nF no Pino 8 p/ LGND.
 * Circuito A (Boia/Sensor): Fio da Boia -> R 1M Ohm -> No DIV_A -> Pino 3 (IN+ A). No DIV_A p/ LGND: R 220k + Cap 470nF. Clamps em DIV_A (Referenciados ao 3.3V_ESP e LGND). Malha A: Pino 1 (OUT A) no Pino 2 (IN- A). Pino 1 -> R 100 Ohms -> GPIO 5. Cap 100nF no pino ADC p/ LGND.
 * Circuito B (Feed Original): Fio Positivo de Alimentacao do Relogio no Painel -> R 1M Ohm -> No DIV_B -> Pino 5 (IN+ B). Repetir filtros e clamps de A. Malha B: Pino 7 (OUT B) no Pino 6 (IN- B). Pino 7 -> R 100 Ohms -> GPIO 6. Cap 100nF no pino ADC p/ LGND.
BAIRRO 7: Display e Alertas
 * TFT: Pinos CS(10), MOSI(11), SCK(12), DC(14). RST(21).
 * Backlight (IRLZ44N): GPIO 13 -> R 100 Ohms -> Gate. Gate -> R 100k -> PGND. Source -> PGND. Drain -> Pino BLK. (Se BLK for LED direto, calcular/adicionar Resistor de Potencia em serie).
 * Alertas: 12V Pos-Chave -> R 2.2k -> Pino 1 PC817. Pino 2 -> Sensor (PGND). Pino 3 (Emissor) -> LGND/AGND. Pino 4 (Coletor) -> GPIO + Pull-up 10k p/ 3.3V_ESP.
 * REQUISITOS OBRIGATORIOS DE FIRMWARE (C++)
 * Ratiometria Absoluta (REGRA DE PROTECAO DE CALCULOS):
 * MANDATORIO: O codigo C++ NUNCA deve dividir os valores 'raw' (contagens brutas) de canais com PGA diferente.
 * A0(Oleo) e A3(Alternador) operam em GAIN_TWOTHIRDS. A1(Agua) e A2(5V_A) operam em GAIN_ONE.
 * O Firmware DEVE converter todas as leituras para Volts reais ANTES de realizar a equacao ratiometrica ou divisao matematica.
 * A3 Histerese (Gestao de Zona Morta):
 * ALTERNADOR se V > 13.5V por 2s.
 * BATERIA se V < 12.8V por 2s.
 * Zona Morta (12.8V a 13.5V): O firmware deve MANTER O ULTIMO ESTADO VALIDO registrado ate que um novo limiar seja cruzado.
 * Validacao Bancada / Road-Test:
 * RPM: Instrumentar com osciloscopio o Flyback para calibrar rejeicao em C++.
 * Oleo: Verificar se o consumo do sensor ativo nao rebaixa o barramento 5V_A para menos de 4.75V (Limite aceitavel para garantir 5% de margem no ratiometrico do A2).
