#include <Arduino.h>
#include "config.h"
#include "event_bus.h"
#include "segment_writer.h"

static SegmentWriter gWriter;

static void sd_writer_task(void*) {
#if !ENABLE_SD
  vTaskDelete(nullptr);
  return;
#else
  if (!gWriter.begin()) {
    Serial.println("SD writer: init failed");
  }

  LogRecord32 rec{};
  for (;;) {
    if (xEventGroupGetBits(gSysEvents) & EV_PWR_FAIL) {
      gWriter.requestPowerFailFlush();
    }

    if (xQueueReceive(gLogQueue, &rec, pdMS_TO_TICKS(20)) == pdTRUE) {
      (void)gWriter.append(rec);
    }
    gWriter.tick();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
#endif
}

void start_sd_writer_task() {
#if ENABLE_SD
  xTaskCreatePinnedToCore(sd_writer_task, "sd_writer", 8192, nullptr, 3, nullptr, 1);
#endif
}
