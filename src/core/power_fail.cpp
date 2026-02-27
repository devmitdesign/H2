\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
static void task(void*){
  analogReadResolution(12);
  uint32_t lowSince=0; bool fail=false;
  for(;;){
    int v=analogRead(PIN_PWR_SENSE_ADC);
    if(v<PWR_FAIL_ADC_THRESHOLD){
      if(lowSince==0) lowSince=millis();
      if(!fail && (millis()-lowSince)>=PWR_FAIL_DEBOUNCE_MS){
        fail=true; xEventGroupSetBits(gSysEvents, EV_PWR_FAIL);
      }
    } else {
      lowSince=0;
      if(fail){ fail=false; xEventGroupClearBits(gSysEvents, EV_PWR_FAIL); }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
void start_power_task(){ xTaskCreatePinnedToCore(task,"pwr",2048,nullptr,3,nullptr,1); }
