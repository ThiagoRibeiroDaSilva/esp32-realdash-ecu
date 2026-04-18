BOM Físico

[A] MÓDULOS E CÉREBROS (Proibido substituir modelos)
 * 1x Placa ESP32-S3 DevKitC-1 (Versão WROOM-1, pinos pré-soldados).
 * 1x Módulo Display TFT 4.0" SPI ST7796S (OBRIGATÓRIO: Pino BLK Lógico com driver interno. Proibido LED direto).
 * 1x Módulo GPS NEO-M8N (OBRIGATÓRIO: Versão operando estritamente em 3.3V no pino VCC).
 * 1x Módulo A/D ADS1115 (Com pull-ups SMD de 10k integrados. Endereço fixo 0x48).
 * 1x Módulo Level Shifter I2C BSS138 (4 canais, com pull-ups integrados).
 * 1x Módulo Step-Down LM2596 (Buck Converter DC-DC).

[B] SEMICONDUTORES (PTH/DIP)
 * 1x CI Op-Amp MCP6002-I/P (DIP-8).
 * 1x LDO Regulador MCP1700-3302 (TO-92).
 * 1x Optoacoplador H11L1M (DIP-6).
 * 2x Optoacopladores PC817 (DIP-4).
 * 1x P-MOSFET IRF9540N (TO-220).
 * 1x N-MOSFET Logic-Level IRLZ44N (TO-220).

[C] DIODOS E PROTEÇÕES
 * 1x Diodo TVS Unidirecional TP6KE22A (DO-201, AEC-Q101).
 * 10x Diodos Schottky SD103B ou SD103C (DO-35).
 * 1x Diodo Zener 15V / 1W (1N4744A).
 * 1x Diodo Zener 5.1V / 0.5W (1N5231B).
 * 1x Diodo de Sinal 1N4148.

[D] CAPACITORES
 * 1x Eletrolítico 470uF / 50V.
 * 1x Eletrolítico 47uF / 16V (ou 25V).
 * 1x Cerâmico 1uF / 50V.
 * 1x Cerâmico 1uF / 16V (OBRIGATÓRIO: Dielétrico X7R p/ o NTC).
 * 2x Cerâmicos 1uF / 16V (Comuns, p/ o LDO).
 * 2x Cerâmicos 470nF / 16V.
 * 10x Cerâmicos 100nF / 50V (Código 104).

[E] RESISTORES (1/4W PTH comum, EXCETO onde anotado)
 * 4x 6.8k Ohms / 0.5W (OBRIGATÓRIO: Filme Metálico / Metal Film).
 * 1x 10 Ohms / 0.5W.
 * 3x 1M Ohm.
 * 2x 220k Ohms.
 * 2x 100k Ohms.
 * 1x 33k Ohms (OBRIGATÓRIO: 1% de tolerância).
 * 2x 10k Ohms (OBRIGATÓRIO: 1% de tolerância).
 * 3x 10k Ohms.
 * 1x 4.7k Ohms (OBRIGATÓRIO: 1% de tolerância).
 * 1x 4.7k Ohms.
 * 1x 2.2k Ohms (OBRIGATÓRIO: 1% de tolerância).
 * 4x 2.2k Ohms (Corrigido para 4 unidades comuns).
 * 2x 1k Ohm.
 * 1x 330 Ohms.
 * 3x 100 Ohms.

❌ O que VAI FALTAR (não consta no kit do Aliexpress)
Você precisará comprar esses três itens avulsos no balcão da loja física, pois o kit não atende seja por valor ou por potência:
1. A Falta de Valor:
• 1x 33k Ohms (1% de tolerância - 1/4W): Se você olhar a tabela do kit, ele simplesmente pula do 20k direto para o 47k. Ele não possui o valor de 33k, que é vital para o divisor de tensão que vai ler o Alternador (Bairro 5).
2. A Falta de Potência (O Risco Físico):
• 4x 6.8k Ohms / 0.5W (Filme Metálico): O kit até tem o 6.8k, mas ele é de 1/4W (0.25W). O dossiê exige 0.5W (meio watt). Esses 4 resistores vão no Bairro 3 para segurar o "coice" de alta tensão da bobina de ignição. Se você usar os de 1/4W do kit, eles vão superaquecer e queimar com o motor rodando.
• 1x 10 Ohms / 0.5W: O kit também tem o 10R, mas novamente, é de 1/4W. Esse resistor é a "porta de entrada" da alimentação do Sensor de Óleo (Bairro 2). Como toda a corrente analógica passa por ele, ele precisa dissipar mais calor (0.5W), senão ele abre o bico.

[F] MECÂNICA, CHICOTE E CONSUMÍVEIS
 * 1x Placa Perfurada Universal Dupla Face 10x15cm (Grid de 39x59 furos).
 * 1x Caixa Patola PB-114 (ou equivalente).
 * 1x Kit Conector Automotivo 16 Vias (Contendo OBRIGATORIAMENTE: 1x Carcaça Macho, 1x Carcaça Fêmea, 32x Terminais de crimpagem, 32x Borrachas/Vedações).
 * 1x Porta-fusível aéreo c/ Fusível Lâmina 2A.
 * 4x Espaçadores de Nylon M3 MACHO-FÊMEA (10mm corpo + 6mm rosca).
 * 4x Parafusos M3x6mm + 4x Porcas M3.
 * 1x Metro de Cabo Blindado (1 via + malha) ou Par Trançado (P/ o sinal do RPM).
 * 1x Rolo (5 metros) de Fio AWG 18 (0.75mm²) para barramentos de potência.
 * 1x Rolo (10 metros) de Fio AWG 22 (0.30mm²) rígido para jumpers na placa.
 * 1x Tubo de Cola PU (Poliuretano) ou Resina Epóxi (Para calçar os módulos).
 * 1x Lata de Spray Verniz Conformal Coating (Para selar a placa após solda).
