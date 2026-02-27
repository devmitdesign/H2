#include "baro_bmp280.h"
#include "config.h"
#include "logger.h"
#include "log_types.h"

#include <Wire.h>
#include <Adafruit_BMP280.h>

static Adafruit_BMP280 gBmp;

bool baro_init() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!gBmp.begin(BMP280_ADDR)) {
    Serial.println("BMP280 not found");
    return false;
  }
  Serial.println("BMP280 ready");
  return true;
}

void baro_task(void*) {
  constexpr TickType_t period = pdMS_TO_TICKS(200); // 5 Hz
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    const float tempC = gBmp.readTemperature();
    const float pressPa = gBmp.readPressure();
    const float pressHpa = pressPa / 100.0f;
    const float altM = gBmp.readAltitude(SEA_LEVEL_HPA);

    LogRecord r{};
    r.t_ms = millis();
    r.type = LogType::BARO;
    r.u.baro.temp_c = tempC;
    r.u.baro.press_hpa = pressHpa;
    r.u.baro.alt_m = altM;

    log_enqueue(r);
    vTaskDelayUntil(&last, period);
  }
}