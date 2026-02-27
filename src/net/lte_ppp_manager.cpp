\
#include <Arduino.h>
#include "config.h"
#include "event_bus.h"

#if ENABLE_LTE_PPP
static void lte_task(void*) {
  for (;;) {
    // TODO: SIM7600 PPP or TinyGSM sockets
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void start_lte_task() {
  xTaskCreatePinnedToCore(lte_task, "lte", 8192, nullptr, 2, nullptr, 1);
}
#else
void start_lte_task() {}
#endif
