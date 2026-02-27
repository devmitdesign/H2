#pragma once
#include <Arduino.h>

// ---------- Serial ----------
static constexpr uint32_t SERIAL_BAUD = 115200;

// ---------- I2C ----------
static constexpr int PIN_I2C_SDA = 21;
static constexpr int PIN_I2C_SCL = 22;

// ADXL345 address: 0x53 or 0x1D depending on SDO/ALT pin :contentReference[oaicite:11]{index=11}
static constexpr uint8_t ADXL345_ADDR = 0x53;

// BMP280 address: commonly 0x76 or 0x77 depending on SDO :contentReference[oaicite:12]{index=12}
static constexpr uint8_t BMP280_ADDR = 0x76;
static constexpr float SEA_LEVEL_HPA = 1013.25f; // adjust for better altitude

// ---------- CAN1 (ESP32 TWAI) ----------
static constexpr int PIN_CAN1_TX = 32;
static constexpr int PIN_CAN1_RX = 33;
static constexpr uint32_t CAN1_BITRATE = 500000; // change if needed

// ---------- CAN2 (MCP2515 via ACAN2515 on HSPI) ----------
static constexpr int PIN_CAN2_SCK  = 26;
static constexpr int PIN_CAN2_MISO = 27;
static constexpr int PIN_CAN2_MOSI = 13;
static constexpr int PIN_CAN2_CS   = 5;
static constexpr int PIN_CAN2_INT  = 35; // input-only OK
static constexpr uint32_t MCP2515_QUARTZ_HZ = 16UL * 1000UL * 1000UL; // verify oscillator can; change if 8MHz
static constexpr uint32_t CAN2_BITRATE = 500000;

// ---------- SD card (VSPI) ----------
static constexpr int PIN_SD_SCK  = 18;
static constexpr int PIN_SD_MISO = 19;
static constexpr int PIN_SD_MOSI = 23;
static constexpr int PIN_SD_CS   = 25;
static constexpr uint32_t SD_SPI_HZ = 10UL * 1000UL * 1000UL;

// ---------- Nextion (UART2) ----------
static constexpr int PIN_NEXTION_RX2 = 16; // ESP32 RX2 <- Nextion TX (level shift down recommended)
static constexpr int PIN_NEXTION_TX2 = 17; // ESP32 TX2 -> Nextion RX

// ---------- SIM800L (UART1) ----------
static constexpr int PIN_SIM800_RX = 34; // ESP32 RX <- SIM800 TXD (no divider needed)
static constexpr int PIN_SIM800_TX = 14; // ESP32 TX -> SIM800 RXD (use divider to ~2.8V)
static constexpr int PIN_SIM800_RST = -1; // optional, set to GPIO if you add reset transistor
static constexpr uint32_t SIM800_BAUD = 9600; // many boards default to 9600

// ---------- GPS (RX-only SoftwareSerial) ----------
static constexpr int PIN_GPS_RX_SOFT = 39; // ESP32 input-only is OK
static constexpr uint32_t GPS_BAUD = 9600;