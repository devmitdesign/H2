\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "log_record.h"
#if ENABLE_SENSORS
#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP280.h>
#include "esp_timer.h"

static Adafruit_ADXL345_Unified gAdxl(12345);
static Adafruit_BMP280 gBmp;

static void pack_f32_le(uint8_t* o, float v){ union{float f; uint8_t b[4];}u; u.f=v; o[0]=u.b[0];o[1]=u.b[1];o[2]=u.b[2];o[3]=u.b[3]; }

static void task(void*){
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if(gAdxl.begin(ADXL345_ADDR)){ gAdxl.setRange(ADXL345_RANGE_16_G); Serial.println("ADXL345 OK"); } else Serial.println("ADXL345 not found");
  if(gBmp.begin(BMP280_ADDR)) Serial.println("BMP280 OK"); else Serial.println("BMP280 not found");

  TickType_t lastA=xTaskGetTickCount(), lastB=xTaskGetTickCount();
  for(;;){
    if(xTaskGetTickCount()-lastA>=pdMS_TO_TICKS(50)){
      lastA=xTaskGetTickCount();
      sensors_event_t ev{};
      if(gAdxl.getEvent(&ev)){
        float ax=ev.acceleration.x/9.80665f, ay=ev.acceleration.y/9.80665f, az=ev.acceleration.z/9.80665f;
        LogRecord32 r{}; r.t_us=(uint64_t)esp_timer_get_time(); r.type=2; r.id=0xA345; r.len=8;
        pack_f32_le(&r.data[0], ax); pack_f32_le(&r.data[4], ay);
        (void)logq_push(r,0); (void)streamq_push(r);
        LogRecord32 r2{}; r2.t_us=r.t_us; r2.type=2; r2.id=0xA346; r2.len=4;
        pack_f32_le(&r2.data[0], az);
        (void)logq_push(r2,0); (void)streamq_push(r2);
      }
    }
    if(xTaskGetTickCount()-lastB>=pdMS_TO_TICKS(200)){
      lastB=xTaskGetTickCount();
      float t=gBmp.readTemperature(), p=gBmp.readPressure()/100.0f, a=gBmp.readAltitude(SEA_LEVEL_HPA);
      LogRecord32 r{}; r.t_us=(uint64_t)esp_timer_get_time(); r.type=3; r.id=0xB280; r.len=8;
      pack_f32_le(&r.data[0], t); pack_f32_le(&r.data[4], p);
      (void)logq_push(r,0); (void)streamq_push(r);
      LogRecord32 r2{}; r2.t_us=r.t_us; r2.type=3; r2.id=0xB281; r2.len=4;
      pack_f32_le(&r2.data[0], a);
      (void)logq_push(r2,0); (void)streamq_push(r2);
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
void start_sensors_task(){ xTaskCreatePinnedToCore(task,"sensors",6144,nullptr,2,nullptr,1); }
#else
void start_sensors_task(){}
#endif
