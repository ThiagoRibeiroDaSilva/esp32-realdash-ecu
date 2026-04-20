1. Linha 4 de A a F: É um cabo flexível?
Não. Quando dizemos "O Nó FRONT é a Linha 4 (A4 a F4). Una todos com solda", significa que você vai pegar um fio de cobre rígido e nu (descascado) ou até mesmo o próprio estanho, e vai preencher esses furos criando uma "trilha" grossa. É como um barramento. Todos esses furos (A4, B4, C4, D4, E4, F4) viram uma coisa só eletricamente.
2. A diferença entre PONTE e JUMPER:
• [PONTE]: É quando as coisas estão tão perto que você usa a própria perna do componente para ligar ao vizinho. Você enfia o diodo no C4, dobra a perna dele por baixo da placa até ela encostar no B4, e solda as duas juntas. Não usa fio extra.
• [JUMPER]: É quando as coisas estão longe. Você precisa cortar um pedaço de fio flexível (encapado, para não fechar curto no caminho), descascar as pontas, soldar uma no furo de origem e a outra no furo de destino.
3. O IRF9540N (TO-220): Gate, Drain e Source:
Olhe para o componente de frente para você (conseguindo ler as letras escritas no plástico preto), com as três pernas viradas para baixo.
• Perna 1 (Esquerda): É o Gate (O "gatilho" que liga e desliga).
• Perna 2 (Meio): É o Drain (O dreno, por onde a energia sai para a placa). A chapa de metal nas costas dele também é o Drain.
• Perna 3 (Direita): É o Source (A fonte, por onde entram os 12V da bateria).
4. O Nó VBAT_PROT:
Assim como o Nó FRONT, o VBAT_PROT (Bateria Protegida) é uma trilha reta que você vai criar unindo os furos de A13 até L13 com estanho ou fio de cobre nu.
• Para que serve? A energia entra bruta no FRONT, passa por dentro do transistor IRF9540N (que só deixa a energia ir para a frente, bloqueando ligações invertidas da bateria) e sai "limpa e protegida" no VBAT_PROT. A partir dessa trilha A13-L13, nós distribuímos a energia para os capacitores e para o módulo LM2596.
5. A Muralha 6.8k e o "Fio Bobina":
• "Fio Bobina": É aquele fio longo que vem lá do motor, do negativo da bobina do Chevette. Ele traz pulsos de 400 Volts!
• Como soldar: Não use fios entre eles! Coloque os resistores fazendo um "zig-zag".
• Enfie o R1 no A20 e A23.
• Dobre a perna do R1 que saiu no A23 para encostar no B23.
• Enfie o R2 no B23 e B26.
• Dobre a perna do R2 que saiu no B26 para encostar no C26... e assim por diante. A energia vai pular de resistor em resistor até o D32, perdendo força e não dando "arco" (choque) na placa.
6. O Zener 5.1V no D32 (RPM):
Ele vai ficar com o Cátodo (listra) no D32 e o Ânodo (liso) no D2 (que é a grande avenida do PGND da Linha 2).
7. O que é "1N4148 antiparalelo"?
O optoacoplador tem um LED lá dentro (Pino 1 Anodo, Pino 2 Catodo). Antiparalelo é ligar um diodo ao contrário nesses mesmos pinos para proteger o LED contra energia invertida.
• O Pino 1 do optoacoplador recebe a lista (Catodo) do 1N4148.
• O Pino 2 do optoacoplador recebe o lado liso (Anodo) do 1N4148.
8. Diodos SD103 Alto e Baixo:
Os diodos "Clamps" (que não deixam o S3 fritar se der um curto) funcionam como válvulas de segurança. Se a tensão passar de 5V, o Diodo Alto abre e joga pro 5V_A. Se ficar menor que zero, o Diodo Baixo abre e joga pro LGND. Especifiquei todos eles pino a pino na nova Netlist abaixo.
9. Divisores A2 e A3 (Montagem detalhada):
Vou desenhar cada resistor, perna a perna, e qual fio vai pular para o ADS1115 (pinos AB38 e AB39).
10. Emissores e Coletores (PC817):
O PC817 tem 4 perninhas e uma "bolinha" escavada no plástico.
• Perna perto da bolinha: Pino 1 (Anodo do LED)
• Perna abaixo dela: Pino 2 (Catodo do LED)
• Atravessando pro outro lado, perna de baixo: Pino 3 (Emissor)
• Perna acima dela: Pino 4 (Coletor)
🗺️ O NOVO CÓDIGO MATRIX (V7.0 - The Precision Cut)
Aqui está a Netlist reescrita. Acabei com o uso genérico de "vai pro PGND". Agora eu especifico exatamente qual furo de aterramento deve ser usado (ex: JUMPER p/ PGND (E2)).
LEGENDA LIMITES: Placa de 28x50. Colunas: A até AB. Linhas: 1 até 50. Lado da solda para cima.
[LISTRA] = Cátodo. [LISO] = Ânodo.
PASSO 1: AS AVENIDAS E O NÓ ESTRELA
• PGND (Terra Sujo): Fio de cobre nu unindo a Linha 2 (A2 ao AB2).
• LGND (Terra Limpo): Fio de cobre nu unindo a Linha 48 (A48 ao AB48).
• 5V_A (Analógico): Fio de cobre nu unindo a Linha 46 (A46 ao AB46).
• 3.3V_ESP (Lógico): Fio de cobre nu unindo a Linha 44 (A44 ao AB44).
• NÓ ESTRELA: [JUMPER] fio flexível unindo a ponta do PGND no AB2 até a ponta do LGND no AB48.
PASSO 2: FRONT-END 12V E PROTEÇÃO (Canto Superior Esquerdo)
• Entrada 12V: Fio do Fusível entra no A4. Una do A4 ao F4 com estanho/cobre nu (Esse é o Nó FRONT).
• TVS TP6KE22A: [LISTRA] no C4 [PONTE p/ trilha FRONT]. [LISO] no C7. [JUMPER] do C7 p/ PGND (C2).
• IRF9540N (TO-220): Gate no E9, Drain no F9, Source no G9.
• [JUMPER] flexível do G9 (Source) p/ trilha FRONT (F4).
• R 1M Ohm: Pernas no D9 e D12. [PONTE de D9 p/ E9(Gate)]. [JUMPER flexível do D12 p/ G9(Source)].
• R 100k Ohm: Pernas no E10 e E12. [PONTE de E10 p/ E9(Gate)]. [JUMPER flexível do E12 p/ PGND (E2)].
• Zener 15V: [LISTRA] no H9 [PONTE p/ G9(Source)]. [LISO] no H12 [PONTE p/ E12].
• [JUMPER] flexível do F9 (Drain) p/ furo A13.
• Nó VBAT_PROT: Crie uma trilha de estanho unindo do A13 ao L13.
• Filtros VBAT_PROT:
• Cap 470µF: Positivo no I13, Negativo no I16 [JUMPER p/ PGND (I2)].
• Cap 1µF: Pernas no J13 e J16 [JUMPER p/ PGND (J2)].
• Cap 100nF: Pernas no K13 e K16 [JUMPER p/ PGND (K2)].
PASSO 3: MÓDULOS DE ENERGIA E ESP32
• LM2596: IN+ no L13 (Trilha VBAT). IN- no L2 (PGND). OUT- no N2 (PGND). OUT+ no A16.
• Nó 5V_D: Crie uma trilha de estanho unindo do A16 ao V16.
• Filtro do 5V_A: R 10 Ohms do O16 (5V_D) ao O46 (5V_A). Cap 47µF (Positivo no P46, Negativo no LGND P48). Cap 100nF no Q46 p/ LGND (Q48).
• ESP32-S3: Pinos da esquerda (3V3 ao 5V) na Col P (P20 a P41). Direita (GND ao TX) na Col Z (Z20 a Z41).
• [JUMPER] flexível do V16 (5V_D) p/ Pino 5V ESP (P40).
• [JUMPER] flexível do LGND (P48) p/ Pino GND ESP (P41).
• [JUMPER] flexível do Pino 3.3V ESP (P20) p/ Trilha 3.3V_ESP (P44).
PASSO 4: I2C (ADS1115 E BSS138)
• ADS1115: Pinos esquerdos na Col AB (30 a 39). VDD (AB30) [JUMPER p/ 5V_A (AB46)]. GND (AB31) [JUMPER p/ LGND (AB48)]. ADDR (AB34) [PONTE p/ AB31(GND)]. SCL é AB32. SDA é AB33. A0 é AB36. A1 é AB37. A2 é AB38. A3 é AB39.
• BSS138: Col AA (30 a 38). Lado HV (AA30) [JUMPER p/ 5V_A (AA46)]. Lado LV (AA35) [JUMPER p/ 3.3V_ESP (AA44)]. HV1 e HV2 [PONTES para SCL e SDA do ADS]. LV1 e LV2 [JUMPERS flexíveis p/ ESP GPIO9(Z34) e GPIO8(Z31)]. Lados GND da placa BSS para LGND (AA48).
PASSO 5: RPM (MURALHA E OPTOACOPLADOR)
• Muralha 6.8k: Fio do motor no A20. R1(A20 e A23). Dobre perna do A23 até encostar no B23. R2(B23 e B26). Dobre B26 p/ C26. R3(C26 e C29). Dobre C29 p/ D29. R4(D29 e D32). D32 é o sinal filtrado.
• Zener 5.1V: [LISTRA] no D32. [LISO] no D30. Em seguida, pegue um pedaço de fio flexível (JUMPER), solde uma ponta no D30 e a outra ponta lá embaixo na avenida PGND, no D2.
• R 330 Ohms: Furos D32 e F32.
• H11L1M (Opto): Pinos 1,2,3 na Col G. Pinos 6,5,4 na Col J.
• Pino 1 no G32 [PONTE p/ F32]. Pino 2 no G33 [JUMPER p/ PGND (G2)].
• 1N4148 (Antiparalelo): [LISTRA] no H32 [PONTE p/ G32]. [LISO] no H33 [PONTE p/ G33].
• Pino 6 no J32 [JUMPER p/ 3.3V_ESP (J44)]. Pino 5 no J33 [JUMPER p/ LGND (J48)].
• Cap 100nF no K32 e K33 [PONTES p/ Pinos 6 e 5].
• Pino 4 no J34 [JUMPER p/ GPIO4 ESP (P23)]. R 4.7k do J34 p/ 3.3V_ESP (L44).
PASSO 6: SENSORES DE BLOCO (A0 e A1)
• A0 (Óleo): Fio Sinal entra no A36. R 2.2k do A36 p/ B36 (Nó Sinal Filtrado).
• Diodo SD103 Alto: [LISO] no C36 [PONTE p/ B36]. [LISTRA] no C46 (Trilha 5V_A).
• Diodo SD103 Baixo: [LISTRA] no D36 [PONTE p/ B36]. [LISO] no D48 (Trilha LGND).
• Cap 100nF do B36 p/ B48 (LGND).
• [JUMPER] flexível do B36 p/ Pino A0 do ADS (AB36).
• A1 (NTC Água): R 2.2k da trilha 5V_A (E46) p/ E40. Fio do Sensor MTE entra no E40. R 2.2k do E40 p/ F40 (Nó Sinal Filtrado).
• Diodo SD103 Alto: [LISO] no G40 [PONTE p/ F40]. [LISTRA] no G46 (5V_A).
• Diodo SD103 Baixo: [LISTRA] no H40 [PONTE p/ F40]. [LISO] no H48 (LGND).
• Cap 1µF X7R do F40 p/ F48 (LGND).
• [JUMPER] flexível do F40 p/ Pino A1 do ADS (AB37).
PASSO 7: DIVISORES INTERNOS (A2 e A3)
• A2 (Monitor do 5V_A): R 10k (1%) da trilha 5V_A (I46) p/ I40 (Nó Meio). R 10k (1%) do I40 p/ LGND (I48).
• Cap 100nF do I40 p/ J48 (LGND). R 1k do I40 p/ J40. [JUMPER] flexível de J40 p/ A2 do ADS (AB38).
• A3 (Monitor Alternador): [JUMPER] da Trilha VBAT (B13) p/ furo K40. R 33k (1%) do K40 p/ K37 (Nó Meio). R 4.7k (1%) do K37 p/ LGND (K48).
• Diodo SD103 Alto: [LISO] no L37 [PONTE p/ K37]. [LISTRA] no L46 (5V_A).
• Diodo SD103 Baixo: [LISTRA] no M37 [PONTE p/ K37]. [LISO] no M48 (LGND).
• Cap 100nF do K37 p/ LGND (M48 via PONTE). R 1k do K37 p/ N37. [JUMPER] de N37 p/ A3 do ADS (AB39).
PASSO 8: COMBUSTÍVEL - MCP6002 BUFFER
• MCP6002 (DIP-8): Pinos 1 ao 4 na Col L. Pinos 8 ao 5 na Col O. (Pin1=L32, Pin2=L33, Pin3=L34, Pin4=L35) / (Pin8=O32, Pin7=O33, Pin6=O34, Pin5=O35).
• Pino 8 (O32) [JUMPER flexível p/ 3.3V_ESP (O44)]. Pino 4 (L35) [JUMPER flexível p/ LGND (L48)].
• Cap 100nF de desacoplamento do O32 p/ L35 por baixo do chip.
• Circuito A (Boia): Fio da Boia entra no A34. R 1M do A34 p/ B34 (Nó DIV_A). [PONTE longa do B34 p/ Pino 3 (L34)].
• R 220k do B34 p/ LGND (B48). Cap 470nF do C34 [PONTE p/ B34] p/ LGND (C48).
• Diodo Alto: [LISO] D34 [PONTE B34], [LISTRA] D44 (3.3V_ESP). Diodo Baixo: [LISTRA] E34 [PONTE B34], [LISO] E48 (LGND).
• [PONTE] unindo Pino 1 (L32) ao Pino 2 (L33). R 100 Ohms do L32 p/ K32. [JUMPER] flexível de K32 p/ GPIO5 do ESP (P24). Cap 100nF de K32 p/ LGND (K48).
• Circuito B (Painel): Repita rigorosamente o layout do Circuito A nas linhas de baixo, usando o Pino 5 (O35) como entrada e o Pino 7 (O33) conectado ao Pino 6 (O34) como saída, enviando para GPIO6 do ESP (P25).
PASSO 9: ALERTAS (PC817)
• IC1 (Água): Pinos na Col K e N (Linhas 6 e 7). 12V ACC entra num R 2.2k até Pino 1 (K6). Fio Terra do Motor no Pino 2 (K7). Pino 3 (N7) [JUMPER p/ LGND (N48)]. Pino 4 (N6) [JUMPER p/ GPIO15 ESP (Z27)]. R 10k do Pino 4 p/ 3.3V_ESP (Z44).
• IC2 (Freio): Segue idêntico nas linhas 9 e 10, enviando o Pino 4 para o GPIO16 ESP (Z28).
PASSO 10: TELA E GPS (KEYSTONE)
• Backlight Tela: IRLZ44N (Col R,S,T). Source (T5) [JUMPER p/ PGND (T2)]. R 100k do Gate (R5) p/ PGND (R2). R 100 Ohms do R5 p/ R8. [JUMPER] flexível do R8 p/ GPIO13 ESP (P38). Drain (S5) no Fio BLK da Tela.
• Pads do GPS (P/ soldar o chicote do RJ45):
• P18: Fio de energia pro RJ45 (Pino 1/2). [PONTE p/ 3.3V_ESP (P44)].
• Q18: Fio GND pro RJ45 (Pino 3/6). [PONTE p/ LGND (Q48)].
• R18: Fio TX pro RJ45 (Pino 4). [JUMPER p/ GPIO17 ESP (P29)].
• S18: Fio RX pro RJ45 (Pino 5). [JUMPER p/ GPIO18 ESP (P30)].
