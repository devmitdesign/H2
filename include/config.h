\
#pragma once
#include <Arduino.h>

// ---------------- Build Feature Flags ----------------
#ifndef ENABLE_SD
#define ENABLE_SD 1
#endif

#ifndef ENABLE_CAN1_TWAI
#define ENABLE_CAN1_TWAI 1
#endif

#ifndef ENABLE_CAN2_MCP2515
#define ENABLE_CAN2_MCP2515 1
#endif

#ifndef ENABLE_SENSORS
#define ENABLE_SENSORS 1
#endif

#ifndef ENABLE_GPS_GTU7
#define ENABLE_GPS_GTU7 1
#endif

#ifndef ENABLE_WIFI
#define ENABLE_WIFI 1
#endif

#ifndef ENABLE_MQTT
#define ENABLE_MQTT 1
#endif

// LTE PPP skeleton is provided but disabled by default (implementation depends on your SIM7600 board and desired stack)
#ifndef ENABLE_LTE_PPP
#define ENABLE_LTE_PPP 0
#endif

// ---------------- Serial ----------------
static constexpr uint32_t SERIAL_BAUD = 115200;

// ---------------- I2C ----------------
static constexpr int PIN_I2C_SDA = 21;
static constexpr int PIN_I2C_SCL = 22;
static constexpr uint8_t ADXL345_ADDR = 0x53; // also possible 0x1D
static constexpr uint8_t BMP280_ADDR  = 0x76; // also possible 0x77
static constexpr float SEA_LEVEL_HPA  = 1013.25f; // adjust for your location

// ---------------- CAN1 (ESP32 TWAI) ----------------
static constexpr int PIN_CAN1_TX = 32;
static constexpr int PIN_CAN1_RX = 33;
static constexpr uint32_t CAN1_BITRATE = 500000;

// ---------------- CAN2 (MCP2515 on HSPI) ----------------
// NOTE: If your MCP2515 module is powered at 5V, protect ESP32 inputs:
//  - MCP SO/MISO -> ESP32 MISO via divider/level shifter
//  - MCP INT     -> ESP32 INT  via divider/level shifter
static constexpr int PIN_CAN2_SCK  = 26;
static constexpr int PIN_CAN2_MISO = 27; // ESP32 input
static constexpr int PIN_CAN2_MOSI = 13;
static constexpr int PIN_CAN2_CS   = 5;
static constexpr int PIN_CAN2_INT  = 35; // input-only OK
static constexpr uint32_t MCP2515_QUARTZ_HZ = 16UL * 1000UL * 1000UL; // set to 8MHz if your oscillator says 8.000
static constexpr uint32_t CAN2_BITRATE = 500000;

// ---------------- SD (VSPI) ----------------
static constexpr int PIN_SD_SCK  = 18;
static constexpr int PIN_SD_MISO = 19;
static constexpr int PIN_SD_MOSI = 23;
static constexpr int PIN_SD_CS   = 25;
static constexpr uint32_t SD_SPI_HZ = 10UL * 1000UL * 1000UL;

// Logging config
static constexpr uint32_t LOG_QUEUE_DEPTH = 512; // enough for bursts
static constexpr uint32_t SEGMENT_MAX_BYTES = 2UL * 1024UL * 1024UL; // 2MB segments
static constexpr uint32_t SD_FLUSH_EVERY_N_RECORDS = 64; // tradeoff wear vs safety
static constexpr uint32_t SD_FLUSH_EVERY_MS = 500;

// ---------------- Nextion (UART2) ----------------
static constexpr int PIN_NEXTION_RX2 = 16; // ESP32 RX2 <- Nextion TX (use divider if 5V)
static constexpr int PIN_NEXTION_TX2 = 17; // ESP32 TX2 -> Nextion RX
static constexpr uint32_t NEXTION_BAUD = 115200; // set to your HMI baud

// ---------------- GPS GT-U7 (UART1) ----------------
static constexpr int PIN_GPS_RX1 = 34; // ESP32 RX1 <- GPS TX (divider if GPS TX is 5V)
static constexpr int PIN_GPS_TX1 = 4;  // ESP32 TX1 -> GPS RX (optional)
static constexpr uint32_t GPS_BAUD = 9600;

// ---------------- Wi-Fi / MQTT ----------------
static constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
static constexpr const char* WIFI_PASS = "YOUR_WIFI_PASS";

// For local broker testing
static constexpr const char* MQTT_HOST = "192.168.1.10";
static constexpr uint16_t    MQTT_PORT = 1883;
static constexpr const char* MQTT_CLIENT_ID = "esp32_dualcan_logger";
static constexpr const char* MQTT_TOPIC_LIVE = "device/esp32/can/live";
static constexpr const char* MQTT_TOPIC_STATUS = "device/esp32/status";

// ---------------- Power-fail detection ----------------
// Sense your 12V or 5V rail using a divider into ADC1 (GPIO36).
// Example: 100k/22k from 12V -> ~2.16V at ADC, safe.
static constexpr int PIN_PWR_SENSE_ADC = 36; // ADC1_CH0
static constexpr uint16_t PWR_FAIL_ADC_THRESHOLD = 1700; // tune per divider and calibration
static constexpr uint32_t PWR_FAIL_DEBOUNCE_MS = 30;
