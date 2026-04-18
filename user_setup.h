User setup - .h

// =============================================================================
// User_Setup.h — TFT_eSPI
// Projeto: Telemetria Chevette ESP32-S3
// Compativel com: Firmware V1.7 | Dossie V21.0
//
// INSTRUÇÕES:
// 1. Localize a pasta da biblioteca TFT_eSPI no Arduino.
// 2. Substitua o arquivo User_Setup.h existente por este.
// 3. Reinicie a IDE se ela já estiver aberta.
// 4. Este arquivo é a fonte autoritativa da pinagem SPI/RST da TFT.
// 5. Se alterar a pinagem aqui, reflita a mudança no firmware.
// =============================================================================

// -----------------------------------------------------------------------------
/* Driver do display */
// -----------------------------------------------------------------------------
#define ST7796_DRIVER

// -----------------------------------------------------------------------------
/* Dimensões físicas do painel */
// -----------------------------------------------------------------------------
#define TFT_WIDTH   320
#define TFT_HEIGHT  480

// -----------------------------------------------------------------------------
/* Pinos SPI - Dossie V21.0 / Firmware V1.7 */
// -----------------------------------------------------------------------------
#define TFT_MOSI  11   // GPIO 11 — MOSI
#define TFT_SCLK  12   // GPIO 12 — SCK
#define TFT_CS    10   // GPIO 10 — Chip Select
#define TFT_DC    14   // GPIO 14 — Data / Command
#define TFT_RST   21   // GPIO 21 — Reset
#define TFT_MISO  -1   // CRÍTICO: desabilita leitura SPI
#define TFT_BL    13   // GPIO 13 — Backlight (referência do projeto)

// -----------------------------------------------------------------------------
/* Frequências SPI */
// -----------------------------------------------------------------------------
#define SPI_FREQUENCY       40000000   // 40 MHz
#define SPI_READ_FREQUENCY  20000000   // Irrelevante com TFT_MISO = -1
#define SPI_TOUCH_FREQUENCY  2500000   // Sem touch neste projeto

// -----------------------------------------------------------------------------
/* Fontes embutidas */
// -----------------------------------------------------------------------------
#define LOAD_GLCD_FONT
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

// -----------------------------------------------------------------------------
/* DMA */
// -----------------------------------------------------------------------------
#define ESP32_DMA

// -----------------------------------------------------------------------------
/* Opções */
// -----------------------------------------------------------------------------
// #define SMOOTH_FONT

// =============================================================================
// FIM - User_Setup.h | Telemetria Chevette ESP32-S3 | Dossie V21.0
// =============================================================================
