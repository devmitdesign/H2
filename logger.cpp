#include "logger.h"
#include "config.h"

#include <SPI.h>
#include <SD.h>

QueueHandle_t gLogQueue = nullptr;

static SPIClass gSpiSD(VSPI);
static File gFile;
static uint32_t gLineCount = 0;

static void print_hex2(char* out, uint8_t v) {
  const char* hex = "0123456789ABCDEF";
  out[0] = hex[(v >> 4) & 0xF];
  out[1] = hex[v & 0xF];
}

static void record_to_csv(const LogRecord& r, String& line) {
  line.reserve(256);
  line = "";
  line += String(r.t_ms);
  line += ",";

  switch (r.type) {
    case LogType::CAN1:
    case LogType::CAN2: {
      line += (r.type == LogType::CAN1) ? "CAN1" : "CAN2";
      line += ",";
      line += "0x" + String(r.u.can.id, HEX);
      line += ",";
      line += (r.u.can.ext ? "EXT" : "STD");
      line += ",";
      line += (r.u.can.rtr ? "RTR" : "DATA");
      line += ",";
      line += String(r.u.can.dlc);
      line += ",";

      // data as hex bytes
      for (uint8_t i = 0; i < r.u.can.dlc; i++) {
        char b[2];
        print_hex2(b, r.u.can.data[i]);
        line += String(b[0]); line += String(b[1]);
        if (i + 1 < r.u.can.dlc) line += " ";
      }
      line += ",";
      // pad remaining columns for consistent CSV
      line += ",,,,,,,,,";
      break;
    }

    case LogType::ACCEL:
      line += "ACCEL,,,,,,";
      line += String(r.u.accel.ax_g, 6); line += ",";
      line += String(r.u.accel.ay_g, 6); line += ",";
      line += String(r.u.accel.az_g, 6); line += ",";
      line += ",,,,,,";
      break;

    case LogType::BARO:
      line += "BARO,,,,,,";
      line += ",,,";
      line += String(r.u.baro.temp_c, 2); line += ",";
      line += String(r.u.baro.press_hpa, 2); line += ",";
      line += String(r.u.baro.alt_m, 2); line += ",";
      line += ",,,,";
      break;

    case LogType::GPS:
      line += "GPS,,,,,,";
      line += ",,,";
      line += ",,,";
      line += (r.u.gps.fix ? "1" : "0"); line += ",";
      line += String(r.u.gps.lat, 7); line += ",";
      line += String(r.u.gps.lon, 7); line += ",";
      line += String(r.u.gps.alt_m, 2); line += ",";
      line += String(r.u.gps.speed_kmph, 2); line += ",";
      line += String(r.u.gps.course_deg, 2); line += ",";
      line += String(r.u.gps.sats); line += ",";
      line += String(r.u.gps.hdop, 2);
      break;
  }
}

// CSV header (consistent columns)
static const char* CSV_HEADER =
"t_ms,type,can_id,fmt,rtr,dlc,data_hex,ax_g,ay_g,az_g,temp_c,press_hpa,baro_alt_m,gps_fix,lat,lon,gps_alt_m,speed_kmph,course_deg,sats,hdop";

bool logger_init_sd() {
  gSpiSD.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, gSpiSD, SD_SPI_HZ)) {
    Serial.println("SD.begin failed");
    return false;
  }

  gFile = SD.open("/log.csv", FILE_APPEND);
  if (!gFile) {
    Serial.println("SD.open(/log.csv) failed");
    return false;
  }

  // If file empty, write header
  if (gFile.size() == 0) {
    gFile.println(CSV_HEADER);
    gFile.flush();
  }

  Serial.println("SD logging ready: /log.csv");
  return true;
}

void log_enqueue(const LogRecord& rec) {
  if (!gLogQueue) return;
  (void)xQueueSend(gLogQueue, &rec, 0);
}

void logger_task(void*) {
  LogRecord rec{};
  String line;

  for (;;) {
    if (xQueueReceive(gLogQueue, &rec, portMAX_DELAY) == pdTRUE) {
      record_to_csv(rec, line);

      // Serial monitor output
      Serial.println(line);

      // SD output
      if (gFile) {
        gFile.println(line);
        gLineCount++;
        // flush every 50 lines (tune as needed)
        if ((gLineCount % 50) == 0) gFile.flush();
      }
    }
  }
}