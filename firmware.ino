// =============================================================================
// TELEMETRIA CHEVETTE 1974 — ESP32-S3
// Firmware V1.7 | Dossie V21.0 (The Software Bridge)
// Framework: Arduino IDE | FreeRTOS Dual-Core
//
// CHANGELOG V1.7 (Blindagem Total):
//   1. CORRIGIDO: GPS não marca mais fix válido no boot sem sentença válida
//      e o timeout agora derruba gps_valid mesmo se o stream parar por completo
//   2. CORRIGIDO: Frases NMEA truncadas/estouradas são descartadas até o '\n'
//      — nunca mais tenta parsear sentença cortada como se fosse íntegra
//   3. CORRIGIDO: Snapshot de RPM agora copia período, contador e timestamp
//      dentro da mesma seção crítica — cálculo mais atômico e robusto
//   4. ADICIONADO: Filtro de glitch por hardware no GPIO de RPM quando a
//      toolchain expõe a API do ESP-IDF; fallback seguro para debounce por software
//   5. CORRIGIDO: Diagnóstico ADS1115 agora lê registrador real via I2C e
//      desacopla falha do conversor de erro físico de sensor na interface
//   6. CORRIGIDO: Água agora também reprova VREF baixo; óleo ganhou guardas
//      elétricas de curto/aberto/faixa antes do cálculo de PSI
//   7. CORRIGIDO: ADC do combustível usa atenuação por pino, não global
//
// CHANGELOG V1.6 (Netlist travado):
//   1. CONFIRMADO: I2C fixado por netlist fisico do projeto
//      SDA = GPIO 8 (W31 / BSS138 LV2 / ADS1115 SDA)
//      SCL = GPIO 9 (W34 / BSS138 LV1 / ADS1115 SCL)
//   2. DOCUMENTADO: mapeamento nao esta mais por suposicao do DevKit
//
// CHANGELOG V1.5 (Blindagem Final):
//   1. CORRIGIDO: Parser GPS agora aceita qualquer sentença $--RMC válida
//      (GPRMC, GNRMC, GLRMC, etc.), não apenas $GPRMC
//   2. CORRIGIDO: Primeiro pulso de RPM após boot/timeout não entra na média
//      — evita contaminação do cálculo na partida e na retomada
//   3. ADICIONADO: Sanidade periódica do ADS1115/I2C com recheck e tentativa
//      de reanexar o conversor em falha intermitente de barramento
//   4. CORRIGIDO: I2C agora usa pinos explícitos SDA/SCL e o estado do
//      alternador é preservado quando o ADS fica indisponível
//
// CHANGELOG V1.4 (Fechamento Final):
//   1. CORRIGIDO: Snapshot do frontend inicializado e preservado mesmo se
//      houver timeout momentâneo de mutex — nunca desenha lixo na tela
//   2. CORRIGIDO: Combustível agora tem diagnóstico separado (feed/boia/faixa)
//      e NUNCA publica falha como 0% de tanque
//
// CHANGELOG V1.3 (Correções Gepeto):
//   1. CORRIGIDO: Combustível ratiométrico — calcFuelRatio() agora usa
//      v_boia / v_feed (Circuito B ativo) em vez de tensão absoluta da boia
//   2. CORRIGIDO: ADS1115 — falha de hardware não mascarada; ads_ok preservado
//      do setup() e nunca sobrescrito com 'true' no ciclo de aquisição
//   3. CORRIGIDO: 5V_A abaixo de 4.75V gera SENSOR_ERR_VREF no óleo,
//      separado de alarme de pressão baixa real
//   4. CORRIGIDO: alt_state nunca publica ALT_STATE_UNDEFINED por timeout
//      de mutex — preserva último estado válido em caso de contenção
//   5. CORRIGIDO: Parser NMEA aplica sinal de hemisfério S/W (Brasil)
//   6. CORRIGIDO: ADC interno configurado explicitamente com atenuação 11dB
//      (range 0–3.3V) antes das leituras de combustível
//
// CHANGELOG V1.2:
//   - CORRIGIDO: PIN_GPS_RX 17→18, PIN_GPS_TX 18→17 (Matrix V5.2, furos W29/W30)
//   - ADICIONADO: gGPS.setRxBufferSize(512) antes do begin()
//   - ATUALIZADO: API backlight → ledcAttach() (ESP32 Arduino Core 3.x)
// =============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <HardwareSerial.h>
#include <math.h>
#if defined(__has_include)
  #if __has_include(<driver/gpio.h>)
    #include <driver/gpio.h>
  #endif
  #if __has_include(<driver/gpio_filter.h>)
    #include <driver/gpio_filter.h>
    #define CHEVETTE_HAS_GPIO_GLITCH_FILTER 1
  #else
    #define CHEVETTE_HAS_GPIO_GLITCH_FILTER 0
  #endif
#else
  #define CHEVETTE_HAS_GPIO_GLITCH_FILTER 0
#endif
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// =============================================================================
// SECAO 1: PINAGEM (Dossie V21.0 + Matrix V5.2 + Netlist confirmado)
// =============================================================================
#define PIN_RPM_IRQ       4    // GPIO 4  — Saída H11L1M (Pull-up externo 4.7k)
#define PIN_FUEL_ADC      5    // GPIO 5  — MCP6002 Circuito A: tensão da boia
#define PIN_CLOCK_ADC     6    // GPIO 6  — MCP6002 Circuito B: alimentação do relógio (feed)
#define PIN_I2C_SDA       8    // GPIO 8  — I2C SDA confirmado no netlist: W31 -> BSS138 LV2 -> HV2 -> ADS1115 SDA
#define PIN_I2C_SCL       9    // GPIO 9  — I2C SCL confirmado no netlist: W34 -> BSS138 LV1 -> HV1 -> ADS1115 SCL
#define PIN_TFT_BL        13   // GPIO 13 — Backlight (Gate IRLZ44N via R 100Ω)
#define PIN_ALERT_WATER   15   // GPIO 15 — Alerta Água (PC817, Active-LOW)
#define PIN_ALERT_BRAKE   16   // GPIO 16 — Alerta Freio (PC817, Active-LOW)
#define PIN_GPS_TX        17   // GPIO 17 — TX ESP32 → RX GPS (furo W29, Matrix V5.2)
#define PIN_GPS_RX        18   // GPIO 18 — RX ESP32 ← TX GPS (furo W30, Matrix V5.2)
// OBS: pinagem SPI/RST da TFT é autoritativa no arquivo User_Setup.h da TFT_eSPI.
// O firmware usa diretamente apenas o pino do backlight neste sketch.

// =============================================================================
// SEÇÃO 2: CONSTANTES DE HARDWARE (Dossiê V21.0)
// =============================================================================

// ADS1115 — LSBs por ganho
// REGRA CRÍTICA: NUNCA dividir raw de canais com PGA diferente.
//                SEMPRE converter para Volts antes de qualquer divisão ratiométrica.
#define ADS_LSB_TWOTHIRDS   0.0001875f  // GAIN_TWOTHIRDS → FSR ±6.144V → 187.5 µV/LSB
#define ADS_LSB_ONE         0.000125f   // GAIN_ONE       → FSR ±4.096V → 125.0 µV/LSB

// Fatores de escala dos divisores resistivos (1%)
#define A2_SCALE_FACTOR     2.0f        // Divisor 10k/10k  → ×2.000
#define A3_SCALE_FACTOR     8.021f      // Divisor 33k/4.7k → ×8.021

// NTC MTE4053
#define NTC_R_REF           2200.0f     // Resistor de referência do divisor (Ω)
#define NTC_R0              2200.0f     // Resistência nominal a T0 — confirmar datasheet
#define NTC_T0_KELVIN       298.15f     // T0 = 25°C em Kelvin — NUNCA usar 25.0 diretamente
#define NTC_BETA            3950.0f     // Coeficiente Beta do MTE4053

// Sensor de Pressão de Óleo (ratiométrico 0.5V–4.5V / 0–100 PSI)
#define OIL_SCALE           125.0f      // Calibrar em bancada para transdutor genérico
#define OIL_OFFSET          0.1f        // 0.5V/5.0V = 10% de Vcc = 0 PSI

// Histerese Bateria/Alternador
#define ALT_THRESHOLD_HIGH  13.5f       // > 13.5V por 2s → ALTERNADOR
#define ALT_THRESHOLD_LOW   12.8f       // < 12.8V por 2s → BATERIA
#define ALT_HYSTERESIS_MS   2000        // Zona morta 12.8–13.5V → manter último estado

// RPM — 4 cilindros, 4 tempos: bobina dispara 2x por revolução
#define RPM_PULSES_AVG      4           // Média móvel de 4 pulsos
#define RPM_TIMEOUT_US      500000UL    // 500ms sem pulso = motor parado
#define RPM_DEBOUNCE_US     3000UL      // Anti-glitch: ignora pulsos < 3ms

// Guard clauses e limites físicos
#define NTC_GUARD_MV        50.0f       // Margem 50mV contra ruído de quantização
#define NTC_TEMP_MIN       -40.0f       // °C mínimo físico MTE4053
#define NTC_TEMP_MAX        150.0f      // °C máximo físico MTE4053
#define VREF_5V_MIN         4.75f       // Mínimo aceitável no barramento 5V_A
#define OIL_GUARD_MV        50.0f       // Margem elétrica contra curto/aberto no A0
#define OIL_SENSOR_MIN_RATIO 0.07f      // Janela permissiva do transdutor ratiométrico
#define OIL_SENSOR_MAX_RATIO 0.93f

// Combustível — limites de tensão do ADC interno (ESP32-S3, atenuação 11dB)
// Circuito A (boia) e B (feed) operam no range 0.25V–2.92V (Dossiê V21.0)
#define FUEL_V_MIN          0.10f       // Abaixo disso: circuito aberto ou boia solta
#define FUEL_V_MAX          3.10f       // Acima disso: clamp não funcionou — rejeitar leitura

// I2C / ADS1115
#define ADS_I2C_ADDR        0x48        // Endereço fixo do ADS1115 no dossiê
#define ADS_RECHECK_MS      1000UL      // Revalidação periódica do barramento I2C/ADS
#define GPS_VALID_TIMEOUT_MS 2000UL      // Sem RMC válida por 2s -> GPS inválido

// Backlight PWM — Core 3.x: canal gerenciado automaticamente
#define BL_PWM_FREQ         5000        // Hz
#define BL_PWM_RES          8           // bits (0–255)
#define BL_BRIGHTNESS       200         // Brilho padrão

// =============================================================================
// SEÇÃO 3: BLE REALDASH (Nordic UART Service + Frame 0x44)
// =============================================================================
#define BLE_DEVICE_NAME     "Chevette_ECU"
#define BLE_SERVICE_UUID    "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_TX_UUID    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_RX_UUID    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define REALDASH_FRAME_ID_1 0x001   // RPM | Água | Óleo | Tensão
#define REALDASH_FRAME_ID_2 0x002   // Velocidade | Combustível | Alertas | Temp MCU

// =============================================================================
// SEÇÃO 4: ESTRUTURA DE DADOS COMPARTILHADA (Thread-Safe via Mutex)
// =============================================================================
typedef enum {
    ALT_STATE_BATTERY,
    ALT_STATE_ALTERNATOR,
    ALT_STATE_UNDEFINED     // Zona morta — nunca publicado por timeout de mutex
} AltState_t;

typedef enum {
    SENSOR_OK,
    SENSOR_ERR_SHORT,       // Tensão ≤ 50mV → sensor em curto
    SENSOR_ERR_OPEN,        // Tensão ≥ Valim − 50mV → fio rompido
    SENSOR_ERR_RANGE,       // Temperatura fora de −40°C a +150°C
    SENSOR_ERR_VREF         // 5V_A abaixo de 4.75V — falha de referência, não de sensor
} SensorStatus_t;

typedef enum {
    FUEL_OK,
    FUEL_ERR_SENSOR,        // Circuito A (boia) fora de faixa ou desconectado
    FUEL_ERR_FEED,          // Circuito B (feed original do relógio) ausente
    FUEL_ERR_RANGE          // Relação boia/feed incoerente ou fora da calibração
} FuelStatus_t;

typedef struct {
    volatile uint16_t   rpm;
    float               oil_psi;
    SensorStatus_t      oil_status;     // Separado: SENSOR_ERR_VREF ≠ pressão baixa real
    float               water_temp_c;
    SensorStatus_t      water_status;
    float               alt_voltage;
    AltState_t          alt_state;
    float               fuel_ratio;     // 0.0–1.0 via LUT ratiométrica (boia/feed)
    FuelStatus_t        fuel_status;    // Separado: falha de feed/boia não vira tanque vazio
    float               vbus_5v;
    float               gps_speed_kmh;
    float               gps_lat;
    float               gps_lon;
    bool                gps_valid;
    bool                alert_water;
    bool                alert_brake;
    float               esp32_temp_c;
    bool                ads_ok;         // Confirmado no boot e revalidado periodicamente em runtime
    bool                ads_fault_active; // ADS offline: UI deve mostrar falha do conversor, não do sensor
    bool                ble_connected;
} TelemetryData_t;

static TelemetryData_t   gData;
static SemaphoreHandle_t gDataMutex;

// =============================================================================
// SEÇÃO 5: VARIÁVEIS ISR RPM (volatile obrigatório — Dossiê V21.0)
// =============================================================================
static volatile uint32_t gLastPulseUs                  = 0;
static volatile uint32_t gPulsePeriods[RPM_PULSES_AVG] = {0};
static volatile uint8_t  gPulseIndex                   = 0;
static volatile uint8_t  gPulseCount                   = 0;

// =============================================================================
// SEÇÃO 6: OBJETOS DE HARDWARE
// =============================================================================
static Adafruit_ADS1115 gADS;
static TFT_eSPI         gTFT = TFT_eSPI();
static HardwareSerial   gGPS(1);

static BLEServer*          gBLEServer    = nullptr;
static BLECharacteristic*  gBLECharTX    = nullptr;
static bool                gBLEConnected = false;
#if CHEVETTE_HAS_GPIO_GLITCH_FILTER
static gpio_glitch_filter_handle_t gRpmGlitchFilter = nullptr;
#endif

// =============================================================================
// SEÇÃO 7: ISR RPM (IRAM_ATTR — evita cache miss de flash durante spike)
// =============================================================================
void IRAM_ATTR isrRpm() {
    uint32_t now = micros();

    // Primeiro pulso após boot: apenas arma a referência temporal.
    // Não contaminar a média com um período artificialmente gigante.
    if (gLastPulseUs == 0) {
        gLastPulseUs = now;
        return;
    }

    uint32_t period = now - gLastPulseUs;
    if (period < RPM_DEBOUNCE_US) return;

    // Retomada após longa ausência de pulsos: reinicia a média móvel.
    // O primeiro pulso após timeout só rearma a referência.
    if (period > RPM_TIMEOUT_US) {
        gLastPulseUs = now;
        gPulseIndex  = 0;
        gPulseCount  = 0;
        for (int i = 0; i < RPM_PULSES_AVG; i++) gPulsePeriods[i] = 0;
        return;
    }

    gPulsePeriods[gPulseIndex] = period;
    gPulseIndex = (gPulseIndex + 1) % RPM_PULSES_AVG;
    if (gPulseCount < RPM_PULSES_AVG) gPulseCount++;
    gLastPulseUs = now;
}

// =============================================================================
// SEÇÃO 8: CÁLCULO DE RPM
// =============================================================================
static uint16_t calcRpm() {
    uint32_t lastPulseUs = 0;
    uint8_t  pulseCount  = 0;
    uint32_t periods[RPM_PULSES_AVG] = {0};

    noInterrupts();
    lastPulseUs = gLastPulseUs;
    pulseCount  = gPulseCount;
    memcpy(periods, (const void*)gPulsePeriods, sizeof(periods));
    interrupts();

    if (lastPulseUs == 0) return 0;
    if ((micros() - lastPulseUs) > RPM_TIMEOUT_US) return 0;
    if (pulseCount < RPM_PULSES_AVG) return 0;

    uint64_t sum = 0;
    for (int i = 0; i < RPM_PULSES_AVG; i++) sum += periods[i];
    uint32_t avgPeriod = (uint32_t)(sum / RPM_PULSES_AVG);
    if (avgPeriod == 0) return 0;

    // RPM = 60.000.000 / (período_us × 2)  [4 cil., 4 tempos, 2 disparos/rotação]
    return (uint16_t)(60000000UL / (avgPeriod * 2));
}

// =============================================================================
// SEÇÃO 9: MATEMÁTICA DOS SENSORES
// =============================================================================

// --- ADS1115: leitura sempre em Volts reais ---
// REGRA V21.0: NUNCA retornar raw. NUNCA dividir canais com PGA diferente sem converter.
static float adsReadVolts(uint8_t channel, adsGain_t gain) {
    gADS.setGain(gain);
    int16_t raw = gADS.readADC_SingleEnded(channel);
    float   lsb = (gain == GAIN_TWOTHIRDS) ? ADS_LSB_TWOTHIRDS : ADS_LSB_ONE;
    return (float)raw * lsb;
}

// --- A2: Referência ratiométrica (primeira leitura de cada ciclo) ---
static float readVbus5v() {
    return adsReadVolts(2, GAIN_ONE) * A2_SCALE_FACTOR;
}

// --- A0: Pressão de Óleo ---
// CORREÇÃO 3: 5V_A abaixo do mínimo retorna SENSOR_ERR_VREF, não -1.0f mascarado como PSI
static SensorStatus_t calcOilPsi(float v_a0, float v_alim, float* result) {
    if (v_alim < VREF_5V_MIN) {
        *result = 0.0f;
        return SENSOR_ERR_VREF;             // Falha de referência — NÃO é baixa pressão
    }

    if (v_a0 <= (OIL_GUARD_MV / 1000.0f)) {
        *result = 0.0f;
        return SENSOR_ERR_SHORT;
    }
    if (v_a0 >= (v_alim - (OIL_GUARD_MV / 1000.0f))) {
        *result = 0.0f;
        return SENSOR_ERR_OPEN;
    }

    float ratio = v_a0 / v_alim;
    if (ratio < OIL_SENSOR_MIN_RATIO || ratio > OIL_SENSOR_MAX_RATIO) {
        *result = 0.0f;
        return SENSOR_ERR_RANGE;
    }

    float psi = OIL_SCALE * (ratio - OIL_OFFSET);
    *result = (psi < 0.0f) ? 0.0f : psi;
    return SENSOR_OK;
}

// --- A1: Temperatura NTC MTE4053 (Equação Beta com Guard Clauses) ---
static SensorStatus_t calcWaterTemp(float v_a1, float v_alim, float* result) {
    if (v_alim < VREF_5V_MIN) {
        *result = 0.0f;
        return SENSOR_ERR_VREF;
    }
    if (v_a1 <= (NTC_GUARD_MV / 1000.0f)) {
        *result = -99.0f;
        return SENSOR_ERR_SHORT;
    }
    if (v_a1 >= (v_alim - (NTC_GUARD_MV / 1000.0f))) {
        *result = 999.0f;
        return SENSOR_ERR_OPEN;
    }
    float r_ntc    = NTC_R_REF * (v_a1 / (v_alim - v_a1));
    // CRÍTICO: NTC_T0_KELVIN = 298.15 (25°C). NUNCA usar 25.0 diretamente.
    float t_kelvin = 1.0f / ((1.0f / NTC_T0_KELVIN) + (1.0f / NTC_BETA) * logf(r_ntc / NTC_R0));
    float t_c      = t_kelvin - 273.15f;
    if (t_c < NTC_TEMP_MIN || t_c > NTC_TEMP_MAX) {
        *result = t_c;
        return SENSOR_ERR_RANGE;
    }
    *result = t_c;
    return SENSOR_OK;
}

// --- A3: Tensão do Alternador/Bateria ---
static float calcAltVoltage(float v_a3) {
    return v_a3 * A3_SCALE_FACTOR;
}

// --- Histerese Bateria/Alternador ---
// CORREÇÃO 4: alt_state só muda se o mutex for obtido.
//             Se não obtido, o chamador preserva o último estado — nunca publica UNDEFINED.
static AltState_t updateAltState(float voltage, AltState_t lastState) {
    static uint32_t highTimer  = 0;
    static uint32_t lowTimer   = 0;
    static bool     highActive = false;
    static bool     lowActive  = false;
    uint32_t now = millis();

    if (voltage > ALT_THRESHOLD_HIGH) {
        if (!highActive) { highTimer = now; highActive = true; }
        lowActive = false;
        if ((now - highTimer) >= ALT_HYSTERESIS_MS) return ALT_STATE_ALTERNATOR;
    } else if (voltage < ALT_THRESHOLD_LOW) {
        if (!lowActive) { lowTimer = now; lowActive = true; }
        highActive = false;
        if ((now - lowTimer) >= ALT_HYSTERESIS_MS) return ALT_STATE_BATTERY;
    } else {
        // Zona morta 12.8–13.5V: manter último estado (Dossiê V21.0)
        highActive = false;
        lowActive  = false;
    }
    return lastState;
}

// --- Combustível: LUT ratiométrica (boia / feed) ---
// CORREÇÃO 1: usa v_feed (Circuito B) como referência, não tensão absoluta.
// Isso compensa variações da tensão de alimentação do relógio original do painel.
// fuel_ratio = f(v_boia / v_feed) → independente da tensão do painel
//
// V1.4+: falha de feed ou boia NÃO vira 0%.
//       O frontend recebe fuel_status separado para exibir FEED/BOIA/ERRO.
// LUT de 5 pontos: coluna X = razão boia/feed, coluna Y = nível 0.0–1.0
// Calibrar em bancada: medir v_boia e v_feed em tanque vazio, 1/4, 1/2, 3/4, cheio.
static FuelStatus_t calcFuelRatio(float v_boia, float v_feed, float* result) {
    *result = 0.0f;

    // Contrato elétrico explícito do frontend analógico de combustível
    if (v_feed < FUEL_V_MIN) return FUEL_ERR_FEED;
    if (v_feed > FUEL_V_MAX) return FUEL_ERR_RANGE;
    if (v_boia < FUEL_V_MIN) return FUEL_ERR_SENSOR;
    if (v_boia > FUEL_V_MAX) return FUEL_ERR_SENSOR;

    float ratio = v_boia / v_feed;  // Normalizado: ~0.0 (vazio) a ~1.0 (cheio)
    if (isnan(ratio) || isinf(ratio) || ratio < 0.0f) return FUEL_ERR_RANGE;
    if (ratio > 1.15f) return FUEL_ERR_RANGE;  // Eletricamente incoerente

    // LUT: {razão_boia_feed, percentual}
    // Valores de exemplo — OBRIGATÓRIO calibrar com o Chevette real
    static const float LUT_R[]   = {0.10f, 0.30f, 0.55f, 0.78f, 0.95f};
    static const float LUT_PCT[] = {0.00f, 0.25f, 0.50f, 0.75f, 1.00f};
    static const int   LUT_SIZE  = 5;

    if (ratio <= LUT_R[0]) {
        *result = LUT_PCT[0];
        return FUEL_OK;
    }
    if (ratio >= LUT_R[LUT_SIZE-1]) {
        *result = LUT_PCT[LUT_SIZE-1];
        return FUEL_OK;
    }

    for (int i = 0; i < LUT_SIZE - 1; i++) {
        if (ratio <= LUT_R[i+1]) {
            float t = (ratio - LUT_R[i]) / (LUT_R[i+1] - LUT_R[i]);
            *result = LUT_PCT[i] + t * (LUT_PCT[i+1] - LUT_PCT[i]);
            return FUEL_OK;
        }
    }

    return FUEL_ERR_RANGE;
}

// =============================================================================
// SEÇÃO 10: PARSER NMEA GPS (RMC) com hemisfério e timeout 2s
// =============================================================================
// CORREÇÃO 5: aplica sinal de hemisfério S (latitude negativa) e W (longitude negativa)
// V1.5: aceita qualquer sentença RMC válida ($GPRMC, $GNRMC, $GLRMC, etc.).
// No Brasil: latitude S → negativa. Longitude W → negativa.
static bool isRMCSentence(const char* sentence) {
    return sentence && sentence[0] == '$' && strlen(sentence) >= 6 &&
           sentence[3] == 'R' && sentence[4] == 'M' && sentence[5] == 'C';
}

static void parseNMEA(const char* sentence, float* speed, float* lat, float* lon, bool* valid) {
    if (!isRMCSentence(sentence)) return;

    char  buf[100];
    strncpy(buf, sentence, 99);
    buf[99] = '\0';

    char* tok    = strtok(buf, ",");
    int   field  = 0;
    char  status = 'V';
    char  ns     = 'N';     // Norte/Sul
    char  ew     = 'E';     // Este/Oeste

    while (tok != nullptr) {
        switch (field) {
            case 2: status = tok[0]; break;
            case 3: {
                if (strlen(tok) > 4) {
                    float r = atof(tok);
                    int   d = (int)(r / 100);
                    *lat = d + (r - d * 100) / 60.0f;
                }
                break;
            }
            case 4: ns = tok[0]; break;     // 'N' ou 'S'
            case 5: {
                if (strlen(tok) > 4) {
                    float r = atof(tok);
                    int   d = (int)(r / 100);
                    *lon = d + (r - d * 100) / 60.0f;
                }
                break;
            }
            case 6: ew = tok[0]; break;     // 'E' ou 'W'
            case 7: *speed = atof(tok) * 1.852f; break;
        }
        tok = strtok(nullptr, ",");
        field++;
    }

    // Aplicar sinal conforme hemisfério — Brasil: S e W → negativos
    if (ns == 'S') *lat = -(*lat);
    if (ew == 'W') *lon = -(*lon);

    *valid = (status == 'A');
}

static bool adsReadRegister16(uint8_t reg, uint16_t* out) {
    if (!out) return false;
    Wire.beginTransmission(ADS_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)ADS_I2C_ADDR, 2) != 2) return false;
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    *out = ((uint16_t)msb << 8) | lsb;
    return true;
}

static bool adsApplyRuntimeConfig() {
    gADS.setDataRate(RATE_ADS1115_128SPS);
    uint16_t cfg = 0;
    return adsReadRegister16(0x01, &cfg);   // Config register real -> prova comunicação útil
}

static bool probeI2CDevice(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}

static bool refreshADSHealth(bool force = false) {
    static bool     cachedStatus = false;
    static uint32_t lastCheckMs  = 0;
    static bool     initialized  = false;

    uint32_t now = millis();
    if (!force && initialized && (now - lastCheckMs) < ADS_RECHECK_MS) {
        return cachedStatus;
    }

    initialized = true;
    lastCheckMs = now;

    uint16_t cfg = 0;
    if (probeI2CDevice(ADS_I2C_ADDR) && adsReadRegister16(0x01, &cfg) && adsApplyRuntimeConfig()) {
        cachedStatus = true;
        return true;
    }

    // Tentar reanexar o ADS em caso de falha momentânea do barramento.
    if (gADS.begin(ADS_I2C_ADDR) && adsApplyRuntimeConfig()) {
        cachedStatus = true;
        return true;
    }

    cachedStatus = false;
    return false;
}

// =============================================================================
// SEÇÃO 11: BLE REALDASH — Frame 0x44 (encoder nativo)
// =============================================================================
static void bleSendFrame(uint16_t frameId, float* values, uint8_t count) {
    if (!gBLEConnected || !gBLECharTX) return;
    if (count > 8) count = 8;

    uint8_t frame[66];
    memset(frame, 0, sizeof(frame));

    frame[0]=0x44; frame[1]=0x33; frame[2]=0x22; frame[3]=0x11;
    frame[4]=(uint8_t)(frameId & 0xFF);
    frame[5]=(uint8_t)((frameId >> 8) & 0xFF);
    frame[6]=count * 4;

    for (int i = 0; i < count; i++) {
        uint8_t* fb = (uint8_t*)&values[i];
        for (int b = 0; b < 4; b++) frame[7 + i*4 + b] = fb[b];
    }

    uint32_t cs = 0;
    for (int i = 0; i < count*4; i++) cs += frame[7+i];
    int csi = 7 + count*4;
    frame[csi+0]=(uint8_t)(cs       & 0xFF);
    frame[csi+1]=(uint8_t)((cs>> 8) & 0xFF);
    frame[csi+2]=(uint8_t)((cs>>16) & 0xFF);
    frame[csi+3]=(uint8_t)((cs>>24) & 0xFF);

    gBLECharTX->setValue(frame, csi + 4);
    gBLECharTX->notify();
}

class BLECallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer*) override {
        gBLEConnected = true;
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(10))) {
            gData.ble_connected = true;
            xSemaphoreGive(gDataMutex);
        }
    }
    void onDisconnect(BLEServer*) override {
        gBLEConnected = false;
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(10))) {
            gData.ble_connected = false;
            xSemaphoreGive(gDataMutex);
        }
        BLEDevice::startAdvertising();
    }
};

// =============================================================================
// SEÇÃO 12: TASK BACKEND — CORE 1 (prioridade 5)
// =============================================================================
static void taskBackend(void* pvParam) {
    vTaskDelay(pdMS_TO_TICKS(100));

    static float          sLastVbus5v    = 5.0f;
    static float          sLastOilPsi    = 0.0f;
    static SensorStatus_t sLastOilStatus = SENSOR_OK;
    static float          sLastWaterTemp = 0.0f;
    static SensorStatus_t sLastWaterSt   = SENSOR_OK;
    static float          sLastAltV      = 0.0f;
    static AltState_t     sLastAltState  = ALT_STATE_BATTERY;

    for (;;) {
        // V1.7: sanidade periódica do ADS1115/I2C com leitura real de registrador.
        bool adsOk = refreshADSHealth();
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(5))) {
            gData.ads_ok = adsOk;
            gData.ads_fault_active = !adsOk;
            xSemaphoreGive(gDataMutex);
        }

        float v_alim     = sLastVbus5v;
        float oil_psi    = sLastOilPsi;
        float water_temp = sLastWaterTemp;
        float alt_v      = sLastAltV;
        float fuel       = 0.0f;
        SensorStatus_t oil_st   = sLastOilStatus;
        SensorStatus_t water_st = sLastWaterSt;
        FuelStatus_t   fuel_st  = FUEL_ERR_FEED;

        if (adsOk) {
            // 1. Referência ratiométrica — DEVE ser a primeira leitura
            v_alim = readVbus5v();

            // 2. Pressão de Óleo
            float v_a0 = adsReadVolts(0, GAIN_TWOTHIRDS);
            oil_st = calcOilPsi(v_a0, v_alim, &oil_psi);

            // 3. Temperatura da Água
            float v_a1 = adsReadVolts(1, GAIN_ONE);
            water_st = calcWaterTemp(v_a1, v_alim, &water_temp);

            // 4. Alternador
            float v_a3 = adsReadVolts(3, GAIN_TWOTHIRDS);
            alt_v = calcAltVoltage(v_a3);

            // Só atualiza cache quando há aquisição íntegra do ADS.
            sLastVbus5v    = v_alim;
            sLastOilPsi    = oil_psi;
            sLastOilStatus = oil_st;
            sLastWaterTemp = water_temp;
            sLastWaterSt   = water_st;
            sLastAltV      = alt_v;
        }

        // 5. Histerese alternador
        // Se ADS falhar, preservar último estado válido em vez de reclassificar por 0V.
        AltState_t alt_st = sLastAltState;
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(5))) {
            alt_st = gData.alt_state;
            if (adsOk) alt_st = updateAltState(alt_v, gData.alt_state);
            xSemaphoreGive(gDataMutex);
        }
        sLastAltState = alt_st;

        // 6. RPM
        uint16_t rpm = calcRpm();

        // 7. Alertas digitais (Active-LOW via PC817)
        bool aw = (digitalRead(PIN_ALERT_WATER) == HIGH);
        bool ab = (digitalRead(PIN_ALERT_BRAKE) == LOW);

        // 8. Temperatura interna ESP32-S3
        float esp_t = temperatureRead();

        // 9. Combustível ratiométrico — Circuito A (boia) e B (feed)
        float v_boia = analogReadMilliVolts(PIN_FUEL_ADC)  / 1000.0f;
        float v_feed = analogReadMilliVolts(PIN_CLOCK_ADC) / 1000.0f;
        fuel_st = calcFuelRatio(v_boia, v_feed, &fuel);

        // 10. Publicar dados (mutex protegido)
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(10))) {
            gData.rpm           = rpm;
            gData.oil_psi       = oil_psi;
            gData.oil_status    = oil_st;
            gData.water_temp_c  = water_temp;
            gData.water_status  = water_st;
            gData.alt_voltage   = alt_v;
            gData.alt_state     = alt_st;
            gData.fuel_ratio    = fuel;
            gData.fuel_status   = fuel_st;
            gData.vbus_5v       = v_alim;
            gData.alert_water   = aw;
            gData.alert_brake   = ab;
            gData.esp32_temp_c  = esp_t;
            xSemaphoreGive(gDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100));     // 10 Hz
    }
}

// =============================================================================
// SEÇÃO 13: TASK GPS — CORE 0 (prioridade 1)
// =============================================================================
static void taskGPS(void* pvParam) {
    char     buf[100];
    uint8_t  idx         = 0;
    bool     discardLine = false;
    uint32_t lastValidMs = 0;

    for (;;) {
        while (gGPS.available()) {
            char c = gGPS.read();
            if (c == '\r') continue;

            if (discardLine) {
                if (c == '\n') {
                    discardLine = false;
                    idx = 0;
                }
                continue;
            }

            if (c == '\n') {
                buf[idx] = '\0';
                idx = 0;

                if (buf[0] != '\0') {
                    float spd = 0.0f, lat = 0.0f, lon = 0.0f;
                    bool  valid = false;
                    parseNMEA(buf, &spd, &lat, &lon, &valid);
                    if (valid) {
                        lastValidMs = millis();
                        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(5))) {
                            gData.gps_speed_kmh = spd;
                            gData.gps_lat       = lat;
                            gData.gps_lon       = lon;
                            xSemaphoreGive(gDataMutex);
                        }
                    }
                }
            } else if (idx >= (sizeof(buf) - 1)) {
                discardLine = true;   // Não parsear sentença truncada
                idx = 0;
            } else {
                buf[idx++] = c;
            }
        }

        bool gv = (lastValidMs != 0) && ((millis() - lastValidMs) < GPS_VALID_TIMEOUT_MS);
        if (xSemaphoreTake(gDataMutex, pdMS_TO_TICKS(5))) {
            gData.gps_valid = gv;
            xSemaphoreGive(gDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// =============================================================================
// SEÇÃO 14: TASK FRONTEND — CORE 0 (prioridade 3)
// TFT ST7796 480×320 + BLE RealDash 10 Hz
// =============================================================================
#define CLR_BG     TFT_BLACK
#define CLR_PANEL  0x1082
#define CLR_ACCENT 0xFD20
#define CLR_GREEN  0x07E0
#define CLR_RED    0xF800
#define CLR_YELLOW 0xFFE0
#define CLR_WHITE  TFT_WHITE
#define CLR_GRAY   0x8410
#define CLR_CYAN   0x07FF
#define TFT_W      480
#define TFT_H      320

static void drawGauge(int x, int y, int w, int h,
                      const char* lbl, const char* val, const char* unit,
                      uint16_t vc, bool alarm) {
    uint16_t bg  = alarm ? 0x3800 : CLR_PANEL;
    uint16_t brd = alarm ? CLR_RED : CLR_ACCENT;
    gTFT.fillRoundRect(x,y,w,h,6,bg);
    gTFT.drawRoundRect(x,y,w,h,6,brd);
    gTFT.setTextColor(CLR_GRAY,bg); gTFT.setTextSize(1);
    gTFT.setCursor(x+6,y+5); gTFT.print(lbl);
    gTFT.setTextColor(vc,bg); gTFT.setTextSize(3);
    int tw = strlen(val)*18;
    gTFT.setCursor(x+(w-tw)/2, y+h/2-12); gTFT.print(val);
    gTFT.setTextColor(CLR_GRAY,bg); gTFT.setTextSize(1);
    int uw = strlen(unit)*6;
    gTFT.setCursor(x+(w-uw)/2, y+h-14); gTFT.print(unit);
}

static void drawFuelBar(int x, int y, int w, int h, float ratio, bool alarm) {
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    gTFT.drawRect(x, y, w, h, alarm ? CLR_RED : CLR_ACCENT);
    if (alarm) {
        gTFT.fillRect(x + 1, y + 1, w - 2, h - 2, CLR_BG);
        return;
    }

    int filled = (int)(ratio * (w - 2));
    uint16_t bc = (ratio < 0.2f) ? CLR_RED : (ratio < 0.4f) ? CLR_YELLOW : CLR_GREEN;
    gTFT.fillRect(x + 1,         y + 1, filled,         h - 2, bc);
    gTFT.fillRect(x + 1 + filled, y + 1, w - 2 - filled, h - 2, CLR_BG);
}

static void drawStatusBar(const TelemetryData_t* d) {
    int sy=TFT_H-18; uint16_t sbg=0x0841;
    gTFT.fillRect(0,sy,TFT_W,18,sbg);
    gTFT.setTextSize(1);

    // BLE
    gTFT.setTextColor(d->ble_connected?CLR_GREEN:CLR_GRAY,sbg);
    gTFT.setCursor(4,sy+5); gTFT.print(d->ble_connected?"BLE OK":"BLE --");

    // GPS
    gTFT.setTextColor(d->gps_valid?CLR_GREEN:CLR_GRAY,sbg);
    gTFT.setCursor(60,sy+5); gTFT.print(d->gps_valid?"GPS OK":"GPS --");

    // 5V_A (se ADS caiu, não fingir leitura física)
    char vb[16];
    if (d->ads_fault_active) {
        snprintf(vb, sizeof(vb), "5VA:--");
        gTFT.setTextColor(CLR_RED, sbg);
    } else {
        snprintf(vb,sizeof(vb),"5VA:%.2fV",d->vbus_5v);
        gTFT.setTextColor(d->vbus_5v>=VREF_5V_MIN?CLR_CYAN:CLR_RED,sbg);
    }
    gTFT.setCursor(120,sy+5); gTFT.print(vb);

    // ADS status
    gTFT.setTextColor(d->ads_ok ? CLR_GRAY : CLR_RED, sbg);
    gTFT.setCursor(190, sy + 5); gTFT.print(d->ads_ok ? "" : "!ADS");

    // Combustível
    gTFT.setTextColor((d->fuel_status == FUEL_OK) ? CLR_GRAY : CLR_RED, sbg);
    gTFT.setCursor(220, sy + 5); gTFT.print((d->fuel_status == FUEL_OK) ? "" : "!FUEL");

    // Temp MCU
    char tb[16]; snprintf(tb,sizeof(tb),"MCU:%.0fC",d->esp32_temp_c);
    gTFT.setTextColor(CLR_GRAY,sbg);
    gTFT.setCursor(275,sy+5); gTFT.print(tb);

    // Alternador/Bateria
    const char* al = (d->alt_state==ALT_STATE_ALTERNATOR)?"ALT":
                     (d->alt_state==ALT_STATE_BATTERY)   ?"BAT":"---";
    char avb[14];
    if (d->ads_fault_active) snprintf(avb,sizeof(avb),"ADS --");
    else                     snprintf(avb,sizeof(avb),"%s %.1fV",al,d->alt_voltage);
    gTFT.setTextColor(CLR_ACCENT,sbg);
    gTFT.setCursor(TFT_W-88,sy+5); gTFT.print(avb);
}

static void taskFrontend(void* pvParam) {
    // Tela de boot
    gTFT.fillScreen(CLR_BG);
    gTFT.setTextColor(CLR_ACCENT,CLR_BG); gTFT.setTextSize(2);
    gTFT.setCursor(100,150); gTFT.print("CHEVETTE ECU  V1.7");
    vTaskDelay(pdMS_TO_TICKS(1500));
    gTFT.fillScreen(CLR_BG);

    // Sub-tarefa GPS no Core 0
    xTaskCreatePinnedToCore(taskGPS,"GPS",4096,nullptr,1,nullptr,0);

    uint32_t        lastBLE = 0;
    TelemetryData_t s = {};

    for (;;) {
        uint32_t t0 = millis();

        // Snapshot thread-safe
        if (xSemaphoreTake(gDataMutex,pdMS_TO_TICKS(10))) {
            memcpy(&s,&gData,sizeof(TelemetryData_t));
            xSemaphoreGive(gDataMutex);
        }

        char buf[24];

        // --- RPM ---
        snprintf(buf,sizeof(buf),"%4d",s.rpm);
        bool rA = (s.rpm > 5500);
        drawGauge(4,4,150,90,"RPM",buf,"rot/min",rA?CLR_RED:CLR_WHITE,rA);

        // --- Velocidade GPS ---
        if (s.gps_valid) snprintf(buf,sizeof(buf),"%3.0f",s.gps_speed_kmh);
        else             strcpy(buf," --");
        drawGauge(160,4,150,90,"VELOCIDADE",buf,"km/h",CLR_WHITE,false);

        // --- Temperatura Água ---
        bool wA;
        if (s.ads_fault_active) {
            strcpy(buf,"ADS");
            wA = true;
        } else {
            wA = (s.water_temp_c > 100.0f) || (s.water_status != SENSOR_OK);
            switch (s.water_status) {
                case SENSOR_ERR_VREF:  strcpy(buf,"REF"); break;
                case SENSOR_ERR_SHORT: strcpy(buf,"CRT"); break;
                case SENSOR_ERR_OPEN:  strcpy(buf,"ABT"); break;
                case SENSOR_ERR_RANGE: strcpy(buf,"ERR"); break;
                default: snprintf(buf,sizeof(buf),"%3.0f",s.water_temp_c);
            }
        }
        drawGauge(316,4,160,90,"AGUA",buf,"graus C",wA?CLR_RED:CLR_CYAN,wA);

        // --- Pressão de Óleo ---
        // CORREÇÃO 3: SENSOR_ERR_VREF mostra "REF" no display, não alarme de pressão
        bool oA;
        if (s.ads_fault_active) {
            strcpy(buf,"ADS");
            oA = true;
        } else if (s.oil_status == SENSOR_ERR_VREF) {
            strcpy(buf,"REF");
            oA = true;  // Alarme visual, mas de referência — não de pressão
        } else if (s.oil_status == SENSOR_ERR_SHORT) {
            strcpy(buf,"CRT");
            oA = true;
        } else if (s.oil_status == SENSOR_ERR_OPEN) {
            strcpy(buf,"ABT");
            oA = true;
        } else if (s.oil_status == SENSOR_ERR_RANGE) {
            strcpy(buf,"ERR");
            oA = true;
        } else {
            snprintf(buf,sizeof(buf),"%3.0f",s.oil_psi);
            oA = (s.oil_psi < 7.0f && s.rpm > 800 && s.oil_status == SENSOR_OK);
        }
        drawGauge(4,100,150,90,"PRESSAO OLEO",buf,"PSI",oA?CLR_RED:CLR_GREEN,oA);

        // --- Tensão Alternador ---
        bool aA;
        if (s.ads_fault_active) {
            strcpy(buf,"ADS");
            aA = true;
        } else {
            snprintf(buf,sizeof(buf),"%.1f",s.alt_voltage);
            aA = (s.alt_voltage < 11.5f || s.alt_voltage > 15.5f);
        }
        drawGauge(160,100,150,90,"TENSAO",buf,"V",aA?CLR_RED:CLR_ACCENT,aA);

        // --- Alertas Água/Freio ---
        {
            int ax=316,ay=100,aw=160,ah=90;
            gTFT.fillRoundRect(ax,ay,aw,ah,6,CLR_PANEL);
            gTFT.drawRoundRect(ax,ay,aw,ah,6,CLR_ACCENT);
            gTFT.setTextSize(1); gTFT.setTextColor(CLR_GRAY,CLR_PANEL);
            gTFT.setCursor(ax+6,ay+5); gTFT.print("ALERTAS");
            gTFT.setTextSize(2);
            gTFT.setTextColor(s.alert_water?CLR_RED:CLR_GRAY,CLR_PANEL);
            gTFT.setCursor(ax+10,ay+22); gTFT.print(s.alert_water?"! AGUA  ":"  AGUA  ");
            gTFT.setTextColor(s.alert_brake?CLR_RED:CLR_GRAY,CLR_PANEL);
            gTFT.setCursor(ax+10,ay+50); gTFT.print(s.alert_brake?"! FREIO ":"  FREIO ");
        }

        // --- Combustível ---
        {
            int fy = 200, fh = 28;
            gTFT.fillRect(4, fy, TFT_W - 8, fh + 30, CLR_BG);
            gTFT.setTextColor(CLR_GRAY, CLR_BG); gTFT.setTextSize(1);
            gTFT.setCursor(4, fy); gTFT.print("COMBUSTIVEL");

            bool fuelAlarm = (s.fuel_status != FUEL_OK);
            drawFuelBar(4, fy + 12, TFT_W - 8, fh, s.fuel_ratio, fuelAlarm);

            if (fuelAlarm) {
                const char* fuelTxt =
                    (s.fuel_status == FUEL_ERR_FEED)   ? "FEED" :
                    (s.fuel_status == FUEL_ERR_SENSOR) ? "BOIA" : "ERRO";
                gTFT.setTextColor(CLR_RED, CLR_BG);
                gTFT.setTextSize(2);
                int tw = strlen(fuelTxt) * 12;
                gTFT.setCursor((TFT_W - tw) / 2, fy + 18);
                gTFT.print(fuelTxt);
                gTFT.setTextSize(1);
            } else {
                snprintf(buf, sizeof(buf), "%3.0f%%", s.fuel_ratio * 100.0f);
                gTFT.setTextColor(CLR_WHITE, CLR_BG);
                gTFT.setCursor(TFT_W / 2 - 12, fy + 18);
                gTFT.print(buf);
            }
        }

        drawStatusBar(&s);

        // --- BLE RealDash 10 Hz ---
        if ((millis()-lastBLE) >= 100) {
            lastBLE = millis();
            float f1[4] = {(float)s.rpm, s.water_temp_c, s.oil_psi, s.alt_voltage};
            bleSendFrame(REALDASH_FRAME_ID_1, f1, 4);
            float f2[4] = {
                s.gps_speed_kmh,
                (s.fuel_status == FUEL_OK) ? (s.fuel_ratio * 100.0f) : -1.0f,
                (float)((s.alert_water?1:0) | (s.alert_brake?2:0)),
                s.esp32_temp_c
            };
            bleSendFrame(REALDASH_FRAME_ID_2, f2, 4);
        }

        // 60 FPS → 16ms por frame
        uint32_t el = millis()-t0;
        if (el < 16) vTaskDelay(pdMS_TO_TICKS(16-el));
    }
}

// =============================================================================
// SEÇÃO 15: SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("[BOOT] Chevette ECU V1.7 - Dossie V21.0");

    // Estado inicial
    gDataMutex = xSemaphoreCreateMutex();
    memset(&gData,0,sizeof(gData));
    gData.alt_state    = ALT_STATE_BATTERY;
    gData.water_status = SENSOR_OK;
    gData.oil_status   = SENSOR_OK;
    gData.fuel_status  = FUEL_OK;
    gData.ads_ok       = false;     // Será confirmado abaixo e revalidado periodicamente
    gData.ads_fault_active = true;

    // GPIO
    pinMode(PIN_RPM_IRQ,     INPUT);        // Pull-up externo 4.7k no hardware
    pinMode(PIN_ALERT_WATER, INPUT_PULLUP);
    pinMode(PIN_ALERT_BRAKE, INPUT_PULLUP);

    // ADC interno — atenuação 11dB por pino nos dois canais de combustível
    analogReadResolution(12);
    analogSetPinAttenuation(PIN_FUEL_ADC, ADC_11db);
    analogSetPinAttenuation(PIN_CLOCK_ADC, ADC_11db);
    Serial.println("[OK] ADC combustivel em 11dB por pino (GPIO5/GPIO6)");

    // Backlight PWM — API unificada ESP32 Arduino Core 3.x
    ledcAttach(PIN_TFT_BL, BL_PWM_FREQ, BL_PWM_RES);
    ledcWrite(PIN_TFT_BL, BL_BRIGHTNESS);

    // I2C — 100 kHz (Dossie V21.0) com pinos explicitos
    // Confirmado no netlist fisico: W31=SDA(GPIO8), W34=SCL(GPIO9)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);
    Serial.printf("[OK] I2C SDA=%d SCL=%d @100kHz\n", PIN_I2C_SDA, PIN_I2C_SCL);

    // ADS1115 — confirmado no boot e revalidado em runtime (V1.7)
    if (gADS.begin(ADS_I2C_ADDR) && adsApplyRuntimeConfig()) {
        gData.ads_ok = true;
        gData.ads_fault_active = false;
        Serial.println("[OK] ADS1115 128SPS + registrador valido");
        refreshADSHealth(true);
    } else {
        gData.ads_ok = false;
        gData.ads_fault_active = true;
        Serial.println("[ERRO] ADS1115 nao encontrado ou sem leitura valida de registrador!");
    }

    // TFT — MISO = -1 definido no User_Setup.h (obrigatório)
    gTFT.init();
    gTFT.setRotation(1);
    gTFT.fillScreen(TFT_BLACK);
    Serial.println("[OK] TFT ST7796");

    // GPS UART1 — buffer 512 bytes (Dossiê V21.0)
    gGPS.setRxBufferSize(512);
    gGPS.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.println("[OK] GPS UART1 RX=18 TX=17 (Matrix V5.2)");

    // BLE RealDash — Nordic UART Service
    BLEDevice::init(BLE_DEVICE_NAME);
    gBLEServer = BLEDevice::createServer();
    gBLEServer->setCallbacks(new BLECallbacks());
    BLEService* svc = gBLEServer->createService(BLE_SERVICE_UUID);
    gBLECharTX = svc->createCharacteristic(BLE_CHAR_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
    gBLECharTX->addDescriptor(new BLE2902());
    svc->createCharacteristic(BLE_CHAR_RX_UUID, BLECharacteristic::PROPERTY_WRITE);
    svc->start();
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SERVICE_UUID);
    adv->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("[OK] BLE: " BLE_DEVICE_NAME);

    // ISR RPM — FALLING: H11L1M saída vai LOW no disparo (captura flyback)
#if CHEVETTE_HAS_GPIO_GLITCH_FILTER
    {
        gpio_pin_glitch_filter_config_t cfg = {};
        cfg.gpio_num = (gpio_num_t)PIN_RPM_IRQ;
        #ifdef GPIO_GLITCH_FILTER_CLK_SRC_DEFAULT
        cfg.clk_src = GPIO_GLITCH_FILTER_CLK_SRC_DEFAULT;
        #endif
        if (gpio_new_pin_glitch_filter(&cfg, &gRpmGlitchFilter) == ESP_OK &&
            gpio_glitch_filter_enable(gRpmGlitchFilter) == ESP_OK) {
            Serial.println("[OK] Glitch filter HW RPM habilitado");
        } else {
            Serial.println("[WARN] Falha ao habilitar glitch filter HW RPM; usando debounce SW");
            gRpmGlitchFilter = nullptr;
        }
    }
#else
    Serial.println("[INFO] API de glitch filter HW indisponivel; usando debounce SW");
#endif
    attachInterrupt(digitalPinToInterrupt(PIN_RPM_IRQ), isrRpm, FALLING);
    Serial.println("[OK] ISR RPM GPIO4 FALLING");

    // FreeRTOS Tasks
    xTaskCreatePinnedToCore(taskBackend,  "Backend",  8192,  nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(taskFrontend, "Frontend", 16384, nullptr, 3, nullptr, 0);

    Serial.println("[BOOT] Sistema operacional.");
}

// =============================================================================
// SEÇÃO 16: LOOP (vazio — lógica nas Tasks FreeRTOS)
// =============================================================================
void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// =============================================================================
// FIM - Chevette ECU Firmware V1.7
// Dossie V21.0 (The Software Bridge) | Hardware Homologado
// Pinos GPS : RX=18 (W30), TX=17 (W29) — Matrix V5.2
// Backlight : ledcAttach() — ESP32 Arduino Core 3.x
// RPM       : glitch filter HW quando disponível + debounce SW obrigatório
// Combustivel: ratiometrico (boia/feed) - Circuito A + B ativos
// I2C confirmado por netlist: SDA=GPIO8 (W31), SCL=GPIO9 (W34)
// =============================================================================
