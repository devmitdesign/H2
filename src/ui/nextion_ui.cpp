\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
static HardwareSerial Nextion(2);
static void task(void*){
  Nextion.begin(NEXTION_BAUD, SERIAL_8N1, PIN_NEXTION_RX2, PIN_NEXTION_TX2);
  for(;;){ vTaskDelay(pdMS_TO_TICKS(1000)); }
}
void start_nextion_task(){ xTaskCreatePinnedToCore(task,"nextion",4096,nullptr,1,nullptr,1); }
