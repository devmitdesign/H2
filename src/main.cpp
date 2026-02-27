\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "start_tasks.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println("\n=== ESP32 Dual-CAN Logger Skeleton ===");

  if (!event_bus_init()) {
    Serial.println("FATAL: event_bus_init failed");
    for (;;) delay(1000);
  }

  start_power_task();
#if ENABLE_SD
  start_sd_writer_task();
#endif
#if ENABLE_CAN1_TWAI
  start_can1_task();
#endif
#if ENABLE_CAN2_MCP2515
  start_can2_task();
#endif
#if ENABLE_SENSORS
  start_sensors_task();
#endif
#if ENABLE_GPS_GTU7
  start_gps_task();
#endif

  start_nextion_task();

#if ENABLE_WIFI
  start_wifi_task();
#endif
#if ENABLE_LTE_PPP
  start_lte_task();
#endif
#if ENABLE_MQTT
  start_mqtt_task();
#endif

  Serial.println("Setup complete. Tasks running.");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
