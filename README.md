ESP32 Dual-CAN Logger Skeleton (PlatformIO/Arduino)
===================================================

What works now:
- CAN1 RX (ESP32 TWAI) -> queue -> SD segment files
- CAN2 RX (MCP2515 via ACAN2515) -> queue -> SD segment files
- ADXL345 + BMP280 periodic logging
- GT-U7 GPS periodic logging
- Wi-Fi connect loop
- MQTT status publisher (skeleton)

What you must add next:
- ACAN2515 library (if PlatformIO can't resolve it automatically)
  Put it under lib/ACAN2515 or use Arduino IDE Library Manager.
- Live CAN streaming "tee" (separate queue or SD-tail reader)
- Backlog upload (read completed segment .bin and publish chunks + ACK)
- LTE modem stack (SIM7600): PPPoS (ESP-IDF) or TinyGSM sockets

Safety:
- If your MCP2515 module is powered at 5V, level-shift MCP->ESP32 signals (SO/MISO, INT).
- Add hold-up energy (supercap/UPS) for best SD survivability.

Build:
- Edit include/config.h (Wi-Fi, pins, bitrate, oscillator)
- pio run -t upload
- pio device monitor
