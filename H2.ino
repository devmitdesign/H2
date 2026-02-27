#include <Arduino.h>
#include "config.h"

#include "logger.h"
#include "can1_twai.h"
#include "can2_mcp2515.h"
#include "accel_adxl345.h"
#include "baro_bmp280.h"
#include "gps_gtu7.h"
#include "sim800l.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println("\nESP32 Multi-Logger starting...");

  gLogQueue = xQueueCreate(256, sizeof(LogRecord));
  if (!gLogQueue) {
    Serial.println("Failed to create log queue");
    for (;;) delay(1000);
  }

  // Init SD first
  if (!logger_init_sd()) {
    Serial.println("SD init failed (logging will NOT work)");
    // You can choose to halt here:
    // for (;;) delay(1000);
  }

  // Init peripherals
  (void)accel_init();
  (void)baro_init();
  (void)gps_init();

  if (!can1_init()) Serial.println("CAN1 init failed");
  if (!can2_init()) Serial.println("CAN2 init failed");

  // Start tasks
  xTaskCreatePinnedToCore(logger_task, "logger", 6144, nullptr, 3, nullptr, 1);

  xTaskCreatePinnedToCore(can1_task, "can1", 4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(can2_task, "can2", 4096, nullptr, 2, nullptr, 0);

  xTaskCreatePinnedToCore(accel_task, "accel", 4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(baro_task,  "baro",  4096, nullptr, 1, nullptr, 1);
  xTaskCreatePinnedToCore(gps_task,   "gps",   4096, nullptr, 1, nullptr, 1);

  if (!sim800l_init()) {
    Serial.println("SIM800L init failed");
  } else {
    xTaskCreatePinnedToCore(sim800l_task, "sim800", 4096, nullptr, 1, nullptr, 1);
  }

  Serial.println("All tasks started");
}

void loop() {
  // nothing - tasks run under FreeRTOS
  vTaskDelay(pdMS_TO_TICKS(1000));
}