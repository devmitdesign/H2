\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"

static void power_task(void*) {
  analogReadResolution(12);
  uint32_t lowSince = 0;
  bool inFail = false;

  for (;;) {
    int v = analogRead(PIN_PWR_SENSE_ADC);

    if (v < PWR_FAIL_ADC_THRESHOLD) {
      if (lowSince == 0) lowSince = millis();
      if (!inFail && (millis() - lowSince) >= PWR_FAIL_DEBOUNCE_MS) {
        inFail = true;
        xEventGroupSetBits(gSysEvents, EV_PWR_FAIL);
        Serial.printf("POWER_FAIL: ADC=%d\n", v);
      }
    } else {
      lowSince = 0;
      if (inFail) {
        inFail = false;
        xEventGroupClearBits(gSysEvents, EV_PWR_FAIL);
        Serial.printf("POWER_RECOVER: ADC=%d\n", v);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void start_power_task() {
  xTaskCreatePinnedToCore(power_task, "pwr", 2048, nullptr, 3, nullptr, 1);
}
