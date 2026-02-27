\
#pragma once
#include <Arduino.h>

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
#define ENABLE_GPS_GTU7 0
#endif
#ifndef ENABLE_MODEM_SIM7600
#define ENABLE_MODEM_SIM7600 1
#endif
#ifndef ENABLE_WIFI
#define ENABLE_WIFI 1
#endif
#ifndef ENABLE_MQTT
#define ENABLE_MQTT 1
#endif

static constexpr uint32_t SERIAL_BAUD = 115200;

// I2C
static constexpr int PIN_I2C_SDA = 21;
static constexpr int PIN_I2C_SCL = 22;
static constexpr uint8_t ADXL345_ADDR = 0x53;
static constexpr uint8_t BMP280_ADDR  = 0x76;
static constexpr float SEA_LEVEL_HPA  = 1013.25f;

// CAN1
static constexpr int PIN_CAN1_TX = 32;
static constexpr int PIN_CAN1_RX = 33;
static constexpr uint32_t CAN1_BITRATE = 500000;

// CAN2 MCP2515 (HSPI)
static constexpr int PIN_CAN2_SCK  = 26;
static constexpr int PIN_CAN2_MISO = 27;
static constexpr int PIN_CAN2_MOSI = 13;
static constexpr int PIN_CAN2_CS   = 5;
static constexpr int PIN_CAN2_INT  = 35;
static constexpr uint32_t MCP2515_QUARTZ_HZ = 16UL * 1000UL * 1000UL;
static constexpr uint32_t CAN2_BITRATE = 500000;

// SD (VSPI)
static constexpr int PIN_SD_SCK  = 18;
static constexpr int PIN_SD_MISO = 19;
static constexpr int PIN_SD_MOSI = 23;
static constexpr int PIN_SD_CS   = 25;
static constexpr uint32_t SD_SPI_HZ = 10UL * 1000UL * 1000UL;

// Logging
static constexpr uint32_t LOG_QUEUE_DEPTH = 512;
static constexpr uint32_t STREAM_QUEUE_DEPTH = 256;
static constexpr uint32_t SEGMENT_MAX_BYTES = 2UL * 1024UL * 1024UL;
static constexpr uint32_t SD_FLUSH_EVERY_N_RECORDS = 64;
static constexpr uint32_t SD_FLUSH_EVERY_MS = 500;

// Nextion (UART2)
static constexpr int PIN_NEXTION_RX2 = 16;
static constexpr int PIN_NEXTION_TX2 = 17;
static constexpr uint32_t NEXTION_BAUD = 115200;

// SIM7600 (UART1)
static constexpr int PIN_SIM7600_RX1 = 34; // ESP32 RX <- SIM7600 TXD
static constexpr int PIN_SIM7600_TX1 = 14; // ESP32 TX -> SIM7600 RXD
static constexpr uint32_t SIM7600_BAUD = 115200;

static constexpr int PIN_SIM7600_PWRKEY = -1;
static constexpr int PIN_SIM7600_RST    = -1;

static constexpr const char* LTE_APN  = "internet";
static constexpr const char* LTE_USER = "";
static constexpr const char* LTE_PASS = "";

// Wi-Fi / MQTT
static constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
static constexpr const char* WIFI_PASS = "YOUR_WIFI_PASS";

static constexpr const char* MQTT_HOST = "trevis.yourdomain.com";
static constexpr uint16_t    MQTT_PORT = 1883;
static constexpr const char* MQTT_CLIENT_ID = "esp32_dualcan_logger";
static constexpr const char* MQTT_TOPIC_LIVE   = "trevis/devices/esp32/can/live";
static constexpr const char* MQTT_TOPIC_STATUS = "trevis/devices/esp32/status";

// Power fail ADC
static constexpr int PIN_PWR_SENSE_ADC = 36;
static constexpr uint16_t PWR_FAIL_ADC_THRESHOLD = 1700;
static constexpr uint32_t PWR_FAIL_DEBOUNCE_MS = 30;
