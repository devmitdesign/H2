#include "sim800l.h"
#include "config.h"

static HardwareSerial gModem(1); // UART1

static bool readLine(String& out, uint32_t timeoutMs) {
  out = "";
  uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    while (gModem.available()) {
      char c = (char)gModem.read();
      if (c == '\r') continue;
      if (c == '\n') {
        if (out.length()) return true;
        continue;
      }
      out += c;
    }
    delay(1);
  }
  return out.length() > 0;
}

static bool sendAT(const char* cmd, const char* expect, uint32_t timeoutMs) {
  while (gModem.available()) gModem.read();
  gModem.print(cmd);
  gModem.print("\r");

  String line;
  uint32_t t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (readLine(line, timeoutMs)) {
      if (line.indexOf(expect) >= 0) return true;
      if (line.indexOf("ERROR") >= 0) return false;
    }
    delay(1);
  }
  return false;
}

bool sim800l_init() {
  // Optional reset (only if you wired it safely via transistor)
  if (PIN_SIM800_RST >= 0) {
    pinMode(PIN_SIM800_RST, OUTPUT);
    digitalWrite(PIN_SIM800_RST, HIGH);
  }

  gModem.begin(SIM800_BAUD, SERIAL_8N1, PIN_SIM800_RX, PIN_SIM800_TX);
  delay(200);

  // Probe until module responds
  for (int i = 0; i < 20; i++) {
    if (sendAT("AT", "OK", 500)) break;
    delay(250);
  }
  if (!sendAT("AT", "OK", 500)) {
    Serial.println("SIM800L: no AT response (power/baud/wiring?)");
    return false;
  }

  sendAT("ATE0", "OK", 500);       // echo off
  sendAT("AT+CMEE=2", "OK", 500);  // verbose errors
  Serial.println("SIM800L: ready (AT OK)");
  return true;
}

static void printQuery(const char* cmd) {
  gModem.print(cmd); gModem.print("\r");
  String line;
  uint32_t t0 = millis();
  while (millis() - t0 < 800) {
    if (!readLine(line, 200)) break;
    if (line == "OK") break;
    if (line.length()) {
      Serial.print("SIM800L ");
      Serial.print(cmd);
      Serial.print(" => ");
      Serial.println(line);
    }
  }
}

void sim800l_task(void*) {
  // Periodic status
  for (;;) {
    printQuery("AT+CSQ");   // signal quality
    printQuery("AT+CREG?"); // network registration (circuit switched)
    printQuery("AT+CGREG?");// GPRS registration
    printQuery("AT+COPS?"); // operator

    // In UAE you may see "not registered" due to 2G sunset. :contentReference[oaicite:5]{index=5}
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}