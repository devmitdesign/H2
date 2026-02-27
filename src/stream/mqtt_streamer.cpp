\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "sim7600_modem.h"
#include "log_record.h"
#if ENABLE_MQTT
#include <WiFi.h>
#include <PubSubClient.h>

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

static Client* activeClient(){
  EventBits_t b=xEventGroupGetBits(gSysEvents);
  if(b & EV_WIFI_UP) return &wifiClient;
  if(b & EV_LTE_UP)  return &sim7600_client();
  return nullptr;
}
static bool netUp(){ return (xEventGroupGetBits(gSysEvents) & EV_NET_UP) != 0; }

static void reconnect(){
  if(!netUp()) return;
  Client* c=activeClient(); if(!c) return;
  mqtt.setClient(*c);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setBufferSize(1024);
  if(mqtt.connected()) return;
  if(mqtt.connect(MQTT_CLIENT_ID)){
    xEventGroupSetBits(gSysEvents, EV_MQTT_UP);
  } else {
    xEventGroupClearBits(gSysEvents, EV_MQTT_UP);
  }
}

static void task(void*){
  while(!netUp()) vTaskDelay(pdMS_TO_TICKS(500));

  uint8_t payload[1 + 32*12];
  uint8_t count=0;
  LogRecord32 rec{};
  uint32_t lastStatus=0;

  for(;;){
    reconnect();
    mqtt.loop();

    if(!(xEventGroupGetBits(gSysEvents) & EV_MQTT_UP)){
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }

    if(millis()-lastStatus>2000){
      lastStatus=millis();
      EventBits_t b=xEventGroupGetBits(gSysEvents);
      char msg[180];
      snprintf(msg,sizeof(msg),
        "{\"uptime_ms\":%lu,\"wifi\":%d,\"lte\":%d,\"sd\":%d,\"can1\":%d,\"can2\":%d}",
        (unsigned long)millis(),
        (b&EV_WIFI_UP)?1:0,(b&EV_LTE_UP)?1:0,(b&EV_SD_READY)?1:0,(b&EV_CAN1_UP)?1:0,(b&EV_CAN2_UP)?1:0
      );
      mqtt.publish(MQTT_TOPIC_STATUS, msg);
    }

    while(xQueueReceive(gStreamQueue, &rec, 0)==pdTRUE){
      memcpy(&payload[1 + count*sizeof(LogRecord32)], &rec, sizeof(LogRecord32));
      count++;
      if(count>=12) break;
    }

    if(count>0){
      payload[0]=count;
      mqtt.publish(MQTT_TOPIC_LIVE, payload, 1 + count*sizeof(LogRecord32));
      count=0;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
void start_mqtt_task(){ xTaskCreatePinnedToCore(task,"mqtt",8192,nullptr,1,nullptr,1); }
#else
void start_mqtt_task(){}
#endif
