\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "can2_mcp2515.h"
#include "log_record.h"

#if ENABLE_CAN2_MCP2515
#include <SPI.h>
#include <ACAN2515.h>
#include "esp_timer.h"

static SPIClass gSpiCAN(HSPI);
static ACAN2515 gCan(PIN_CAN2_CS, gSpiCAN, PIN_CAN2_INT);

static void IRAM_ATTR can_isr() { gCan.isr(); }

static bool can2_init() {
  pinMode(PIN_CAN2_CS, OUTPUT);
  digitalWrite(PIN_CAN2_CS, HIGH);

  gSpiCAN.begin(PIN_CAN2_SCK, PIN_CAN2_MISO, PIN_CAN2_MOSI, PIN_CAN2_CS);

  ACAN2515Settings settings(MCP2515_QUARTZ_HZ, CAN2_BITRATE);
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = 64;
  settings.mTransmitBufferSize = 16;

  uint16_t err = gCan.begin(settings, can_isr);
  if (err != 0) {
    Serial.printf("CAN2: ACAN2515 begin error 0x%04X\n", err);
    return false;
  }
  Serial.println("CAN2: MCP2515 started");
  xEventGroupSetBits(gSysEvents, EV_CAN2_UP);
  return true;
}

static void can2_task(void*) {
  if (!can2_init()) { vTaskDelete(nullptr); return; }

  CANMessage m;
  for (;;) {
    while (gCan.receive(m)) {
      LogRecord32 r{};
      r.t_us = (uint64_t)esp_timer_get_time();
      r.type = 1;
      r.source = 2;
      r.len = m.len;
      r.flags = (m.ext ? 0x01 : 0) | (m.rtr ? 0x02 : 0);
      r.id = m.id;
      for (uint8_t i = 0; i < 8; i++) r.data[i] = m.data[i];
      (void)logq_push(r, 0);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void start_can2_task() {
  xTaskCreatePinnedToCore(can2_task, "can2", 4096, nullptr, 4, nullptr, 0);
}
#else
void start_can2_task() {}
#endif
