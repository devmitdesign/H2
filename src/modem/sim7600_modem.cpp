\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "log_record.h"
#include "sim7600_modem.h"

#if ENABLE_MODEM_SIM7600
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_DEBUG Serial
#include <TinyGsmClient.h>
#include "esp_timer.h"

static HardwareSerial SerialAT(1);
static TinyGsm modem(SerialAT);
static TinyGsmClient gsmClient(modem);
static bool gNetUp=false;

Client& sim7600_client(){ return gsmClient; }
bool sim7600_is_net_up(){ return gNetUp; }

static void pack_f32_le(uint8_t* o, float v){ union{float f; uint8_t b[4];}u; u.f=v; o[0]=u.b[0];o[1]=u.b[1];o[2]=u.b[2];o[3]=u.b[3]; }

static bool parse_cgpsinfo(const String& line, float& lat, float& lon, float& alt_m, float& speed_kmh, uint8_t& fix){
  int idx=line.indexOf(":"); if(idx<0) return false;
  String csv=line.substring(idx+1); csv.trim();
  String parts[9]; int p=0, s=0;
  for(int i=0;i<=csv.length() && p<9;i++){
    if(i==csv.length() || csv[i]==','){ parts[p++]=csv.substring(s,i); s=i+1; }
  }
  if(p<6) return false;
  String slat=parts[0]; slat.trim();
  String ns=parts[1]; ns.trim();
  String slon=parts[2]; slon.trim();
  String ew=parts[3]; ew.trim();
  String salt=parts[6]; salt.trim();
  String sspd=parts[7]; sspd.trim();
  if(slat.length()==0 || slon.length()==0){ fix=0; return true; }

  auto ddmm_to_deg=[](const String& v)->float{
    double x=v.toDouble();
    int deg=int(x/100.0);
    double min=x-double(deg)*100.0;
    return float(double(deg) + min/60.0);
  };
  lat=ddmm_to_deg(slat);
  lon=ddmm_to_deg(slon);
  if(ns=="S") lat=-lat;
  if(ew=="W") lon=-lon;
  alt_m=salt.toFloat();
  speed_kmh=sspd.toFloat()*1.852f; // knots -> km/h
  fix=1;
  return true;
}

static void bringup(){
  Serial.println("SIM7600: init");
  modem.init();
  Serial.print("SIM7600: info: "); Serial.println(modem.getModemInfo());

  Serial.println("SIM7600: waitForNetwork");
  if(!modem.waitForNetwork(60000L)){
    Serial.println("SIM7600: network fail");
    gNetUp=false;
    xEventGroupClearBits(gSysEvents, EV_LTE_UP);
    return;
  }
  Serial.println("SIM7600: network ok");

  Serial.print("SIM7600: gprsConnect APN="); Serial.println(LTE_APN);
  if(!modem.gprsConnect(LTE_APN, LTE_USER, LTE_PASS)){
    Serial.println("SIM7600: data fail (APN?)");
    gNetUp=false;
    xEventGroupClearBits(gSysEvents, EV_LTE_UP);
    return;
  }

  gNetUp=true;
  xEventGroupSetBits(gSysEvents, EV_LTE_UP | EV_NET_UP);
  Serial.print("SIM7600: IP="); Serial.println(modem.localIP());
}

static void gnss_enable(){
  modem.sendAT(GF("+CGPS=1,1"));
  modem.waitResponse(2000);
}

static void gnss_poll_and_log(){
  modem.sendAT(GF("+CGPSINFO"));
  // wait until the +CGPSINFO line arrives
  if(modem.waitResponse(2000, GF(GSM_NL "+CGPSINFO:")) != 1){
    modem.waitResponse(200);
    return;
  }
  String rest = modem.stream.readStringUntil('\n'); rest.trim();
  String full = "+CGPSINFO:" + rest;

  float lat=0,lon=0,alt=0,spd=0; uint8_t fix=0;
  if(!parse_cgpsinfo(full, lat, lon, alt, spd, fix)) return;

  LogRecord32 r{}; r.t_us=(uint64_t)esp_timer_get_time(); r.type=4; r.id=0x4750; r.len=8; r.flags = fix?0x80:0;
  pack_f32_le(&r.data[0], lat); pack_f32_le(&r.data[4], lon);
  (void)logq_push(r,0); (void)streamq_push(r);

  LogRecord32 r2{}; r2.t_us=r.t_us; r2.type=4; r2.id=0x4751; r2.len=8; r2.flags=r.flags;
  pack_f32_le(&r2.data[0], alt); pack_f32_le(&r2.data[4], spd);
  (void)logq_push(r2,0); (void)streamq_push(r2);
}

static void task(void*){
  SerialAT.begin(SIM7600_BAUD, SERIAL_8N1, PIN_SIM7600_RX1, PIN_SIM7600_TX1);
  delay(300);

  for(;;){
    bringup();
    gnss_enable();

    uint32_t lastGnss=0, lastStatus=0;
    while(gNetUp){
      if(!modem.isNetworkConnected()){
        Serial.println("SIM7600: network lost");
        gNetUp=false;
        xEventGroupClearBits(gSysEvents, EV_LTE_UP);
        EventBits_t b=xEventGroupGetBits(gSysEvents);
        if(!(b & EV_WIFI_UP)) xEventGroupClearBits(gSysEvents, EV_NET_UP);
        modem.gprsDisconnect();
        break;
      }

      if(millis()-lastStatus>5000){
        lastStatus=millis();
        int16_t csq=modem.getSignalQuality();
        Serial.printf("SIM7600: CSQ=%d IP=%s\n", csq, modem.localIP().toString().c_str());
        LogRecord32 st{}; st.t_us=(uint64_t)esp_timer_get_time(); st.type=5; st.id=0x7600; st.len=2;
        st.data[0]=(uint8_t)(csq&0xFF); st.data[1]=(uint8_t)(gNetUp?1:0);
        (void)logq_push(st,0); (void)streamq_push(st);
      }

      if(millis()-lastGnss>1000){
        lastGnss=millis();
        gnss_poll_and_log();
      }

      vTaskDelay(pdMS_TO_TICKS(50));
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void start_sim7600_task(){ xTaskCreatePinnedToCore(task,"sim7600",8192,nullptr,2,nullptr,1); }
#else
Client& sim7600_client(){ static WiFiClient dummy; return dummy; }
bool sim7600_is_net_up(){ return false; }
void start_sim7600_task(){}
#endif
