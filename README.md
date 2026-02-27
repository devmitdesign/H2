ESP32 Dual-CAN Logger + Live MQTT Stream (SIM7600 + Wi-Fi)
=========================================================

What this does (now):
- Always logs LogRecord32 records to SD in append-only 2MB segments (/log/seg_XXXXXX.wip -> .bin)
- Best-effort live stream over MQTT when Wi-Fi or LTE is available
- Uses SIM7600 GNSS via AT+CGPS=1,1 and AT+CGPSINFO (lat/lon/alt/speed)

Supabase ingestion recommendation (Trevis):
- Keep Supabase service_role keys on the Trevis server, not on the ESP32.
- ESP32 publishes MQTT to Trevis broker (mosquitto).
- Trevis consumer subscribes and writes to Supabase (batching + retries).

Config:
- include/config.h:
  - WIFI_SSID/PASS
  - LTE_APN/USER/PASS
  - MQTT_HOST/PORT/TOPICS
  - MCP2515_QUARTZ_HZ (8MHz vs 16MHz)

Dependencies:
- TinyGSM 0.12.0
- PubSubClient
- Adafruit sensor libs
- ACAN2515: clone https://github.com/pierremolinaro/acan2515 into lib/ACAN2515 if needed.

MQTT payload format:
- payload[0] = N records (1..12)
- payload[1..] = N * 32 bytes of LogRecord32 (packed)

Security / TLS:
- Start with MQTT 1883 inside VPN/private network.
- Move to TLS later (8883). SIM7600 SSL/TLS can be firmware-dependent (SNI matters).
  If you want stable HTTPS/TLS on-device, PPPoS is the robust long-term path.
