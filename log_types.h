#pragma once
#include <Arduino.h>

enum class LogType : uint8_t {
  CAN1 = 1,
  CAN2 = 2,
  ACCEL = 3,
  BARO = 4,
  GPS = 5
};

struct CanLog {
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t dlc;
  uint8_t data[8];
};

struct AccelLog {
  float ax_g;
  float ay_g;
  float az_g;
};

struct BaroLog {
  float temp_c;
  float press_hpa;
  float alt_m;
};

struct GpsLog {
  bool fix;
  double lat;
  double lon;
  double alt_m;
  double speed_kmph;
  double course_deg;
  uint32_t sats;
  double hdop;
};

struct LogRecord {
  uint32_t t_ms;
  LogType type;
  union {
    CanLog can;
    AccelLog accel;
    BaroLog baro;
    GpsLog gps;
  } u;
};