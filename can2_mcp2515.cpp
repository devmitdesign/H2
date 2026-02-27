#include "can2_mcp2515.h"
#include "config.h"
#include "logger.h"
#include "log_types.h"

#include <SPI.h>
#include <ACAN2515.h>

// ACAN2515 supports choosing an SPI instance (SPI, SPI2, etc.) :contentReference[oaicite:16]{index=16}
static SPIClass gSpiCAN(HSPI);
static ACAN2515 gCan2(PIN_CAN2_CS, gSpiCAN, PIN_CAN2_INT);

static void IRAM_ATTR can2_isr() {
  gCan2.isr();
}

bool can2_init() {
  pinMode(PIN_CAN2_CS, OUTPUT);
  digitalWrite(PIN_CAN2_CS, HIGH);

  gSpiCAN.begin(PIN_CAN2_SCK, PIN_CAN2_MISO, PIN_CAN2_MOSI, PIN_CAN2_CS);

  ACAN2515Settings settings(MCP2515_QUARTZ_HZ, CAN2_BITRATE);
  settings.mRequestedMode = ACAN2515Settings::NormalMode;
  settings.mReceiveBufferSize = 64;
  settings.mTransmitBufferSize = 16;

  const uint16_t errorCode = gCan2.begin(settings, can2_isr);
  if (errorCode != 0) {
    Serial.print("CAN2: ACAN2515 begin error 0x");
    Serial.println(errorCode, HEX);
    return false;
  }

  Serial.println("CAN2 (MCP2515) started");
  return true;
}

void can2_task(void*) {
  CANMessage frame;

  for (;;) {
    if (gCan2.receive(frame)) {
      LogRecord r{};
      r.t_ms = millis();
      r.type = LogType::CAN2;

      r.u.can.id  = frame.id;
      r.u.can.ext = frame.ext;
      r.u.can.rtr = frame.rtr;
      r.u.can.dlc = frame.len;
      for (uint8_t i = 0; i < 8; i++) r.u.can.data[i] = frame.data[i];

      log_enqueue(r);
    } else {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}