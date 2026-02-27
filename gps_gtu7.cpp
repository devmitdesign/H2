#include "gps_gtu7.h"
#include "config.h"
#include "logger.h"
#include "log_types.h"

#include <TinyGPSPlus.h>

static HardwareSerial gGpsSerial(1);
static TinyGPSPlus gGps;

bool gps_init() {
  // GT-U7 commonly defaults to 9600 baud and outputs NMEA over UART :contentReference[oaicite:17]{index=17}
  gGpsSerial.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX1, PIN_GPS_TX1);
  Serial.println("GPS UART started");
  return true;
}

void gps_task(void*) {
  uint32_t lastEmitMs = 0;

  for (;;) {
    while (gGpsSerial.available()) {
      gGps.encode(gGpsSerial.read());
    }

    // Emit GPS record at 1 Hz (or when you prefer)
    const uint32_t now = millis();
    if (now - lastEmitMs >= 1000) {
      lastEmitMs = now;

      LogRecord r{};
      r.t_ms = now;
      r.type = LogType::GPS;

      r.u.gps.fix = gGps.location.isValid();
      r.u.gps.lat = gGps.location.isValid() ? gGps.location.lat() : 0.0;
      r.u.gps.lon = gGps.location.isValid() ? gGps.location.lng() : 0.0;

      r.u.gps.alt_m = gGps.altitude.isValid() ? gGps.altitude.meters() : 0.0;
      r.u.gps.speed_kmph = gGps.speed.isValid() ? gGps.speed.kmph() : 0.0;
      r.u.gps.course_deg = gGps.course.isValid() ? gGps.course.deg() : 0.0;

      r.u.gps.sats = gGps.satellites.isValid() ? gGps.satellites.value() : 0;
      r.u.gps.hdop = gGps.hdop.isValid() ? gGps.hdop.hdop() : 0.0;

      log_enqueue(r);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}