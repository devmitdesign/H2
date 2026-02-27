\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"

#if ENABLE_WIFI
#include <WiFi.h>

static void wifi_task(void*) {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("WiFi: connecting to %s\n", WIFI_SSID);
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      uint32_t t0 = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
        vTaskDelay(pdMS_TO_TICKS(250));
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      xEventGroupSetBits(gSysEvents, EV_WIFI_UP | EV_NET_UP);
    } else {
      xEventGroupClearBits(gSysEvents, EV_WIFI_UP);
      EventBits_t b = xEventGroupGetBits(gSysEvents);
      if (!(b & EV_LTE_UP)) xEventGroupClearBits(gSysEvents, EV_NET_UP);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void start_wifi_task() {
  xTaskCreatePinnedToCore(wifi_task, "wifi", 4096, nullptr, 2, nullptr, 1);
}
#else
void start_wifi_task() {}
#endif
