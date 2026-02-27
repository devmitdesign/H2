\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "can2_mcp2515.h"
#include "log_record.h"
#if ENABLE_CAN2_MCP2515
#include <SPI.h>
#include <ACAN2515.h>
#include "esp_timer.h"

static SPIClass gSpiCAN(HSPI);
static ACAN2515 gCan(PIN_CAN2_CS, gSpiCAN, PIN_CAN2_INT);
static void IRAM_ATTR isr(){ gCan.isr(); }

static bool init(){
  pinMode(PIN_CAN2_CS, OUTPUT);
  digitalWrite(PIN_CAN2_CS, HIGH);
  gSpiCAN.begin(PIN_CAN2_SCK, PIN_CAN2_MISO, PIN_CAN2_MOSI, PIN_CAN2_CS);

  ACAN2515Settings s(MCP2515_QUARTZ_HZ, CAN2_BITRATE);
  s.mRequestedMode = ACAN2515Settings::NormalMode;
  s.mReceiveBufferSize = 64;
  s.mTransmitBufferSize = 16;
  uint16_t err=gCan.begin(s, isr);
  if(err!=0){ Serial.printf("CAN2 begin err 0x%04X\n", err); return false; }
  xEventGroupSetBits(gSysEvents, EV_CAN2_UP);
  return true;
}
static void task(void*){
  if(!init()){ vTaskDelete(nullptr); return; }
  CANMessage m;
  for(;;){
    while(gCan.receive(m)){
      LogRecord32 r{};
      r.t_us=(uint64_t)esp_timer_get_time();
      r.type=1; r.source=2; r.len=m.len;
      r.flags=(m.ext?0x01:0)|(m.rtr?0x02:0);
      r.id=m.id;
      for(uint8_t i=0;i<8;i++) r.data[i]=m.data[i];
      (void)logq_push(r,0); (void)streamq_push(r);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
void start_can2_task(){ xTaskCreatePinnedToCore(task,"can2",4096,nullptr,4,nullptr,0); }
#else
void start_can2_task(){}
#endif
