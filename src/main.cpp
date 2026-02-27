\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "start_tasks.h"
#include "can1_twai.h"
#include "can2_mcp2515.h"
#include "sensors_task.h"
#include "power_fail.h"
#include "wifi_manager.h"
#include "nextion_ui.h"
#include "sim7600_modem.h"
#include "mqtt_streamer.h"

void setup(){
  Serial.begin(SERIAL_BAUD);
  delay(200);
  Serial.println("\\n=== ESP32 Dual-CAN Logger (SIM7600 + MQTT) ===");

  if(!event_bus_init()){
    Serial.println("FATAL: event bus init failed");
    for(;;) delay(1000);
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

  start_nextion_task();

#if ENABLE_WIFI
  start_wifi_task();
#endif
#if ENABLE_MODEM_SIM7600
  start_sim7600_task();
#endif
#if ENABLE_MQTT
  start_mqtt_task();
#endif

  Serial.println("Setup complete.");
}

void loop(){ vTaskDelay(pdMS_TO_TICKS(1000)); }
