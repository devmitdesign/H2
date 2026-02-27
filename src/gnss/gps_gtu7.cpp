\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "log_record.h"

#if ENABLE_GPS_GTU7
#include <TinyGPSPlus.h>
#include "esp_timer.h"

static HardwareSerial gGps(1);
static TinyGPSPlus gParser;

static void pack_f32_le(uint8_t* out, float v) {
  union { float f; uint8_t b[4]; } u;
  u.f = v;
  out[0]=u.b[0]; out[1]=u.b[1]; out[2]=u.b[2]; out[3]=u.b[3];
}

static void gps_task(void*) {
  gGps.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX1, PIN_GPS_TX1);
  Serial.println("GPS: UART started");

  uint32_t lastEmitMs = 0;
  for (;;) {
    while (gGps.available()) gParser.encode((char)gGps.read());

    uint32_t now = millis();
    if (now - lastEmitMs >= 1000) {
      lastEmitMs = now;
      bool fix = gParser.location.isValid();
      float lat = fix ? (float)gParser.location.lat() : 0.0f;
      float lon = fix ? (float)gParser.location.lng() : 0.0f;
      float alt = gParser.altitude.isValid() ? (float)gParser.altitude.meters() : 0.0f;
      float spd = gParser.speed.isValid() ? (float)gParser.speed.kmph() : 0.0f;

      LogRecord32 r{};
      r.t_us = (uint64_t)esp_timer_get_time();
      r.type = 4; r.source = 0; r.flags = fix ? 1 : 0; r.id = 0x4750; r.len = 8;
      pack_f32_le(&r.data[0], lat);
      pack_f32_le(&r.data[4], lon);
      (void)logq_push(r, 0);

      LogRecord32 r2{};
      r2.t_us = r.t_us;
      r2.type = 4; r2.source = 0; r2.flags = r.flags; r2.id = 0x4751; r2.len = 8;
      pack_f32_le(&r2.data[0], alt);
      pack_f32_le(&r2.data[4], spd);
      (void)logq_push(r2, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void start_gps_task() {
  xTaskCreatePinnedToCore(gps_task, "gps", 4096, nullptr, 1, nullptr, 1);
}
#else
void start_gps_task() {}
#endif
