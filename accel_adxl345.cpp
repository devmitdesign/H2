#include "accel_adxl345.h"
#include "config.h"
#include "logger.h"
#include "log_types.h"

#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_Sensor.h>

static Adafruit_ADXL345_Unified gAccel = Adafruit_ADXL345_Unified(12345);

bool accel_init() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!gAccel.begin(ADXL345_ADDR)) {
    Serial.println("ADXL345 not found");
    return false;
  }
  // Optional: set range (2/4/8/16g)
  gAccel.setRange(ADXL345_RANGE_16_G);
  Serial.println("ADXL345 ready");
  return true;
}

void accel_task(void*) {
  constexpr TickType_t period = pdMS_TO_TICKS(50); // 20 Hz
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    sensors_event_t event;
    gAccel.getEvent(&event);

    LogRecord r{};
    r.t_ms = millis();
    r.type = LogType::ACCEL;
    // event.acceleration is m/s^2; convert to g
    r.u.accel.ax_g = event.acceleration.x / 9.80665f;
    r.u.accel.ay_g = event.acceleration.y / 9.80665f;
    r.u.accel.az_g = event.acceleration.z / 9.80665f;

    log_enqueue(r);
    vTaskDelayUntil(&last, period);
  }
}