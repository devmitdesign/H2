\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "can1_twai.h"
#include "log_record.h"
#if ENABLE_CAN1_TWAI
#include "driver/twai.h"
#include "esp_timer.h"

static twai_timing_config_t timing(uint32_t b){
  switch(b){ case 1000000: return TWAI_TIMING_CONFIG_1MBITS();
             case 500000: return TWAI_TIMING_CONFIG_500KBITS();
             case 250000: return TWAI_TIMING_CONFIG_250KBITS();
             case 125000: return TWAI_TIMING_CONFIG_125KBITS();
             default: return TWAI_TIMING_CONFIG_500KBITS(); }
}
static bool init(){
  auto g=TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)PIN_CAN1_TX,(gpio_num_t)PIN_CAN1_RX,TWAI_MODE_NORMAL);
  auto t=timing(CAN1_BITRATE);
  auto f=TWAI_FILTER_CONFIG_ACCEPT_ALL();
  if(twai_driver_install(&g,&t,&f)!=ESP_OK) return false;
  if(twai_start()!=ESP_OK) return false;
  xEventGroupSetBits(gSysEvents, EV_CAN1_UP);
  return true;
}
static void task(void*){
  if(!init()){ vTaskDelete(nullptr); return; }
  twai_message_t msg{};
  for(;;){
    if(twai_receive(&msg, pdMS_TO_TICKS(10))==ESP_OK){
      LogRecord32 r{};
      r.t_us=(uint64_t)esp_timer_get_time();
      r.type=1; r.source=1; r.len=msg.data_length_code;
      r.flags=(msg.extd?0x01:0)|(msg.rtr?0x02:0);
      r.id=msg.identifier;
      for(uint8_t i=0;i<8;i++) r.data[i]=msg.data[i];
      (void)logq_push(r,0); (void)streamq_push(r);
    } else vTaskDelay(pdMS_TO_TICKS(1));
  }
}
void start_can1_task(){ xTaskCreatePinnedToCore(task,"can1",4096,nullptr,4,nullptr,0); }
#else
void start_can1_task(){}
#endif
