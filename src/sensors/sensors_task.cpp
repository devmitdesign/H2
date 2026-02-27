\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "log_record.h"

#if ENABLE_SENSORS
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP280.h>
#include "esp_timer.h"

static Adafruit_ADXL345_Unified gAdxl(12345);
static Adafruit_BMP280 gBmp;

static bool sensors_init() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!gAdxl.begin(ADXL345_ADDR)) Serial.println("ADXL345: not found");
  else { gAdxl.setRange(ADXL345_RANGE_16_G); Serial.println("ADXL345: OK"); }

  if (!gBmp.begin(BMP280_ADDR)) Serial.println("BMP280: not found");
  else Serial.println("BMP280: OK");

  return true;
}

static void pack_f32_le(uint8_t* out, float v) {
  union { float f; uint8_t b[4]; } u;
  u.f = v;
  out[0]=u.b[0]; out[1]=u.b[1]; out[2]=u.b[2]; out[3]=u.b[3];
}

static void sensors_task(void*) {
  sensors_init();

  TickType_t lastAccel = xTaskGetTickCount();
  TickType_t lastBaro  = xTaskGetTickCount();

  for (;;) {
    if (xTaskGetTickCount() - lastAccel >= pdMS_TO_TICKS(50)) {
      lastAccel = xTaskGetTickCount();
      sensors_event_t ev{};
      if (gAdxl.getEvent(&ev)) {
        float ax = ev.acceleration.x / 9.80665f;
        float ay = ev.acceleration.y / 9.80665f;
        float az = ev.acceleration.z / 9.80665f;

        LogRecord32 r{};
        r.t_us = (uint64_t)esp_timer_get_time();
        r.type = 2; r.source = 0; r.flags = 0; r.id = 0xA345; r.len = 8;
        pack_f32_le(&r.data[0], ax);
        pack_f32_le(&r.data[4], ay);
        (void)logq_push(r, 0);

        LogRecord32 r2{};
        r2.t_us = r.t_us;
        r2.type = 2; r2.source = 0; r2.flags = 0; r2.id = 0xA346; r2.len = 4;
        pack_f32_le(&r2.data[0], az);
        (void)logq_push(r2, 0);
      }
    }

    if (xTaskGetTickCount() - lastBaro >= pdMS_TO_TICKS(200)) {
      lastBaro = xTaskGetTickCount();
      float tempC = gBmp.readTemperature();
      float pressHpa = gBmp.readPressure() / 100.0f;
      float altM = gBmp.readAltitude(SEA_LEVEL_HPA);

      LogRecord32 r{};
      r.t_us = (uint64_t)esp_timer_get_time();
      r.type = 3; r.source = 0; r.flags = 0; r.id = 0xB280; r.len = 8;
      pack_f32_le(&r.data[0], tempC);
      pack_f32_le(&r.data[4], pressHpa);
      (void)logq_push(r, 0);

      LogRecord32 r2{};
      r2.t_us = r.t_us;
      r2.type = 3; r2.source = 0; r2.flags = 0; r2.id = 0xB281; r2.len = 4;
      pack_f32_le(&r2.data[0], altM);
      (void)logq_push(r2, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void start_sensors_task() {
  xTaskCreatePinnedToCore(sensors_task, "sensors", 6144, nullptr, 2, nullptr, 1);
}
#else
void start_sensors_task() {}
#endif
