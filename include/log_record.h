\
#pragma once
#include <stdint.h>

// Fixed-size binary record (32 bytes) designed for append-only logging.
#pragma pack(push, 1)
struct LogRecord32 {
  uint64_t t_us;    // esp_timer_get_time()
  uint8_t  type;    // 1=CAN, 2=ACCEL, 3=BARO, 4=GPS, 5=STATUS
  uint8_t  source;  // CAN bus: 1=CAN1, 2=CAN2
  uint8_t  len;     // DLC or payload length
  uint8_t  flags;   // bit0 ext, bit1 rtr
  uint32_t id;      // CAN ID or sensor tag
  uint8_t  data[8]; // payload
  uint16_t crc16;   // CRC over bytes [0..(crc16 offset-1)]
  uint16_t rsv;     // reserved
};
#pragma pack(pop)

static_assert(sizeof(LogRecord32) == 32, "LogRecord32 must be 32 bytes");
