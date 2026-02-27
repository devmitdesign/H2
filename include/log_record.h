\
#pragma once
#include <stdint.h>
#pragma pack(push, 1)
struct LogRecord32 {
  uint64_t t_us;
  uint8_t  type;    // 1=CAN, 2=ACCEL, 3=BARO, 4=GNSS, 5=STATUS
  uint8_t  source;  // 1=CAN1, 2=CAN2, 0=other
  uint8_t  len;
  uint8_t  flags;   // bit0 ext, bit1 rtr, bit7 fix(for GNSS)
  uint32_t id;
  uint8_t  data[8];
  uint16_t crc16;
  uint16_t rsv;
};
#pragma pack(pop)
static_assert(sizeof(LogRecord32) == 32, "LogRecord32 must be 32 bytes");
