#include "can1_twai.h"
#include "config.h"
#include "logger.h"
#include "log_types.h"

#include "driver/twai.h"

// TWAI is ESP32's built-in CAN controller :contentReference[oaicite:14]{index=14}

static twai_timing_config_t timing_from_bitrate(uint32_t bitrate) {
  switch (bitrate) {
    case 1000000: return TWAI_TIMING_CONFIG_1MBITS();
    case 500000:  return TWAI_TIMING_CONFIG_500KBITS();
    case 250000:  return TWAI_TIMING_CONFIG_250KBITS();
    case 125000:  return TWAI_TIMING_CONFIG_125KBITS();
    default:      return TWAI_TIMING_CONFIG_500KBITS();
  }
}

bool can1_init() {
  const twai_general_config_t g_config =
    TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)PIN_CAN1_TX, (gpio_num_t)PIN_CAN1_RX, TWAI_MODE_NORMAL);

  const twai_timing_config_t t_config = timing_from_bitrate(CAN1_BITRATE);
  const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("CAN1: twai_driver_install failed");
    return false;
  }
  if (twai_start() != ESP_OK) {
    Serial.println("CAN1: twai_start failed");
    return false;
  }
  Serial.println("CAN1 (TWAI) started");
  return true;
}

void can1_task(void*) {
  twai_message_t msg{};

  for (;;) {
    // wait for frame
    if (twai_receive(&msg, pdMS_TO_TICKS(50)) == ESP_OK) { // receive API per Espressif docs :contentReference[oaicite:15]{index=15}
      LogRecord r{};
      r.t_ms = millis();
      r.type = LogType::CAN1;

      r.u.can.id  = msg.identifier;
      r.u.can.ext = (msg.extd != 0);
      r.u.can.rtr = (msg.rtr != 0);
      r.u.can.dlc = msg.data_length_code;
      for (uint8_t i = 0; i < 8; i++) r.u.can.data[i] = msg.data[i];

      log_enqueue(r);
    } else {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}