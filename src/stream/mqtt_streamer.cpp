\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"

#if ENABLE_MQTT
#include <WiFi.h>
#include <PubSubClient.h>

static WiFiClient gClient;
static PubSubClient gMqtt(gClient);

static void mqttReconnect() {
  if (gMqtt.connected()) return;
  if (WiFi.status() != WL_CONNECTED) return;

  gMqtt.setServer(MQTT_HOST, MQTT_PORT);
  Serial.printf("MQTT: connecting %s:%u\n", MQTT_HOST, MQTT_PORT);

  if (gMqtt.connect(MQTT_CLIENT_ID)) {
    Serial.println("MQTT: connected");
    xEventGroupSetBits(gSysEvents, EV_MQTT_UP);
  } else {
    xEventGroupClearBits(gSysEvents, EV_MQTT_UP);
    Serial.printf("MQTT: connect failed rc=%d\n", gMqtt.state());
  }
}

static void mqtt_task(void*) {
  for (;;) {
    mqttReconnect();
    gMqtt.loop();

    EventBits_t b = xEventGroupGetBits(gSysEvents);
    const bool netUp  = (b & EV_NET_UP) != 0;
    const bool mqttUp = (b & EV_MQTT_UP) != 0;

    if (netUp && mqttUp) {
      static uint32_t lastStatus = 0;
      if (millis() - lastStatus > 2000) {
        lastStatus = millis();
        char msg[96];
        snprintf(msg, sizeof(msg), "{\"uptime_ms\":%lu}", (unsigned long)millis());
        gMqtt.publish(MQTT_TOPIC_STATUS, msg);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void start_mqtt_task() {
  xTaskCreatePinnedToCore(mqtt_task, "mqtt", 6144, nullptr, 1, nullptr, 1);
}
#else
void start_mqtt_task() {}
#endif
