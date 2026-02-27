\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"

static HardwareSerial gNextion(2);

static void nextion_task(void*) {
  gNextion.begin(NEXTION_BAUD, SERIAL_8N1, PIN_NEXTION_RX2, PIN_NEXTION_TX2);
  vTaskDelay(pdMS_TO_TICKS(500));
  Serial.println("Nextion: UART2 started (stub)");

  for (;;) {
    // TODO: your Nextion protocol
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void start_nextion_task() {
  xTaskCreatePinnedToCore(nextion_task, "nextion", 4096, nullptr, 1, nullptr, 1);
}
