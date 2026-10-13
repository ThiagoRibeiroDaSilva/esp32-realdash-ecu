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
  * **Sa
