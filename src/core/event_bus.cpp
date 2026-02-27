#include "event_bus.h"
#include "config.h"

QueueHandle_t gLogQueue = nullptr;
EventGroupHandle_t gSysEvents = nullptr;

bool event_bus_init() {
  gSysEvents = xEventGroupCreate();
  if (!gSysEvents) return false;

  gLogQueue = xQueueCreate(LOG_QUEUE_DEPTH, sizeof(LogRecord32));
  if (!gLogQueue) return false;

  return true;
}

bool logq_push_isr(const LogRecord32& rec, BaseType_t* hpTaskWoken) {
  if (!gLogQueue) return false;
  return xQueueSendFromISR(gLogQueue, &rec, hpTaskWoken) == pdTRUE;
}

bool logq_push(const LogRecord32& rec, TickType_t toTicks) {
  if (!gLogQueue) return false;
  return xQueueSend(gLogQueue, &rec, toTicks) == pdTRUE;
}
